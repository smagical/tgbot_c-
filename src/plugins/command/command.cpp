//
// Created by 19766 on 2024/12/2.
//

#include "include/command.h"

#include <random>

#include "../../include/bot_util.h"

#define SPIDER_LIMIT 200
#define SPIDER_RETRY 5
namespace tg::plugin::command {


    SpiderCommand::SpiderCommand() {
    }


    std::vector<std::pair<command::CommandType, std::pair<std::string,std::string>> > SpiderCommand::get_cmds() const {
        std::vector<std::pair<CommandType, std::pair<std::string,std::string>> > cmds;
        cmds.push_back(std::make_pair(CommandType::MANGER, std::make_pair("/spider","/spider chat_id last_message_id(0 meaning lastMessage) limit [spider chat]")));
        cmds.push_back(std::make_pair(CommandType::MANGER,std::make_pair( "/spider_update","/spider_update chat_id [update spider to lastMessage]")));
        cmds.push_back(std::make_pair(CommandType::MANGER, std::make_pair("/spider_update_all","/spider_update_all [spider all chat if chat in db]")));
        cmds.push_back(std::make_pair(CommandType::MANGER,std::make_pair("/spider_del","/spider_del chat_id [del spider chat_id]")));
        cmds.push_back(std::make_pair(CommandType::MANGER,std::make_pair("/spider_list","/spider_list [spider chat list]")));
        cmds.push_back(std::make_pair(CommandType::MANGER,std::make_pair("/spider_ad_add","/spider_ad_add word [add spider ad word]")));
        cmds.push_back(std::make_pair(CommandType::MANGER,std::make_pair("/spider_ad_del","/spider_ad_del word [del spider ad word]")));
        cmds.push_back(std::make_pair(CommandType::MANGER,std::make_pair("/spider_ad_list","/spider_ad_list   [spider ad list]")));


        return cmds;
    }

    bool SpiderCommand::supprot(std::string command) const {
        if (command.starts_with("spider")) {
            return true;
        }
        return false;
    }

    bool SpiderCommand::handle(Bot *bot,CommandType type,td::td_api::int53 chat_id_send,std::string command)  {
        std::vector<std::string> commands = split(command," ");
        if (commands.size() < 1) return false;
        const std::string cmd = commands[0];
        td::td_api::int53 chat_id = 0;
        td::td_api::int53 message_id = 0;
        try {
            if (type >= command::MANGER && cmd == "spider") {
                if (commands.size() <= 1) return false;
                chat_id = std::stoll(commands[1].c_str());
                int limit = INT32_MAX;
                if (commands.size() > 2) {
                    message_id = std::stoll(commands[2].c_str());
                }
                if (commands.size() > 3) {
                    limit = std::stoll(commands[3].c_str());
                }
                util::send(bot,chat_id_send,commands[1] + " start spider");
                spider(bot,chat_id,message_id,limit);
                util::send(bot,chat_id_send,commands[1] + " end spider");
            }
            else if (type >= command::MANGER && cmd == "spider_update") {
                if (commands.size() <= 1) return false;
                chat_id = std::stoll(commands[1].c_str());
                util::send(bot,chat_id_send,commands[1] + " start spider update");
                spider(bot,chat_id,message_id);
                util::send(bot,chat_id_send,commands[1] + " end spider update");
            }
            else if (type >= command::MANGER && cmd == "spider_update_all") {
                util::send(bot,chat_id_send,"start spider all update");
                auto chats = get_spider_chat(bot);
                for (auto it = chats.begin(); it != chats.end(); ++it) {
                    try {
                        spider(bot,*it);
                    }catch (...){}
                }
                util::send(bot,chat_id_send, "end spider all update");
            }
            else if (type >= command::MANGER && cmd == "spider_del") {
                if (commands.size() <= 1) return false;
                bot->get_redis_utils()->srem(bot->get_config()->get_string_value(SPIDER_PLUGIN_SPIDER_CHAT_KEY),commands[1]);
                util::send(bot,chat_id_send,commands[1] + " end spider del");
            }
            else if (type >= command::MANGER && cmd == "spider_list") {
                auto chats = get_spider_chat(bot);
                std::stringstream ss;
                for (auto it = chats.begin(); it != chats.end(); ++it) {
                    auto chat = bot->get_chat(*it);
                    if (chat) {
                        ss << *it <<":"<<chat->get()->title_<< std::endl;
                    }else {
                        ss << *it <<std::endl;
                    }
                }
                util::send(bot,chat_id_send,ss.str());
            }
            else if (type >= command::MANGER && cmd == "spider_ad_add") {
                if (commands.size() <= 1) return false;
                bot->get_redis_utils()->sadd(bot->get_config()->get_string_value(SPIDER_PLUGIN_AD_PREFIX_KEY),commands[1]);
                this->pull_ad(bot,true);
                util::send(bot,chat_id_send,"add spider ad word successful");
            }
            else if (type >= command::MANGER && cmd == "spider_ad_del") {
                if (commands.size() <= 1) return false;
                bot->get_redis_utils()->srem(bot->get_config()->get_string_value(SPIDER_PLUGIN_AD_PREFIX_KEY),commands[1]);
                this->pull_ad(bot,true);
                util::send(bot,chat_id_send,"del spider ad word successful");
            }
            else if (type >= command::MANGER && cmd == "spider_ad_list") {
                std::stringstream ss;
                int count = 0;
                pull_ad(bot,true);
                std::shared_lock<std::shared_mutex> lock(this->ad_lock);
                for (auto ad_white : this->ad_white) {
                    ss <<count<<". "<< ad_white << std::endl;
                    count++;
                }
                for (auto ad_black : this->ad_black) {
                    ss <<count<<". "<<  ad_black << std::endl;
                    count++;
                }
                lock.unlock();
                std::string str = ss.str();
                if (str.size() <= 0) {
                    str = "no ad word";
                }
                util::send(bot,chat_id_send,str);
            }
            else {
                return false;
            }
        }catch (...) {
            util::send(bot,chat_id_send,command + "  error spider");
            return false;
        }
        return true;
    }

