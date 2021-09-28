/**
 * @author: Daniel Fuchs
 * @contact: fuxeysolutions@gmail.com
 *
 * distributed under the MIT License (MIT).
 * Copyright (c) Daniel Fuchs
 *
 */
#pragma once

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

const std::vector<std::string> cpuIdentifiers{"user", "nice",    "system", "idle",  "iowait",
                                              "irq",  "softirq", "steal",  "guest", "guest_nice"};

class cpuLoad
{

  public:
    cpuLoad() = delete;

    static shared_ptr<cpuLoad> createInstance(std::string procFileName = "/proc/stat")
    {
        return make_shared<cpuLoad>(procFileName);
    }

    /**
     * @brief constructor
     * @param procFileName
     */
    explicit cpuLoad(std::string procFileName = "/proc/stat") : procFile(procFileName), cpuName("")
    {
        initCpuUsage();
    };

    /**
     * @brief initialize the parsing algo
     */
    void initCpuUsage()
    {
        this->cpuLoadMap = this->parseStatFile(this->procFile);
        this->currentTime = std::chrono::system_clock::now() - std::chrono::milliseconds(2000);
        this->upDateCPUUsage();
    }
    /**
     * @brief get current cpu load in 0-100%
     * @return current cpu load
     */
    double getCurrentCpuUsage()
    {
        this->upDateCPUUsage();
        return this->cpuUsage.at("cpu");
    }

    /**
     * @brief get Cpu user / nice / system /idle time. used for cpu usage per
     * process
     * @return tuple<user,nice,system,idle>
     */
    std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> getCpuTimes()
    {
        auto cpuLoad_ = this->parseStatFile(this->procFile);
        return std::make_tuple(cpuLoad_.at("cpu").at("user"), cpuLoad_.at("cpu").at("nice"),
                               cpuLoad_.at("cpu").at("system"), cpuLoad_.at("cpu").at("idle"));
    }

    /**
     * @brief get cpu Usage of all cores in percent
     * @return vector of cpu load per core 0-100%
     */
    std::vector<double> getCurrentMultiCoreUsage()
    {
        this->upDateCPUUsage();
        std::vector<double> percents;
        for (const auto &elem : this->cpuUsage)
        {
            if (elem.first == "cpu")
            {
                continue;
            }
            percents.push_back(elem.second);
        }
        return percents;
    }
    /**
     * @brief get CPU Description
     * @param cpuNameFile - typical /proc/cpuinfo
     * @return cpu Type string
     */
    std::string getCPUName(const std::string &cpuNameFile = "/proc/cpuinfo")
    {

        if (!this->cpuName.empty())
        {
            return this->cpuName;
        }

        std::ifstream file;
        file.open(cpuNameFile);

        if (!file.is_open())
        {
            std::throw_with_nested(std::runtime_error("unable to open " + cpuNameFile));
        }
        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("model name") != std::string::npos)
            {
                size_t pos = line.find(':');
                if (pos != std::string::npos)
                {
                    this->cpuName = line.substr(pos, line.size());
                    return this->cpuName;
                }
            }
        }
        return std::string();
    }

  private:
    void calculateCpuUsage()
    {
        try
        {
            for (const auto &elem : this->cpuLoadMap)
            {
                auto &v = elem.second;
                auto &old = this->oldCpuLoadMap.at(elem.first);
                if (v.at("user") < old.at("user") || v.at("nice") < old.at("nice") ||
                    v.at("system") < old.at("system") || v.at("idle") < old.at("idle"))
                {
                }
                else
                {
                    auto total = (v.at("user") - old.at("user")) + (v.at("nice") - old.at("nice")) +
                                 (v.at("system") - old.at("system"));

                    double percent = total;
                    total += (v.at("idle") - old.at("idle"));
                    percent /= total;
                    percent *= 100.0;
                    this->cpuUsage[elem.first] = percent;
                }
            }
        }
        catch (const std::exception &e)
        {
            cerr << "calculateCpuUsage" << e.what() << endl;
        }
    }
    std::map<std::string, std::unordered_map<std::string, uint64_t>> parseStatFile(const std::string &fileName)
    {

        std::map<std::string, std::unordered_map<std::string, uint64_t>> cpuLoad_;

        try
        {
            std::ifstream cpuFile(fileName);

            uint32_t lineCnt = 0;
            bool infoValid = true;
            for (std::string line; std::getline(cpuFile, line) && infoValid; lineCnt++)
            {

                std::stringstream strStream(line);
                std::string strPart;
                std::string cpuNum;
                auto it = cpuIdentifiers.begin();
                std::unordered_map<std::string, uint64_t> cpuValues;
                while (strStream >> strPart)
                {
                    if (cpuNum.empty())
                    {
                        if (strPart.find("cpu") != std::string::npos)
                        {
                            cpuNum = strPart;
                            continue;
                        }
                        else
                        {
                            infoValid = false;
                            break;
                        }
                    }
                    if (it != cpuIdentifiers.end())
                    {
                        cpuValues[it->data()] = std::stoull(strPart);
                    }
                    if (it->data() == cpuIdentifiers.at(4))
                    {
                        break;
                    }
                    it++;
                }
                if (!cpuNum.empty())
                {
                    cpuLoad_[cpuNum] = cpuValues;
                }
            }
        }
        catch (std::ifstream::failure &e)
        {
            std::throw_with_nested(std::runtime_error("Exception: " + fileName + std::string(e.what())));
        }
        return cpuLoad_;
    }
    void upDateCPUUsage()
    {
        if (!((this->currentTime + std::chrono::milliseconds(1000)) > std::chrono::system_clock::now()))
        {
            this->oldCpuLoadMap = this->cpuLoadMap;
            this->currentTime = std::chrono::system_clock::now();
            this->cpuLoadMap = this->parseStatFile(this->procFile);
            this->calculateCpuUsage();
        }
    }
    std::chrono::system_clock::time_point currentTime;
    std::string procFile;
    std::string cpuName;
    std::map<std::string, double> cpuUsage;
    std::map<std::string, std::unordered_map<std::string, uint64_t>> cpuLoadMap;
    std::map<std::string, std::unordered_map<std::string, uint64_t>> oldCpuLoadMap;
};
