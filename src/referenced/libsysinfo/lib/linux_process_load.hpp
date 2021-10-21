/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2021-09-23 17:37:56
 * @LastEditTime: 2021-09-23 20:52:45
 * @LastEditors: Chuanbin Wang
 * @FilePath: /Linux-System-Monitoring-Library/lib/linux_process_load.hpp
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

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>

#include "linux_cpuload.hpp"
#include "linux_memoryload.hpp"

static const std::list<std::string> stats{
    "pid",        "comm",        "state",       "ppid",      "pgrp",        "session",     "tty_nr",
    "tpgid",      "flags",       "minflt",      "cminflt",   "majflt",      "cmajflt",     "utime",
    "stime",      "cutime",      "cstime",      "priority",  "nice",        "num_threads", "itrealvalue",
    "starttime",  "vsize",       "rss",         "rsslim",    "startcode",   "endcode",     "startstack",
    "kstkesp",    "kstkeip",     "signal",      "blocked",   "sigignore",   "sigcatch",    "wchan",
    "nswap",      "cnswap",      "exit_signal", "processor", "rt_priority", "policy",      "delaycct_blkio_ticks",
    "guest_time", "cguest_time", "start_data",  "end_data",  "start_brk",   "arg_start",   "arg_end",
    "env_start",  "env_end",     "exit_code"};

class linuxProcessLoad
{

  public:
    /**
     * @brief get a map of [pid] which contains the cpu load between two calls.
     *          function needs to be called regularly (e.g.: 5s.) to get the cpu
     * load per process
     * @return const map[pid][cpuload]
     */
    std::map<std::string, double> getProcessCpuLoad()
    {
        this->findProcesses();
        return this->procCPUUsage;
    }

  private:
    void parseProcess(const std::string &pid)
    {
        std::string path{"/proc/" + pid + "/stat"};
        std::ifstream ethFile(path);

        std::string strPart;
        std::unordered_map<std::string, std::string> procStat;
        auto identifierStart = stats.begin();
        enum state
        {
            normal,
            isProcessParse
        };
        bool isProcessFound = false;
        while (ethFile >> strPart)
        {

            if (identifierStart != stats.end())
            {
                if (isProcessFound)
                {
                    procStat[identifierStart->data()] += " " + strPart;
                }
                if (std::count_if(strPart.begin(), strPart.end(), [](auto c) { return c == '('; }))
                {
                    isProcessFound = true;
                    procStat[identifierStart->data()] = strPart;
                }
                if (!isProcessFound)
                {
                    procStat[identifierStart->data()] = strPart;
                }

                if (std::count_if(strPart.begin(), strPart.end(), [](auto c) { return c == ')'; }))
                {
                    isProcessFound = false;
                }
            }
            if (!isProcessFound)
            {
                identifierStart++;
                continue;
            }
        }

        procStat["comm"].erase(std::remove_if(procStat["comm"].begin(), procStat["comm"].end(),
                                              [](auto c) { return c == '(' || c == ')'; }),
                               procStat["comm"].end());
        processStat[pid] = procStat;
    }

    void findProcesses()
    {

        auto cpuMonitoring = std::make_unique<cpuLoad>("/proc/stat");

        this->CpuTimes = cpuMonitoring->getCpuTimes();

        std::string path{"/proc/"};
        std::vector<std::string> processes;
        for (auto &elem : std::filesystem::directory_iterator(path))
        {
            auto procPath = elem.path().string();
            if (elem.exists())
            {
                procPath.erase(procPath.begin(), procPath.begin() + static_cast<int32_t>(path.size()));
                if (std::isdigit(procPath.at(0)))
                {
                    if (!std::count_if(procPath.begin(), procPath.end(), [](auto c) { return std::isalpha(c); }))
                    {
                        parseProcess(procPath);
                    }
                }
            }
        }
        this->calculateProcessLoad();

        this->oldProcessStat = this->processStat;
        this->oldCpuTimes = this->CpuTimes;
    }

    void calculateProcessLoad()
    {
        auto cpuTotalUserTime, cpuTotalUserLowTime, cpuTotalSysTime, cpuTotalIdleTime = CpuTimes;

        auto oldCpuTotalUserTime, oldCpuTotalUserLowTime, oldCpuTotalSysTime, oldCpuTotalIdleTime = oldCpuTimes;
        auto TotalUserTime = cpuTotalUserTime - oldCpuTotalUserTime;
        auto TotalSysTime = cpuTotalSysTime - oldCpuTotalSysTime;

        this->procCPUUsage.clear();
        for (const auto &elem : processStat)
        {
            auto pid = elem.first;
            try
            {
                auto oldProc = oldProcessStat.at(pid);
                auto proc = elem.second;
                auto procName = proc.at("comm");
                uint64_t cpuTime = 0;
                cpuTime += (std::stoull(proc.at("utime")) - std::stoull(oldProc.at("utime")));
                cpuTime += (std::stoull(proc.at("stime")) - std::stoull(oldProc.at("stime")));

                double percentage =
                    ((static_cast<double>(cpuTime) * 100.0) / static_cast<double>((TotalUserTime + TotalSysTime)));

                if (percentage > 0.1)
                {
                    this->procCPUUsage[procName] = percentage;
                }
            }
            catch (...)
            {
                std::cerr << "process: " << pid << " disappeared in meantime" << std::endl;
            }
        }
    }

    std::map<std::string, double> procCPUUsage;
    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> oldCpuTimes;
    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> CpuTimes;
    std::map<std::string, std::unordered_map<std::string, std::string>> processStat;
    std::map<std::string, std::unordered_map<std::string, std::string>> oldProcessStat;
};
