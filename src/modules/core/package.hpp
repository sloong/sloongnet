/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-01-11 10:44:53
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/package.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#pragma once

#include "protocol/base.pb.h"
using namespace Base;

#include <list>
using namespace std;

namespace Sloong
{
    class Package : public DataPackage
    {
    public:
        uint64_t sessionid() { return SessionID; }
        void set_sessionid(uint64_t sessionid) { SessionID = sessionid; }
        list<uint64_t> clocks() { return Clocks; }
        void add_clocks(uint64_t clock) { Clocks.push_back(clock); }

    private:
        uint64_t SessionID = 0;
        list<uint64_t> Clocks;
    };

    typedef shared_ptr<Package> SmartPackage;
    typedef unique_ptr<Package> UniquePackage;

} // namespace Sloong