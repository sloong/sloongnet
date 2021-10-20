/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2021-10-20 15:13:23
 * @LastEditTime: 2021-10-20 15:13:24
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/extend/logger_ex.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description:
 */

#include "std.h"

using namespace spdlog;

class logger_ex : public logger
{
  public:
    logger_ex(std::shared_ptr<logger> base) : logger(*base)
    {
    }

    void set_name(const string& name)
    {
        logger::name_ = name;
    }

    void add_sink(shared_ptr<sinks::sink> sink)
    {
        logger::sinks_.emplace_back(sink);
    }
};