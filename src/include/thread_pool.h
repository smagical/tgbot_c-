//
// Created by 19766 on 2024/11/14.
//

#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <future>
#include <condition_variable>
#include <atomic>
#include <memory.h>

#include "stdext.h"
#include "tg.h"
namespace tg {
    class ThreadPool {
        public:
            ThreadPool(int threads) {
                this->state.store(State::READY);
                this->threads.resize(threads);
                for (int i = 0; i < this->threads.size(); ++i) {
                    this->threads[i] = std::thread(worker( this));
                }
                this->state.store(State::RUNING);
            };
            ThreadPool(const ThreadPool&) = delete;
            ThreadPool(ThreadPool&&) = delete;
            ThreadPool& operator=(const ThreadPool&) = delete;
            ThreadPool& operator=(ThreadPool&&) = delete;
            ~ThreadPool() {
                shutdown();
            };

            template<typename F, typename... Args>
            auto submit(F &&f,Args&&... args) -> std::future<decltype(f(args...))> {
                if (this->state.load() == State::STOP || this->state.load() == State::WAIT_STOP) {
                    throw std::runtime_error("ThreadPool stopped");
                }
                // func为左值
                std::function<decltype(f(args...))()> func = [&f, args...]() {
                    return f(args...);
                };
                // 使用packaged_task获取函数签名，链接func和future，使之能够进行异步操作
                auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
                // 包装可进入线程的线程函数
                std::function<void()> wrapper_func = [task_ptr]() {
                    (*task_ptr)();
                };
                // 将包装的函数加入队列
                queue.push(
                    std::move(wrapper_func)
                );
                // 唤醒一个线程
                condition.notify_one();
                // 返回前面注册的任务指针
                return task_ptr->get_future();
            } ;
            void shutdownNow() {
                if (this->state.load() == State::STOP|| this->state.load() == State::WAIT_STOP) {
                    return;
                }
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    if (this->state.load() == State::STOP|| this->state.load() == State::WAIT_STOP) {
                        return;
                    }
                    state.store(State::STOP);
                }
                condition.notify_all();
                for (int i = 0; i < this->threads.size(); ++i) {
                    if (this->threads[i].joinable()) {
                        this->threads[i].join();
                    }
                }
                queue.clear();
            };
            void shutdown() {
                if (this->state.load() == State::STOP|| this->state.load() == State::WAIT_STOP) {
                    return;
                }
                auto f = submit([]() {
                   std::cout << "ThreadPool shutdown" << std::endl;
               });
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    if (this->state.load() == State::STOP|| this->state.load() == State::WAIT_STOP) {
                        return;
                    }
                    state.store(State::WAIT_STOP);
                }
                f.get();
                // 线程池关闭
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    state.store(State::STOP);
                }
                // 唤醒所有阻塞的线程(在线程池中阻塞的线程)
                condition.notify_all();
                for (int i = 0; i < this->threads.size(); ++i) {
                    if (this->threads[i].joinable()) {
                        this->threads[i].join();
                    }
                }
                queue.clear();
            };
    private:
        std::vector<std::thread> threads;
        std::SafeQueue<std::function<void()>> queue;
        std::mutex mutex;
        std::condition_variable condition;
        enum State {
            READY,RUNING,WAIT_STOP,STOP
        };
        std::atomic<State> state;
        class worker {
        private:
            ThreadPool* thread_pool;
        public:
            worker(ThreadPool* thread_pool) : thread_pool(thread_pool) {};
            void operator()() const {
                while (true){
                    try {
                        if (!thread_pool || thread_pool->state.load() == State::STOP) {
                            return;
                        }
                        std::unique_lock<std::mutex> lock(thread_pool->mutex);
                        thread_pool -> condition.wait(lock,
                            [this]() {
                                return thread_pool -> state.load() ==  State::STOP || !thread_pool->queue.empty();
                            });
                        std::unique_ptr<std::function<void()>> task = thread_pool -> queue.pop();
                        lock.unlock();
                        if (task != nullptr) {
                            task->operator()();
                        }

                    }catch (std::exception &e) {
                        std::cout << e.what() << std::endl;
                    }
                }

            };
        };
    };
}
#endif //THREAD_POOL_H
