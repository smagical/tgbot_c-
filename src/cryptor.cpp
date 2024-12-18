//
// Created by 19766 on 2024/12/18.
//

#include "include/cryptor.h"

#include <fstream>
#include <iostream>
#include <string>
#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>
#include <cryptopp/modes.h>
#include <cryptopp/secblock.h>

namespace tg {
    namespace encryptor {
        bool file_aes_encrypt(std::string file_path, std::string key,std::string iv,std::string out_file) {
            if (std::ifstream file(file_path); !file.good()) {
                std::cout << file_path << " could not be opened" << std::endl;
                return false;
            }
            if (out_file == "") {
                out_file = file_path+".encrypt";
            }
            CryptoPP::SecByteBlock secret_key(reinterpret_cast<const unsigned char *>(key.c_str()),key.size());
            CryptoPP::SecByteBlock iv_sec(reinterpret_cast<const unsigned char *>(iv.c_str()), iv.size());
            CryptoPP::OFB_Mode<CryptoPP::AES>::Encryption encryptor(secret_key,secret_key.size(),iv_sec);
            CryptoPP::FileSource file_source(file_path.c_str(),true,
                new CryptoPP::StreamTransformationFilter(encryptor,
                    new CryptoPP::Base64Encoder(new CryptoPP::FileSink(out_file.c_str()))));
            return  true;
        }
    } // cryptor

    namespace decryptor {

        bool file_aes_decrypt(std::string file_path, std::string key,std::string iv,std::string out_file) {
            if (std::ifstream file(file_path); !file.good()) {
                std::cout << file_path << " could not be opened" << std::endl;
                return false;
            }
            if (out_file == "") {
                out_file = file_path+".decrypt";
            }
            CryptoPP::SecByteBlock secret_key(reinterpret_cast<const unsigned char *>(key.c_str()),key.size());
            CryptoPP::SecByteBlock iv_sec(reinterpret_cast<const unsigned char *>(iv.c_str()), iv.size());
            CryptoPP::OFB_Mode<CryptoPP::AES>::Decryption decryption(secret_key,secret_key.size(),iv_sec);
            CryptoPP::FileSource file_source(file_path.c_str(),true,
                new CryptoPP::Base64Decoder(new CryptoPP::StreamTransformationFilter(decryption,new CryptoPP::FileSink(out_file.c_str()))));
            return  true;
        }

        std::string file_aes_decrypt_to_str(std::string file_path, std::string key,std::string iv) {
            if (std::ifstream file(file_path); !file.good()) {
                std::cout << file_path << " could not be opened" << std::endl;
                return "";
            }
            CryptoPP::SecByteBlock secret_key(reinterpret_cast<const unsigned char *>(key.c_str()),key.size());
            CryptoPP::SecByteBlock iv_sec(reinterpret_cast<const unsigned char *>(iv.c_str()), iv.size());
            CryptoPP::OFB_Mode<CryptoPP::AES>::Decryption decryption(secret_key,secret_key.size(),iv_sec);
            std::string paint_string;
            CryptoPP::FileSource file_source(file_path.c_str(),true,
                new CryptoPP::Base64Decoder(new CryptoPP::StreamTransformationFilter(decryption,new CryptoPP::StringSink(paint_string))));
            return  paint_string;
        }

    } // cryptor

} // tg