    bool SpiderCommand::spider(Bot *bot, td::td_api::int53 chat_id, td::td_api::int53 last_message_id, int limit, int repeat) {
        save_spider_chat(bot,chat_id);
        int last_limit = limit;
        int retry = 0;
        while (last_limit > 0 && retry < SPIDER_RETRY && repeat > 0) {
            int current_limit = std::min(SPIDER_LIMIT, last_limit);
            if (current_limit <= 0) return true;
            std::vector<td::td_api::object_ptr<td::td_api::message>> messages =
                    util::load_history(bot, chat_id, last_message_id, current_limit);
            if (messages.size() > 0 && messages.begin()->get()->id_ == last_message_id) {
                messages.erase(messages.begin());
            }
            if (messages.size() == 0) {
                retry++;
                continue;
            }
            retry = 0;
            last_limit -= messages.size();
            last_message_id = messages.rbegin()->get()->id_;
            tg::redis::RedisUtils* redis_utils = bot->get_redis_utils();

            std::string prefix = bot->get_config()->get_string_value(PLUGIN_MESSAGE_PREFIX);

            std::vector<std::string> index_prefix;
            index_prefix.push_back(prefix);

            std::vector<std::string> schme;
            schme.push_back("content");
            schme.push_back("TEXT");
            schme.push_back("type");
            schme.push_back("TAG");

            redis_utils->create_index(bot->get_config()->get_string_value(PLUGIN_INDEX),"hash",index_prefix,"chinese",schme);
            for (auto it = messages.begin(); messages.end() != it; ++it) {
                if (exits(bot->get_redis_utils(),prefix,chat_id,last_message_id)) {
                    repeat--;
                    continue;
                }
                std::string link = util::load_message_link(bot,chat_id,it->get()->id_);
                save_message(bot,std::move(*it),prefix,link);
            }

        }
        return true;
    }

