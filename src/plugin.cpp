//
// Created by 19766 on 2024/11/30.
//

#include "include/plugin.h"

#include "include/tgbot.h"

namespace tg::plugin {
    Plugin::Plugin(Bot* bot):bot(bot) {
        this->plugins = std::make_unique<std::unordered_map<int,std::unique_ptr<PluginInterface>>>();
    }

    Plugin::~Plugin() {
        this->plugins->clear();
        this->plugins.reset();
    }

    bool Plugin::hand_new_message(td::td_api::updateNewMessage &&update_new_message) const{
        if (!this->bot -> can_slove_message(update_new_message.message_->chat_id_,update_new_message.message_->chat_id_)) {
            return true;
        }
        auto it = this->plugins->begin();
        while (it != this->plugins->end()) {
            try {
                it->second->hand_new_message(this->bot,std::forward<td::td_api::updateNewMessage>(update_new_message));
            }catch(std::exception& e) {
                log_error(e.what());
            }
            ++it;
        }
        return true;
    }

    bool Plugin::hand_new_call_callback_query(td::td_api::updateNewCallbackQuery &&update_new_callback_query) const{
        if (!this->bot -> can_slove_message(update_new_callback_query.chat_id_,update_new_callback_query.id_)) {
            return true;
        }
        auto it = this->plugins->begin();
        while (it != this->plugins->end()) {
            try {
                it->second->hand_new_call_callback_query(this->bot,std::forward<td::td_api::updateNewCallbackQuery>(update_new_callback_query));
            }catch(std::exception& e) {
                log_error(e.what());
            }
            ++it;
        }
        return true;
    }

    void Plugin::register_plugin(std::unique_ptr<PluginInterface> plugin) const {
        auto id = plugin->get_id();
        if (plugins->contains(id)) {
            plugins->at(id) = std::move(plugin);
        }else {
            plugins->emplace(id, std::move(plugin));
        }

    }

    bool hand_new_message(Bot* bot,td::td_api::updateNewMessage && update_new_message){
        if (bot) {
            bot -> dispatchMessage(std::forward<td::td_api::updateNewMessage>(update_new_message));
        }
        return true;
    }

    bool hand_new_callback_query(Bot* bot,td::td_api::updateNewCallbackQuery && update_new_callback_query) {
        if (bot) {
            bot -> dispatchMessage(std::forward<td::td_api::updateNewCallbackQuery>(update_new_callback_query));
        }
        return true;
    };

    bool is_bot(Bot* bot) {
        return *bot->get_login_type() == LoginType::TOKEN;
    };

    bool is_user(Bot* bot) {
        return !is_bot(bot);
    }
}
