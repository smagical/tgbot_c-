//
// Created by 19766 on 2024/11/14.
//

#include "include/tgbot.h"

#include "handler/include/authorization_state.h"
#include "handler/include/update_option.h"
#include "handler/include/noop_handler.h"
#include "handler/include/user.h"
#include "handler/include/chat.h"
#include "include/bot_util.h"
#include "plugins/command/include/command.h"
#include "plugins/include/bot_plugin.h"
#include "plugins/include/user_plugin.h"

namespace tg {

    Bot::Bot(BotConfig conf) {
        init(conf);
    }

    void Bot::init(BotConfig config) {
        this->config = std::make_unique<BotConfig>(config);
        this->init_handler();
        this->init_plugin();
        this->init_redis();
        this->request_id_sequence = std::make_unique<std::SafeSequence>();
        this->thread_pool = std::make_unique<tg::ThreadPool>(this->config->get_int_value(BotConfig::BOT_THREAD_NUMS));
        this->request_and_handler = std::make_unique<std::SafeMap<std::uint64_t,std::unique_ptr<tg::handler::Handler>>>(10000000,500000);
        this->response = std::make_unique<std::SafeMap<std::uint64_t,td::ClientManager::Response>>(10000000,500000);
        this->response_id_sequence = std::make_unique<std::SafeSequence>();

        this->chats =  std::make_unique<std::SafeMap<td::td_api::int53,td::td_api::object_ptr<td::td_api::chat>>>(10000000,500000);
        this->users =  std::make_unique<std::SafeMap<td::td_api::int53,td::td_api::object_ptr<td::td_api::user>>>(10000000,500000);
        this->user_full_info = std::make_unique<std::SafeMap<td::td_api::int53,td::td_api::object_ptr<td::td_api::userFullInfo>>>(10000000,500000);
        td::ClientManager::execute(td::td_api::make_object<td::td_api::setLogVerbosityLevel>(this->config->get_int_value(BotConfig::BOT_LOG_LEVEL)));
    }

    void Bot::init_handler() {
        this->dispatch = std::make_unique<handler::DispathHandler>(this);
        this->dispatch->add_handler(std::make_unique<handler::AuthorizationStateHandler>(this));
        this->dispatch->add_handler(std::make_unique<handler::UserDispathHandler>(this));
        this->dispatch->add_handler(std::make_unique<handler::ChatDispatchHandler>(this));
        this->dispatch->add_handler(std::make_unique<handler::UpdateOption>(this));
        this->dispatch->add_handler(std::make_unique<handler::NoopHandler>(this));
    }

    void Bot::init_plugin() {
        this->plugins = std::make_unique<plugin::Plugin>(this);
    }

    void Bot::init_redis() {

        std::string host = this->get_config()->get_string_value(tg::BotConfig::REDIS_HOST);
        int port = this->get_config()->get_int_value(tg::BotConfig::REDIS_PORT);
        std::string username = this->get_config()->get_string_value(tg::BotConfig::REDIS_USERNAME);
        std::string password = this->get_config()->get_string_value(tg::BotConfig::REDIS_PASSWORD);
        std::string key_prefix = this->get_config()->get_string_value(tg::BotConfig::REDIS_KEY_PREFIX);
        int db = this->get_config()->get_int_value(tg::BotConfig::REDIS_DB);
        this->redis_utils = std::make_unique<tg::redis::RedisUtils>(host,port,username,password,key_prefix,db);
    }

    void Bot::reset() {
        if (this->running.load()) {
            this->running.store(false);
            this->client_future->wait();
            this->client_future.reset();
            this->client_thread.reset();
            this->client_manager.reset();
        }
        this->thread_pool->shutdown();
        this->thread_pool.reset();
        this->dispatch.reset();
        this->request_and_handler.reset();
        this->config.reset();
        this->client_manager.reset();
        this->loginType.reset();
        this->response.reset();
        this->request_id_sequence.reset();
        this->chats.reset();
        this->users.reset();
        this->user_full_info.reset();
        if (this->self)
            this->self.reset();
        this->plugins.reset();
    }

    Bot::~Bot() {
       this->reset();
    }

    LoginType *Bot::get_login_type() const {
        return  this->loginType.get();
    }
    std::string Bot::get_token() const {
        return this->token;
    }

    tg::redis::RedisUtils *Bot::get_redis_utils() const {
        return this->redis_utils.get();
    }


