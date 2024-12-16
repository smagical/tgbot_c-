//
// Created by 19766 on 2024/11/26.
//

#include "include/user.h"

#include "../include/tgbot.h"

namespace tg {
    namespace handler {
        UserDispathHandler::UserDispathHandler(Bot *bot): DispathHandler(bot) {
            add_handler(std::make_unique<UserHandler>(bot));
            add_handler(std::make_unique<UserFullInfoHandler>(bot));
        }


        void UserHandler::hand(td::td_api::Object &&object) {
            td::td_api::updateUser user =
                static_cast<td::td_api::updateUser &&>(object);
            td::td_api::int53 id = user.user_->id_;
            get_bot()->add_user(id,static_cast<td::td_api::object_ptr<td::td_api::user>>(std::move(user.user_)));
        }
        void UserFullInfoHandler::hand(td::td_api::Object &&object) {
            td::td_api::updateUserFullInfo user_full_info =
               static_cast<td::td_api::updateUserFullInfo &&>(object);
            get_bot()->add_user(user_full_info.user_id_,std::move(user_full_info.user_full_info_));
        }

        //todo UserStatusHandler
    } // handler
} // tg