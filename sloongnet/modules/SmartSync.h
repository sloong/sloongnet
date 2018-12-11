#pragma once

#include <mutex>
#include <condition_variable>
using namespace std;

namespace Sloong
{

	class lSmartSync
	{
	public:
		lSmartSync();
		~lSmartSync() {}

		void wait();
		// 返回值：
		//  true ：时间触发返回 
		//  false ：超时返回 
		bool wait_for(int nSecond);
		void notify_one();
		void notify_all();

	protected:
		condition_variable m_oCV;
		mutex m_oMutex;
	};

}
