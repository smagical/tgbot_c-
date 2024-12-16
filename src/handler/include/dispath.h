//
// Created by 19766 on 2024/11/14.
//

#ifndef DISPATH_H
#define DISPATH_H
#include "handler.h"
#include "shared_mutex"

namespace tg {
    namespace handler {

        class DispathHandler:public Handler{
            public:
              DispathHandler(Bot*);
              ~DispathHandler();
              void hand(td::td_api::Object &&object) override;
              bool support(td::td_api::Object &object) override;
              void add_handler(std::unique_ptr<Handler> handler);
              void set_no_handler(std::unique_ptr<Handler> no_handler);
            private:
              std::shared_mutex mutex;
              std::vector<std::unique_ptr<tg::handler::Handler>> handlers;
              std::unique_ptr<tg::handler::Handler> noHandler;
        };

    } // handler
} // tg

#endif //DISPATH_H
