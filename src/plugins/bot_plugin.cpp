//
// Created by 19766 on 2024/12/1.
//

#include "include/bot_plugin.h"

#include <ranges>
#include <valarray>

#include "../include/bot_config.h"
#include "../include/bot_util.h"
#include "../include/tg.h"
#include "../include/tgbot.h"
#include "command/include/command.h"

namespace tg {
    namespace plugin {

        bool BotPlugin::hand_new_message(Bot *bot, td::td_api::updateNewMessage &&update_new_message) {
            if (!is_bot(bot)) return false;
            td::td_api::object_ptr<td::td_api::message> message= std::move(update_new_message.message_);
            const td::td_api::int53 chat_id = message->chat_id_;
            td::td_api::int53 messageId = message->id_;


            // log_info(td::td_api::to_string(message));
            bool is_chat = true;
            if (message->content_->get_id() == td::td_api::messageText::ID) {
                td::td_api::MessageContent *content = message->content_.get();
                td::td_api::messageText *message_text = static_cast<td::td_api::messageText *>(content);
                const auto text = message_text->text_->text_;

                //删除自己发的信息
                if (text.ends_with(tg::END)) {
                    return true;
                }


                td::td_api::MessageSender *sender = message->sender_id_.get();
                if (sender->get_id() == td::td_api::messageSenderUser::ID) {
                    auto user_sender =
                            static_cast<td::td_api::messageSenderUser *>(sender);
                    if (bot->is_only_admin() && user_sender->user_id_ != bot->get_config()->get_long_long_value(BotConfig::BOT_ADMIN)) {
                        return true;
                    }
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
                    if (bot->is_only_admin() && chat_sender_id != bot->get_config()->get_long_long_value(BotConfig::BOT_ADMIN)) {
                        return true;
                    }
                }
                //处理命令
                if (text.starts_with("/")) {
                    return this->solve_cmd(bot, command::CommandType::CHAT, chat_id, message_text);
                }

                if (is_chat) {
                    //判断是否识别
                    std::string premison_key = command::PermissionsCommand::PERMISSIONS_PLUGIN_LIST_KEY + ":" +
                                               std::to_string(chat_id);
                    auto premison_size = bot->get_redis_utils()->srandmember(premison_key, 1);
                    if (premison_size.empty()) return false;
                }

                auto serach_text = text;
                return this->solve_message_search(bot,chat_id,messageId,serach_text);
                //todo 搜寻命令
            }

            return true;
        }

        bool BotPlugin::hand_new_call_callback_query(Bot *bot, td::td_api::updateNewCallbackQuery &&update_new_callback_query) {
            if (!is_bot(bot)) return false;
            td::td_api::int53 chat_id = update_new_callback_query.chat_id_;
            td::td_api::int53 message_id = update_new_callback_query.message_id_;
            if (update_new_callback_query.payload_->get_id() == td::td_api::callbackQueryPayloadData::ID) {
                auto call_data = static_cast<td::td_api::callbackQueryPayloadData *>(update_new_callback_query.payload_.get());
                td::td_api::bytes data = call_data->data_;
                auto datas = std::split(data," ");
                try {
                    if (datas.size() != 5) {
                        return false;
                    }
                    int plugin_id = std::stoi(datas[0]);
                    if (plugin_id != get_id()) {
                        return false;
                    }
                    std::string search_text = datas[1];
                    int page = std::stoi(datas[2]);
                    int limit = std::stoi(datas[3]);
                    int current_page = std::stoi(datas[4]);
                    if (page == current_page) {
                        return true;
                    }
                    return this->solve_message_search(bot,chat_id,message_id,search_text,current_page,limit,true);

                }catch(...) {}
                return true;
            }
            return false;
        }

        int BotPlugin::get_id() {
            return 0;
        }

        std::string BotPlugin::get_name() {
            return "BotPlugin";
        }



        void BotPlugin::addCommand(std::unique_ptr<tg::plugin::command::Command> command) {
            this->commands_.push_back(std::move(command));
        }

        void BotPlugin::send_commands(Bot *bot) {
            std::vector<std::pair<std::string, std::string>> chat_commands;
            std::vector<std::pair<std::string, std::string>> user_commands;
            for (const auto &command : this->commands_) {
                auto cmds = command->get_cmds();
                for (const auto &cmd : cmds) {
                    if (cmd.first == command::CHAT) {
                        chat_commands.push_back(std::make_pair(cmd.second.first, cmd.second.second));
                        user_commands.push_back(std::make_pair(cmd.second.first, cmd.second.second));
                    }else {
                        user_commands.push_back(std::make_pair(cmd.second.first, cmd.second.second));
                    }
                }
            }
            util::send_bot_command(bot, chat_commands, td::td_api::make_object<td::td_api::botCommandScopeAllChatAdministrators>());
            util::send_bot_command(bot, user_commands, td::td_api::make_object<td::td_api::botCommandScopeAllPrivateChats>());
        }


        bool BotPlugin::solve_cmd(Bot *bot, command::CommandType type, td::td_api::int53 chat_id,
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
                    const int key_index_utf16_size = std::cover_utf16(key_index).length();
                    int key_utf16_size = std::cover_utf16(key).length();
                    int key_content_utf16_size = std::cover_utf16(key_content).length();
                    td::td_api::textEntity entity(len + key_index_utf16_size, key_utf16_size,td::td_api::make_object<td::td_api::textEntityTypeBotCommand>());
                    entities.push_back(td::td_api::make_object<td::td_api::textEntity>(std::move(entity)));
                    len += key_index_utf16_size + key_utf16_size + 2 + key_content_utf16_size;
                }
                std::string text = ss.str();
                if (text.empty()) {
                    text = "not found";
                }

