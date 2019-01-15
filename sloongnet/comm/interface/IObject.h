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
             m_iC = iMsg;
             m_pLog = TYPE_TRANS<CLog*>(m_iC->Get(DATA_ITEM::Logger));
        }

    protected:
        IControl*   m_iC;
        CLog*       m_pLog;
    };
}