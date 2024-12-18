//
// Created by 19766 on 2024/12/2.
//

#ifndef COMMAND_H
#define COMMAND_H
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <td/telegram/td_api.h>


namespace tg{
    namespace redis {
        class RedisUtils;
    }

    class Bot;
    namespace plugin::command {
        #define PAGE_SIZE  10;
        enum CommandType {
           CHAT,USER,MANGER
        };
        class Command {
            public:
                virtual bool supprot(std::string command) const = 0;
                virtual bool handle(Bot* bot,CommandType type,td::td_api::int53 chat_id,std::string command)  = 0;
                virtual std::vector<std::pair<CommandType,std::pair<std::string,std::string>>> get_cmds() const = 0;
                static std::string PLUGIN_MESSAGE_PREFIX;
                static std::string PLUGIN_INDEX;
        };

        class SpiderCommand : public Command {
            public:
                explicit SpiderCommand();
                virtual bool supprot(std::string command) const override;
                bool handle(Bot *bot,CommandType type,td::td_api::int53 chat_id ,std::string command)  override;
                std::vector<std::pair<CommandType,std::pair<std::string,std::string>>> get_cmds() const override;
                static std::string SPIDER_PLUGIN_SPIDER_CHAT_KEY;
                static std::string SPIDER_PLUGIN_AD_PREFIX_KEY;

            protected:
                virtual bool spider(Bot* bot,td::td_api::int53 chat_id,td::td_api::int53 last_message_id = 0,int limit = INT32_MAX,int repeat = 100);
                bool save_message(Bot* bot,td::td_api::object_ptr<td::td_api::message> message,std::string prefix = "",std::string link = "");
                bool exits(tg::redis::RedisUtils* redis_utils,std::string prefix = "",td::td_api::int53 chat_id = 0,td::td_api::int53 message_id = 0);
                bool save_spider_chat(Bot* bot,td::td_api::int53 chat_id);
                std::vector<td::td_api::int53> get_spider_chat(Bot* bot);
                bool is_ad(Bot* bot,std::unordered_map<std::string, std::string> &hash);
                void pull_ad(Bot* bot,bool frace = false);
                std::vector<std::string> ad_black;
                std::vector<std::string> ad_white;
                std::shared_mutex ad_lock;
                long long last_pull_time = 0;

        };

        class InfoCommand : public Command {
            public:
              explicit InfoCommand();
              virtual bool supprot(std::string command) const override;
              bool handle(Bot *bot,CommandType type,td::td_api::int53 chat_id, std::string command)  override;
              std::vector<std::pair<CommandType,std::pair<std::string,std::string>>> get_cmds() const override;
        };

        class PermissionsCommand : public Command {
            public:
                explicit PermissionsCommand();
                virtual bool supprot(std::string command) const override;
                bool handle(Bot *bot,CommandType type, td::td_api::int53 chat_id, std::string command) override;
                std::vector<std::pair<CommandType,std::pair<std::string,std::string>>> get_cmds() const override;
                static std::string PERMISSIONS_PLUGIN_PREFIX;
                static std::string PERMISSIONS_PLUGIN_LIST_KEY;
                static std::string PERMISSIONS_PLUGIN_TOKEN_PREFIX;

        };

        class BotCommand : public Command {
            public:
                explicit BotCommand();
                virtual bool supprot(std::string command) const override;
                bool handle(Bot *bot,CommandType type, td::td_api::int53 chat_id, std::string command) override;
                std::vector<std::pair<CommandType,std::pair<std::string,std::string>>> get_cmds() const override;
        };

    }
}

#endif //COMMAND_H
