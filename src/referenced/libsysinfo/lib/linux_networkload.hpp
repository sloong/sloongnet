/**
 * @author: Daniel Fuchs
 * @contact: fuxeysolutions@gmail.com
 *
 * distributed under the MIT License (MIT).
 * Copyright (c) Daniel Fuchs
 *
 */
#pragma once

#include <map>
#include <memory>
#include <string>
#include <list>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <utility>

const std::list<std::string> identifiers{"RXbytes",
                                         "RXpackets",
                                         "RXerrs",
                                         "RXdrop",
                                         "RXfifo",
                                         "RXframe",
                                         "RXcompressed",
                                         "RXmulticast",
                                         "TXbytes",
                                         "TXpackets",
                                         "TXerrs",
                                         "TXdrop",
                                         "TXfifo",
                                         "TXcolls",
                                         "TXcarrier",
                                         "TXcompressed"};

class networkParser
{
private:
    std::chrono::system_clock::time_point currentTime;
    std::chrono::system_clock::time_point timeBefore;

    std::map<std::string, std::unordered_map<std::string, uint64_t>> ethObj;
    std::map<std::string, std::unordered_map<std::string, uint64_t>> ethObjOld;

public:
    static std::shared_ptr<networkParser> createNetworkParser()
    {
        return std::make_shared<networkParser>();
    }

    networkParser()
    {
        this->currentTime = std::chrono::system_clock::now() - std::chrono::milliseconds(2000);
        this->timeBefore = std::chrono::system_clock::now();
    }
    void parse(const std::string &netFile = "/proc/net/dev")
    {

        if ((this->currentTime + std::chrono::milliseconds(1000)) > std::chrono::system_clock::now())
        {
            return;
        }
        else
        {
            this->timeBefore = this->currentTime;
            this->currentTime = std::chrono::system_clock::now();
        }

        std::ifstream ethFile;

        try
        {
            ethFile.open(netFile);
        }
        catch (std::ifstream::failure &e)
        {
            throw std::runtime_error("Exception: " + netFile + std::string(e.what()));
        }

        this->timeBefore = std::chrono::system_clock::now();
        this->ethObjOld = this->ethObj;

        uint32_t lineCnt = 0;
        for (std::string line; std::getline(ethFile, line); lineCnt++)
        {
            if (lineCnt < 2)
            {
                continue;
            }
            std::stringstream strStream(line);
            std::string strPart;
            std::string ifName = "";
            std::unordered_map<std::string, uint64_t> ifValues;
            auto it = identifiers.begin();
            while (strStream >> strPart)
            {
                if (ifName.empty())
                {
                    strPart.erase(std::remove_if(strPart.begin(), strPart.end(), [](auto c)
                                                 { return !std::isalnum(c); }),
                                  strPart.end());
                    ifName = strPart;
                }
                else
                {
                    if (it != identifiers.end())
                    {
                        ifValues[it->data()] = std::stoull(strPart);
                    }
                    it++;
                }
            }
            this->ethObj[ifName] = ifValues;
        }
        if (this->ethObjOld.empty())
        {
            this->ethObjOld = this->ethObj;
        }
    }
    const std::unordered_map<std::string, uint64_t> &getEthObj(const std::string &ethDevice) const
    {
        return this->ethObj.at(ethDevice);
    }

    std::list<std::string> getNetworkDevices(std::string netFile = "/proc/net/dev")
    {
        std::list<std::string> ifList;
        this->parse(netFile);
        for (const auto &elem : this->ethObj)
        {
            ifList.push_back(elem.first);
        }
        return ifList;
    }
    const std::chrono::system_clock::time_point getTimeStamp() const
    {
        return this->currentTime;
    }
    const std::unordered_map<std::string, uint64_t> &getEthObjOld(const std::string &ethDevice) const
    {
        return this->ethObjOld.at(ethDevice);
    }

    const std::chrono::system_clock::time_point getTimeBefore() const
    {
        return this->timeBefore;
    }
};

class networkLoad
{

public:
    static std::list<std::string> scanNetworkDevices(std::shared_ptr<networkParser> parser = nullptr, const std::string &ethernetDataFile = "/proc/net/dev")
    {
        if (parser == nullptr)
            parser = networkParser::createNetworkParser();
        return parser->getNetworkDevices(ethernetDataFile);
    }
    static std::vector<std::shared_ptr<networkLoad>> createLinuxEthernetScanList(std::shared_ptr<networkParser> parser = nullptr, const std::string &ethernetDataFileName = "/proc/net/dev")
    {
        std::vector<std::shared_ptr<networkLoad>> v;
        for (const auto &elem : networkLoad::scanNetworkDevices(parser, ethernetDataFileName))
        {
            v.push_back(std::make_shared<networkLoad>(ethernetDataFileName, elem));
        }
        return v;
    }

