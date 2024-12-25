//
// Created by 19766 on 2024/12/2.
//
#include "include/bot_util.h"
#define PULL_MAX 50
#define RETRY_COUNT 3
namespace tg::util {

    //todo 有bug 多线程可能导致取出的指针为空
     td::td_api::object_ptr<td::td_api::user> get_me(Bot *bot) {
        if (!bot) return nullptr;
        td::td_api::object_ptr<td::td_api::user> user = nullptr;
        td::td_api::object_ptr<td::td_api::getMe> me = td::td_api::make_object<td::td_api::getMe>();
        std::shared_ptr<std::CountDownLatch> latch = std::make_shared<std::CountDownLatch>(1);
        bot->send(std::move(me),[latch, &user](td::td_api::Object && object) {
            if (object.get_id() == td::td_api::user::ID) {
                td::td_api::user&& tmp_user =
                    static_cast<td::td_api::user &&>(object);
                user = td::td_api::make_object<td::td_api::user>(std::move(tmp_user));
            }
            latch->countDown();
        });
        latch->await(5);
        return user;
    }

     void load_all_chats(Bot *bot, const ChatType type) {
        if (bot == nullptr) return;
        std::shared_ptr<std::CountDownLatch> latch;
        if (type == ALL) {
            latch = std::make_shared<std::CountDownLatch>(3);
        }else {
            latch = std::make_shared<std::CountDownLatch>(1);
        }

        if (type == ALL || type == MAIN) {
            td::td_api::loadChats load_main_chats(td::td_api::make_object<td::td_api::chatListMain>(),200);
            bot->send(td::td_api::make_object<td::td_api::loadChats >(std::move(load_main_chats)),
            [latch, type,bot](td::td_api::Object && object) {
                        switch (object.get_id()) {
                            case td::td_api::error::ID: {
                                break;
                            }
                            case td::td_api::ok::ID: {
                                load_all_chats(bot,MAIN);
                                break;
                            }
                        }
                // int a = latch.use_count();
                        latch->countDown();
                });
        }

        if (type == ALL || type == ARCHIVE) {
             td::td_api::loadChats load_archive_chats(td::td_api::make_object<td::td_api::chatListArchive>(),100);
              bot->send(td::td_api::make_object<td::td_api::loadChats >(std::move(load_archive_chats)),
            [latch,type, bot](td::td_api::Object && object) {
                        switch (object.get_id()) {
                            case td::td_api::error::ID: {
                                break;
                            }
                            case td::td_api::ok::ID: {
                                load_all_chats(bot,ARCHIVE);
                                break;
                            }
                        }
                // int a = latch.use_count();
                        latch->countDown();
                });
        }

        if (type == ALL || type == FOLDER) {
            td::td_api::loadChats load_folder_chats(td::td_api::make_object<td::td_api::chatListFolder>(),100);
            bot->send(td::td_api::make_object<td::td_api::loadChats >(std::move(load_folder_chats)),
            [latch,type, bot](td::td_api::Object && object) {
                        switch (object.get_id()) {
                            case td::td_api::error::ID: {
                                break;
                            }
                            case td::td_api::ok::ID: {
                                load_all_chats(bot,FOLDER);
                                break;
                            }
                        }
                        // int a = latch.use_count();
                        latch->countDown();
                });
        }

        latch->await(5);
         // std::cout << "load_all_chats endl" << std::endl;

    }

     td::td_api::object_ptr<td::td_api::user>* load_user(Bot *bot, const td::td_api::int53 user_id) {
        if (!bot) return nullptr;
        td::td_api::object_ptr<td::td_api::user>* result = nullptr;
        result = bot->get_user(user_id);
        if (!result) return nullptr;
        std::shared_ptr<std::CountDownLatch > latch = std::make_shared<std::CountDownLatch>(1);
        td::td_api::getUser get_user(user_id);
        bot->send(td::make_tl_object<td::td_api::getUser >(std::move(get_user)),
            [latch,&bot, &result](td::td_api::Object && object) {
                if (object.get_id() == td::td_api::user::ID) {
                    td::td_api::user tmp_user =static_cast<td::td_api::user &&>(object);
                    const td::td_api::int53 user_id = tmp_user.id_;
                    bot->add_user(user_id,td::td_api::make_object<td::td_api::user>(std::move(tmp_user)));
                    result = bot->get_user(user_id);
                }
                latch->countDown();
            }
        );
        latch->await(5);
        return result;
    };