    bool SpiderCommand::save_message(Bot* bot,td::td_api::object_ptr<td::td_api::message> message,std::string prefix,std::string link) {

            std::unordered_map<std::string, std::string> hash;
            std::string chat_str = std::to_string(message->chat_id_);
            std::string message_str = std::to_string(message->id_);
            std::string album_str = std::to_string(message->media_album_id_);
            std::string key = prefix+":"+chat_str+":"+message_str;
            std::int32_t type_id = message->content_->get_id();
            hash["id"] = message_str;
            hash["chat_id"] = chat_str;
            hash["album_id"] = album_str;
            hash["link"] = link;
            if (type_id == td::td_api::messagePhoto::ID) {
                const auto message_photo = static_cast<td::td_api::messagePhoto*>(message->content_.get());
                std::string content = message_photo->caption_->text_;
                hash["content"] = std::move(content);
            }else if (td::td_api::messageText::ID == type_id) {
                const auto message_text = static_cast<td::td_api::messageText*>(message->content_.get());
                std::string content = message_text->text_->text_;
                hash["content"] = std::move(content);
            }else if (td::td_api::messageVideo::ID == type_id) {
                const auto message_video = static_cast<td::td_api::messageVideo*>(message->content_.get());
                std::string content = message_video->caption_->text_;
                hash["content"] = std::move(content);
            }else {
                return true;
            }
            if (is_ad(bot,hash)) {
                hash["type"] = "ad";
            }else {
                hash["type"] = "message";
            }
            // redis_utils->create_index()
            bot->get_redis_utils()->hmset(key,hash);
            return true;
        }
    bool SpiderCommand::save_spider_chat(Bot* bot,td::td_api::int53 chat_id) {
        return bot->get_redis_utils()->sadd(bot->get_config()->get_string_value(SPIDER_PLUGIN_SPIDER_CHAT_KEY),std::to_string(chat_id));
    }

    std::vector<td::td_api::int53> SpiderCommand::get_spider_chat(Bot* bot) {
        std::vector<td::td_api::int53> chat_ids;
        auto set = bot->get_redis_utils()->smember(bot->get_config()->get_string_value(SPIDER_PLUGIN_SPIDER_CHAT_KEY));
        for (auto value : set) {
            chat_ids.push_back(std::stoll(value));
        }
        return chat_ids;
    }


    bool SpiderCommand::exits(tg::redis::RedisUtils* redis_utils,std::string prefix,td::td_api::int53 chat_id,td::td_api::int53 message_id) {
            std::string chat_str = std::to_string(chat_id);
            std::string message_str = std::to_string(message_id);
            std::string key = prefix + ":" + chat_str + ":" + message_str;
            return redis_utils->exists(key);
        };

    void SpiderCommand::pull_ad(Bot* bot,bool frace) {
        std::chrono::milliseconds now = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
        if (now.count() - this->last_pull_time < (60 * 1000) && !frace) {
            return;
        }
        std::unique_lock<std::shared_mutex> lock(this->ad_lock);
        now = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
        if (now.count() - this->last_pull_time < (60 * 1000) && !frace) {
            return;
        }
        auto values = bot->get_redis_utils()->smember(bot->get_config()->get_string_value(SPIDER_PLUGIN_AD_PREFIX_KEY));
        this->ad_black.clear();
        this->ad_white.clear();
        for (auto basic_string : values) {
            if (basic_string.starts_with("!")) this->ad_white.push_back(basic_string);
            else this->ad_black.push_back(basic_string);
        }
        this->last_pull_time = now.count();
    }

    bool SpiderCommand::is_ad(Bot* bot,std::unordered_map<std::string,std::string>& hash) {
        pull_ad(bot,false);
        std::string& content = hash["content"];
        if (content == "") {
            return true;
        }
        std::shared_lock<std::shared_mutex> lock(this->ad_lock);
        for (auto white : ad_white) {
            if (content.find(white) != std::string::npos) {
                return false;
            }
        }
        for (auto ad :ad_black) {
            if (content.find(ad)!=std::string::npos) {
                return true;
            }
        }
        return false;
    };

    std::string SpiderCommand::SPIDER_PLUGIN_SPIDER_CHAT_KEY = "PLUGIN.SPIDER.SPIDER_CHAT_KEY";
    std::string SpiderCommand::SPIDER_PLUGIN_AD_PREFIX_KEY = "PLUGIN.SPIDER.AD_PREFIX_KEY";
    std::string Command::PLUGIN_MESSAGE_PREFIX = "PLUGIN.MESSAGE_PREFIX";
    std::string Command::PLUGIN_INDEX = "PLUGIN.INDEX";


