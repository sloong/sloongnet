/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 20:00:37
 * @Description: file content
 */
#pragma once

#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <map>
using namespace std;

#include "protocol/base.pb.h"
using namespace Base;

#include "protocol/core.pb.h"
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
        inline bool IsError() const {
            return m_emResult == ResultType::Error ? true : false;
        }
        inline bool IsSucceed() const {
            return m_emResult == ResultType::Succeed ? true : false;
        }
        inline bool IsFialed() const {
            return m_emResult == ResultType::Succeed ? false : true;
        }
        inline bool IsWarn() const {
            return m_emResult == ResultType::Warning ? true : false;
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
        static inline CResult Make_Warning(const string& what) {
            return CResult(ResultType::Warning, what);
        }
        static inline CResult Make_OK(const string& result) {
            return CResult(ResultType::Succeed, result);
        }
        static CResult Succeed;
        static CResult Invalid;
        static CResult Ignore;
        static CResult Retry;
    protected:
        ResultType m_emResult;
        string m_strMessage;
    };

    template<class T>
    class TResult : public CResult
    {
    public:
        TResult(const CResult& res):CResult(res){}
        TResult(CResult&& res):CResult(res){}
        TResult(ResultType res):CResult(res) {}
        TResult(ResultType res, const string& what):CResult(res,what){}
        TResult(ResultType res, const string& what, T result) :CResult(res, what) {
            m_HaveResultObject = true;
            m_tResultObject = move(result);
        }
        inline T& GetResultObject() {
            return m_tResultObject;
        }
        inline T MoveResultObject(){
            return std::move(m_tResultObject);
        }
        inline bool HaveResultObject(){
            return m_HaveResultObject;
        }
    public:
        static inline TResult Succeed(){
            return TResult(ResultType::Succeed );
        }
        static inline TResult Invalid(){
            return TResult(ResultType::Invalid );
        }
        static inline TResult Ignore(){
            return TResult(ResultType::Ignore );
        }
        static inline TResult Retry(){
            return TResult(ResultType::Retry );
        }
        static inline TResult Make_Error(const string& what) {
            return TResult(ResultType::Error, what );
        }
        static inline TResult Make_Warning(const string& what) {
            return TResult(ResultType::Warning, what);
        }
        static inline TResult Make_OK(const string& msg ) {
            return TResult(ResultType::Succeed, msg);
        }
        static inline TResult Make_OKResult(T result, const string& msg = "") {
            return TResult(ResultType::Succeed, msg, move(result));
        }
    protected:
        bool m_HaveResultObject = false;
        T m_tResultObject;
    };

    typedef TResult<uint64_t> NResult;
	typedef TResult<std::pair<char*,int>> PResult;
    typedef TResult<string> SResult;
    typedef TResult<vector<string>> VStrResult;
    typedef TResult<unique_ptr<DataPackage>> PackageResult;
} // namespace  Sloong
