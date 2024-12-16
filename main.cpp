

//FT._LIST

#include "src/include/tgbot.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        return 0;
    }


    return 0;
    std::string config_path = std::string(argv[1]);
    tg::BotConfig config;
    config.load_config(config_path);
    tg::Bot bot(config);
    std::string type = config.get_string_value(tg::BotConfig::LOGIN_TYPE);
    std::string login_info = config.get_string_value(tg::BotConfig::LOGIN_INFO);
    if (type == "PHONE") {
        bot.loginPhone(login_info);
    }else if (type == "OCR") {
        bot.loginOcr();
    }else if (type == "TOKEN") {
        bot.loginToken(login_info);
    }else {
        return 0;
    }
    std::this_thread::sleep_for(std::chrono::seconds(LLONG_MAX));
    // _sleep(INT64_MAX);
    return 0;
}
