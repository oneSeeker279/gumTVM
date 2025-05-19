//
// Created by reverccqin on 25-5-17.
//

#ifndef LOGGER_MANAGER_H
#define LOGGER_MANAGER_H
#include <string>
#include <fstream>
#include "common.h"
class LoggerManager {
public:
    LoggerManager(std::string module_name, module_range_t module_range) : module_name(std::move(module_name)),
                                                                          module_range(module_range) {}

    ~LoggerManager()= default;

    void write_info(std::stringstream& line);

    std::string dump_reg_value(uint64_t regValue, const char* regName, size_t count = 32);

    bool isValidAddress(uint64_t address);

    bool is_ascii_printable_string(const uint8_t* data, size_t length);

    bool safeReadMemory(uint64_t address, uint8_t* buffer, size_t length);

    void set_enable_to_file(bool enable, const std::string& file_name);


private:
    static bool check_and_mkdir(std::string& path);
    std::ofstream trace_out;
    std::string trace_file_name;
    std::string module_name;
    module_range_t module_range;
};

#endif //LOGGER_MANAGER_H
