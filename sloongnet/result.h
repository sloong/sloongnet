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
        CResult(ResultType res) {
            m_emResult = res;
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
        
    public:
        static CResult Make_Error(const string& what) {
            return CResult(ResultType::Error, what);
        }
        static CResult Make_OK(const string& result) {
            return CResult(ResultType::Succeed, result);
        }
    protected:
        ResultType m_emResult;
        string m_strMessage;
        
    public: 
        static CResult Succeed;
    };


    template<class T>
    class TResult : public CResult
    {
    public:
        TResult(ResultType res, const string& what):CResult(res,what){}
        TResult(ResultType res, const string& what, T result) :CResult(res, what) {
            m_tResultObject = result;
        }
        T ResultObject() const {
            return m_tResultObject;
        }
        static TResult Make_Error(const string& what) {
            return TResult(ResultType::Error, what );
        }
        static TResult Make_OK(T result) {
            return TResult(ResultType::Succeed, "", result);
        }
    protected:
        T m_tResultObject;
    };
} // namespace  Sloong


#endif