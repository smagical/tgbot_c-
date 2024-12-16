//
// Created by 19766 on 2024/11/14.
//

#ifndef HANDLER_H
#define HANDLER_H
#include <functional>
#include <iostream>
#include <ostream>
#include <limits.h>
#include <shared_mutex>
#include "td/telegram/td_api.h"
namespace tg {
    class Bot;
    namespace handler {

        class Handler {
            public:
                virtual ~Handler(){};
                Handler(Bot *bot):bot(bot){};
                virtual void  hand(td::td_api::Object &&object);
                virtual bool  support(td::td_api::Object &object);
                bool operator<(const Handler &other) const {
                    return this->order < other.order;
                }
            private:
                int order = INT_MAX - 1024;
                Bot *bot;

            protected:
                void set_order(int order);
                int get_order() const;
                Bot* get_bot() const;
        };

        class FunctionWrapper : public Handler {
            public:
                FunctionWrapper(Bot *bot,std::function<void(td::td_api::Object &&)> &&);
                FunctionWrapper(Bot* bot,std::function<void(td::td_api::Object &&)> &&,std::function<bool(td::td_api::Object &)> &&);
                void  hand(td::td_api::Object &&object) override;
                bool  support(td::td_api::Object &object) override ;
            private:
                std::function<void(td::td_api::Object &&)>  handFunction;
                std::function<bool(td::td_api::Object &)> supportFunction;
        };

        template<typename T,typename = std::enable_if_t<std::is_base_of_v<td::td_api::Object,T>>>
        class HandlerTemplate : public Handler {
          public:
            HandlerTemplate(Bot *bot):Handler(bot){};
            bool support(td::td_api::Object &object) override{
                return  T::ID == object.get_id();
            };
            void hand(td::td_api::Object &&object) override{};
        };

        class NoHandler final : public Handler {
          public:
            explicit NoHandler():Handler(nullptr) {
                set_order(INT_MAX);
            };
            explicit NoHandler(Bot *bot):Handler(bot){
                set_order(INT_MAX);
            };
            bool support(td::td_api::Object &object) override{
              return true;
            }
        };

        class OkAndErrorHandler  : public Handler {
           public:
             explicit OkAndErrorHandler(Bot *bot):Handler(bot){}
             bool support(td::td_api::Object &object) override;
             void hand(td::td_api::Object &&object) override;
            protected:
              virtual void ok(td::td_api::Object &&object);
              virtual void error(td::td_api::Object &&object);
              virtual void other(td::td_api::Object &&object);
        };

        template<typename T,typename = std::enable_if_t<std::is_base_of_v<td::td_api::Object,T>>>
        class RetryHandlerTemplate : public OkAndErrorHandler {
            public:
                explicit RetryHandlerTemplate(Bot *bot):OkAndErrorHandler(bot){};
                explicit RetryHandlerTemplate(Bot *bot,int retry_max):OkAndErrorHandler(bot),retry_max(retry_max){};
                bool support(td::td_api::Object &object) override {
                    return T::ID == object.get_id();
                };
                void set_retry_max(int retry_max) {
                    this->retry_max = retry_max;
                }
            protected:
                std::function<void(td::td_api::Object &&)>  retry_handler = [this](td::td_api::Object &&object) {
                    this->hand(std::move(object));
                };
            private:
                int retry_max = 3;
                int retry_count = 0;
                td::td_api::object_ptr<td::td_api::Object> obj;
                void error(td::td_api::Object &&object) override {
                    if (retry_count >= retry_max) {
                        return;
                    }

                    retry_count++;
                    this->doHand(this->obj.operator*());
                };
                void other(td::td_api::Object &&object) override {
                    if (T::ID != object.get_id()) {
                        obj.reset();
                        return;
                    };
                    this->retry_count = 0;
                    this->obj = td::td_api::make_object<T>(static_cast<T&&>(object));
                    this->doHand(this->obj.operator*());
                };
                void ok(td::td_api::Object &&object) override {

                    obj.reset();
                };
                virtual void doHand(td::td_api::Object &object) {
                    std::cout<<"Retrying"<<std::endl;
                }
        };


    } // handler
} // tg

#endif //HANDLER_H
