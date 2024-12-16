//
// Created by 19766 on 2024/11/13.
//

#ifndef STDEXT_H
#define STDEXT_H
#include <limits.h>
#include <codecvt>
#include <locale>
#include <map>
#include <mutex>
#include <queue>
#include <shared_mutex>



namespace std{

    static std::vector<std::string> split(const std::string &str, const std::string &delimiter = " ") {
        std::vector<std::string> tokens;
        std::string token = str + delimiter;
        std::size_t pos = 0;
        while ((pos = token.find(delimiter)) != std::string::npos) {
            if (std::string args = token.substr(0, pos); args.size() > 0) {
                tokens.emplace_back(args);
            }
            token = token.substr(pos + delimiter.size(), token.length() - (pos + delimiter.size()));
        }
        return tokens;
    };

    static std::wstring cover_utf16(std::string s) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(s);;
    }

    template <typename T>
     class SafeQueue {
        public:
            SafeQueue(){};
            SafeQueue(long long limit) {
                this->max_size = limit;
                if (this->del_size < limit) {
                    this->del_size = limit-1;
                }
            }
            SafeQueue(long long limit, long long del_size) {
                this->max_size = limit;
                this->del_size = del_size;
                if (this->del_size < limit) {
                    this->del_size = limit-1;
                }
            }
            virtual ~SafeQueue() {
                unique_lock<std::shared_mutex> lock(mutex);
                while (!queue.empty()) {
                    queue.pop();
                }
            };;
            void push(T&& item) {
                std::unique_lock<std::shared_mutex> lock(mutex);
                queue.push(item);
                if (queue.size() > max_size) {
                    int  tmp = 0;
                    while (!queue.empty() && del_size != tmp++) {
                        queue.pop();
                    }
                }
            };
            std::unique_ptr<T> pop() {
                std::unique_lock<std::shared_mutex> lock(mutex);
                if (queue.empty()) {
                    return nullptr;
                }
                std::unique_ptr<T> item = std::make_unique<T>(std::move(queue.front()));
                queue.pop();
                return item;
            };
            auto  size() {
                std::shared_lock<std::shared_mutex> lock(mutex);
                return queue.size();
            };
            bool empty() {
                std::shared_lock<std::shared_mutex> lock(mutex);
                return queue.empty();
            };
            bool clear() {
                std::shared_lock<std::shared_mutex> lock(mutex);
                while (!queue.empty()) {
                    queue.pop();
                }
                return true;
            }

        private:
            std::queue<T> queue;
            std::shared_mutex mutex;
            long long max_size = LLONG_MAX;
            long long del_size = 100;
    };

    template <typename K,typename V>
     class SafeMap{
        public:
            SafeMap(){};
            SafeMap(long long limit) {
                this->max_size = limit;
                if (this->del_size < limit) {
                    this->del_size = limit-1;
                }
            }
            SafeMap(long long limit, long long del_size) {
                this->max_size = limit;
                this->del_size = del_size;
                if (this->del_size < limit) {
                    this->del_size = limit-1;
                }
            }
            virtual ~SafeMap() {
                std::unique_lock<std::shared_mutex> lock(mutex);
                map.clear();
            }

            void insert(K&& key,V&& value) {
                std::unique_lock<std::shared_mutex> lock(mutex);
                auto it = map.find(key);
                if(it == map.end()) {
                    map.emplace(std::make_pair<K,V>(std::forward<K>(key),std::forward<V>(value)));
                    if (map.size() > max_size) {
                        int  tmp = 0;
                        while (!map.empty() && del_size != tmp++) {
                            map.erase(map.begin());
                        }
                    }
                }
                else {
                    it->second = std::move(value);
                }
            }



            V* get(K key) {
                std::shared_lock<std::shared_mutex> lock(mutex);
                auto it = map.find(key);
                if (it == map.end()) {
                    return nullptr;
                }
                return &it->second;
            };
            bool contains(K key) {
                std::shared_lock<std::shared_mutex> lock(mutex);
                return map.find(key) != map.end();
            };
            bool remove(K key) {
                std::unique_lock<std::shared_mutex> lock(mutex);
                return map.erase(key);
            };
            std::unique_ptr<V> getAndRemove(K key) {
                std::unique_lock<std::shared_mutex> lock(mutex);
                auto it = map.find(key);
                if (it == map.end()) {
                    return nullptr;
                }
                std::unique_ptr<V> result = std::make_unique<V>(std::move(it->second));
                map.erase(key);
                return result;
            };
            auto  size() {
                std::shared_lock<std::shared_mutex> lock(mutex);
                return map.size();
            };
            bool empty() {
                std::shared_lock<std::shared_mutex> lock(mutex);
                return map.empty();
            };
            bool clear() {
                std::shared_lock<std::shared_mutex> lock(mutex);
                return map.clear();
            }
            std::vector<V*> get_vaule_vector() {
                std::shared_lock<std::shared_mutex> lock(mutex);
                std::vector<V*> result;
                auto it = map.begin();
                while (it != map.end()) {
                    result.push_back(&it->second);
                    ++it;
                }
                return result;
            }

            private:
                std::map<K,V> map;
                std::shared_mutex mutex;
                long long max_size = LLONG_MAX;
                long  del_size = 100;

        };

    class SafeSequence {
        public:
          int get_next_int() {
              return static_cast<int>(get_next());
          }
          long get_next_long() {
              return static_cast<long>(get_next());
          }
          std::uint64_t get_next() {
              std::lock_guard<std::mutex> lock(mutex);
              this->sequence++;
              return this->sequence;
          }
        private:
          std::mutex mutex;
          std::uint64_t sequence = 0;
    };

    class CountDownLatch {
        public:
        explicit CountDownLatch(int count):count(count) {}
            void await() {
                if (count.load() == 0) return;
                std::unique_lock<std::mutex>lock(mutex);
                condition.wait(lock, [&]() {return count == 0; });
            }
            bool await(long long timeout) {
                if (count.load() == 0) return true;
                std::unique_lock<std::mutex>lock(mutex);
                return condition.wait_for(lock, std::chrono::seconds(timeout),[&]() {return count == 0; });
            }
            void countDown() {
                int old_c = count.load();
                while (old_c > 0) {
                    if (count.compare_exchange_strong(old_c, old_c - 1)) {
                        if (old_c == 1) {//唤醒等待的线程
                            std::unique_lock<std::mutex>lock(mutex);
                            condition.notify_all();
                        }
                        break;
                    }
                    old_c = count.load();
                }

            }
        private:
            std::mutex mutex;
            std::condition_variable condition;
            std::atomic<int> count;

    };
}
#endif //STDEXT_H

