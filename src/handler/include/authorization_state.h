//
// Created by 19766 on 2024/11/14.
//

#ifndef AUTHORIZATIONSTATE_H
#define AUTHORIZATIONSTATE_H
#include "dispath.h"
#include "handler.h"

namespace tg {
    namespace handler {

        class AuthorizationStateHandler:public DispathHandler{
            public:
                explicit AuthorizationStateHandler(Bot* bot);
                bool support(td::td_api::Object &object) override;
                void hand(td::td_api::Object &&object) override;

        };

        class AuthorizationStateWaitTdlibParametersHandler:public RetryHandlerTemplate<td::td_api::authorizationStateWaitTdlibParameters> {
            public:
              explicit AuthorizationStateWaitTdlibParametersHandler(Bot* bot);
              void doHand(td::td_api::Object &object) override;
        };


        class AuthorizationStateWaitPhoneNumberHandler:public RetryHandlerTemplate<td::td_api::authorizationStateWaitPhoneNumber> {
            public:
                explicit AuthorizationStateWaitPhoneNumberHandler(Bot* bot);
                void doHand(td::td_api::Object &object) override;
        };


        class AuthorizationStateWaitOtherDeviceConfirmationHandler:public HandlerTemplate<td::td_api::authorizationStateWaitOtherDeviceConfirmation> {
            public:
                explicit AuthorizationStateWaitOtherDeviceConfirmationHandler(Bot* bot);
                void hand(td::td_api::Object &&object) override;
        };


        class AuthorizationStateWaitCodeHandler:public RetryHandlerTemplate<td::td_api::authorizationStateWaitCode> {
            public:
                explicit AuthorizationStateWaitCodeHandler(Bot* bot);
                void doHand(td::td_api::Object &object) override;
        };


        class AuthorizationStateReadyHandler:public HandlerTemplate<td::td_api::authorizationStateReady> {
            public:
              explicit AuthorizationStateReadyHandler(Bot* bot);
              void hand(td::td_api::Object &&object) override;
        };

        class AuthorizationStateClosedHandler:public HandlerTemplate<td::td_api::authorizationStateClosed> {
        public:
            explicit AuthorizationStateClosedHandler(Bot* bot);
            void hand(td::td_api::Object &&object) override;
        };
    } // handler
} // tg

#endif //AUTHORIZATIONSTATE_H
