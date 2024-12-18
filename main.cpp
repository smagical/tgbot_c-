

//FT._LIST

#include "src/include/bot_config.h"
#include "src/include/cryptor.h"
#include "src/include/tgbot.h"
//KEY 密匙文本 用双引号包括
auto KEY = (
      #include "KEY"
  );
auto IV = (
      #include "IV"
);
int main(int argc, char** argv)
{
    if (argc != 2) {
        return 0;
    }
    std::string config_str = tg::decryptor::file_aes_decrypt_to_str(argv[1], KEY, IV);
    tg::BotConfig config;
    config.load_from_str(config_str);
    config.load_from_redis();
    tg::Bot bot(config);
    std::string type = config.get_string_value(tg::BotConfig::BOT_LOGIN_TYPE);
    std::string login_info = config.get_string_value(tg::BotConfig::BOT_LOGIN_INFO);
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
