//
// Created by 19766 on 2024/11/26.
//

#ifndef CHAT_H
#define CHAT_H
#include "dispath.h"
#include "handler.h"

namespace tg {
    namespace handler {

        class ChatDispatchHandler:public DispathHandler {
            public:
              explicit ChatDispatchHandler(Bot* bot);
        };

        class NewChatHandler:public HandlerTemplate<td::td_api::updateNewChat>{
            public:
                explicit NewChatHandler(Bot* bot);
                void hand(td::td_api::Object &&object) override;

        };

        class NewMessageHandler:public HandlerTemplate<td::td_api::updateNewMessage> {
            public:
                 explicit NewMessageHandler(Bot* bot);
                 void hand(td::td_api::Object &&object) override;
            private:
                long long timestamp = std::time(nullptr);
        };

        class NewCallbackQueryHandler:public HandlerTemplate<td::td_api::updateNewCallbackQuery> {
            public:
              explicit NewCallbackQueryHandler(Bot* bot);
              void hand(td::td_api::Object &&object) override;
            private:
                long long timestamp = std::time(nullptr);
        };
    } // handler
} // tg

#endif //CHAT_H
