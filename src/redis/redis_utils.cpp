﻿//
// Created by 19766 on 2024/11/28.
//

#include "include/redis_utils.h"

#include <iostream>
#include <set>
#include <unordered_set>

#include "../include/tg.h"

namespace tg {

   namespace redis {
        RedisUtils::RedisUtils(std::string ip, int port, std::string username, std::string password,std::string key_prefix ,int db):key_prefix(key_prefix) {
            sw::redis::ConnectionOptions option;
            if (username != "") {
                option.user = username;
            }
            if (password != "") {
                option.password = password;
            }
            option.host = ip;
            option.port = port;
            option.db = db;
            option.keep_alive = true;
            option.connect_timeout = std::chrono::milliseconds(0);
            try {
                this->redis =  std::make_unique<sw::redis::Redis>(option);
                log_debug("redis utils: connecting to redis server");
                this->redis->ping();
            }catch (sw::redis::ReplyError &e) {
                log_error(e.what());
                exit(2);
            }catch (sw::redis::TimeoutError &e) {
                log_error(e.what());
                exit(2);
            }catch (std::exception &e) {
                log_error(e.what());
                throw e;
            }
        }

        RedisUtils::~RedisUtils() {
            if (this->redis) {
                this->redis.reset();
            }
        }

        bool RedisUtils::can_solve(std::string chat_id, std::string message_id, std::string op, std::string bot_id) {
            std::string key = chat_id + ":" + message_id + ":" + op;
            return this->can_solve(key, bot_id);
        }

        bool RedisUtils::can_solve(std::string key, std::string bot_id) {
            try {
                if (this->lua_lock_hash && this->redis->script_exists(this->lua_lock_hash->c_str())) {

                }else {
                    std::string hash = this->redis->script_load(this->lua_lock);
                    this->lua_lock_hash = std::make_unique<std::string>(std::move(hash));
                }
                std::string key_prefix = prefix(key);
                const auto keys = {key_prefix.c_str()};
                const auto argv = {bot_id.c_str(),"EX","60"};
                return this->redis->evalsha<bool>(this->lua_lock_hash->c_str(),keys.begin(),keys.end(),argv.begin(),argv.end());

            }catch (sw::redis::ReplyError &e) {
                log_error(e.what());
                exit(2);
            }catch (sw::redis::TimeoutError &e) {
                log_error(e.what());
                exit(2);
            }catch (std::exception &e) {
                log_error(e.what());
                throw e;
            }
        }

        bool RedisUtils::delete_index(std::string index) const {
            auto cmd_str = {"FT.DROPINDEX", index.c_str()};
             try {
                 this->redis->command<sw::redis::OptionalString>(cmd_str.begin(),cmd_str.end());
                 return true;
             }catch (sw::redis::ReplyError &e) {
                 log_error(e.what());
                 exit(2);
             }catch (sw::redis::TimeoutError &e) {
                 log_error(e.what());
                 exit(2);
             }catch (std::exception &e) {
                 log_error(e.what());
                 throw e;
             }
             return false;
        }


        std::unordered_map<std::string, std::unordered_map<std::string,std::string>>  RedisUtils::search(std::string keyword,std::string index,int offset,int limit) const {
            std::string limit_str = std::to_string(limit);
            std::string offset_str = std::to_string(offset);
            auto cmd_str = {"FT.SEARCH", index.c_str(),keyword.c_str(),"LIMIT",offset_str.c_str(),limit_str.c_str()};
            using Result = std::variant<long long,double,std::string,std::unordered_map<std::string, std::string>>;
            std::unordered_map<std::string, std::unordered_map<std::string,std::string>> result;

            try {
                if (auto val = redis->command<std::vector<Result>>(cmd_str.begin(), cmd_str.end()); !val.empty()) {
                    auto count = std::get<0>(val[0]);
                    std::unordered_map<std::string,std::string> total_;
                    total_[TOTAL_KEY] = std::to_string(count);
                    result[TOTAL_KEY] = total_;
                    count = val.size()/2;
                    for (auto i = 0; i < count; i++) {
                        auto key = std::get<std::string>(val[i * 2 +1]);
                        auto value = std::get<std::unordered_map<std::string,std::string>>(val[i *2 +2]);
                        result[key] = std::move(value);
                    }
                }
            }catch (sw::redis::ReplyError &e) {
                log_error(e.what());
                exit(2);
            }catch (sw::redis::TimeoutError &e) {
                log_error(e.what());
                exit(2);
            }catch (std::exception &e) {
                log_error(e.what());
                throw e;
            }

            return result;
        }