    std::vector<std::pair<CommandType, std::pair<std::string,std::string>> > InfoCommand::get_cmds() const {
        std::vector<std::pair<CommandType, std::pair<std::string,std::string>> > cmds;
        cmds.push_back(std::make_pair(command::CommandType::CHAT,std::make_pair("/info","/info get user_id")));
        cmds.push_back(std::make_pair(CommandType::MANGER,std::make_pair("/info_user","/info_user offset(0) [get bot users]")));
        cmds.push_back(std::make_pair(CommandType::MANGER,std::make_pair("/info_chat","/info_chat offset(0) [get chat]")));
        return cmds;
    }

    InfoCommand::InfoCommand() {

    }


    bool InfoCommand::supprot(std::string command) const {
        return command.starts_with("info");
    }

    bool InfoCommand::handle(Bot *bot,CommandType type,td::td_api::int53 chat_id ,std::string command) {
        auto cmds = split(command," ");
        if (type >= CommandType::CHAT && cmds[0] == "info") {
            util::send(bot,chat_id,std::to_string(chat_id));
        }
        else if (type >= CommandType::MANGER && cmds[0] == "info_user") {
            int offset = 0;
            if (cmds.size() > 1) {
                offset = std::stoll(cmds[1]);
            }
            if (offset < 0 ) offset = 0;
            auto users = bot -> get_all_users();
            int page_size = PAGE_SIZE;
            int start = offset * page_size;
            int end_ = (users.size() + page_size - 1) / page_size;
            if (start >= users.size()) {
                util::send(bot,chat_id,"end page is "+std::to_string(end_) +"\n not found page");
            }else {
                int last = start + PAGE_SIZE;
                int size = users.size();
                last = std::min(last,size);
                std::vector<std::string> keys;
                std::vector<std::string> values;
                for (int i = start; i < last; i++) {
                    auto user = users[i];
                    std::string key = user -> get() ->first_name_ + " " + user -> get() -> last_name_;
                    std::string value = std::to_string(user -> get() -> id_);
                    keys.push_back(key);
                    values.push_back(value);
                }
                std::string end =  std::to_string(offset) + "/" + std::to_string(end_) + "\n";
                util::send(
                    bot,
                    chat_id,
                    [keys,values,end]() {
                        int size = values.size();
                        std::stringstream ss;
                        int len = 0;
                        td::td_api::array<td::td_api::object_ptr<td::td_api::textEntity>> entities;
                        for (int i = 0; i < size; i++) {
                            std::string key = std::to_string(i) + ". "+keys[i]+":";
                            std::string value = values[i];
                            const int key_utf16_len = std::cover_utf16(key).length();
                            const int value_utf16_len = std::cover_utf16(value).length();
                            ss << key<<value <<std::endl;
                            td::td_api::textEntity entity(len + key_utf16_len,value_utf16_len,
                                td::td_api::make_object<td::td_api::textEntityTypeCode>());
                            entities.push_back(td::td_api::make_object<td::td_api::textEntity>(std::move(entity)));
                            len += key_utf16_len + value_utf16_len + 1;
                        }
                        ss << end;
                        ss<< tg::END;
                        std::string str = ss.str();
                        return td::td_api::make_object<td::td_api::formattedText>(str,std::move(entities));
                    },
                            [](){return nullptr;},
                            [](){return nullptr;},
                            3
                    );
            }
        }
        else if (type >= CommandType::MANGER && cmds[0] == "info_chat") {
            int offset = 0;
            if (cmds.size() > 1) {
                offset = std::stoll(cmds[1]);
            }
            if (offset < 0 ) offset = 0;
            auto chats = bot -> get_all_chats();
            int page_size = PAGE_SIZE;
            int start = offset * page_size;
            int end_ = (chats.size() + page_size - 1) / page_size;
            if (start >= chats.size()) {
                util::send(bot,chat_id,"end page is "+std::to_string(end_) +" not found page");
            }else {
                int last = start + PAGE_SIZE;
                int size = chats.size();
                last = std::min(last,size);
                std::vector<std::string> keys;
                std::vector<std::string> values;
                for (int i = start; i < last; i++) {
                    auto chat = chats[i];
                    std::string key = chat -> get() ->title_;
                    std::string value = std::to_string(chat -> get() -> id_);
                    keys.push_back(key);
                    values.push_back(value);
                }
                std::string end =  std::to_string(offset) + "/" + std::to_string(end_) + "\n";
                util::send(
                    bot,
                    chat_id,
                    [keys,values,end]() {
                        int size = values.size();
                        std::stringstream ss;
                        int len = 0;
                        td::td_api::array<td::td_api::object_ptr<td::td_api::textEntity>> entities;
                        for (int i = 0; i < size; i++) {
                            std::string key = std::to_string(i) + ". "+keys[i]+":";
                             std::string value = values[i];
                             const int key_utf16_len = std::cover_utf16(key).length();
                             const int value_utf16_len = std::cover_utf16(value).length();
                             ss << key<<value <<std::endl;
                             td::td_api::textEntity entity(len + key_utf16_len,value_utf16_len,
                                 td::td_api::make_object<td::td_api::textEntityTypeCode>());
                             entities.push_back(td::td_api::make_object<td::td_api::textEntity>(std::move(entity)));
                             len += key_utf16_len + value_utf16_len + 1;
                        }
                        ss << end;
                        ss<< tg::END;
                        std::string str = ss.str();
                        return td::td_api::make_object<td::td_api::formattedText>(str,std::move(entities));
                    },
                            [](){return nullptr;},
                            [](){return nullptr;},
                            3
                    );
            }

        }
        else if (type >= CommandType::MANGER && cmds[0] == "info_user_all") {
            auto users = bot -> get_all_users();
            int last = users.size();
            std::vector<std::string> keys;
            std::vector<std::string> values;
            for (int i = 0; i < last; i++) {
                auto user = users[i];
                std::string key = user -> get() ->first_name_ + " " + user -> get() -> last_name_;
                std::string value = std::to_string(user -> get() -> id_);
                keys.push_back(key);
                values.push_back(value);
            }
            util::send(
                bot,
                chat_id,
                [keys,values]() {
                    int size = values.size();
                    std::stringstream ss;
                    int len = 0;
                    td::td_api::array<td::td_api::object_ptr<td::td_api::textEntity>> entities;
                    for (int i = 0; i < size; i++) {
                          std::string key = std::to_string(i) + ". "+keys[i]+":";
                           std::string value = values[i];
                           const int key_utf16_len = std::cover_utf16(key).length();
                           const int value_utf16_len = std::cover_utf16(value).length();
                           ss <<key<< value <<std::endl;
                           td::td_api::textEntity entity(len + key_utf16_len,value_utf16_len,
                               td::td_api::make_object<td::td_api::textEntityTypeCode>());
                           entities.push_back(td::td_api::make_object<td::td_api::textEntity>(std::move(entity)));
                           len += key_utf16_len + value_utf16_len + 1;
                           ss << "\n";
                    }
                    ss<< tg::END;
                    std::string str = ss.str();
                    return td::td_api::make_object<td::td_api::formattedText>(str,std::move(entities));
                },
                        [](){return nullptr;},
                        [](){return nullptr;},
                        3
                );
        }
        else if (type >= CommandType::MANGER && cmds[0] == "info_chat_all") {
            int offset = 0;
            if (cmds.size() > 1) {
                offset = std::stoll(cmds[1]);
            }
            if (offset < 0 ) offset = 0;
            auto chats = bot -> get_all_chats();
            int last = chats.size();
            std::vector<std::string> keys;
            std::vector<std::string> values;
            for (int i = 0; i < last; i++) {
                auto chat = chats[i];
                std::string key = chat -> get() ->title_;
                std::string value = std::to_string(chat -> get() -> id_);
                keys.push_back(key);
                values.push_back(value);
            }
            util::send(
                bot,
                chat_id,
                [keys,values]() {
                    int size = values.size();
                    std::stringstream ss;
                    int len = 0;
                    td::td_api::array<td::td_api::object_ptr<td::td_api::textEntity>> entities;
                    for (int i = 0; i < size; i++) {
                        std::string key = std::to_string(i) + ". "+keys[i]+":";
                           std::string value = values[i];
                           const int key_utf16_len = std::cover_utf16(key).length();
                           const int value_utf16_len = std::cover_utf16(value).length();
                           ss<<key << value <<std::endl;
                           td::td_api::textEntity entity(len + key_utf16_len,value_utf16_len,
                               td::td_api::make_object<td::td_api::textEntityTypeCode>());
                           entities.push_back(td::td_api::make_object<td::td_api::textEntity>(std::move(entity)));
                           len += key_utf16_len + value_utf16_len + 1;
                           ss << "\n";
                    }
                    ss<< tg::END;
                    std::string str = ss.str();
                    return td::td_api::make_object<td::td_api::formattedText>(str,std::move(entities));
                },
                        [](){return nullptr;},
                        [](){return nullptr;},
                        3
                );
        }
        else {
            return false;
        }
        return true;
    }


