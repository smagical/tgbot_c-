//
// Created by 19766 on 2024/11/15.
//

#include "include/qr_code_generator.h"

#include <ZXing/MultiFormatWriter.h>
#include <ZXing/BarcodeFormat.h>
#include <ZXing/BitMatrix.h>

#include "../include/tg.h"

namespace tg {
    namespace orc {
        void QRCodeGenerator::printQRCode(const std::string &content, int width, int height) {
            ZXing::MultiFormatWriter writer(ZXing::BarcodeFormat::QRCode);
            writer.setEccLevel(0);
            writer.setMargin(10);
            ZXing::BitMatrix matrix = writer.encode(content,width,height);
            std::string str;
            for (int i =0;i<matrix.height();i++) {
                for (int j=0;j<matrix.width();j++) {
                    bool flag = matrix.get(i,j);
                    if (flag) {
                        str += "\033[47m  \033[0m";
                    }
                    else {
                        str += "\033[30m  \033[0;39m";
                    }
                }
                str += "\n";
            }
            tg::log_info(content);
            tg::log_info("");
            tg::log_info(str);
        }

    } // orc
} // tg