#pragma once

#include "main.h"

#include "IControl.h"

namespace Sloong
{
	class IObject
	{
    public:
        void Initialize(IControl* iMsg)
        {
             m_iMsg = iMsg;
             m_pLog = (CLog*)m_iMsg->Get(Logger);
        }

    protected:
        IControl*   m_iMsg;
        CLog*       m_pLog;
    };
}