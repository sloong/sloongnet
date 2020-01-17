#ifndef SLOONGNET_RESULT_H
#define SLOONGNET_RESULT_H

#include "stdafx.h"
#include "protocol/protocol.pb.h"
using namespace Protocol;
namespace Sloong
{
    class CResult
    {
    public:
        CResult(bool res){
            m_emResult = res ? ResultType::Succeed : ResultType::Error;
        }
        CResult(ResultType res){
            m_emResult = res;
        }
        CResult(bool res, const string& what){
            m_emResult = res ? ResultType::Succeed : ResultType::Error;
            m_strMessage = what;
        }
        CResult(ResultType res, const string& what){
            m_emResult = res;
            m_strMessage = what;
        }

        ResultType Result() const {
            return m_emResult;
        }

        bool IsSucceed() const {
            return m_emResult == ResultType::Succeed ? true : false;
        }

        bool IsFialed() const {
            return m_emResult == ResultType::Succeed ? false : true;
        }
        string Message() const {
            return m_strMessage;
        }
    protected:
        ResultType m_emResult;
        string m_strMessage;

    public: 
        static CResult Succeed;

    };
} // namespace  Sloong


#endif