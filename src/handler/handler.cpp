//
// Created by 19766 on 2024/11/14.
//


#include <climits>
#include <functional>

#include "include/handler.h"

#include "../include/tg.h"

namespace tg {

    namespace handler {



        void Handler::hand(td::td_api::Object &&object) {
            log_info(td::td_api::to_string(object));
        }

        bool Handler::support(td::td_api::Object &object) {
            return false;
        }
        void Handler::set_order(int order) {
            this->order = order;
        }
        int Handler::get_order() const{
            return this->order;
        }
        Bot *Handler::get_bot() const {
            return this->bot;
        }

        FunctionWrapper::FunctionWrapper(Bot *bot, std::function<void(td::td_api::Object &&)>&& hand):Handler(bot),handFunction(hand) {
            this->supportFunction =
                [](td::td_api::Object &object) {
                    return true;
                };

        }

        FunctionWrapper::FunctionWrapper(Bot *bot, std::function<void(td::td_api::Object &&)> && hand, std::function<bool(td::td_api::Object &)> && support):Handler(bot),handFunction(hand),supportFunction(support) {

        }

        void FunctionWrapper::hand(td::td_api::Object &&object) {
            this->handFunction(std::forward<td::td_api::Object>(object));
        }

        bool FunctionWrapper::support(td::td_api::Object &object) {
            return this->supportFunction(object);
        }


        bool OkAndErrorHandler::support(td::td_api::Object &object) {
            return true;
        }

        void OkAndErrorHandler::hand(td::td_api::Object &&object) {
           switch (object.get_id()) {
             case  td::td_api::ok::ID:{
               ok(std::forward<td::td_api::Object>(object));
               break;
             }case td::td_api::error::ID: {
                 error(std::forward<td::td_api::error::Object>(object));
                 break;
             }default: {
                 other(std::forward<td::td_api::Object>(object));
             }
           }
        }
        void OkAndErrorHandler::ok(td::td_api::Object &&object) {
                //do nothing
        }

        void OkAndErrorHandler::error(td::td_api::Object &&object) {
                //do nothing
        }

        void OkAndErrorHandler::other(td::td_api::Object &&object) {

        }





    }

} // tg