/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-26 12:04:27
 * @Description: file content
 */
#pragma once

#include <mutex>
#include <condition_variable>
#include <chrono>
using namespace std;

namespace Sloong
{

	class CEasySync
	{
	public:
		inline void wait()
		{
			unique_lock<mutex> lck(m_oMutex);
			m_oCV.wait(lck);
		}
		// 返回值：
		//  true ：时间触发返回
		//  false ：超时返回
		inline bool wait_for(int ms)
		{
			unique_lock<mutex> lck(m_oMutex);
			if (m_oCV.wait_for(lck, chrono::milliseconds(ms)) == std::cv_status::timeout)
			{
				return false;
			}
			else
				return true;
		}
		inline void notify_one()
		{
			m_oCV.notify_one();
		}
		inline void notify_all()
		{
			m_oCV.notify_all();
		}

	protected:
		condition_variable m_oCV;
		mutex m_oMutex;
	};

} // namespace Sloong
