//
// Created by 19766 on 2024/11/30.
//

#ifndef PLUGIN_H
#define PLUGIN_H

#include <functional>
#include <memory>
#include <unordered_map>
#include "td/telegram/td_api.h"

namespace tg {
    namespace handler {
        class Handler;
    }

    class Bot;
    namespace plugin {

        class PluginInterface {
             public:
                virtual ~PluginInterface() = default;
                virtual bool hand_new_message(Bot* bot,td::td_api::updateNewMessage && update_new_message) = 0;
                virtual bool hand_new_call_callback_query(Bot* bot,td::td_api::updateNewCallbackQuery && update_new_callback_query) = 0;
                virtual int get_id() = 0;
                virtual std::string get_name() = 0;
        };

         class Plugin {
             public:
                explicit Plugin(Bot* bot);
                 ~Plugin();
                 bool hand_new_message(td::td_api::updateNewMessage && update_new_message) const;
                 bool hand_new_call_callback_query(td::td_api::updateNewCallbackQuery && update_new_callback_query) const;
                 void register_plugin(std::unique_ptr<PluginInterface> plugin) const;

             private:
              Bot* bot;
              std::unique_ptr<std::unordered_map<int,std::unique_ptr<PluginInterface>>> plugins;
         };


        bool hand_new_message(Bot* bot,td::td_api::updateNewMessage && update_new_message) ;;
        bool hand_new_callback_query(Bot* bot,td::td_api::updateNewCallbackQuery && update_new_callback_query) ;

        bool is_bot(Bot* bot) ;
        bool is_user(Bot* bot);
    }
} // tg

#endif //PLUGIN_H
