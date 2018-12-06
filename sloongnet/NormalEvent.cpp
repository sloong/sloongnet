#include "NormalEvent.h"

using namespace Sloong::Events;

CNormalEvent::CNormalEvent()
{
}

CNormalEvent::~CNormalEvent()
{
}

void CNormalEvent::SetEvent(MSG_TYPE t)
{
	m_emType = t;
}

MSG_TYPE CNormalEvent::GetEvent()
{
	return m_emType;
}

void CNormalEvent::SetParams(SMARTER p)
{
	m_pParams = p;
}

SMARTER CNormalEvent::GetParams()
{
	return m_pParams;
}

void CNormalEvent::SetCallbackFunc(LPSMARTFUNC func)
{
	m_pCallbackFunc = func;
}

LPSMARTFUNC CNormalEvent::GetCallbackFunc()
{
	return m_pCallbackFunc;
}


void Sloong::Events::CNormalEvent::SetMessage(string str)
{
	m_strMessage = str;
}

string Sloong::Events::CNormalEvent::GetMessage()
{
	return m_strMessage;
}

void Sloong::Events::CNormalEvent::CallCallbackFunc(SMARTER pParams)
{
	if( m_pCallbackFunc )
		(*m_pCallbackFunc)(pParams);
}

LPVOID Sloong::Events::CNormalEvent::GetHandler()
{
	return m_pObj;
}

void Sloong::Events::CNormalEvent::SetHandler(LPVOID obj)
{
	m_pObj = obj;
}