    PermissionsCommand::PermissionsCommand() {

    }

    std::vector<std::pair<CommandType, std::pair<std::string,std::string>> > PermissionsCommand::get_cmds() const {
        std::vector<std::pair<CommandType, std::pair<std::string,std::string>> > cmds;
        cmds.push_back(std::make_pair(CommandType::USER,std::make_pair("/premiss_add","/premiss_add chat_id [add chat_id]")));
        cmds.push_back(std::make_pair(CommandType::USER,std::make_pair("/premiss_del","/premiss_del chat_id [del chat_id]")));
        cmds.push_back(std::make_pair(CommandType::USER,std::make_pair("/premiss_genera_token","/premiss_genera_token [get chat invite token]")));
        cmds.push_back(std::make_pair(CommandType::CHAT,std::make_pair("/premiss_invite_token","/premiss_invite_token token [add chat use invite token]")));
        return cmds;
    }

    bool PermissionsCommand::handle(Bot *bot,CommandType type, td::td_api::int53 chat_id, std::string command) {
        auto commands = split(command, " ");
        try {
            if (type >= CommandType::USER && commands[0] == "premiss_add") {
                if (commands.size() <= 1) {
                    return false;
                }
                td::td_api::int53 chat_id = std::stoll(commands[1]);
                std::string user_id_str = std::to_string(chat_id);
                bot->get_redis_utils()->sadd(bot->get_config()->get_string_value(PERMISSIONS_PLUGIN_LIST_KEY)+":"+commands[1],user_id_str);
                util::send(bot,chat_id,"add "+commands[1]+" successful");
            }
            else if (type >= CommandType::USER && commands[0] == "premiss_del") {
                if (commands.size() <= 1) {
                    return false;
                }
                td::td_api::int53 chat_id = std::stoll(commands[1]);
                std::string user_id_str = std::to_string(chat_id);
                std::string key = bot->get_config()->get_string_value(PERMISSIONS_PLUGIN_LIST_KEY)+":"+commands[1];
                if ( bot->get_redis_utils()->sismember(key,user_id_str)) {
                    bot->get_redis_utils()->srem(key,user_id_str);
                }
                util::send(bot,chat_id,"del "+commands[1]+" successful");
            }
            else if (type >= CommandType::USER && commands[0] == "premiss_genera_token") {
                static std::random_device              rd;
                static std::mt19937                    gen(rd());
                static std::uniform_int_distribution<> dis(0, 15);
                std::stringstream ss;
                ss >> std::hex;
                for (int i =0;i<32;i++) {
                    ss<< dis(gen);
                }
                std::string token = ss.str();
                std::string user_id_str = std::to_string(chat_id);
                bot->get_redis_utils()->set(bot->get_config()->get_string_value(PERMISSIONS_PLUGIN_TOKEN_PREFIX)+":"+token,user_id_str,5*60*1000);
                util::send(
                        bot,
                        chat_id,
                        [token,user_id_str]() {
                            td::td_api::array<td::td_api::object_ptr<td::td_api::textEntity>> entities;
                            std::string prefix = "INVITE_TOKEN: ";
                            int prefix_utf16_size = std::cover_utf16(prefix).length();
                            int token_utf16_size = std::cover_utf16(token).length();
                            entities.push_back(
                                td::td_api::make_object<td::td_api::textEntity>(
                                        prefix_utf16_size,
                                        token_utf16_size,
                                        td::td_api::make_object<td::td_api::textEntityTypeCode>()
                                    )
                            );
                            td::td_api::object_ptr<td::td_api::formattedText> formatted =
                                td::td_api::make_object<td::td_api::formattedText>(prefix+token,std::move(entities));
                            return std::move(formatted);
                        },
                        [](){return nullptr;},
                        [](){return nullptr;},
                        3
                    );
            }
            else if (type >= CommandType::CHAT && commands[0] == "premiss_invite_token") {
                if (commands.size() <= 1) {
                    return false;
                }
                std::string token = commands[1];
                std::string key = bot->get_config()->get_string_value(PERMISSIONS_PLUGIN_TOKEN_PREFIX)+":"+token;
                if (bot->get_redis_utils()->exists(key)) {
                    std::string chat_str = std::to_string(chat_id);
                    std::string chat_ = bot->get_redis_utils()->get(key);
                    bot->get_redis_utils()->del(key);
                    bot->get_redis_utils()->sadd(bot->get_config()->get_string_value(PERMISSIONS_PLUGIN_LIST_KEY)+":"+chat_str,chat_);
                    util::send(bot,chat_id,"add successful");
                }else {
                    util::send(bot,chat_id,"token error");
                }
            }
        }catch(...) {
            return false;
        }
        return true;
    }

