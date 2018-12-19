#pragma once

#include "main.h"

#include "IMessage.h"

namespace Sloong
{
	class IObject
	{
    public:
        void Initialize(IMessage* iMsg)
        {
             m_iMsg = iMsg;
             m_pLog = (CLog*)m_iMsg->Get(Logger);
        }

    protected:
        IMessage*   m_iMsg;
        CLog*       m_pLog;
    };
}