#ifndef SLOONGNET_RESULT_H
#define SLOONGNET_RESULT_H

#include "stdafx.h"
namespace Sloong
{
    
enum ResultEnum
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
            m_emResult = res ? ResultEnum::Succeed : ResultEnum::Error;
        }
        CResult(ResultEnum res){
            m_emResult = res;
        }
        CResult(bool res, const string& what){
            m_emResult = res ? ResultEnum::Succeed : ResultEnum::Error;
            m_strMessage = what;
        }
        CResult(ResultEnum res, const string& what){
            m_emResult = res;
            m_strMessage = what;
        }

        ResultEnum Result(){
            return m_emResult;
        }

        bool IsSucceed(){
            return m_emResult == ResultEnum::Succeed ? true : false;
        }
        string Message(){
            return m_strMessage;
        }
    protected:
        ResultEnum m_emResult; 
        string m_strMessage;

    public: 
        static CResult Succeed;

    };
} // namespace  Sloong


#endif