//
// Created by 19766 on 2024/11/26.
//

#ifndef BOT_UTIL_H
#define BOT_UTIL_H
#include "tgbot.h"
namespace tg::util {
    enum ChatType {
        MAIN,ARCHIVE,FOLDER,ALL
    };

     td::td_api::object_ptr<td::td_api::user> get_me(Bot *bot);
     void load_all_chats(Bot *bot, const ChatType type = ALL);

     td::td_api::object_ptr<td::td_api::user>* load_user(Bot *bot, const td::td_api::int53 user_id);
     td::td_api::object_ptr<td::td_api::chat>* load_chat(Bot *bot, const td::td_api::int53 chat_id);
     td::td_api::object_ptr<td::td_api::userFullInfo>* load_user_full_info(Bot *bot, const td::td_api::int53 user_id);

     std::vector<td::td_api::object_ptr<td::td_api::message>> load_history(Bot *bot, const td::td_api::int53 chat_id,td::td_api::int53 last_message_id = 0,int limit = 50);
     std::string load_message_link(Bot *bot, const td::td_api::int53 chat_id,const td::td_api::int53 message_id);


     void send(Bot* bot, const td::td_api::int53 chat_id, std::string message,int reply_message_id = -1,int retry = 3);
     void send(Bot *bot, const td::td_api::int53 chat_id,td::td_api::object_ptr<td::td_api::formattedText> formatted_text,
       td::td_api::object_ptr<td::td_api::InputMessageReplyTo> reply_to,td::td_api::object_ptr<td::td_api::ReplyMarkup> reply_markup);
     void send(Bot *bot, const td::td_api::int53 chat_id,
      std::function<td::td_api::object_ptr<td::td_api::formattedText>()> formatted_text,
      std::function<td::td_api::object_ptr<td::td_api::InputMessageReplyTo>()> reply_to,
      std::function<td::td_api::object_ptr<td::td_api::ReplyMarkup>()> reply_markup,
      int retry);

    void update_send(Bot* bot, const td::td_api::int53 chat_id,td::td_api::int53 message_id, std::string message,int retry = 3);
    void update_send(Bot *bot, const td::td_api::int53 chat_id,td::td_api::int53 message_id,td::td_api::object_ptr<td::td_api::formattedText> formatted_text,
        td::td_api::object_ptr<td::td_api::ReplyMarkup> reply_markup);
    void update_send(Bot *bot, const td::td_api::int53 chat_id,td::td_api::int53 message_id,
         std::function<td::td_api::object_ptr<td::td_api::formattedText>()> formatted_text,
         std::function<td::td_api::object_ptr<td::td_api::ReplyMarkup>()> reply_markup,
     int retry);


    void send_bot_command(Bot *bot, const std::vector<std::pair<std::string,std::string>> cmds,td::td_api::object_ptr<td::td_api::BotCommandScope> scope_ = td::td_api::make_object<td::td_api::botCommandScopeDefault>());
}
#endif //BOT_UTIL_H
