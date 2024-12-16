//
// Created by 19766 on 2024/11/14.
//

#ifndef TGBOT_H
#define TGBOT_H
#include <future>
#include <string>

#include "bot_config.h"
#include "plugin.h"
#include "stdext.h"
#include "../handler/include/dispath.h"
#include "../handler/include/handler.h"
#include "../redis/include/redis_utils.h"
#include "thread_pool.h"
#include "td/telegram/td_api.h"
#include "td/telegram/Client.h"

namespace tg {
    class ThreadPool;


    enum LoginType {
        OCR,PHONE,TOKEN
    };
    class Bot {
        public:
            Bot(BotConfig conf);
             ~Bot();
            void loginOcr();
            void loginPhone(std::string phone);
            void loginToken(std::string token);
            void send(td::td_api::object_ptr<td::td_api::Function> option, std::unique_ptr<tg::handler::Handler> handler) const;
            void send(td::td_api::object_ptr<td::td_api::Function> option,std::function<void(td::td_api::Object&&)> func);
            void add_user(td::td_api::int53 id,td::td_api::object_ptr<td::td_api::userFullInfo> user_full_info) const;
            void add_user(td::td_api::int53 id,td::td_api::object_ptr<td::td_api::user> user) const;
            void delete_user(td::td_api::int53 id) const;
            void add_chat(td::td_api::int53 id,td::td_api::object_ptr<td::td_api::chat> chat) const;
            void delete_chat(td::td_api::int53 id) const;
            void load();
            void add_plugin(std::unique_ptr<plugin::PluginInterface> plugin) const;
            void dispatchMessage(td::td_api::updateNewMessage && update_new_message) const;
            void dispatchMessage(td::td_api::updateNewCallbackQuery && update_new_callback_query) const;
            td::td_api::object_ptr<td::td_api::user>* get_user(td::td_api::int53 id) ;
            td::td_api::object_ptr<td::td_api::userFullInfo>* get_user_full_info(td::td_api::int53 id) ;
            td::td_api::object_ptr<td::td_api::chat>* get_chat(td::td_api::int53 id);
            std::vector<td::td_api::object_ptr<td::td_api::user>*> get_all_users() const;
            std::vector<td::td_api::object_ptr<td::td_api::chat>*> get_all_chats() const;
            td::td_api::object_ptr<td::td_api::user>* get_me();
            BotConfig* get_config() const;
            LoginType* get_login_type() const;
            std::string get_token() const;
            //todo 收紧权限
            tg::redis::RedisUtils* get_redis_utils() const;
            bool can_slove_message(td::td_api::int53 chat_id,td::td_api::int53 message_id) const;
        private:
          std::unique_ptr<LoginType> loginType;
          std::string token;
          std::unique_ptr<BotConfig> config;
          std::unique_ptr<tg::handler::DispathHandler> dispatch;
          std::unique_ptr<td::ClientManager> client_manager;
          std::unique_ptr<std::thread> client_thread;
          std::unique_ptr<std::future<int>> client_future;
          std::int32_t client_id;
          std::atomic<bool> running;
          std::unique_ptr<ThreadPool> thread_pool;
          std::unique_ptr<std::SafeSequence> request_id_sequence;
          std::unique_ptr<std::SafeSequence> response_id_sequence;
          std::unique_ptr<std::SafeMap<std::uint64_t,std::unique_ptr<tg::handler::Handler>>> request_and_handler;
          std::unique_ptr<std::SafeMap<std::uint64_t,td::ClientManager::Response>> response;

          std::shared_mutex users_mutex;
          std::unique_ptr<std::SafeMap<td::td_api::int53,td::td_api::object_ptr<td::td_api::user>>> users;

          std::shared_mutex user_full_info_mutex;
          std::unique_ptr<std::SafeMap<td::td_api::int53,td::td_api::object_ptr<td::td_api::userFullInfo>>> user_full_info;
          td::td_api::object_ptr<td::td_api::user> self;

          std::shared_mutex chats_mutex;
          std::unique_ptr<std::SafeMap<td::td_api::int53,td::td_api::object_ptr<td::td_api::chat>>> chats;

          std::unique_ptr<tg::plugin::Plugin> plugins;

          std::unique_ptr<tg::redis::RedisUtils> redis_utils;

          static std::string BOT_REDIS_PREFIX;
          void run() const;
          void login();
          void init(BotConfig config);
          void init_handler();
          void init_plugin();
          void init_redis();
          void reset();

    };

} // tg

#endif //TGBOT_H
