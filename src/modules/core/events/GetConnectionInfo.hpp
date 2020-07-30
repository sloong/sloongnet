/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-05-14 17:43:08
 * @LastEditTime: 2020-07-24 16:40:17
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/events/GetConnectionInfo.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: GetConnectionInfoEvent object
 */

#pragma once
#include "NormalEvent.hpp"
namespace Sloong
{
    typedef struct ConnectionInfo
    {
        string Address;
        int Port;
    } ConnectionInfo;
    namespace Events
    {
        class GetConnectionInfoEvent : public NormalEvent
        {
        public:
            GetConnectionInfoEvent(int64_t session) : NormalEvent(EVENT_TYPE::GetConnectionInfo)
            {
                m_SessionID = session;
            }
            virtual ~GetConnectionInfoEvent() {}

            inline void SetCallbackFunc(std::function<void(IEvent *, ConnectionInfo)> p) { m_pCallback = p; }
            inline void CallCallbackFunc(ConnectionInfo info)
            {
                if (m_pCallback)
                    m_pCallback(this, info);
            }
            inline bool HaveCallbackFunc() { return m_pCallback != nullptr; }

            inline int64_t GetSessionID() { return m_SessionID; }

        protected:
            int64_t m_SessionID;
            std::function<void(IEvent *, ConnectionInfo)> m_pCallback = nullptr;
        };
    } // namespace Events
} // namespace Sloong