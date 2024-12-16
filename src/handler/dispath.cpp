//
// Created by 19766 on 2024/11/14.
//

#include "include/dispath.h"

#include <algorithm>
#include <iostream>

namespace tg {
    namespace handler {

        DispathHandler::DispathHandler(Bot* bot):Handler(bot) {
            set_no_handler(std::make_unique<tg::handler::NoHandler>(bot));
        }

        DispathHandler::~DispathHandler() {
            std::unique_lock<std::shared_mutex> lock(mutex);
            handlers.clear();
        }

        void DispathHandler::add_handler(std::unique_ptr<tg::handler::Handler> handler) {
            std::unique_lock<std::shared_mutex> lock(mutex);
            handlers.push_back(std::move(handler));
            std::sort(handlers.begin(), handlers.end());
        }

        void DispathHandler::set_no_handler(std::unique_ptr<Handler> no_handler) {
            std::unique_lock<std::shared_mutex> lock(mutex);
            this->noHandler = std::move(no_handler);
        }


        void DispathHandler::hand(td::td_api::Object &&object) {
            std::shared_lock<std::shared_mutex> lock(mutex);
            auto it = handlers.begin();
            bool found = false;
            while (it != handlers.end()) {
                if (it->operator*().support(object)) {
                   it->operator*().hand(std::forward<td::td_api::Object>(object));
                    found = true;
                }
                ++it;
            }
            if (!found) {
                this->noHandler->hand(std::forward<td::td_api::Object>(object));
            }
        }

        bool DispathHandler::support(td::td_api::Object &object) {
            std::shared_lock<std::shared_mutex> lock(mutex);
             auto it = handlers.begin();
            while (it != handlers.end()) {
                if (it->operator*().support(object)) {
                    return true;
                }
                ++it;
            }
            return false;
        }




    } // handler
} // tg