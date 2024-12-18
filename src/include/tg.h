//
// Created by 19766 on 2024/11/14.
//

#ifndef TG_H
#define TG_H
#include <iostream>
#include <ostream>
#include <string>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>


namespace tg {
    static std::string END = std::string({static_cast<char>(0xe2),static_cast<char>(0x80),static_cast<char>(0x8b)});
    static int log_level = 0;

    static void log_error(std::string str) {
        if (log_level <= 4)
            std::cout << ("\033[0;31m "+str+" \033[0m") << std::endl;
    }

    static void log_warning(std::string str) {
        if (log_level <= 3)
            std::cout << ("\033[0;33m "+str+" \033[0m") << std::endl;
    }
    static void log_info(std::string str) {
        if (log_level <= 2)
            std::cout << str << std::endl;
    }
    static void log_debug(std::string str) {
        if (log_level <= 1)
        std::cout << str << std::endl;
    }
    static void log_fatal(std::string str) {
        if (log_level <= 0)
            std::cout << ("\033[0;37m "+str+" \033[0m") << std::endl;
    }
    static std::string input(std::string str) {
        std::cout <<std::endl << str << std::flush;
        std::string input;
        std::getline(std::cin, input);
        return input;
    }


}
#endif //TG_H
