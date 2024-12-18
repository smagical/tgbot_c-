//
// Created by 19766 on 2024/12/1.
//

#include "include/user_plugin.h"

#include "../include/bot_util.h"
#include "../include/tg.h"
#include "../include/tgbot.h"

namespace tg {
    namespace plugin {
        bool UserPlugin::hand_new_message(Bot *bot, td::td_api::updateNewMessage &&update_new_message) {
            if (!is_user(bot)) return false;
            td::td_api::object_ptr<td::td_api::message> message = std::move(update_new_message.message_);
            const td::td_api::int53 chat_id = message->chat_id_;
            td::td_api::int53 messageId = message->id_;

            log_info(td::td_api::to_string(message));
            if (message->content_->get_id() == td::td_api::messageText::ID) {
                td::td_api::MessageContent *content = message->content_.get();
                td::td_api::messageText *message_text = static_cast<td::td_api::messageText *>(content);
                const auto text = message_text->text_->text_;

                //删除自己发的信息
                if (text.ends_with(tg::END)) {
                    return true;
                }


                td::td_api::MessageSender *sender = message->sender_id_.get();
                bool is_chat = true;
                if (sender->get_id() == td::td_api::messageSenderUser::ID) {
                    auto user_sender =
                            static_cast<td::td_api::messageSenderUser *>(sender);
                    if (bot->get_me() && bot->get_me()->get()->id_ == user_sender->user_id_) {
                        //todo 自己暂时不开启
                        //return true;
                    }

                    //cmd只对个人
                    td::td_api::object_ptr<td::td_api::chat> *chat = bot->get_chat(chat_id);
                    if (chat && chat->get()->type_->get_id() == td::td_api::chatTypePrivate::ID) {
                        if (text.starts_with("/")) {
                            if (chat_id == bot->get_config()->get_long_long_value(BotConfig::BOT_ADMIN)) {
                                return this->solve_cmd(bot, command::CommandType::MANGER, chat_id, message_text);
                            }
                            return this->solve_cmd(bot, command::CommandType::USER, chat_id, message_text);
                        }
                        is_chat = false;
                    }

                    td::td_api::object_ptr<td::td_api::user> *user = bot->get_user(user_sender->user_id_);
                    //机器人信息不管
                    if (user && user->get()->type_->get_id() == td::td_api::userTypeBot::ID) {
                        return true;
                    }
                } else {
                    auto chat_sender =
                            static_cast<td::td_api::messageSenderChat *>(sender);
                    auto chat_sender_id = chat_sender->chat_id_;
                }
                //处理命令
                if (text.starts_with("/")) {
                    return this->solve_cmd(bot, command::CommandType::CHAT, chat_id, message_text);
                }

                if (is_chat) {
                    //判断是否识别
                    std::string premison_key = bot->get_config()->get_string_value(command::PermissionsCommand::PERMISSIONS_PLUGIN_LIST_KEY) + ":" +
                                               std::to_string(chat_id);
                    auto premison_size = bot->get_redis_utils()->srandmember(premison_key, 1);
                    if (premison_size.empty()) return false;
                }

                auto serach_text = text;
                auto res = bot->get_redis_utils()->search(serach_text,
                                                          bot->get_config()->get_string_value(command::Command::PLUGIN_INDEX),
                                                          0, 50);
                if (res.find(tg::redis::RedisUtils::TOTAL_KEY) != res.end()) {
                    res.erase(tg::redis::RedisUtils::TOTAL_KEY);
                }
                std::vector<std::pair<std::string, std::string> > values;
                for (auto it = res.begin(); it != res.end(); ++it) {
                    values.push_back(std::make_pair(it->second["content"], it->second["link"]));
                }

                util::send(
                    bot,
                    chat_id,
                    [values]() {
                        int size = values.size();
                        std::stringstream ss;
                        int len = 0;
                        td::td_api::array<td::td_api::object_ptr<td::td_api::textEntity> > entities;

                        for (int i = 0; i < size; i++) {
                            int key_size = values[i].first.length();
                            std::string key_index = std::to_string(i) + ". ";
                            std::string key = values[i].first.substr(0, std::min(key_size, 30));
                            ss << key_index <<key << std::endl;
                            const int key_index_utf16_size = std::cover_utf16(key_index).length();
                            int key_utf16_size = std::cover_utf16(key).length();
                            td::td_api::textEntity entity(len + key_index_utf16_size, key_utf16_size,
                                                            td::td_api::make_object<td::td_api::textEntityTypeTextUrl>(
                                                                values[i].second));
                            entities.push_back(td::td_api::make_object<td::td_api::textEntity>(std::move(entity)));
                            len += key_index_utf16_size + key_utf16_size + 1;
                        }
                        if (size == 0) {
                            ss << "not search any";
                        }
                        ss << tg::END;
                        return td::td_api::make_object<td::td_api::formattedText>(ss.str(), std::move(entities));
                    },
                    [messageId]() {
                        td::td_api::object_ptr<td::td_api::inputMessageReplyToMessage> reply =
                                td::td_api::make_object<td::td_api::inputMessageReplyToMessage>(messageId, nullptr);
                        return std::move(reply);
                    },
                    []() { return nullptr; },
                    3
                );
                //todo 搜寻命令
            }

            return true;
        }

