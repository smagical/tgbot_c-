//
// Created by 19766 on 2024/11/26.
//

#ifndef USER_H
#define USER_H
#include "dispath.h"

namespace tg {
    namespace handler {

        class UserDispathHandler:public DispathHandler{
            public:
                UserDispathHandler(Bot* _bot);
        };

        class UserHandler final :public HandlerTemplate<td::td_api::updateUser> {
            public:
              UserHandler(Bot* _bot):HandlerTemplate(_bot){};
              void hand(td::td_api::Object &&object) override;
        };

        class UserFullInfoHandler final :public HandlerTemplate<td::td_api::updateUserFullInfo> {
            public:
             UserFullInfoHandler(Bot* _bot):HandlerTemplate(_bot){};
             void hand(td::td_api::Object &&object) override;
        };


    } // handler
} // tg

#endif //USER_H
