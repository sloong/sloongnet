/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2021-09-23 17:27:14
 * @LastEditTime: 2021-09-24 11:24:01
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/referenced/libsysinfo/lib/linux_memoryload.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
/**
 * @author: Daniel Fuchs
 * @contact: fuxeysolutions@gmail.com
 *
 * distributed under the MIT License (MIT).
 * Copyright (c) Daniel Fuchs
 *
 */
#pragma once

#include <cmath>
#include <fstream>
#include <iostream>
#include <cinttypes>
#include <string>
#include <chrono>

class memoryLoad
{
public:
    explicit memoryLoad(std::string memInfo = "/proc/meminfo",
                        std::string memInfoOfProcess = "/proc/self/status",
                        std::string memInfoOfProcessPrefix = "/proc/self/") : totalMemoryInKB(0),
                                                                              currentMemoryUsageInKB(0),
                                                                              memInfoFile(memInfo),
                                                                              memInfoOfProcessFile(memInfoOfProcess),
                                                                              memInfoOfProcessPrefixFile(memInfoOfProcessPrefix){};
    /**
     * @brief get total memory of the system in KB
     * @return total memory in KB
     */
    uint64_t getTotalMemoryInKB()
    {
        this->parseMemoryFile();
        return this->totalMemoryInKB;
    }
    /**
     * @brief get current Memory Usage of the system in KB
     * @return used memory in KB
     */
    uint64_t getCurrentMemUsageInKB()
    {
        this->parseMemoryFile();
        return this->getTotalMemoryInKB() - this->currentMemoryUsageInKB;
    }

    /**
     * @brief get current memory usage of the system in percent 0-100%
     * @return 0-100%
     */
    float getCurrentMemUsageInPercent()
    {
        this->parseMemoryFile();
        uint64_t memavail = this->getCurrentMemUsageInKB();
        return round((((memavail * 100.0 / this->getTotalMemoryInKB()))) * 100.0) / 100.0;
    }
    /**
     * @brief get the current memory usage of a process
     * @param pid  - process id
     * @return memory usage in KB
     */
    static uint64_t getMemoryUsedByProcess(int pid)
    {
        return memoryLoad::parseProcessMemoryFile("/proc/" + std::to_string(pid) + "/status");
    }
    /**
     * @brief get memory usage of this process (self)
     * @return memory usage in KB
     */
    uint64_t getMemoryUsageByThisProcess()
    {
        return this->parseProcessMemoryFile(this->memInfoOfProcessFile);
    }

private:
    bool parseMemoryFile()
    {
        if (timeStamp + std::chrono::milliseconds(100) > std::chrono::steady_clock::now())
        {
            return true;
        }
        std::ifstream memoryFile;
        memoryFile.open(this->memInfoFile);
        this->timeStamp = std::chrono::steady_clock::now();
        if (!memoryFile.is_open())
        {
            return false;
        }

        std::string line;
        while (std::getline(memoryFile, line))
        {
            sscanf(line.c_str(), "MemTotal: %" PRIu64, &this->totalMemoryInKB);
            sscanf(line.c_str(), "MemAvailable: %" PRIu64, &this->currentMemoryUsageInKB);
        }
        memoryFile.close();
        return true;
    }
    static uint64_t parseProcessMemoryFile(std::string fileToParse)
    {
        uint64_t MemFree = 0;
        std::ifstream memoryFile;
        memoryFile.open(fileToParse);
        std::string line;
        while (std::getline(memoryFile, line))
        {
            sscanf(line.c_str(), "VmSize: %" PRIu64, &MemFree);
        }
        return MemFree;
    }
    uint64_t totalMemoryInKB;
    uint64_t currentMemoryUsageInKB;
    std::string memInfoFile;
    std::string memInfoOfProcessFile;
    std::string memInfoOfProcessPrefixFile;
    std::chrono::time_point<std::chrono::steady_clock> timeStamp;
};