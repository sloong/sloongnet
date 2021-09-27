/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2021-09-23 17:41:16
 * @LastEditTime: 2021-09-23 17:44:25
 * @LastEditors: Chuanbin Wang
 * @FilePath: /Linux-System-Monitoring-Library/lib/linux_systemutil.hpp
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

#include <cinttypes>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <pwd.h>
#include <string>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

class linuxUtil
{

  public:
    static bool isDeviceOnline(std::string address)
    {
        const std::string processPrefix = {"ping -c 1 -w 1 "};
        const std::string processPostfix = {" 2>&1"};
        auto fd = popen((processPrefix + address + processPostfix).c_str(), "r");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (fd == nullptr)
        {
            return false;
        }
        char buff[1000];
        char *ptr = buff;
        size_t sz = sizeof(buff);
        while (getline(&ptr, &sz, fd) != -1)
        {
            std::string line(buff);
            if (line.find(" 1 received") != std::string::npos)
            {
                pclose(fd);
                return true;
            }
            if (line.find("100% packet loss") != std::string::npos)
            {
                pclose(fd);
                return false;
            }
        }
        return false;
    }
    static std::string getOSVersion_Signature(void)
    {
        std::ifstream versionFile;
        versionFile.open("/proc/version_signature");

        if (!versionFile.is_open())
        {
            return std::string();
        }
        std::string line;
        std::getline(versionFile, line);

        versionFile.close();
        return line;
    }
    static std::string getOsVersionString(void)
    {
        std::ifstream versionFile;
        versionFile.open("/proc/version");

        if (!versionFile.is_open())
        {
            return std::string();
        }
        std::string line;
        std::getline(versionFile, line);

        versionFile.close();
        return line;
    }
    static std::string getIFaceMacAddress(std::string deviceName)
    {
        const std::string sysClassPath = "/sys/class/net/";

        uint32_t Threads = 0;
        std::ifstream IFaceFile;
        IFaceFile.open(sysClassPath + deviceName + "/address");
        std::string line;
        std::getline(IFaceFile, line);
        return line;
    }

    static int killProcessById(int pid, const std::string &procName)
    {
        if (pid == -1)
        {
            throw std::runtime_error("Nothing to Kill, no Process " + procName + " PID " + std::to_string(pid));
        }
        int ret = kill(pid, 9);
        if (ret == -1)
        {
            std::throw_with_nested(std::runtime_error("killing " + procName + " was not successful!"));
        }
        return ret;
    }
    static int getProcIdByName(const std::string &procName)
    {
        int pid = -1;
        auto dp = opendir("/proc");
        if (dp != nullptr)
        {
            struct dirent *dirp;
            while (pid < 0 && (dirp = readdir(dp)))
            {
                int id = atoi(dirp->d_name);
                if (id > 0)
                {
                    std::string cmdPath{"/proc/"};
                    cmdPath.append(dirp->d_name);
                    cmdPath.append("/cmdline");
                    std::ifstream cmdFile(cmdPath.c_str());
                    std::string cmdLine;
                    getline(cmdFile, cmdLine);
                    if (!cmdLine.empty())
                    {
                        size_t pos = cmdLine.find('\0');
                        if (pos != std::string::npos)
                            cmdLine = cmdLine.substr(0, pos);
                        pos = cmdLine.rfind('/');
                        if (pos != std::string::npos)
                            cmdLine = cmdLine.substr(pos + 1);
                        if (strcmp(procName.c_str(), cmdLine.c_str()) == 0)
                        {
                            pid = id;
                        }
                    }
                }
            }
        }
        closedir(dp);
        return pid;
    }

    static bool startAppAsDaemon()
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            return false;
        }

        if (pid > 0)
            std::exit(EXIT_SUCCESS);

        if (setsid() < 0)
            std::exit(EXIT_FAILURE);

        // TODO: Implement a working signal handler */
        std::signal(SIGCHLD, SIG_IGN);
        std::signal(SIGHUP, SIG_IGN);

        pid = fork();
        if (pid < 0)
            std::exit(EXIT_FAILURE);

        if (pid > 0)
            std::exit(EXIT_SUCCESS);

        umask(0);
        auto retval = chdir("/test");
        if (retval < 0)
        {
            return false;
        }

        for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
        {
            close(x);
        }
        return true;
    }

    static uint64_t userAvailableFreeSpace()
    {
        struct statvfs stat;
        struct passwd *pw = getpwuid(getuid());
        if (nullptr != pw && 0 == statvfs(pw->pw_dir, &stat))
        {
            std::cout << "path " << pw->pw_dir << std::endl;
            uint64_t freeBytes = stat.f_bavail * stat.f_frsize;
            return freeBytes;
        }
        return 0ULL;
    }
    static int64_t getTemperature(const std::string &thermalZone = "thermal_zone0")
    {
        std::ifstream temperatureFile;
        const std::string parsePath = "/sys/class/thermal/" + thermalZone + "/temp";
        temperatureFile.open(parsePath);

        int64_t temperature;
        std::string line;
        while (std::getline(temperatureFile, line))
        {
            scanf(line.c_str(), "%ld", &temperature);
        }
        temperatureFile.close();
        return temperature;
    }

    static uint64_t getFreeDiskSpace(std::string absoluteFilePath)
    {
        struct statvfs buf;

        if (!statvfs(absoluteFilePath.c_str(), &buf))
        {
            uint64_t blksize, blocks, freeblks, disk_size, used, free;
            std::cout << "blksize :" << buf.f_bsize << std::endl;
            std::cout << "blocks :  " << buf.f_blocks;
            std::cout << "bfree :  " << buf.f_bfree;
            std::cout << "bavail: " << buf.f_bavail;
            std::cout << "f_frsize: " << buf.f_frsize;
            blksize = buf.f_bsize;
            blocks = buf.f_blocks;
            freeblks = buf.f_bfree;
            disk_size = blocks * blksize;
            free = freeblks * blksize;
            used = disk_size - free;

            std::cout << "disk " << absoluteFilePath << " disksize: " << disk_size << " free: " << free
                      << " used: " << used << std::endl;
            return free;
        }
        else
        {
            return 0;
        }
        return 0;
    }
    static uint64_t getSysUpTime()
    {
        std::ifstream upTimeFile;
        upTimeFile.open("/proc/uptime");

        if (!upTimeFile.is_open())
        {
            return 0;
        }

        uint64_t beforeBootTime;
        uint64_t sysUptime = 0;
        std::string line;
        while (std::getline(upTimeFile, line))
        {
            sscanf(line.c_str(), "%" PRIu64 "%" PRIu64, &sysUptime, &beforeBootTime);
        }
        upTimeFile.close();
        return sysUptime;
    }

    static uint32_t getNumOfThreadsByThisProcess()
    {
        uint32_t Threads = 0;
        std::ifstream memoryFile;
        memoryFile.open("/proc/self/status");
        std::string line;
        while (std::getline(memoryFile, line))
        {
            sscanf(line.c_str(), "Threads: %u", &Threads);
        }
        return Threads;
    }

    static uint32_t getNumOfThreadsByPID(int Pid)
    {
        uint32_t Threads = 0;
        std::ifstream memoryFile;
        memoryFile.open("/proc/self/" + std::to_string(Pid));
        std::string line;
        while (std::getline(memoryFile, line))
        {
            sscanf(line.c_str(), "Threads: %u", &Threads);
        }
        return Threads;
    }
};
