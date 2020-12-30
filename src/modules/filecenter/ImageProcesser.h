/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-08-17 11:16:12
 * @LastEditTime: 2020-08-17 11:19:00
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/filecenter/ImageProcesser.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#pragma once

#include "core.h"
#include "protocol/filecenter.pb.h"
using namespace FileCenter;

// cimag
// #define cimg_display 0
// #include "CImg.h"
// using namespace cimg_library;

//#include "jpeg.h"

namespace Sloong
{
    class ImageProcesser
    {
    public:
        static CResult ConvertFormat(const string &, const string &, FileCenter::SupportFormat );
        static CResult GetThumbnail(const string &, const string &, int , int , int );
    };
} // namespace Sloong
