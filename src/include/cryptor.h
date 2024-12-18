//
// Created by 19766 on 2024/12/18.
//

#ifndef CRYPTOR_H
#define CRYPTOR_H
#include <string>

namespace tg {
    namespace encryptor {
        bool file_aes_encrypt(std::string file_path, std::string key,std::string iv,std::string out_file = "");


    }

    namespace decryptor {
        bool file_aes_decrypt(std::string file_path, std::string key,std::string iv,std::string out_file);
        std::string file_aes_decrypt_to_str(std::string file_path, std::string key,std::string iv);
    }
} // tg

#endif //CRYPTOR_H
