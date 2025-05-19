//
// Created by reverccqin on 25-5-18.
//

#include "Utils.h"

#include <unordered_map>
#include "elfio/elfio.hpp"
std::pair<size_t, size_t> Utils::get_plt_range(const std::string& elf_file) {
    ELFIO::elfio reader;
    std::unordered_map<ELFIO::Elf64_Addr, std::string> plt_map;

    if (!reader.load(elf_file)) {
        std::cerr << "Failed to load ELF file: " << elf_file << std::endl;
        return std::make_pair(0, 0);
    }

    ELFIO::section* plt_sec = reader.sections[".plt"];  // 获取 `.plt` 段
    if (!plt_sec) {
        std::cerr << "Missing required sections (.plt, .rela.plt, .dynsym)" << std::endl;
        return std::make_pair(0, 0);;
    }

    // `.plt` 段的起始地址
    ELFIO::Elf64_Addr plt_start = plt_sec->get_address();
    ELFIO::Elf64_Addr plt_end = plt_start + plt_sec->get_size();
    return std::make_pair(plt_start, plt_end);
}