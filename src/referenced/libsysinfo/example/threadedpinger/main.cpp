/**
 * @author: Daniel Fuchs
 * @contact: fuxeysolutions@gmail.com
 *
 * distributed under the MIT License (MIT).
 * Copyright (c) Daniel Fuchs
 *
 */

#include <map>
#include <iostream>
#include <csignal>
#include <atomic>

#include <lib/util/timer.hpp>
#include <lib/util/ip_iterator.hpp>
#include <lib/linux_systemutil.hpp>


std::atomic_bool run;

static void signalHandler(int signum) {
    std::cerr << "Signal " << signum<<  " was catched, shutdown app" << std::endl;
    run = false;
}

static void installHandler() {
    std::signal(SIGKILL, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGPIPE, signalHandler);
}


class PingService: public virtual ITimerObserver {
public:

    PingService(std::chrono::milliseconds ms, ipv4_Range::ipv4_Address start,
                                 ipv4_Range::ipv4_Address end) {
        this->ipRange = std::make_unique<ipv4_Range>(start,end);
        this->interval = ms;
        Timer::createTimer()->Attach(this,continuous, interval);
    }

    ~PingService() {
        Timer::createTimer()->Detach(this);
    }

    void Update() override {
        this->pingRange();
    }

private:
    std::map<std::string, bool> result;
    std::unique_ptr<ipv4_Range> ipRange;
    std::chrono::milliseconds interval;


    void pingRange() {

        auto hwThreads = std::thread::hardware_concurrency();
        std::cout << "max threads" << hwThreads << std::endl;

        if(hwThreads == 0) {
            hwThreads = 8;
        }

        std::atomic<uint8_t> threadCnt = 0;
        std::mutex resultLock;


        auto isDeviceOnline = [&](std::string ip) {

            bool ok = linuxUtil::isDeviceOnline(ip);
            std::lock_guard<std::mutex> guard(resultLock);
            this->result[ip] = ok;
            threadCnt--;
        };


        auto detachedPing = [&](auto ipv4) {
            auto t = new std::thread(isDeviceOnline, ipv4.toString());
            t->detach();
            threadCnt++;
        };




        for(auto ipv4: *this->ipRange) {
            if(threadCnt < hwThreads) {
                detachedPing(ipv4);
            } else {
                while (threadCnt >= hwThreads) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                detachedPing(ipv4);
            }
        }
        while(threadCnt != 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }




        uint32_t cnt = 1;
        for(auto pair: result) {
            if(pair.second) {
                std::cout << "Device[ " << cnt++ << "]:" << pair.first << " is Online" << std::endl;
            }
        }

        for(auto pair: result) {
            if(!pair.second) {
                std::cout  << "Device[ " << cnt++ << "]:"  << pair.first << " is Offline" << std::endl;
            }
        }


    }

};





int main(int argc, char *argv[]) {

    (void) argc;
    (void) argv;
    installHandler();
    run = true;


    ipv4_Range::ipv4_Address start(192, 168, 1, 1);
    ipv4_Range::ipv4_Address end(192, 168, 1, 255 );

    auto pinger = std::make_unique<PingService>(std::chrono::minutes(1),start,end);

    while(Timer::isRunning()) {
        std::this_thread::sleep_for(std::chrono::minutes (1));
        Timer::stop();
    }



}
