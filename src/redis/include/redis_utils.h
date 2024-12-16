//
// Created by 19766 on 2024/11/28.
//

#ifndef REDIS_H
#define REDIS_H
#include <string>
#include <unordered_set>
#include <sw/redis++/redis.h>

namespace tg {

    namespace redis {
        class RedisUtils {
            public:
                RedisUtils(std::string ip, int port = 6379,std::string username = "",std::string password = "",std::string key_prefix = "",int db = 0);
                ~RedisUtils();
                bool can_solve(std::string key,std::string bot_id);
                bool can_solve(std::string chat_id,std::string message_id,std::string op,std::string bot_id);
                std::unordered_map<std::string,std::unordered_map<std::string,std::string>> search(std::string keyword,std::string index,int offset = 0,int limit = 1000000) const;
                bool delete_index(std::string index) const;
                bool create_index(std::string index,std::string type,std::vector<std::string> prefix,std::string language,std::vector<std::string> schma) const;
                bool hmset(std::string key,std::unordered_map<std::string,std::string> mp) const;
                bool hset(std::string key,std::string field,std::string value) const;
                std::string hget(std::string key,std::string field) const;
                std::unordered_map<std::string,std::string> hget_all(std::string key) const;
                bool del(std::string key) const;
                bool exists(std::string key) const;
                bool set(std::string key,std::string value) const;
                bool set(std::string key,std::string value,long long timeout) const;
                std::string get(std::string key) const;
                bool sadd(std::string key,std::string value) const;
                bool sismember(std::string key,std::string field) const;
                std::unordered_set<std::string> smember(std::string key) const;
                std::unordered_set<std::string> srandmember(std::string key,int size) const;
                bool srem(std::string key,std::string field) const;
                static std::string TOTAL_KEY ;
            private:
                std::unique_ptr<sw::redis::Redis> redis;
                std::string key_prefix;
                std::string lua_lock = R"(
                    if redis.call('set',KEYS[1], ARGV[1],ARGV[2], ARGV[3],'NX') then
                        return 1;
                    elseif redis.call('get',KEYS[1]) == ARGV[1] then
                        return redis.call('expire', KEYS[1], ARGV[3]);
                    else
                        return 0;
                    end
            )";;
                std::unique_ptr<std::string> lua_lock_hash;
                std::string prefix(std::string key) const;
            };
    }

} // tg

#endif //REDIS_H
