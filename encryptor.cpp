
#include <iostream>

#include "src/include/cryptor.h"
// Created by 19766 on 2024/12/18.
//

auto KEY = (
      #include "KEY"
  );
auto IV = (
      #include "IV"
);


int main(int argc, char *argv[]) {
    if (argc != 2) {
        return 1;
    }
     tg::encryptor::file_aes_encrypt(argv[1],KEY,IV);
    return 0;
}
