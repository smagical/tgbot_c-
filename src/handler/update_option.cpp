//
// Created by 19766 on 2024/11/26.
//

#include "include/update_option.h"

#include "../include/tg.h"

namespace tg {
    namespace handler {
        UpdateOption::UpdateOption(Bot *bot):HandlerTemplate<td::td_api::updateOption, void>(bot) {

        }
        void UpdateOption::hand(td::td_api::Object &&object) {
            // log_info("UpdateOption::hand "+td::td_api::to_string(object));
        }


    } // handler
} // tg