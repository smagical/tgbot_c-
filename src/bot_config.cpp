//
// Created by 19766 on 2024/11/15.
//

#include "include/bot_config.h"

#include <iostream>
#include <random>
#include <sstream>

#include "include/tg.h"
#include "plugins/command/include/command.h"


namespace tg {
    BotConfig::BotConfig() {
        set_default_value(BOT_MAX_WAIT_TIME,"100");
        set_default_value(BOT_WAIT_INTERVAL,"1");
        set_default_value(BOT_MIN_WAIT_TIME,"1");
        set_default_value(BOT_THREAD_NUMS,"1");
        set_default_value(BOT_LOG_LEVEL,"1");
        static std::random_device              rd;
        static std::mt19937                    gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        std::stringstream ss;
        ss >> std::hex;
        for (int i =0;i<10;i++) {
            ss<< dis(gen);
        }
        set_default_value(BOT_ID,std::move(ss.str()));
        set_default_value(REDIS_PORT,"6379");
        set_default_value(REDIS_DB,"0");
        set_default_value(BOT_CONFIG_PREFIX,"config");
        set_default_value(BOT_NAME_FILED,"CONFIG_PREFIX");
        set_default_value(BOT_REPORT_KEY,"online");
    }

    BotConfig::~BotConfig() {
        defaultConfig.clear();
        valueConfig.clear();
    }
    void BotConfig::load_from_file(std::string config_file) {
        std::ifstream file(config_file.c_str());
        if (!file.good()) return;
        std::string line;
        std::string prefix = "";
        while (std::getline(file, line)) {
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
            if (line.starts_with('#')) {}
            else if (line.starts_with("[")) {
                if (line.find_last_of("]") != std::string::npos) {
                    prefix = line.substr(1, line.find_last_of("]")-1);
                }
            }else {
                auto index = line.find_first_of("=");
                if (index != std::string::npos) {
                    std::string key = line.substr(0,index);
                    std::string value = line.substr(index+1);
                    if (key.size() == 0 || value.size() == 0) {
                        continue;
                    }
                    if (value.size() ==4) {
                        std::string tmp = value;
                        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
                        if (tmp == "true") {
                            value = "true";
                        }else if (tmp == "false") {
                            value = "false";
                        }
                    }
                    valueConfig[prefix+"."+key] = value;
                }
            }
        }
        file.close();
    }

    void BotConfig::load_from_str(std::string str) {
        std::stringstream ss(str);
        std::string line;
        std::string prefix = "";
        while (std::getline(ss, line)) {
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
            if (line.starts_with('#')) {}
            else if (line.starts_with("[")) {
                if (line.find_last_of("]") != std::string::npos) {
                    prefix = line.substr(1, line.find_last_of("]")-1);
                }
            }else {
                auto index = line.find_first_of("=");
                if (index != std::string::npos) {
                    std::string key = line.substr(0,index);
                    std::string value = line.substr(index+1);
                    if (key.size() == 0 || value.size() == 0) {
                        continue;
                    }
                    if (value.size() ==4) {
                        std::string tmp = value;
                        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
                        if (tmp == "true") {
                            value = "true";
                        }else if (tmp == "false") {
                            value = "false";
                        }
                    }
                    valueConfig[prefix+"."+key] = value;
                }
            }
        }
    }

    void BotConfig::load_from_redis() {
        auto host = get_string_value(REDIS_HOST);
        auto port = get_int_value(REDIS_PORT);
        auto db = get_string_value(REDIS_DB);
        auto user = get_string_value(REDIS_USERNAME);
        auto password = get_string_value(REDIS_PASSWORD);
        auto key_prefix = get_string_value(REDIS_KEY_PREFIX);
        auto config_prefix = get_string_value(BOT_CONFIG_PREFIX);
        try {
            auto redis = redis::RedisUtils(host, port,  user, password,key_prefix,0);
            if (redis.exists(config_prefix)) {
                // std::transform(str.begin(),str.end(),str.begin(),::toupper);
                auto keys = redis.smember(config_prefix);
                for (auto it = keys.begin(); it != keys.end(); it++) {
                    std::string key = *it;
                    auto value = redis.hget_all(key);
                    std::string conf_key = value[get_string_value(BOT_NAME_FILED)];
                    value.erase(get_string_value(BOT_NAME_FILED));
                    for (auto pair : value) {
                      std::string tmp = conf_key + "."+pair.first;
                      std::transform(tmp.begin(),tmp.end(),tmp.begin(),::toupper);
                      valueConfig[tmp] = pair.second;
                    }

                }
            }
            log_info("load config from redis successful");
        }catch (std::exception &e) {
            log_error(e.what());
            log_info("load_from_redis failed ");
        }
    }


