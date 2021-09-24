/**
 * @author: Daniel Fuchs
 * @contact: fuxeysolutions@gmail.com
 *
 * distributed under the MIT License (MIT).
 * Copyright (c) Daniel Fuchs
 *
 */
#include <iostream>
#include <csignal>
#include <memory>
#include <atomic>
#include "lib/linux_memoryload.hpp"
#include "lib/linux_cpuload.hpp"
#include "lib/linux_process_load.hpp"
#include "lib/linux_networkload.hpp"
#include "lib/util/record_value.hpp"
#include "lib/util/timer.hpp"
#include <thread>
#include <lib/linux_systemutil.hpp>



static void signalHandler(int signum) {
    std::cerr << "Signal " << signum<<  " was catched, shutdown app" << std::endl;
    Timer::stop();
}

static void installHandler() {
    std::signal(SIGKILL, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGPIPE, signalHandler);
}


int main(int argc, char *argv[]) {

    (void)argc;
    (void)argv;
    installHandler();



    auto processes = std::make_unique<linuxProcessLoad>();
    auto memoryMonitoring = std::make_unique<memoryLoad>();
    auto cpuMonitoring = std::make_unique<cpuLoad>("/proc/stat");
    auto ethernetMonitoring = networkLoad::createLinuxEthernetScanList();
    cpuMonitoring->initCpuUsage();


    for(auto elem: ethernetMonitoring) {
        std::cout << "get Mac adr of dev: " << elem->getDeviceName() << " : " << linuxUtil::getIFaceMacAddress(elem.get()->getDeviceName()) << std::endl;
    }


    // print process cpu load > 1.0%
    Timer::periodicShot([&processes](){
        auto procCPUUsage = processes->getProcessCpuLoad();
        for (const auto& elem: procCPUUsage ) {
            if(elem.second > 1.0) {
                std::cout   << " proc: " << elem.first << " usage: "
                            << elem.second <<  " % "
                            << std::endl;
            }
        }
    }, std::chrono::milliseconds(3003));

    // print cpu usage of all cpu cores
    Timer::periodicShot([&] {
        auto cpus = cpuMonitoring->getCurrentMultiCoreUsage();
        uint32_t i{0};
        for(auto elem: cpus) {
            std::cout << "cpu" << std::to_string(i++) << ": " << std::to_string(elem) << "%  ";
        }
        std::cout << std::endl;
    }, std::chrono::milliseconds(5007));




    auto recordTest = std::make_unique<recordValue<double>>(std::chrono::hours(1), std::chrono::seconds(1));
    auto recordTest2 = std::make_unique<recordValue<double>>(std::chrono::hours(6), std::chrono::minutes(5));

    // print cpu usage + calculate average + min&max
    Timer::periodicShot([&](){
        double currentCpuLoad = cpuMonitoring->getCurrentCpuUsage();
        recordTest->addRecord(currentCpuLoad);

        std::cout << "----------------------------------------------" << std::endl
                  << " current CPULoad:" << currentCpuLoad << std::endl
                  << " average CPULoad " << recordTest->getAverageRecord() << std::endl
                  << " Max     CPULoad " << recordTest->getMaxRecord() << std::endl
                  << " Min     CPULoad " << recordTest->getMinRecord() << std::endl
                  << " CPU: " <<   cpuMonitoring->getCPUName() << std::endl;

    },std::chrono::milliseconds (1003));

    Timer::periodicShot([&]() {
        recordTest2->addRecord(recordTest->getAverageRecord());
    }, std::chrono::minutes(5));


    // print memory load
    Timer::periodicShot([&](){
        std::cout << "----------------------------------------------" ;
        std::cout << "----------------------------------------------" << std::endl;
        std::cout   << " memory load: " << memoryMonitoring->getCurrentMemUsageInPercent() << "% maxmemory: "
                    << memoryMonitoring->getTotalMemoryInKB() << " Kb used: " << memoryMonitoring->getCurrentMemUsageInKB() << " Kb  Memload of this Process "
                    << memoryMonitoring->getMemoryUsageByThisProcess() << " KB "
                    <<  std::endl;
    }, std::chrono::milliseconds (2009));

    // print networkload of "wlp0s20f3"
    Timer::periodicShot([&]() {
        for(auto elem: ethernetMonitoring) {
            // get available networkdevices with command ifconfig
            if(elem->getDeviceName() == "wlp0s20f3") {
                std::cout << "----------------------------------------------" << std::endl;
                std::cout << " network load: " << elem->getDeviceName() << " : "
                          << elem->getBitsPerSeceondString(elem->getParamPerSecond(networkLoad::mapEnumToString(networkLoad::RXbytes))) << " : "
                          << elem->getBitsPerSeceondString(elem->getParamPerSecond(networkLoad::mapEnumToString(networkLoad::TXbytes))) << " : "
                          << " RX Bytes Startup: " << elem->getBytesString(elem->getParamSinceStartup(networkLoad::mapEnumToString(networkLoad::RXbytes)))
                          << " TX Bytes Startup: " << elem->getBytesString(elem->getParamSinceStartup(networkLoad::mapEnumToString(networkLoad::TXbytes)))
                          << std::endl;
            }
        }
    }, std::chrono::milliseconds (5007));

    while(Timer::isRunning()) {
        std::this_thread::sleep_for(std::chrono::minutes (5));
        Timer::stop();
        std::cout << " Bye bye!" << std::endl;
    }

    std::exit(1);
}