//
// Created by 19766 on 2024/11/26.
//

#ifndef UPDATE_OPTION_H
#define UPDATE_OPTION_H
#include "handler.h"

namespace tg {
    namespace handler {

        class UpdateOption:public HandlerTemplate<td::td_api::updateOption> {
            public:
                UpdateOption(Bot* bot);
                void hand(td::td_api::Object &&object) override;
        };

    } // handler
} // tg

#endif //UPDATE_OPTION_H
