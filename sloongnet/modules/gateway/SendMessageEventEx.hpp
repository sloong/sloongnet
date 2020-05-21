/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 20:23:47
 * @Description: file content
 */
#pragma once
#include "SendMessageEvent.hpp"
namespace Sloong
{
	namespace Events
	{
        struct RequestInfo{
            int function;
			int priority;
			int SerialNumber;
			string Sender;
            EasyConnect*    RequestConnect = nullptr;
        };
		class CSendPackageExEvent : public CSendPackageEvent
		{
		public:
			CSendPackageExEvent(){}
			virtual	~CSendPackageExEvent(){}

			inline void SetRequestInfo(EasyConnect* conn, int func, int priority, int serial, string sender){
                m_RequestInfo.RequestConnect = conn;
                m_RequestInfo.function = func;
				m_RequestInfo.priority = priority;
				m_RequestInfo.SerialNumber = serial;
				m_RequestInfo.Sender = sender;
            }
            inline RequestInfo* GetRequestInfo(){
                return &m_RequestInfo;
            }
		protected:
			RequestInfo         m_RequestInfo;
		};

	}
}

