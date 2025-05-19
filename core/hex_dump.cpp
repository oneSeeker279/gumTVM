/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025  g2wfw
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "hex_dump.h"
#include <iomanip>
void HexDump::Dump(std::stringstream& logbuf) const {
    size_t offset = 0;
    while (offset < byte_count_) {
        // 输出当前行的基址
        logbuf << std::hex << std::setw(8) << std::setfill('0') << (address_ + offset) << ": ";

        // 输出每一行的十六进制数据和ASCII字符
        std::string ascii; // 暂存 ASCII 字符串
        for (size_t i = 0; i < 16; ++i) {
            if (offset + i < byte_count_) {
                uint8_t byte = buffer_[offset + i];
                logbuf << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte) << " ";
                ascii += (std::isprint(byte) ? static_cast<char>(byte) : '.'); // 可打印字符直接显示，否则用 .
            } else {
                logbuf << "   "; // 如果数据不足16字节，填充空格
                ascii += " ";
            }
            if (i == 7) logbuf << " "; // 中间分隔
        }

        // 输出 ASCII 表示
        logbuf << " |" << ascii << "|" << std::endl;
        offset += 16;
    }
}