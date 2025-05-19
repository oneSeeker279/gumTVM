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

#ifndef HEX_DUMP_H
#define HEX_DUMP_H
#include "common.h"
#include <sstream>

class HexDump {
public:
    HexDump(const uint8_t* buffer, size_t byte_count, uint64_t address)
        : buffer_(buffer), byte_count_(byte_count), address_(address) {}

    void Dump(std::stringstream& logbuf) const;

private:
    const uint8_t* buffer_;
    uint64_t address_;
    const size_t byte_count_;
    DISALLOW_COPY_AND_ASSIGN(HexDump);
};

inline std::stringstream& operator<<(std::stringstream& logbuf, const HexDump& rhs) {
    rhs.Dump(logbuf);
    return logbuf;
}


#endif //HEX_DUMP_H