    bool PermissionsCommand::supprot(std::string command) const {
        return command.starts_with("premiss");
    }

    std::string PermissionsCommand::PERMISSIONS_PLUGIN_PREFIX = "PREFIX";
    std::string PermissionsCommand::PERMISSIONS_PLUGIN_LIST_KEY = "LIST_KEY";
    std::string PermissionsCommand::PERMISSIONS_PLUGIN_TOKEN_PREFIX = "TOKEN_PREFIX";

    BotCommand::BotCommand() {

    }

    std::vector<std::pair<CommandType, std::pair<std::string, std::string> > > BotCommand::get_cmds() const {
        std::vector<std::pair<CommandType, std::pair<std::string, std::string> > > cmds;
        cmds.push_back(std::pair<CommandType, std::pair<std::string, std::string> >(std::make_pair(CommandType::MANGER,std::make_pair("/bot_online","/bot_online [online bot list]"))));
        // cmds.push_back(std::pair<CommandType, std::pair<std::string, std::string> >(std::make_pair(CommandType::MANGER,std::make_pair("/bot_restart","/bot_restart bot_id [restart bot]"))));
        return cmds;
    }


    bool BotCommand::handle(Bot *bot, CommandType type, td::td_api::int53 chat_id, std::string command) {
        auto commands = std::split(command);
        if (commands.size() <=0) {
            return false;
        }
        if (commands[0] == "bot_online") {
            auto bots = bot->get_redis_utils()->keys(bot->get_config()->get_string_value(BotConfig::BOT_REPORT_KEY)+"*");
            int count = 0;
            std::stringstream ss;
            for (auto basic_string : bots) {
                std::string bot_name = basic_string;
                if (bot_name.find_last_of(":")!=std::string::npos) {
                    bot_name = bot_name.substr(bot_name.find_last_of(":")+1);
                }
                ss << count << ". " << bot_name<<std::endl;
                count++;
            }
            std::string str = ss.str();
            util::send(bot,chat_id,str);
            return true;
        }
//         else if (commands[0] == "bot_restart") {
// // #ifdef _WIN32
// // #elif __FreeBSD__ || __linux__
// //             util::send(bot,chat_id,"restart ...");
// // #endif
//
//         }
        return false;
    }


    bool BotCommand::supprot(std::string command) const {
        return command.starts_with("bot");
    }


}