     td::td_api::object_ptr<td::td_api::chat>* load_chat(Bot *bot, const td::td_api::int53 chat_id) {
        if (!bot) return nullptr;
        td::td_api::object_ptr<td::td_api::chat>* result = nullptr;
        std::shared_ptr<std::CountDownLatch> latch = std::make_shared<std::CountDownLatch>(1);
        td::td_api::getChat get_chat(chat_id);
        bot->send(td::make_tl_object<td::td_api::getChat >(std::move(get_chat)),
            [latch,&bot, &result](td::td_api::Object && object) {
                if (object.get_id() == td::td_api::chat::ID) {
                    td::td_api::chat tmp_chat =static_cast<td::td_api::chat &&>(object);
                    const td::td_api::int53 chat_id = tmp_chat.id_;
                    bot->add_chat(chat_id,td::td_api::make_object<td::td_api::chat>(std::move(tmp_chat)));
                    result = bot->get_chat(chat_id);
                }
                latch->countDown();
            }
        );
        latch->await(5);
        return result;
    };


    //todo 待测试
     std::vector<td::td_api::object_ptr<td::td_api::message>> load_history(Bot *bot, const td::td_api::int53 chat_id,td::td_api::int53 last_message_id,int limit){
        std::vector<td::td_api::object_ptr<td::td_api::message>> result;
        std::map<td::td_api::int53,td::td_api::object_ptr<td::td_api::message>> hash;
        if (!bot) return result;
        bool return_flag = false;
        int last_size = hash.size();
        int retry = RETRY_COUNT;
        while (hash.size() < limit && !return_flag) {

            const int last_limit = limit - hash.size() > 0 ? limit - hash.size() : 0;
            td::td_api::getChatHistory get_chat_history(chat_id,last_message_id,0,std::min(PULL_MAX,last_limit),false);
            std::shared_ptr<std::CountDownLatch> latch = std::make_shared<std::CountDownLatch>(1);
            bot ->send(
                td::td_api::make_object<td::td_api::getChatHistory >(std::move(get_chat_history)),
                    [latch,&bot, &return_flag, &hash,&last_message_id](td::td_api::Object && object) {
                             if (object.get_id() == td::td_api::error::ID) {
                                    return_flag = true;
                             }else if (object.get_id() == td::td_api::messages::ID) {
                                  auto messages =static_cast<td::td_api::messages &&>(object);
                                 auto messages_array = &messages.messages_;
                                 for (auto it = messages_array->begin(); it != messages_array->end(); ++it) {
                                     const td::td_api::int53 id = it->get()->id_;
                                     hash[id] = std::move(*it);
                                 }

                                 if (!hash.empty())
                                    last_message_id = hash.begin()->first;
                             }
                             latch->countDown();
                        }
                );
            latch->await(10);

            if (last_size != hash.size()) {
                last_size = hash.size();
                retry = RETRY_COUNT;
            }else {
                retry--;
                if (retry == 0) {
                    return_flag = true;
                }
            }
        }

        for (auto it = hash.rbegin(); it != hash.rend(); ++it) {
            result.push_back(std::move(it->second));
        }
        return result;
    };

     std::string load_message_link(Bot *bot, const td::td_api::int53 chat_id,const td::td_api::int53 message_id) {
        if (!bot) return "";
        std::string result = "";
        std::shared_ptr<std::CountDownLatch> latch = std::make_shared<std::CountDownLatch>(1);
        td::td_api::getMessageLink get_message_link(chat_id,message_id,0,true,false);
        bot->send(
            td::td_api::make_object<td::td_api::getMessageLink >(std::move(get_message_link)),
            [latch,&result](td::td_api::Object && object) {
                        if (object.get_id() == td::td_api::messageLink::ID) {
                            const auto link = static_cast<td::td_api::messageLink &&>(object);
                            result = link.link_;
                        }
                        latch->countDown();

            }
        );
        latch->await(5);
        return result;
    };

