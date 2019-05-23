#ifndef SLOONGNET_RESULT_H
#define SLOONGNET_RESULT_H

#include "stdafx.h"
namespace Sloong
{
    
enum NetworkResult
{
	Succeed = 1,
	Retry = 0,
	Error = -1,
	Invalid = -2,
};


    class CResult
    {
    public:
        CResult(bool res){
            m_emResult = res ? NetworkResult::Succeed : NetworkResult::Error;
        }
        CResult(NetworkResult res){
            m_emResult = res;
        }
        CResult(bool res, const string& what){
            m_emResult = res ? NetworkResult::Succeed : NetworkResult::Error;
            m_strMessage = what;
        }
        CResult(NetworkResult res, const string& what){
            m_emResult = res;
            m_strMessage = what;
        }

        NetworkResult Result(){
            return m_emResult;
        }

        bool IsSucceed(){
            return m_emResult == NetworkResult::Succeed ? true : false;
        }
        string Message(){
            return m_strMessage;
        }
    protected:
        NetworkResult m_emResult; 
        string m_strMessage;

    public: 
        static CResult Succeed;

    };
} // namespace  Sloong


#endif