    int BotConfig::get_int_value(std::string key) {
        if (valueConfig.contains(key)) {
            std::string value = valueConfig[key];
            try {
                return std::stoi(value);
            }catch (std::exception& e) {
                return std::stoi(defaultConfig[key]);
            }
        }
        return std::stoi(defaultConfig[key]);
    }

    long long BotConfig::get_long_long_value(std::string key) {
        if (valueConfig.contains(key)) {
            std::string value = valueConfig[key];
            try {
                return std::stoll(value);
            }catch (std::exception& e) {
                return std::stoll(defaultConfig[key]);
            }
        }
        return std::stoi(defaultConfig[key]);
    }

    std::string BotConfig::get_string_value(std::string key) {
        if (valueConfig.contains(key)) {
            return valueConfig[key];
        }
        return defaultConfig[key];
    }

    double BotConfig::get_double_value(std::string key) {
        if (valueConfig.contains(key)) {
            std::string value = valueConfig[key];
            try {
                return std::stod(value);
            }catch (std::exception& e) {
                return std::stod(defaultConfig[key]);
            }
        }
        return std::stod(defaultConfig[key]);
    }

    bool BotConfig::get_bool_value(std::string key) {
        if (valueConfig.contains(key)) {
            std::string value = valueConfig[key];
            if (value == "true") {
                return true;
            }else if (value == "false") {
                return false;
            }
        }
        std::string value = defaultConfig[key];
        if (value == "true") {
            return true;
        }else if (value == "false") {
            return false;
        }else {
            return false;
        }
    }

    void BotConfig::set_value(std::string key, std::string value) {
        valueConfig[key] = value;
    }

    void BotConfig::remove_value(std::string key) {
        valueConfig.erase(key);
    }




    std::string   BotConfig::BOT_MAX_WAIT_TIME  = "BOT.MAX_WAIT_TIME";
    std::string   BotConfig::BOT_WAIT_INTERVAL = "BOT.WAIT_INTERVAL";
    std::string   BotConfig::BOT_MIN_WAIT_TIME  = "BOT.MIN_WAIT_TIME";
    std::string   BotConfig::BOT_THREAD_NUMS  = "BOT.THREAD_NUMS";
    std::string   BotConfig::BOT_LOG_LEVEL = "BOT.LOG_LEVEL";
    std::string   BotConfig::BOT_ID = "BOT.ID";
    std::string   BotConfig::BOT_ADMIN = "BOT.ADMIN";
    std::string   BotConfig::BOT_LOGIN_INFO = "BOT.LOGIN_INFO";
    std::string   BotConfig::BOT_LOGIN_TYPE = "BOT.LOGIN_TYPE";
    std::string   BotConfig::BOT_CONFIG_PREFIX = "BOT.CONFIG_PREFIX";
    std::string   BotConfig::BOT_NAME_FILED = "BOT.NAME_FILED";
    std::string   BotConfig::BOT_REPORT_KEY = "BOT.REPORT_KEY";



    std::string   BotConfig::TD_API_ID = "TD.API_ID";
    std::string   BotConfig::TD_API_HASH = "TD.API_HASH";
    std::string   BotConfig::TD_TITLE = "TD.TITLE";
    std::string   BotConfig::TD_SHORT_NAME = "TD.SHORT_NAME";
    std::string   BotConfig::TD_APPLICATION_VERSION = "TD.APPLICATION_VERSION";
    std::string   BotConfig::TD_LANGUAGE_CODE = "TD.LANGUAGE_CODE";
    std::string   BotConfig::TD_USE_TEST = "TD.USE_TEST";
    std::string   BotConfig::TD_DATABASE_DIRECTORY = "TD.DATABASE_DIRECTORY";
    std::string   BotConfig::TD_DATABASE_ENCRYPTION_KEY = "TD.DATABASE_ENCRYPTION_KEY";
    std::string   BotConfig::TD_REQUEST_RETRY = "TD.REQUEST_RETRY";

    //REDIS
     std::string   BotConfig::REDIS_HOST = "REDIS.HOST";
     std::string   BotConfig::REDIS_PORT = "REDIS.PORT";
     std::string   BotConfig::REDIS_USERNAME = "REDIS.USERNAME";
     std::string   BotConfig::REDIS_PASSWORD = "REDIS.PASSWORD";
     std::string   BotConfig::REDIS_KEY_PREFIX = "REDIS.KEY_PREFIX";
     std::string   BotConfig::REDIS_DB = "REDIS.DB";

} // tg