        bool RedisUtils::create_index(std::string index, std::string type, std::vector<std::string> prefix, std::string language, std::vector<std::string> schma) const {
                std::vector<const char*> args;
                args.push_back("FT.CREATE");
                args.push_back(index.c_str());
                args.push_back("ON");
                args.push_back(type.c_str());
                args.push_back("prefix");
                std::string size = std::to_string(prefix.size()).c_str();
                args.push_back(size.c_str());
                for (auto i = 0;i < prefix.size();i++) {
                    prefix[i] = this->prefix(prefix[i]);
                }
                for (auto i = 0; i < prefix.size(); i++) {
                    args.push_back(prefix[i].c_str());
                }
                args.push_back("language");
                args.push_back(language.c_str());
                args.push_back("SCHEMA");
                for (auto i = 0; i < schma.size(); i++) {
                    args.push_back(schma[i].c_str());
                }
                try {
                    for (auto arg : args) std::cout << arg << std::endl;
                    this->redis->command<sw::redis::OptionalString>(args.begin(), args.end());
                    return true;
                }catch (sw::redis::ReplyError &e) {
                    if (strcmp(e.what(),"Index already exists")==0) return true;
                    log_error(e.what());
                    exit(2);
                }catch (sw::redis::TimeoutError &e) {
                    log_error(e.what());
                    exit(2);
                }catch (std::exception &e) {
                    log_error(e.what());
                    throw e;
                }
            return false;
        }

        bool RedisUtils::hmset(std::string key, std::unordered_map<std::string, std::string> mp) const {
            try {
                key = this->prefix(key);
                this->redis->hmset(key, mp.begin(), mp.end());
            }catch (sw::redis::ReplyError &e) {
                log_error(e.what());
                exit(2);
            }catch (sw::redis::TimeoutError &e) {
                log_error(e.what());
                exit(2);
            }catch (std::exception &e) {
                log_error(e.what());
                throw e;
            }
            return true;
        }

        bool RedisUtils::hset(std::string key, std::string field, std::string value) const {
            try {
                key = this->prefix(key);
                this->redis->hset(prefix(key), field, value);
            }catch (sw::redis::ReplyError &e) {
                log_error(e.what());
                exit(2);
            }catch (sw::redis::TimeoutError &e) {
                log_error(e.what());
                exit(2);
            }catch (std::exception &e) {
                log_error(e.what());
                throw e;
            }
            return true;
        }

        std::string RedisUtils::hget(std::string key,std::string field) const {
            try {
                key = this->prefix(key);
                return this->redis->hget(prefix(key),field).value_or("");
            }catch (sw::redis::ReplyError &e) {
                log_error(e.what());
                exit(2);
            }catch (sw::redis::TimeoutError &e) {
                log_error(e.what());
                exit(2);
            }catch (std::exception &e) {
                log_error(e.what());
                throw e;
            }
        }

        std::unordered_map<std::string, std::string> RedisUtils::hget_all(std::string key) const {
            std::unordered_map<std::string, std::string> result;
            try {
                key = this->prefix(key);
                this->redis->hgetall(key,std::inserter(result,result.begin()));
            }catch (sw::redis::ReplyError &e) {
                log_error(e.what());
                exit(2);
            }catch (sw::redis::TimeoutError &e) {
                log_error(e.what());
                exit(2);
            }catch (std::exception &e) {
                log_error(e.what());
                throw e;
            }
            return result;
        }



        bool RedisUtils::del(std::string key) const {
            try {
                key = this->prefix(key);
                this->redis->del(key);
            }catch (sw::redis::ReplyError &e) {
                log_error(e.what());
                exit(2);
            }catch (sw::redis::TimeoutError &e) {
                log_error(e.what());
                exit(2);
            }catch (std::exception &e) {
                log_error(e.what());
                throw e;
            }
            return true;
        }




        bool RedisUtils::exists(std::string key) const {
           try {
               key = this->prefix(key);
               return this->redis->exists(key);
           }catch (sw::redis::ReplyError &e) {
               log_error(e.what());
               exit(2);
           }catch (sw::redis::TimeoutError &e) {
               log_error(e.what());
               exit(2);
           }catch (std::exception &e) {
               log_error(e.what());
               throw e;
           }

        }

