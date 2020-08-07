/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-05-18 20:03:43
 * @LastEditTime: 2020-08-07 16:42:30
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/events/SendPackageToManager.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: Event object for SendPackageToManager
 */

#pragma once
#include "SendPackage.hpp"
namespace Sloong
{
    namespace Events
    {
        class SendPackageToManagerEvent : public SendPackageEvent
        {
        public:
            SendPackageToManagerEvent(int func, string content) : SendPackageEvent(0)
            {
                m_emType = EVENT_TYPE::SendPackageToManager;
                FunctionID = func;
                Content = content;
            }
            virtual ~SendPackageToManagerEvent() {}

            inline int GetFunctionID() { return FunctionID; }
            inline const string &GetContent() { return Content; }

        protected:
            int FunctionID = 0;
            string Content = "";
        };

    } // namespace Events
} // namespace Sloong