    void send(Bot* bot, const td::td_api::int53 chat_id, std::string message,int reply_message_id,int retry) {
        if (!bot) return;
        if (!message.ends_with(tg::END)) {
            message+=tg::END;
        }
        td::td_api::formattedText text(
                message ,td::td_api::array<td::td_api::object_ptr<td::td_api::textEntity>>()
        );
           td::td_api::linkPreviewOptions options(true,"",false,false,false);
            td::td_api::inputMessageText input_message(
            td::td_api::make_object<td::td_api::formattedText>(std::move(text)),
            td::td_api::make_object<td::td_api::linkPreviewOptions>(std::move(options)),true
            );

        td::td_api::object_ptr<td::td_api::inputMessageReplyToMessage> reply_to = nullptr;
        if (reply_message_id != -1) {
            reply_to = td::td_api::make_object<td::td_api::inputMessageReplyToMessage>(reply_message_id,nullptr);
        }
        td::td_api::sendMessage send_message(chat_id,0,std::move(reply_to),nullptr,nullptr,
            td::td_api::make_object<td::td_api::inputMessageText>(std::move(input_message)));
        bot->send(td::td_api::make_object<td::td_api::sendMessage>(std::move(send_message)),
            [bot,chat_id,message,retry, reply_message_id](td::td_api::Object && object) {
                   if (object.get_id() == td::td_api::error::ID) {
                       if (retry <= 1) {
                           return;
                       }
                       send(bot,chat_id,message,reply_message_id,retry-1);
                   }
        });
    };

    void send(Bot *bot, const td::td_api::int53 chat_id,td::td_api::object_ptr<td::td_api::formattedText> formatted_text,
        td::td_api::object_ptr<td::td_api::InputMessageReplyTo> reply_to,td::td_api::object_ptr<td::td_api::ReplyMarkup> reply_markup) {
        if (!bot) return;
        if (!formatted_text->text_.ends_with(tg::END)) {
            formatted_text->text_ += tg::END;
        }
          td::td_api::linkPreviewOptions options(true,"",false,false,false);
         td::td_api::inputMessageText input_message(
            std::move(formatted_text),
            td::td_api::make_object<td::td_api::linkPreviewOptions>(std::move(options)),true
            );
        td::td_api::sendMessage send_message(chat_id,0,std::move(reply_to),nullptr,std::move(reply_markup),
        td::td_api::make_object<td::td_api::inputMessageText>(std::move(input_message)));
        bot->send( td::td_api::make_object<td::td_api::sendMessage>(std::move(send_message)),
                [](td::td_api::Object && object) {
                        if (object.get_id() == td::td_api::error::ID) {

                        }
                }
            );

    };

    void send(Bot *bot, const td::td_api::int53 chat_id,
        std::function<td::td_api::object_ptr<td::td_api::formattedText>()> formatted_text,
        std::function<td::td_api::object_ptr<td::td_api::InputMessageReplyTo>()> reply_to,
        std::function<td::td_api::object_ptr<td::td_api::ReplyMarkup>()> reply_markup,
        int retry) {
        if (!bot) return;
        td::td_api::object_ptr<td::td_api::formattedText> formatted_text_object = formatted_text();
        if (!formatted_text_object->text_.ends_with(tg::END)) {
            formatted_text_object->text_ += tg::END;
        }

        auto reply_markup_object = std::move(reply_markup());
        td::td_api::linkPreviewOptions options(true,"",false,false,false);
        td::td_api::inputMessageText input_message(std::move(formatted_text_object),td::td_api::make_object<td::td_api::linkPreviewOptions>(std::move(options)),true);
        td::td_api::sendMessage send_message(chat_id,0,std::move(reply_to()),nullptr,std::move(reply_markup_object),
        td::td_api::make_object<td::td_api::inputMessageText>(std::move(input_message)));
        bot->send( td::td_api::make_object<td::td_api::sendMessage>(std::move(send_message)),
                [bot,chat_id,formatted_text,reply_to,reply_markup,retry](td::td_api::Object && object) {
                        if (object.get_id() == td::td_api::error::ID) {
                            if (retry <= 1) {
                                return;
                            }
                            send(bot,chat_id,formatted_text,reply_to,reply_markup,retry-1);
                        }
                }
            );

    };

