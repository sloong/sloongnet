/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 20:03:43
 * @Description: Event object for SendPackageToManager
 */
#pragma once
#include "NormalEvent.hpp"
namespace Sloong
{
    namespace Events
    {
        class SendPackageToManagerEvent : public CNormalEvent
        {
        public:
            SendPackageToManagerEvent(int func, string content) : CNormalEvent(EVENT_TYPE::SendPackageToManager)
            {
                FunctionID = func;
                Content = content;
            }
            virtual ~SendPackageToManagerEvent() {}

            inline void SetCallbackFunc(std::function<void(IEvent *, DataPackage *)> p) { m_pCallback = p; }
            inline void CallCallbackFunc(DataPackage *p)
            {
                if (m_pCallback)
                    m_pCallback(this, p);
            }
            inline bool HaveCallbackFunc() { return m_pCallback != nullptr; }

            inline int GetFunctionID() { return FunctionID; }
            inline const string &GetContent() { return Content; }

        protected:
            int FunctionID = 0;
            string Content = "";
            std::function<void(IEvent *, DataPackage *)> m_pCallback = nullptr;
        };

    } // namespace Events
} // namespace Sloong
