#pragma once

#include "main.h"

#include "IMessage.h"
#include "IData.h"

namespace Sloong
{
	namespace Interface
	{
		class IObject
		{
        public:
            void Initialize(IMessage* iMsg, IData* iData)
            {
                m_iData = iData;
                m_iMsg = iMsg;
                m_pLog = (CLog*)m_iData->Get(Logger);
            }

        protected:
            IMessage*   m_iMsg;
            IData*      m_iData;
            CLog*       m_pLog;
        };
    }
}