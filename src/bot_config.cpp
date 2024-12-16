//
// Created by 19766 on 2024/11/15.
//

#include "include/bot_config.h"

#include <iostream>
#include <random>
#include <sstream>


namespace tg {
    BotConfig::BotConfig() {
        set_default_value(MAX_WAIT_TIME,"100");
        set_default_value(WAIT_INTERVAL,"1");
        set_default_value(MIN_WAIT_TIME,"1");
        set_default_value(THREAD_NUMS,"1");
        set_default_value(LOG_LEVEL,"1");
        static std::random_device              rd;
        static std::mt19937                    gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        std::stringstream ss;
        ss >> std::hex;
        for (int i =0;i<10;i++) {
            ss<< dis(gen);
        }
        set_default_value(ID,std::move(ss.str()));
        set_default_value(PORT,"6379");
        set_default_value(DB,"0");
    }

    BotConfig::~BotConfig() {
        defaultConfig.clear();
        valueConfig.clear();
    }

    void BotConfig::load_config(std::string config_file) {
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




    std::string   BotConfig::MAX_WAIT_TIME  = "BOT.MAX_WAIT_TIME";
    std::string   BotConfig::WAIT_INTERVAL = "BOT.WAIT_INTERVAL";
    std::string   BotConfig::MIN_WAIT_TIME  = "BOT.MIN_WAIT_TIME";
    std::string   BotConfig::THREAD_NUMS  = "BOT.THREAD_NUMS";
    std::string   BotConfig::LOG_LEVEL = "BOT.LOG_LEVEL";
    std::string   BotConfig::ID = "BOT.ID";
    std::string   BotConfig::ADMIN = "BOT.ADMIN";
    std::string   BotConfig::LOGIN_INFO = "BOT.LOGIN_INFO";
    std::string   BotConfig::LOGIN_TYPE = "BOT.LOGIN_TYPE";


    std::string   BotConfig::API_ID = "TD.API_ID";
    std::string   BotConfig::API_HASH = "TD.API_HASH";
    std::string   BotConfig::TITLE = "TD.TITLE";
    std::string   BotConfig::SHORT_NAME = "TD.SHORT_NAME";
    std::string   BotConfig::APPLICATION_VERSION = "TD.APPLICATION_VERSION";
    std::string   BotConfig::LANGUAGE_CODE = "TD.LANGUAGE_CODE";
    std::string   BotConfig::USE_TEST = "TD.USE_TEST";
    std::string   BotConfig::DATABASE_DIRECTORY = "TD.DATABASE_DIRECTORY";
    std::string   BotConfig::DATABASE_ENCRYPTION_KEY = "TD.DATABASE_ENCRYPTION_KEY";
    std::string   BotConfig::REQUEST_RETRY = "TD.REQUEST_RETRY";

    //REDIS
     std::string   BotConfig::HOST = "REDIS.HOST";
     std::string   BotConfig::PORT = "REDIS.PORT";
     std::string   BotConfig::USERNAME = "REDIS.USERNAME";
     std::string   BotConfig::PASSWORD = "REDIS.PASSWORD";
     std::string   BotConfig::KEY_PREFIX = "REDIS.KEY_PREFIX";
     std::string   BotConfig::DB = "REDIS.DB";

} // tg