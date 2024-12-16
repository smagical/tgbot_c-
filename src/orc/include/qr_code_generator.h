//
// Created by 19766 on 2024/11/15.
//

#ifndef QR_H
#define QR_H
#include <string>

namespace tg {
    namespace orc {

        class QRCodeGenerator  {
        public:
            static void printQRCode(const std::string& content, int width = 1, int height = 1);
        };

    } // orc
} // tg

#endif //QR_H
