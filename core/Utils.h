//
// Created by reverccqin on 25-5-18.
//

#ifndef UTILS_H
#define UTILS_H
#include "elfio/elfio.hpp"
class Utils {
public:
    Utils() = delete;

    ~Utils() = delete;

    static std::pair<size_t, size_t> get_plt_range(const std::string& elf_file);

};
#endif //UTILS_H
