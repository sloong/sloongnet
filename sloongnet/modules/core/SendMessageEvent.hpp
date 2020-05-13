/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-13 16:26:13
 * @Description: file content
 */
#pragma once
#include "NetworkEvent.hpp"
#include "protocol/core.pb.h"
namespace Sloong
{
	class CDataTransPackage;
	namespace Events
	{
		class CSendMessageEvent : public CNetworkEvent
		{
		public:
			CSendMessageEvent(){m_emType = EVENT_TYPE::SendMessage; }
			virtual	~CSendMessageEvent(){}
			
			void SetRequest( SOCKET target, int serialnumber, int priority, int func, string content,  string extend = "", DataPackage_PackageType type = DataPackage_PackageType::DataPackage_PackageType_RequestPackage)
			{
				m_pData = make_shared<DataPackage>();
				m_pData->set_type(type);
				m_pData->set_function(func);
				m_pData->set_content(content);
				m_pData->set_extend(extend);
				m_pData->set_prioritylevel(priority);
				m_pData->set_serialnumber(serialnumber);

				m_nSocketID = target;
			}

			shared_ptr<DataPackage> GetDataPackage()
			{
				return m_pData;
			}
		protected:
			shared_ptr<DataPackage> m_pData;
		};

	}
}

