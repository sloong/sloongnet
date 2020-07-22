/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 17:44:45
 * @Description: file content
 */
#pragma once
#include "NormalEvent.hpp"
#include "core.h"
namespace Sloong
{
	class CDataTransPackage;
	namespace Events
	{
		class CNetworkEvent : public NormalEvent
		{
		public:
			CNetworkEvent(EVENT_TYPE t):NormalEvent(t){}
			virtual	~CNetworkEvent(){}

			inline int GetSocketID() { return m_nSocketID; }
			inline void SetSocketID(int id) { m_nSocketID = id; }

			inline shared_ptr<CDataTransPackage> GetDataPackage() { return m_pData; };
			inline void SetDataPackage(shared_ptr<CDataTransPackage> data) { m_pData = data; };
		protected:
			int m_nSocketID;
			shared_ptr<CDataTransPackage> m_pData;
		};

	}
}

