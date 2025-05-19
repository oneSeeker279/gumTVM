//
// Created by reverccqin on 25-5-17.
//

#include "logger_manager.h"
#include "hex_dump.h"
#include <cstdint>
#include <sys/stat.h>
#include <sstream>
#include <sys/mman.h>
#include <sys/uio.h>
#include <unistd.h>

void LoggerManager::set_enable_to_file(bool enable, const std::string& file_name) {
    if (enable) {
        if (trace_file_name.empty()) {
            std::string trace_dir = "/sdcard/Download/qbdiTVM/";
            if (!check_and_mkdir(trace_dir)) {
                LOGE("mkdir failed %s", trace_dir.c_str());
            }
            trace_file_name = trace_dir + file_name;
            trace_out.open(trace_file_name, std::ios::out);
            if(trace_out.is_open()) {
                LOGD("%s file opened. ", trace_file_name.c_str());
            }
        }
    } else {
        if (trace_out.is_open()) {
            trace_out.close();
            trace_out.flush();
        }
    }
}


bool LoggerManager::check_and_mkdir(std::string &path) {
    struct stat info;
    if (stat(path.c_str(), &info) == 0) {
        if (info.st_mode & S_IFDIR) {
            return true;
        } else {
            return false;
        }
    }
    if (mkdir(path.c_str(), 0755) == 0) {
        return true;
    } else {
        return false;
    }
    return true;
}

void LoggerManager::write_info(std::stringstream &line) {
    if (this->trace_out.is_open()) {
        this->trace_out << line.str();
    }
}

std::string LoggerManager::dump_reg_value(uint64_t regValue, const char* regName, size_t count) {
    std::stringstream regOutput;
    if (isValidAddress(regValue)) {
        size_t maxLen = 0x800;
        uint8_t buffer[0x801] = {};
        if (safeReadMemory(regValue, buffer, maxLen)) {
            if (is_ascii_printable_string(buffer, maxLen)) {
                regOutput << "String for " << regName << " at address 0x"
                          << std::hex << regValue << " : "
                          << std::string(reinterpret_cast<const char *>(buffer))
                          << "\n";
            } else {
                regOutput << "Hexdump for " << regName << " at address 0x"
                          << std::hex << regValue << ":\n";
                const HexDump hex_dump(buffer, count, regValue);
                regOutput << hex_dump;
            }
        }
    }
    return regOutput.str();
}

bool LoggerManager::isValidAddress(uint64_t address) {
    long pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize <= 0) {
        return false;
    }
    void* alignedAddress = reinterpret_cast<void*>(address & ~(pageSize - 1));
    unsigned char vec;
    if (mincore(alignedAddress, pageSize, &vec) == 0) {
        return true;
    }
    return false;
}


bool LoggerManager::is_ascii_printable_string(const uint8_t* data, size_t length) {
    if (data == nullptr) {
        LOGD("Error: data is NULL");
        return false;
    }

    bool hasNonSpaceChar = false;
    for (size_t i = 0; i < length; ++i) {
        if (data[i] == '\0') {
            return hasNonSpaceChar;
        }
        if (data[i] < 0x20 || data[i] > 0x7E) {
            return false;
        }
        if (data[i] != ' ') {
            hasNonSpaceChar = true;
        }
    }
    return hasNonSpaceChar;
}

bool LoggerManager::safeReadMemory(uint64_t address, uint8_t* buffer, size_t length) {
    struct iovec local_iov{};
    struct iovec remote_iov{};

    local_iov.iov_base = buffer;
    local_iov.iov_len = length;

    remote_iov.iov_base = reinterpret_cast<void*>(address);
    remote_iov.iov_len = length;
    ssize_t nread = process_vm_readv(getpid(), &local_iov, 1, &remote_iov, 1, 0);
    if (nread == static_cast<ssize_t>(length)) {
        return true;
    } else {
        // LOGE("process_vm_readv failed at address 0x%lx: %s", address, strerror(errno));
        return false;
    }
}