#include "EasySync.h"

#include <chrono>
using namespace Sloong;

Sloong::CEasySync::CEasySync()
{

}

void Sloong::CEasySync::wait()
{
	unique_lock<mutex> lck(m_oMutex);
	m_oCV.wait(lck);
}

bool Sloong::CEasySync::wait_for(int nSecond)
{
	unique_lock<mutex> lck(m_oMutex);
	if (m_oCV.wait_for(lck, chrono::seconds(nSecond)) == std::cv_status::timeout)
		return false;
	else
		return true;
}

void Sloong::CEasySync::notify_one()
{
	m_oCV.notify_one();
}

void Sloong::CEasySync::notify_all()
{
	m_oCV.notify_all();
}