        bool UserPlugin::hand_new_call_callback_query(
            Bot *bot, td::td_api::updateNewCallbackQuery &&update_new_callback_query) {
            if (!is_user(bot)) return false;
            log_debug("UserPlugin::hand_new_call_callback_query user login not support");
            return true;
        }

        void UserPlugin::addCommand(std::unique_ptr<tg::plugin::command::Command> command) {
            this->commands_.push_back(std::move(command));
        }


        int UserPlugin::get_id() {
            return 1;
        }

        std::string UserPlugin::get_name() {
            return "UserPlugin";
        }


        bool UserPlugin::solve_cmd(Bot *bot, command::CommandType type, td::td_api::int53 chat_id,
                                   td::td_api::messageText *message_text) const {
            const auto cmd = message_text->text_->text_.substr(1, message_text->text_->text_.length());
            for (const auto &command: this->commands_) {
                if (command->supprot(cmd)) {
                    if (command->handle(bot, type, chat_id, cmd)) return true;
                }
            }
            if (type == command::CHAT) return true;
            std::vector<std::pair<std::string, std::string> > values;
            for (const auto &command: this->commands_) {
                auto cmd = command->get_cmds();
                for (const auto &cmd: cmd) {
                    if (cmd.first <= type) {
                        values.push_back(cmd.second);
                    }
                }
            }
            auto get_format_text = [values]() {
                td::td_api::array<td::td_api::object_ptr<td::td_api::textEntity> > entities;
                int size = values.size();
                std::stringstream ss;
                int len = 0;
                for (int i = 0; i < size; i++) {
                    std::string key_index = std::to_string(i) + ". ";
                    std::string key = values[i].first;
                    std::string key_content = values[i].second;
                    if (key_content.starts_with(key)) {
                        key_content = key_content.substr(key.length());
                    }
                    ss << key_index <<key<<" "<<key_content << std::endl;
                    int key_index_utf16_size = std::cover_utf16(key_index).length();
                    int key_utf16_size = std::cover_utf16(key).length();
                    int key_content_utf16_size = std::cover_utf16(key_content).length();
                    td::td_api::textEntity entity(len + key_index_utf16_size, key_utf16_size,td::td_api::make_object<td::td_api::textEntityTypeCode>());
                    entities.push_back(td::td_api::make_object<td::td_api::textEntity>(std::move(entity)));
                    len += key_index_utf16_size + key_utf16_size + 2 + key_content_utf16_size;
                }
                std::string text = ss.str();
                return td::td_api::make_object<td::td_api::formattedText>(ss.str(), std::move(entities));
            };
            util::send(bot,chat_id,get_format_text,[](){return nullptr;},[](){return nullptr;},3);
            return true;
        }
    } // plugin
} // tg
