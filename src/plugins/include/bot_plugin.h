//
// Created by 19766 on 2024/12/1.
//

#ifndef BOT_PLUGIN_H
#define BOT_PLUGIN_H
#include "../../include/plugin.h"
#include "../command/include/command.h"

namespace tg::plugin {
    // namespace command {
    //     class Command;
    //     enum CommandType;
    // }
    class BotPlugin:public PluginInterface {
    public:
        bool hand_new_message(Bot *bot, td::td_api::updateNewMessage &&update_new_message) override;
        bool hand_new_call_callback_query(Bot *bot, td::td_api::updateNewCallbackQuery &&update_new_callback_query) override;
        int get_id() override;
        std::string get_name() override;
        void addCommand(std::unique_ptr<tg::plugin::command::Command> command);
        void send_commands(Bot* bot);

    private:
        bool solve_cmd(Bot* bot,command::CommandType type,td::td_api::int53 chat_id,td::td_api::messageText* message_text) const;
        bool solve_message_search(Bot* bot,td::td_api::int53 chat_id,td::td_api::int53 message_id,std::string search_text,int page = 0,int limit = 10,bool is_update = false);
        std::vector<std::unique_ptr<tg::plugin::command::Command>> commands_;
    };

}

#endif //BOT_PLUGIN_H
