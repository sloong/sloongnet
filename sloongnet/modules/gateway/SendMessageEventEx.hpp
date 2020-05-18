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
            shared_ptr<DataPackage> RequestDataPackage = nullptr;
            SmartConnect    RequestConnect = nullptr;
        };
		class CSendPackageExEvent : public CSendPackageEvent
		{
		public:
			CSendPackageExEvent(){}
			virtual	~CSendPackageExEvent(){}

			inline void SetRequestInfo(SmartConnect conn, shared_ptr<DataPackage> pack){
                m_RequestInfo.RequestConnect = conn;
                m_RequestInfo.RequestDataPackage = pack;
            }
            inline RequestInfo* GetRequestInfo(){
                return &m_RequestInfo;
            }
		protected:
			RequestInfo         m_RequestInfo;
		};

	}
}