    void Bot::loginOcr() {
        this->loginType = std::make_unique<LoginType>(LoginType::OCR);
        this->login();
    }

    void Bot::loginPhone(std::string phone) {
        this->token= phone;
        this->loginType = std::make_unique<LoginType>(LoginType::PHONE);
        this->login();
    }

    void Bot::loginToken(std::string token) {
        this->token= token;
        this->loginType = std::make_unique<LoginType>(LoginType::TOKEN);
        this->login();
    }
    void Bot::login() {
        this->client_manager = std::make_unique<td::ClientManager>();
        this->client_id = this->client_manager->create_client_id();
        std::packaged_task<int()> task([this]() {
            this->run();
            return 0;
        });
        this->client_future = std::make_unique<std::future<int>>(
            task.get_future()
        );
        this->client_thread = std::make_unique<std::thread>(std::move(task));
        this->client_thread->detach();
        this->running.store(true);
        send(td::td_api::make_object<td::td_api::getOption>("version"),[](td::td_api::Object&& object) {
           tg::log_info("version  " + td::td_api::to_string(object));
      });
    }



    void Bot::send(td::td_api::object_ptr<td::td_api::Function> option, std::unique_ptr<tg::handler::Handler> handler) const {
        std::uint64_t request_id = this->request_id_sequence->get_next();
        if (handler != nullptr) {
            this -> request_and_handler->insert(std::move(request_id),std::move(handler));
        }
        this->client_manager -> send(client_id, request_id, std::move(option));
    }

    void Bot::send(td::td_api::object_ptr<td::td_api::Function> option, std::function<void(td::td_api::Object&&)> func) {
        this -> send(std::move(option),std::make_unique<tg::handler::FunctionWrapper>(this,std::move(func)));
    }

    void Bot::add_chat(td::td_api::int53 id, td::td_api::object_ptr<td::td_api::chat> chat) const {
        this->chats->insert(std::move(id),std::move(chat));
    }

    void Bot::delete_chat(td::td_api::int53 id) const {
        this->chats->remove(std::move(id));
    }

    void Bot::add_user(td::td_api::int53 id, td::td_api::object_ptr<td::td_api::user> user) const {
        this->users->insert(std::move(id),std::move(user));
    }
    void Bot::add_user(td::td_api::int53 id, td::td_api::object_ptr<td::td_api::userFullInfo> user_full_info) const {
        this->user_full_info->insert(std::move(id),std::move(user_full_info));
    }
    void Bot::delete_user(td::td_api::int53 id) const {
        this->user_full_info->remove(std::move(id));
        this->users->remove(std::move(id));
    }
    td::td_api::object_ptr<td::td_api::user> *Bot::get_me() {
        return &this->self;
    }
    td::td_api::object_ptr<td::td_api::chat> *Bot::get_chat(td::td_api::int53 id)  {
        td::td_api::object_ptr<td::td_api::chat> * chat = this->chats->get(id);
        if (chat) return chat;
        std::unique_lock<std::shared_mutex> lock(chats_mutex);
        return  tg::util::load_chat(this,id);
    }

    std::vector<td::td_api::object_ptr<td::td_api::chat> *> Bot::get_all_chats() const {
        return this->chats->get_vaule_vector();
    }

    td::td_api::object_ptr<td::td_api::user> *Bot::get_user(td::td_api::int53 id)  {
        td::td_api::object_ptr<td::td_api::user> * user = this->users->get(id);
        if (user) return user;
        std::unique_lock<std::shared_mutex> lock(users_mutex);
        return  tg::util::load_user(this,id);
    }
    std::vector<td::td_api::object_ptr<td::td_api::user> *> Bot::get_all_users() const {
       return  this->users->get_vaule_vector();
    }


    td::td_api::object_ptr<td::td_api::userFullInfo> *Bot::get_user_full_info(td::td_api::int53 id)  {
        return  this->user_full_info->get(id);
    }

