//
// Created by 19766 on 2024/12/1.
//

#ifndef USER_PLUGIN_H
#define USER_PLUGIN_H
#include "../../include/plugin.h"
#include "../command/include/command.h"

namespace tg {
    namespace plugin {
        class UserPlugin:public PluginInterface {
            public:
                bool hand_new_message(Bot *bot, td::td_api::updateNewMessage &&update_new_message) override;
                bool hand_new_call_callback_query(Bot *bot, td::td_api::updateNewCallbackQuery &&update_new_callback_query) override;
                int get_id() override;
                std::string get_name() override;
                void addCommand(std::unique_ptr<tg::plugin::command::Command> command);

            private:
                bool solve_cmd(Bot* bot,command::CommandType type,td::td_api::int53 chat_id,td::td_api::messageText* message_text) const;
                std::vector<std::unique_ptr<tg::plugin::command::Command>> commands_;

        };

    } // plugin
} // tg

#endif //USER_PLUGIN_H
