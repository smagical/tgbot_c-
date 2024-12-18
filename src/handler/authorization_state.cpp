//
// Created by 19766 on 2024/11/14.
//

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#elif  __linux__
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <string>
#include "include/authorization_state.h"

#include "../include/tg.h"
#include "../include/tgbot.h"
#include "../orc/include/qr_code_generator.h"
namespace tg {
    namespace handler {

        AuthorizationStateHandler::AuthorizationStateHandler(Bot *bot):DispathHandler(bot) {
            add_handler(std::make_unique<AuthorizationStateWaitTdlibParametersHandler>(bot));
            add_handler(std::make_unique<AuthorizationStateWaitPhoneNumberHandler>(bot));
            add_handler(std::make_unique<AuthorizationStateWaitOtherDeviceConfirmationHandler>(bot));
            add_handler(std::make_unique<AuthorizationStateReadyHandler>(bot));
            add_handler(std::make_unique<AuthorizationStateWaitCodeHandler>(bot));
            add_handler(std::make_unique<AuthorizationStateReadyHandler>(bot));
        }

        void AuthorizationStateHandler::hand(td::td_api::Object &&object) {
            td::td_api::updateAuthorizationState state = static_cast<td::td_api::updateAuthorizationState &&>(object);
            DispathHandler::hand(std::move(state.authorization_state_.operator*()));
        }


        bool AuthorizationStateHandler::support(td::td_api::Object &object) {
            return td::td_api::updateAuthorizationState::ID == object.get_id();
        }


        AuthorizationStateWaitTdlibParametersHandler::AuthorizationStateWaitTdlibParametersHandler(Bot *bot):
            RetryHandlerTemplate<td::td_api::authorizationStateWaitTdlibParameters, void>(bot) {
        }
        void AuthorizationStateWaitTdlibParametersHandler::doHand(td::td_api::Object &object) {
            log_debug("AuthorizationStateWaitTdlibParametersHandler::hand");
            Bot *bot = get_bot();
            BotConfig *config = bot->get_config();
            td::td_api::authorizationStateWaitTdlibParameters state =
                static_cast<td::td_api::authorizationStateWaitTdlibParameters &&>(object);
            auto request = td::td_api::make_object<td::td_api::setTdlibParameters>();
            request->database_directory_ = config->get_string_value(BotConfig::TD_DATABASE_DIRECTORY);
            request->use_message_database_ = true;
            request->use_chat_info_database_ = true;
            request->use_file_database_ = true;
            request->use_secret_chats_ = false;
            request->api_id_ = config->get_int_value(BotConfig::TD_API_ID);   ;
            request->api_hash_ = config -> get_string_value(BotConfig::TD_API_HASH);
            request->system_language_code_ = config->get_string_value(BotConfig::TD_LANGUAGE_CODE);
            request->device_model_ = "Desktop";
            request->use_test_dc_ = config->get_bool_value(BotConfig::TD_USE_TEST);
            request->application_version_ = config->get_string_value(BotConfig::TD_APPLICATION_VERSION);
            request->database_encryption_key_ = config -> get_string_value(BotConfig::TD_DATABASE_ENCRYPTION_KEY);
#ifdef _WIN32
                if (_access(request->database_directory_.c_str(),0) == -1) {
                    _mkdir(request->database_directory_.c_str());
                }

#elif  __linux__
            if (access(request->database_directory_.c_str(),0) == -1) {
                mkdir(request->database_directory_.c_str(),0666);
            }
#endif

            bot->send(std::move(request),this->retry_handler);
        }


        AuthorizationStateWaitPhoneNumberHandler::AuthorizationStateWaitPhoneNumberHandler(Bot *bot):RetryHandlerTemplate
            <td::td_api::authorizationStateWaitPhoneNumber, void>(bot) {
        }
        void AuthorizationStateWaitPhoneNumberHandler::doHand(td::td_api::Object &object) {
             log_debug("AuthorizationStateWaitPhoneNumberHandler::hand");
             Bot *bot = get_bot();
             LoginType type = *bot->get_login_type();
             if (type == tg::LoginType::OCR) {
                  bot->send(
                      td::td_api::make_object<td::td_api::requestQrCodeAuthentication>(),
                      nullptr
                      );
             }else if (type == tg::LoginType::PHONE) {

                  td::td_api::object_ptr<td::td_api::phoneNumberAuthenticationSettings> settings=
                      td::td_api::make_object<td::td_api::phoneNumberAuthenticationSettings>();

                  settings->allow_flash_call_ = false;
                  settings->has_unknown_phone_number_ = false;
                  settings->is_current_phone_number_ = true;
                  settings->allow_missed_call_ = false;

                 bot->send(
                     td::td_api::make_object< td::td_api::setAuthenticationPhoneNumber>(
                                bot->get_token(),std::move(settings)
                                ),
                                this->retry_handler

                     );
             }else if (type == tg::LoginType::TOKEN) {
                 bot->send(
                    td::td_api::make_object<td::td_api::checkAuthenticationBotToken>(bot->get_token()),
                    this->retry_handler
                 );
             }

        }

        AuthorizationStateWaitOtherDeviceConfirmationHandler::AuthorizationStateWaitOtherDeviceConfirmationHandler(Bot *bot):
            HandlerTemplate<td::td_api::authorizationStateWaitOtherDeviceConfirmation, void>(bot){

        }
        void AuthorizationStateWaitOtherDeviceConfirmationHandler::hand(td::td_api::Object &&object) {
            log_info("AuthorizationStateWaitOtherDeviceConfirmationHandler::hand");
            td::td_api::authorizationStateWaitOtherDeviceConfirmation state =
                static_cast<td::td_api::authorizationStateWaitOtherDeviceConfirmation &&>(object);
            tg::orc::QRCodeGenerator::printQRCode(state.link_);
        }


        AuthorizationStateWaitCodeHandler::AuthorizationStateWaitCodeHandler(Bot *bot):
            RetryHandlerTemplate<td::td_api::authorizationStateWaitCode, void>(bot) {

        }
        void AuthorizationStateWaitCodeHandler::doHand(td::td_api::Object &object) {
            log_debug("AuthorizationStateWaitCodeHandler::hand");
            td::td_api::authorizationStateWaitCode state =
                static_cast<td::td_api::authorizationStateWaitCode &&>(object);

            Bot *bot = get_bot();
            std::string code = tg::input("Enter authentication password: ");
            bot->send(
                td::td_api::make_object<td::td_api::checkAuthenticationCode>(code),
                this->retry_handler
            );

        }

        AuthorizationStateReadyHandler::AuthorizationStateReadyHandler(Bot *bot):
            HandlerTemplate<td::td_api::authorizationStateReady, void>(bot) {

        }
        void AuthorizationStateReadyHandler::hand(td::td_api::Object &&object) {
            log_debug("AuthorizationStateReadyHandler::hand");
            get_bot()->load();
        }

        AuthorizationStateClosedHandler::AuthorizationStateClosedHandler(Bot *bot):
           HandlerTemplate<td::td_api::authorizationStateClosed, void>(bot) {

        }
        void AuthorizationStateClosedHandler::hand(td::td_api::Object &&object) {
            log_debug("AuthorizationStateReadyHandler::hand");
        }



    } // handler
} // tg

