//
// Created by 19766 on 2024/11/15.
//

#ifndef BOTCONFIG_H
#define BOTCONFIG_H
#include <algorithm>
#include <fstream>
#include <map>
#include <string>

#include "../redis/include/redis_utils.h"

namespace tg {

    class BotConfig {
        public:
            //BOT
            static std::string   BOT_MAX_WAIT_TIME ;
            static std::string   BOT_WAIT_INTERVAL ;
            static std::string   BOT_MIN_WAIT_TIME ;
            static std::string   BOT_THREAD_NUMS ;
            static std::string   BOT_LOG_LEVEL ;
            static std::string   BOT_ID;
            static std::string   BOT_ADMIN ;
            static std::string   BOT_LOGIN_INFO ;
            static std::string   BOT_LOGIN_TYPE ;
            static std::string   BOT_CONFIG_PREFIX ;
            static std::string   BOT_NAME_FILED ;
            static std::string   BOT_REPORT_KEY ;


            //TD
            static std::string   TD_API_ID;
            static std::string   TD_API_HASH;
            static std::string   TD_TITLE;
            static std::string   TD_SHORT_NAME;
            static std::string   TD_APPLICATION_VERSION;
            static std::string   TD_LANGUAGE_CODE;
            static std::string   TD_USE_TEST;
            static std::string   TD_DATABASE_DIRECTORY ;
            static std::string   TD_DATABASE_ENCRYPTION_KEY;
            static std::string   TD_REQUEST_RETRY;

            //REDIS
            static std::string   REDIS_HOST;
            static std::string   REDIS_PORT;
            static std::string   REDIS_USERNAME;
            static std::string   REDIS_PASSWORD;
            static std::string   REDIS_KEY_PREFIX;
            static std::string   REDIS_DB;


            BotConfig();
            ~BotConfig();
            void load_from_file(std::string config_file);
            void load_from_str(std::string config);
            void load_from_redis();
            int get_int_value(std::string key);

            long long get_long_long_value(std::string key);
            std::string get_string_value(std::string key);
            double get_double_value(std::string key);
            bool get_bool_value(std::string key);
            void set_value(std::string key, std::string value);
            void remove_value(std::string key);
        private:
            std::map<std::string, std::string> defaultConfig;
            std::map<std::string, std::string> valueConfig;
            void set_default_value(std::string key, std::string value) {
                defaultConfig[key] = value;
            }
    };


} // tg

#endif //BOTCONFIG_H
