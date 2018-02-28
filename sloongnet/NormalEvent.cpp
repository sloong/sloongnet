#include "NormalEvent.h"

using namespace Sloong::Events;

CNormalEvent::CNormalEvent()
{

}

CNormalEvent::~CNormalEvent()
{
	if( m_bReleaseWhenShutdown )
		SAFE_DELETE_ARR(m_pParams);
}

void CNormalEvent::SetEvent(MSG_TYPE t)
{
	m_emType = t;
}

MSG_TYPE CNormalEvent::GetEvent()
{
	return m_emType;
}

void CNormalEvent::SetParams(LPVOID p, bool bRelase)
{
	m_pParams = p;
	m_bReleaseWhenShutdown = bRelase;
}

LPVOID CNormalEvent::GetParams()
{
	return m_pParams;
}

void CNormalEvent::SetCallbackFunc(LPCALLBACK2FUNC func)
{
	m_pCallbackFunc = func;
}

LPCALLBACK2FUNC CNormalEvent::GetCallbackFunc()
{
	return m_pCallbackFunc;
}

void CNormalEvent::SetProcessingFunc(LPCALLBACK2FUNC func)
{
	m_pProcessingFunc = func;
}

LPCALLBACK2FUNC CNormalEvent::GetProcessingFunc()
{
	return m_pProcessingFunc;
}


void Sloong::Events::CNormalEvent::SetMessage(string str)
{
	m_strMessage = str;
}

string Sloong::Events::CNormalEvent::GetMessage()
{
	return m_strMessage;
}

void Sloong::Events::CNormalEvent::CallCallbackFunc(LPVOID pParams)
{
	if( m_pCallbackFunc )
		(*m_pCallbackFunc)(pParams, m_pObj);
}

LPVOID Sloong::Events::CNormalEvent::GetHandler()
{
	return m_pObj;
}

void Sloong::Events::CNormalEvent::SetHandler(LPVOID obj)
{
	m_pObj = obj;
}
