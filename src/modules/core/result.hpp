/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 20:00:37
 * @Description: file content
 */
#ifndef SLOONGNET_RESULT_H
#define SLOONGNET_RESULT_H

#include "core.h"
using namespace Core;
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
        inline ResultType GetResult() const {
            return m_emResult;
        }
        inline void SetResult(ResultType res){
            m_emResult = res;
        }
        inline bool IsSucceed() const {
            return m_emResult == ResultType::Succeed ? true : false;
        }
        inline bool IsFialed() const {
            return m_emResult == ResultType::Succeed ? false : true;
        }
        inline void SetMessage(const string& str){
            m_strMessage = str;
        }
        inline const string& GetMessage() const {
            return m_strMessage;
        }
    public:
        static inline CResult Make_Error(const string& what) {
            return CResult(ResultType::Error, what);
        }
        static inline CResult Make_OK(const string& result) {
            return CResult(ResultType::Succeed, result);
        }
        static inline CResult Succeed(){
            return CResult(ResultType::Succeed);
        }
        static inline CResult Invalid(){
            return CResult(ResultType::Invalid);
        }
    protected:
        ResultType m_emResult;
        string m_strMessage;
        
    };

    template<class T>
    class TResult : public CResult
    {
    public:
        TResult(ResultType res, const string& what):CResult(res,what){}
        TResult(ResultType res, const string& what, T result) :CResult(res, what) {
            m_tResultObject = result;
        }
        inline T& GetResultObject() {
            return m_tResultObject;
        }
        static inline TResult Make_Error(const string& what) {
            return TResult(ResultType::Error, what );
        }
        static inline TResult Make_OK(T& result, const string& msg = "") {
            return TResult(ResultType::Succeed, msg, result);
        }
    protected:
        T m_tResultObject;
    };

    typedef TResult<int> NResult;
	typedef TResult<std::pair<char*,int>> PResult;
    typedef TResult<string> SResult;
    typedef TResult<vector<string>> VStrResult;
} // namespace  Sloong


#endif