        std::string RedisUtils::get(std::string key) const {
            try {
                key = this->prefix(key);
                return this->redis->get(key).value_or("");
            }catch (sw::redis::ReplyError &e) {
                log_error(e.what());
                exit(2);
            }catch (sw::redis::TimeoutError &e) {
                log_error(e.what());
                exit(2);
            }catch (std::exception &e) {
                log_error(e.what());
                throw e;
            }
        }

        bool RedisUtils::set(std::string key, std::string value) const {
           try {
               key = this->prefix(key);
               this->redis->set(key, value);
               return true;
           }catch (sw::redis::ReplyError &e) {
               log_error(e.what());
               exit(2);
           }catch (sw::redis::TimeoutError &e) {
               log_error(e.what());
               exit(2);
           }catch (std::exception &e) {
               log_error(e.what());
               throw e;
           }
            return false;
        }

        bool RedisUtils::set(std::string key, std::string value, long long timeout) const {
            try {
                key = this->prefix(key);
                auto a = this->redis->set(key, value, std::chrono::milliseconds(timeout));
                return true;
            }catch (sw::redis::ReplyError &e) {
                log_error(e.what());
                exit(2);
            }catch (sw::redis::TimeoutError &e) {
                log_error(e.what());
                exit(2);
            }catch (std::exception &e) {
                log_error(e.what());
                throw e;
            }
            return false;
        }


       bool RedisUtils::sadd(std::string key, std::string value) const {
           try {
               key = this->prefix(key);
               this->redis->sadd(key, value);
               return true;
           }catch (sw::redis::ReplyError &e) {
               log_error(e.what());
               exit(2);
           }catch (sw::redis::TimeoutError &e) {
               log_error(e.what());
               exit(2);
           }catch (std::exception &e) {
               log_error(e.what());
               throw e;
           }
            return false;
       }

       bool RedisUtils::sismember(std::string key, std::string field) const {
         try {
             key = this->prefix(key);
             return this->redis->sismember(key, field);
         }catch (sw::redis::ReplyError &e) {
             log_error(e.what());
             exit(2);
         }catch (sw::redis::TimeoutError &e) {
             log_error(e.what());
             exit(2);
         }catch (std::exception &e) {
             log_error(e.what());
             throw e;
         }
            return false;

       }

       std::unordered_set<std::string> RedisUtils::smember(std::string key) const {
           try {
               key = this->prefix(key);
               std::unordered_set<std::string> res;
               this->redis->smembers(key,std::inserter(res,res.begin()));
               return res;
           }catch (sw::redis::ReplyError &e) {
               log_error(e.what());
               exit(2);
           }catch (sw::redis::TimeoutError &e) {
               log_error(e.what());
               exit(2);
           }catch (std::exception &e) {
               log_error(e.what());
               throw e;
           }
            return std::unordered_set<std::string>();
       }

        std::unordered_set<std::string> RedisUtils::srandmember(std::string key, int size) const {
            try {
                key = this->prefix(key);
                std::unordered_set<std::string> res;
                this->redis->srandmember(key,size,std::inserter(res,res.begin()));
                return res;
            }catch (sw::redis::ReplyError &e) {
                log_error(e.what());
                exit(2);
            }catch (sw::redis::TimeoutError &e) {
                log_error(e.what());
                exit(2);
            }catch (std::exception &e) {
                log_error(e.what());
                throw e;
            }
            return std::unordered_set<std::string>();
        }


       bool RedisUtils::srem(std::string key, std::string field) const {
           try {
               key = this->prefix(key);
               this->redis->srem(key, field);
               return true;
           }catch (sw::redis::ReplyError &e) {
               log_error(e.what());
               exit(2);
           }catch (sw::redis::TimeoutError &e) {
               log_error(e.what());
               exit(2);
           }catch (std::exception &e) {
               log_error(e.what());
               throw e;
           }
            return false;
       }

       std::vector<std::string> RedisUtils::keys(std::string key) const {
            try {
                key = this->prefix(key);
                std::vector<std::string> res;
                this->redis->keys(key,std::inserter(res,res.begin()));
                return res;
            }catch (sw::redis::ReplyError &e) {
                log_error(e.what());
                exit(2);
            }catch (sw::redis::TimeoutError &e) {
                log_error(e.what());
                exit(2);
            }catch (std::exception &e) {
                log_error(e.what());
                throw e;
            }
            return std::vector<std::string>();
       }






        std::string RedisUtils::prefix(std::string key) const {
            return std::string(this->key_prefix +":"+ key);
        }

        std::string RedisUtils::TOTAL_KEY = "_total_";
   }


} // tg