    void Bot::load() {
        this->self = std::move(tg::util::get_me(this));
        log_info(td::td_api::to_string(this->self));
        tg::util::load_all_chats(this);

        if (this->loginType.operator*() == LoginType::TOKEN) {
            this->add_plugin(std::make_unique<tg::plugin::BotPlugin>());
            auto bot_plugin = std::make_unique<tg::plugin::BotPlugin>();
            //std::string ad_path = this->config->get_string_value(tg::plugin::command::SpiderCommand::AD_PATH);
            //bot_plugin->addCommand(std::make_unique<tg::plugin::command::SpiderCommand>(ad_path));
            bot_plugin->addCommand(std::make_unique<tg::plugin::command::InfoCommand>());
            bot_plugin->addCommand(std::make_unique<tg::plugin::command::PermissionsCommand>());
            bot_plugin->addCommand(std::make_unique<tg::plugin::command::BotCommand>());
            bot_plugin->send_commands(this);
            this->add_plugin(std::move(bot_plugin));
        }else {
            this->add_plugin(std::make_unique<tg::plugin::BotPlugin>());
            auto user_plugin = std::make_unique<tg::plugin::UserPlugin>();
            user_plugin->addCommand(std::make_unique<tg::plugin::command::SpiderCommand>());
            user_plugin->addCommand(std::make_unique<tg::plugin::command::InfoCommand>());
            user_plugin->addCommand(std::make_unique<tg::plugin::command::PermissionsCommand>());
            user_plugin->addCommand(std::make_unique<tg::plugin::command::BotCommand>());
            this->add_plugin(std::move(user_plugin));
        }
        this->report_time = std::time(nullptr);
        const std::string tmp = config->get_string_value(BotConfig::BOT_REPORT_KEY)+":"+config->get_string_value(BotConfig::BOT_ID);
        this->redis_utils->set(tmp,"....",(report_timeout + report_timeout/2));
    }

    void Bot::add_plugin(std::unique_ptr<tg::plugin::PluginInterface> plugin) const {
        this->plugins->register_plugin(std::move(plugin));
    }


    void Bot::dispatchMessage(td::td_api::updateNewMessage&& update_new_message) const {
        this->plugins->hand_new_message(std::forward<td::td_api::updateNewMessage>(update_new_message));
    }

    void Bot::dispatchMessage(td::td_api::updateNewCallbackQuery && update_new_callback_query) const {
        this->plugins->hand_new_call_callback_query(std::forward<td::td_api::updateNewCallbackQuery>(update_new_callback_query));
    }

    bool Bot::can_slove_message(td::td_api::int53 chat_id, td::td_api::int53 message_id) const {
        std::string chat_id_str = std::to_string(chat_id);
        std::string message_id_str = std::to_string(message_id);
        std::string key = BOT_REDIS_PREFIX +":"+chat_id_str + ":" + message_id_str;
        return this->redis_utils->can_solve(key,this->config->get_string_value(BotConfig::BOT_ID));
    }



    void Bot::run() const {
        tg::log_info("Bot running... ");
        int waitTime = config -> get_int_value(BotConfig::BOT_MIN_WAIT_TIME);
        while (running.load()) {

            td::ClientManager::Response response = client_manager->receive(waitTime);
            if (!response.object) {
                waitTime += config -> get_int_value(BotConfig::BOT_WAIT_INTERVAL);
                waitTime = std::min(waitTime, config -> get_int_value(BotConfig::BOT_MAX_WAIT_TIME));
                continue;
            }
            if (std::time(nullptr) - this->report_time >= this->report_timeout) {
                this->redis_utils->set(config->get_string_value(BotConfig::BOT_REPORT_KEY)+":"+config->get_string_value(BotConfig::BOT_ID),".",report_timeout + report_timeout/2);
            }
            waitTime = config -> get_int_value(BotConfig::BOT_MIN_WAIT_TIME); ;
            std::uint64_t response_id = this->response_id_sequence->get_next();
            this->response ->insert(std::move(response_id), std::move(response));
            this -> thread_pool->submit(
                [this](std::uint64_t id){
                    if (std::unique_ptr<td::ClientManager::Response> response = this->response->getAndRemove(id)) {
                        if (response->request_id == 0) {
                              this->dispatch->hand(std::move(*response->object));
                          }else if (std::unique_ptr<std::unique_ptr<tg::handler::Handler>> handler = this->request_and_handler->getAndRemove(response->request_id)) {
                                  handler->operator*().hand(std::move(*response->object));
                          }else {
                               handler::NoHandler no_handler;
                               no_handler.hand(std::move(*response->object));
                          }

                      }
            },response_id);

        }
        tg::log_info("Bot stopped... ");
    }


    BotConfig* Bot::get_config() const {
        return this->config.get();
    }

    std::string Bot::BOT_REDIS_PREFIX = "bot_select";

}