    void update_send(Bot* bot, const td::td_api::int53 chat_id,td::td_api::int53 message_id, std::string message,int retry) {
        if (!bot) return;
        if (message.ends_with(tg::END)) {
            message+=tg::END;
        }
        td::td_api::formattedText text(
                message ,td::td_api::array<td::td_api::object_ptr<td::td_api::textEntity>>()
        );
         td::td_api::linkPreviewOptions options(true,"",false,false,false);
         td::td_api::inputMessageText input_message(
            td::td_api::make_object<td::td_api::formattedText>(std::move(text)),
            td::td_api::make_object<td::td_api::linkPreviewOptions>(std::move(options)),true
            );


        td::td_api::editMessageText edit_message(chat_id,message_id,nullptr,
            td::td_api::make_object<td::td_api::inputMessageText>(std::move(input_message)));
        bot->send(td::td_api::make_object<td::td_api::editMessageText>(std::move(edit_message)),
            [bot,chat_id,message_id,message,retry](td::td_api::Object && object) {
                   if (object.get_id() == td::td_api::error::ID) {
                       if (retry <= 1) {
                           return;
                       }
                       update_send(bot,chat_id,message_id,message,retry-1);
                   }
        });
    };

    void update_send(Bot *bot, const td::td_api::int53 chat_id,td::td_api::int53 message_id,td::td_api::object_ptr<td::td_api::formattedText> formatted_text,
        td::td_api::object_ptr<td::td_api::ReplyMarkup> reply_markup) {
        if (!bot) return;
        if (!formatted_text->text_.ends_with(tg::END)) {
            formatted_text->text_ += tg::END;
        }
        td::td_api::linkPreviewOptions options(true,"",false,false,false);
        td::td_api::inputMessageText input_message(std::move(formatted_text),td::td_api::make_object<td::td_api::linkPreviewOptions>(std::move(options)),true);
        td::td_api::editMessageText edit_message(chat_id,message_id,std::move(reply_markup),
           td::td_api::make_object<td::td_api::inputMessageText>(std::move(input_message)));
        bot->send( td::td_api::make_object<td::td_api::editMessageText>(std::move(edit_message)),
                [](td::td_api::Object && object) {
                        if (object.get_id() == td::td_api::error::ID) {

                        }
                }
            );

    };

    void update_send(Bot *bot, const td::td_api::int53 chat_id,td::td_api::int53 message_id,
        std::function<td::td_api::object_ptr<td::td_api::formattedText>()> formatted_text,
        std::function<td::td_api::object_ptr<td::td_api::ReplyMarkup>()> reply_markup,
        int retry) {
        if (!bot) return;
        td::td_api::object_ptr<td::td_api::formattedText> formatted_text_object = formatted_text();
        if (!formatted_text_object->text_.ends_with(tg::END)) {
            formatted_text_object->text_ += tg::END;
        }
          td::td_api::linkPreviewOptions options(true,"",false,false,false);
        td::td_api::inputMessageText input_message(std::move(formatted_text_object),td::td_api::make_object<td::td_api::linkPreviewOptions>(std::move(options)),true);
        td::td_api::editMessageText edit_message(chat_id,message_id,std::move(reply_markup()),
           td::td_api::make_object<td::td_api::inputMessageText>(std::move(input_message)));
        bot->send( td::td_api::make_object<td::td_api::editMessageText>(std::move(edit_message)),
                [bot,chat_id,message_id,formatted_text,reply_markup,retry](td::td_api::Object && object) {
                        if (object.get_id() == td::td_api::error::ID) {
                            if (retry <= 1) {
                                return;
                            }
                            update_send(bot,chat_id,message_id,formatted_text,reply_markup,retry-1);
                        }
                }
            );

    };

    void send_bot_command(Bot *bot, const std::vector<std::pair<std::string,std::string>> cmds,td::td_api::object_ptr<td::td_api::BotCommandScope> scope_) {
        if (!bot) return;
        td::td_api::array<td::td_api::object_ptr<td::td_api::botCommand>> commands;
        for (auto cmd : cmds) {
            auto cmd_ = td::td_api::make_object<td::td_api::botCommand>(cmd.first,cmd.second);
            commands.push_back(std::move(cmd_));
        }
        td::td_api::setCommands set_commands(std::move(scope_),"zh",std::move(commands));
        bot->send(
            td::td_api::make_object<td::td_api::setCommands>(std::move(set_commands)),
            [](td::td_api::Object && object) {
                if (td::td_api::error::ID == object.get_id()) {

                }
            }
        );
    };
}