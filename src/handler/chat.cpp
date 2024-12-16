//
// Created by 19766 on 2024/11/26.
//

#include "include/chat.h"

#include "../include/plugin.h"
#include "../include/tgbot.h"

namespace tg {
    namespace handler {

        ChatDispatchHandler::ChatDispatchHandler(Bot* bot):DispathHandler(bot) {
            add_handler(std::make_unique<NewChatHandler>(bot));
            add_handler(std::make_unique<NewMessageHandler>(bot));
            add_handler(std::make_unique<NewCallbackQueryHandler>(bot));
        }

        NewChatHandler::NewChatHandler(Bot *bot):HandlerTemplate<td::td_api::updateNewChat, void>(bot) {

        }
        void NewChatHandler::hand(td::td_api::Object &&object) {
            auto new_chat =
                static_cast<td::td_api::updateNewChat &&>(object);
            td::td_api::int53 id = new_chat.chat_->id_;
            get_bot()->add_chat(id,std::move(new_chat.chat_));
        }



        //message handler
        NewMessageHandler::NewMessageHandler(Bot *bot):HandlerTemplate<td::td_api::updateNewMessage, void>(bot) {

        }
        void NewMessageHandler::hand(td::td_api::Object &&object) {
            auto new_message = static_cast<td::td_api::updateNewMessage &&>(object);
            if ((new_message.message_->edit_date_ !=0 && new_message.message_->edit_date_ < timestamp) || new_message.message_->date_ < this->timestamp) return;
            plugin::hand_new_message(this->get_bot(),std::move(new_message));
        }

        NewCallbackQueryHandler::NewCallbackQueryHandler(Bot* bot):HandlerTemplate<td::td_api::updateNewCallbackQuery, void>(bot) {

        }
        void NewCallbackQueryHandler::hand(td::td_api::Object &&object) {
            auto new_callback_query = static_cast<td::td_api::updateNewCallbackQuery &&>(object);
            plugin::hand_new_callback_query(this->get_bot(),std::move(new_callback_query));
        }



    } // handler
} // tg