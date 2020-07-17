/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-17 15:30:42
 * @Description: file content
 */
#ifndef SLOONGNET_INTERFACE_OBJECT_H
#define SLOONGNET_INTERFACE_OBJECT_H

#include "IControl.h"

namespace Sloong
{
    class IObject
    {
    public:
        inline void Initialize(IControl *iMsg)
        {
            m_iC = iMsg;
            m_pLog = STATIC_TRANS<CLog *>(m_iC->Get(DATA_ITEM::Logger));
        }

    protected:
        IControl *m_iC;
        CLog *m_pLog;
    };
} // namespace Sloong

#endif // SLOONGNET_INTERFACE_OBJECT_H