                return td::td_api::make_object<td::td_api::formattedText>(ss.str(), std::move(entities));
            };
            util::send(bot,chat_id,get_format_text,[](){return nullptr;},[](){return nullptr;},3);
            return true;
        }

        bool BotPlugin::solve_message_search(Bot *bot, td::td_api::int53 chat_id,td::td_api::int53 message_id,
            std::string search_text, int page, int limit,bool is_update) {
                if (page < 0) page = 0;
                if (limit < 1) limit = 1;
                std::stringstream ss;
                ss << "((@content:";
                ss << search_text;
                ss<<") (-@type:{ad}))";
                if (search_text == "*") {
                    ss.str("(-@type:{ad})");
                }
                std::string search = ss.str();
                log_debug(search);
                auto res = bot->get_redis_utils()->search(search,
                                                          bot->get_config()->get_string_value(command::Command::PLUGIN_INDEX),
                                                          page * limit, limit);
                 int total = res.size();
                 if (res.contains(tg::redis::RedisUtils::TOTAL_KEY)) {
                     total = std::stoi(res[tg::redis::RedisUtils::TOTAL_KEY][tg::redis::RedisUtils::TOTAL_KEY]);
                     res.erase(tg::redis::RedisUtils::TOTAL_KEY);
                 }
                int total_pages = total  / limit;
                std::vector<std::pair<std::string, std::string> > values;
                for (auto it = res.begin(); it != res.end(); ++it) {
                    values.push_back(std::make_pair(it->second["content"], it->second["link"]));
                }
                int plugin_id = get_id();
                auto get_format_text_fun = [values]() {
                    int size = values.size();
                    std::stringstream ss;
                    int len = 0;
                    td::td_api::array<td::td_api::object_ptr<td::td_api::textEntity> > entities;
                    for (int i = 0; i < size; i++) {
                        std::string key_index = std::to_string(i) + ". ";
                        std::string key = std::strsub_utf16(values[i].first,30);
                        ss << key_index <<key << std::endl;
                        const int key_index_utf16_size = std::cover_utf16(key_index).length();
                        int key_utf16_size = std::cover_utf16(key).length();
                        td::td_api::textEntity entity(len + key_index_utf16_size, key_utf16_size,
                                                      td::td_api::make_object<td::td_api::textEntityTypeTextUrl>(
                                                          values[i].second));
                        entities.push_back(td::td_api::make_object<td::td_api::textEntity>(std::move(entity)));
                        len += key_index_utf16_size + key_utf16_size + 1;
                    }
                    std::string text = ss.str();
                    if (text.empty()) {
                        text = "not found";
                    }
                    return td::td_api::make_object<td::td_api::formattedText>(std::move(text), std::move(entities));
                };

                auto get_replay_to_fun = [message_id]() {
                    td::td_api::object_ptr<td::td_api::inputMessageReplyToMessage> reply =
                            td::td_api::make_object<td::td_api::inputMessageReplyToMessage>(message_id, nullptr);
                    return std::move(reply);
                };

                auto get_replay_mark_fun = [search_text,total_pages,page,limit,plugin_id]() {
                            td::td_api::array<td::td_api::array<td::td_api::object_ptr<td::td_api::inlineKeyboardButton>>>
                                buttonses;
                            td::td_api::array<td::td_api::object_ptr<td::td_api::inlineKeyboardButton>> buttons_one_line;
                            auto get_button_fun = [](int plugin_id,std::string search_text,int page,int limit,int next_page,std::string title) {
                                std::stringstream ss;
                                ss << plugin_id <<" ";
                                ss << search_text << " ";
                                ss << page << " ";
                                ss << limit << " ";
                                ss << next_page ;
                                std::string text = ss.str();
                                auto home_page_button =  td::td_api::make_object<td::td_api::inlineKeyboardButton> (title,
                                    td::td_api::make_object<td::td_api::inlineKeyboardButtonTypeCallback>(std::move(text)));
                                return std::move(home_page_button);
                            };
                            std::u8string home = u8"首页";
                            buttons_one_line.push_back(std::move(get_button_fun(plugin_id,search_text,page,limit,0,std::string(home.begin(), home.end()))));

                            if (page > 0) {
                                std::u8string title = u8"上一页";
                                buttons_one_line.push_back(std::move(get_button_fun(plugin_id,search_text,page,limit,page-1,std::string(title.begin(), title.end()))));
                            }
                            buttons_one_line.push_back(std::move(get_button_fun(plugin_id,search_text,page,limit,page,std::to_string(page+1) + "/" + std::to_string(total_pages+1))));
                            if (page < total_pages) {
                                std::u8string title = u8"下一页";
                                buttons_one_line.push_back(std::move(get_button_fun(plugin_id,search_text,page,limit,page+1,std::string(title.begin(), title.end()))));
                            }

                            buttonses.push_back(std::move(buttons_one_line));
                            td::td_api::object_ptr<td::td_api::replyMarkupInlineKeyboard> reply =
                                td::td_api::make_object<td::td_api::replyMarkupInlineKeyboard>(std::move(buttonses));
                            return std::move(reply);
                        };

                if (is_update) {
                    tg::util::update_send(
                        bot,
                        chat_id,
                        message_id,
                        std::move(get_format_text_fun),
                        std::move(get_replay_mark_fun),
                        3
                    );
                }else {
                    tg::util::send(
                        bot,
                        chat_id,
                        std::move(get_format_text_fun),
                        std::move(get_replay_to_fun),
                        std::move(get_replay_mark_fun),
                        3
                    );
                }
            return true;
        }



    } // plugin
} // tg