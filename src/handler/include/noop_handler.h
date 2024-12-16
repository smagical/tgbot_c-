//
// Created by 19766 on 2024/11/26.
//

#ifndef NOOPHANDLER_H
#define NOOPHANDLER_H
#include "handler.h"
#include <set>
namespace tg{
    namespace handler{
        class NoopHandler:public Handler{
            public:
                NoopHandler(Bot* bot):Handler(bot) {
                    set.insert(td::td_api::updateNotification::ID);
                    set.insert(td::td_api::updateChatTheme::ID);
                    set.insert(td::td_api::updateChatThemes::ID);
                    set.insert(td::td_api::updateAttachmentMenuBots::ID);
                    set.insert(td::td_api::updateActiveEmojiReactions::ID);
                    set.insert(td::td_api::updateDiceEmojis::ID);
                    set.insert(td::td_api::updateDefaultBackground::ID);
                    set.insert(td::td_api::updateProfileAccentColors::ID);
                    set.insert(td::td_api::updateAccentColors::ID);
                    set.insert(td::td_api::updateAnimationSearchParameters::ID);
                    set.insert(td::td_api::updateAvailableMessageEffects::ID);
                    set.insert(td::td_api::updateHavePendingNotifications::ID);
                    set.insert(td::td_api::updateScopeNotificationSettings::ID);
                    set.insert(td::td_api::updateReactionNotificationSettings::ID);
                    set.insert(td::td_api::updateSuggestedActions::ID);
                };
                void hand(td::td_api::Object &&object) override {

                };
                bool support(td::td_api::Object &object) override {
                    if (set.contains(object.get_id())) {
                        return true;
                    }
                    return false;
                };
            private:
               std::set<std::int32_t> set;
        };
    }
}
#endif //NOOPHANDLER_H
