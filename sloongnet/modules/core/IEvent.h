/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-16 16:25:07
 * @Description: file content
 */
#ifndef SLOONGNET_INTERFACE_EVENT_H
#define SLOONGNET_INTERFACE_EVENT_H

#include <memory>
using namespace std;

#include "defines.h"
// 事件的接口
// 由于涉及到多处理函数，而且释放必须由最后一个使用者来释放
// 所以将由框架在发送之前，根据处理函数的个数来自动调用AddRef函数来增加计数。
// 要求处理函数在处理之后，手动调用SAFE_RELEASE_EVENT宏来进行释放。


namespace Sloong
{
	class IEvent
	{
	public:
		IEvent() {}
		virtual ~IEvent() {}
		virtual EVENT_TYPE GetEvent() = 0;
	protected:
		int m_nRefCount = 0;
	};
	template<typename T> inline
	T EVENT_TRANS(IEvent* p)
	{
		T tmp = dynamic_cast<T>(p);
		assert(tmp);
		return tmp;
	}
}

#endif