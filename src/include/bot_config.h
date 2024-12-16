//
// Created by 19766 on 2024/11/15.
//

#ifndef BOTCONFIG_H
#define BOTCONFIG_H
#include <algorithm>
#include <fstream>
#include <map>
#include <string>

namespace tg {

    class BotConfig {
        public:
            //BOT
            static std::string   MAX_WAIT_TIME ;
            static std::string   WAIT_INTERVAL ;
            static std::string   MIN_WAIT_TIME ;
            static std::string   THREAD_NUMS ;
            static std::string   LOG_LEVEL ;
            static std::string   ID;
            static std::string   ADMIN ;
            static std::string   LOGIN_INFO ;
            static std::string   LOGIN_TYPE ;

            //TD
            static std::string   API_ID;
            static std::string   API_HASH;
            static std::string   TITLE;
            static std::string   SHORT_NAME;
            static std::string   APPLICATION_VERSION;
            static std::string   LANGUAGE_CODE;
            static std::string   USE_TEST;
            static std::string   DATABASE_DIRECTORY ;
            static std::string   DATABASE_ENCRYPTION_KEY;
            static std::string   REQUEST_RETRY;

            //REDIS
            static std::string   HOST;
            static std::string   PORT;
            static std::string   USERNAME;
            static std::string   PASSWORD;
            static std::string   KEY_PREFIX;
            static std::string   DB;

            BotConfig();
            ~BotConfig();
            void load_config(std::string config_file);
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
