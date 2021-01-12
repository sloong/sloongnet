/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-01-12 14:47:57
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/package.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: The extend class for DataPackage.
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

        /*** 
         * @description: 这个函数只针对需要提前计算正确的包大小的情况，因为hash字段是在发送之前由Connect对象设置的，所以会先检查Hash字段是否设置过，如果没有则会设置空进去，然后计算大小之后再清空hash。
         */
        size_t ByteSizeLongEx(){
            auto len = ByteSizeLong();
            if( this->hash().length() == 0 )
            {
                set_hash(string(32,0x00));
                len = ByteSizeLong();
                clear_hash();
            }
            return len;
        }

    private:
        uint64_t SessionID = 0;
        list<uint64_t> Clocks;
    };

    typedef shared_ptr<Package> SmartPackage;
    typedef unique_ptr<Package> UniquePackage;

} // namespace Sloong