    explicit networkLoad(std::shared_ptr<networkParser> parser = nullptr, std::string ethernetDataFileName = "/proc/net/dev", std::string ethName = "eth0") : ethernetDataFile(std::move(ethernetDataFileName)), ethDev(std::move(ethName))
    {
        if (parser == nullptr)
            parser = networkParser::createNetworkParser();
        parser->parse(ethernetDataFileName);
    }
    uint64_t getParamPerSecond(std::string designator, std::shared_ptr<networkParser> parser = nullptr)
    {
        if (parser == nullptr)
            parser = networkParser::createNetworkParser();
        parser->parse(this->ethernetDataFile);
        if (!std::count_if(identifiers.begin(), identifiers.end(), [designator](auto elem)
                           { return elem == designator; }))
        {
            throw std::runtime_error("invalid designator: " + designator);
        }
        auto before = parser->getTimeBefore();
        auto current = parser->getTimeStamp();

        auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(
            before - current);

        uint64_t Bytes = (parser->getEthObj(this->ethDev).at(designator) -
                          parser->getEthObjOld(this->ethDev).at(designator));
        if (static_cast<unsigned long>(msec.count()) <= 0)
        {
            Bytes /= 1;
        }
        else
        {
            Bytes /= static_cast<unsigned long>(msec.count());
        }
        return Bytes;
    }

    uint64_t getParamSinceStartup(std::string designator, std::shared_ptr<networkParser> parser = nullptr)
    {
        if (parser == nullptr)
            parser = networkParser::createNetworkParser();
        parser->parse(this->ethernetDataFile);
        auto ifObj = parser->getEthObj(this->ethDev);
        if (!std::count_if(identifiers.begin(), identifiers.end(), [designator](auto elem)
                           { return elem == designator; }))
        {
            throw std::runtime_error("invalid designator: " + designator);
        }
        return ifObj[designator];
    }

    static std::string getBytesPerSeceondString(uint64_t bytesPerSecond)
    {
        return getBytesString(bytesPerSecond) + "/s";
    }
    static std::string getBitsPerSeceondString(uint64_t bytesPerSecond)
    {
        return getBitsString(bytesPerSecond) + "/s";
    }
    static std::string getBytesString(uint64_t totalBytes)
    {
        uint64_t Bytes = totalBytes;
        uint64_t kByte = 0;
        uint64_t mByte = 0;
        uint64_t gByte = 0;
        if (Bytes > 1024)
        {
            kByte = Bytes / 1024;
            Bytes %= 1024;
        }
        if (kByte > 1024)
        {
            mByte = kByte / 1024;
            kByte %= 1024;
        }
        if (mByte > 1024)
        {
            gByte = mByte / 1024;
            mByte %= 1024;
        }

        if (gByte > 0)
        {
            std::string net;
            net += std::to_string(gByte);
            net += ".";
            net += std::to_string(mByte / 100);
            net += "gByte";
            return net;
        }

        if (mByte > 0)
        {
            std::string net;
            net += std::to_string(mByte);
            net += ".";
            net += std::to_string(kByte / 100);
            net += "mByte";
            return net;
        }

        if (kByte > 0)
        {
            std::string net;
            net += std::to_string(kByte);
            net += ".";
            net += std::to_string(Bytes / 100);
            net += "kByte";
            return net;
        }

        std::string net;
        net += std::to_string(Bytes);
        net += "Byte";
        return net;
    }

    static std::string getBitsString(uint64_t totalBytes)
    {
        uint64_t Bytes = totalBytes * 8;
        uint64_t kByte = 0;
        uint64_t mByte = 0;
        uint64_t gByte = 0;
        if (Bytes > 1024)
        {
            kByte = Bytes / 1024;
            Bytes %= 1024;
        }
        if (kByte > 1024)
        {
            mByte = kByte / 1024;
            kByte %= 1024;
        }
        if (mByte > 1024)
        {
            gByte = mByte / 1024;
            mByte %= 1024;
        }

        if (gByte > 0)
        {
            std::string net;
            net += std::to_string(gByte);
            net += ".";
            net += std::to_string(mByte / 100);
            net += "gBit";
            return net;
        }

        if (mByte > 0)
        {
            std::string net;
            net += std::to_string(mByte);
            net += ".";
            net += std::to_string(kByte / 100);
            net += "mBit";
            return net;
        }

        if (kByte > 0)
        {
            std::string net;
            net += std::to_string(kByte);
            net += ".";
            net += std::to_string(Bytes / 100);
            net += "kBit";
            return net;
        }

        std::string net;
        net += std::to_string(Bytes);
        net += "Bit";
        return net;
    }

    bool isDeviceUp() const
    {
        return this->isDeviceAvailable;
    }

    std::string getDeviceName()
    {
        return this->ethDev;
    }

    enum networkParam
    {
        RXbytes = 0,
        RXpackets,
        RXerrs,
        RXdrop,
        RXfifo,
        RXframe,
        RXcompressed,
        RXmulticast,
        TXbytes,
        TXpackets,
        TXerrs,
        TXdrop,
        TXfifo,
        TXcolls,
        TXcarrier,
        TXcompressed
    };
    static std::string mapEnumToString(networkLoad::networkParam param)
    {
        auto it = identifiers.begin();
        std::advance(it, static_cast<uint32_t>(param));
        return it->data();
    }

private:
    std::string ethernetDataFile;
    std::string ethDev;
    bool isDeviceAvailable = false;
};
