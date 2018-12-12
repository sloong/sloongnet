#include "DataTransPackage.h"

#include "main.h"

Sloong::CDataTransPackage::CDataTransPackage(SmartConnect conn)
{
    m_pConn = conn;
}


/**
 * @Remarks: 
 * @Params: 
 * @Return: if send succeed, return true. need listen read message. 
 *          else return false, need listen write message.
 */
// bool Sloong::CDataTransPackage::SendPackage(SmartConnect pCon){
//     unique_lock<mutex> lck(pInfo->m_oSockSendMutex);

// 	// 首先检查是不是已经发送过部分的数据了
// 	if( nSent > 0 )
// 	{
// 		// 先检查普通数据发送状态
// 		if( nSent < nSize)
// 		{
// 			int nSentSize = pCon->Write(pSendBuffer, nSize, nSent);
// 			if( nSentSize < 0 )
// 			{
// 				return -1;
// 			}
// 			else
// 			{
// 				nSent = nSentSize;
// 			}

// 		}
// 		// 已经发送完普通数据了，需要继续发送扩展数据
// 		if ( nSent >= nSize && nExSize > 0 )
// 		{
// 			int nSentSize = pCon->Write(pExBuffer, nExSize, nSent - nSize);
// 			if( nSentSize < 0 )
// 			{
// 				return -1;
// 			}
// 			else
// 			{
// 				nSent = nSize + nSentSize;
// 			}
// 		}
// 	}
// 	else
// 	{
// 		// send normal data.
// 		nSent = m_pCon->Write( pSendBuffer, nSize, nSent);
// 		// when send nurmal data succeeded, try send exdata in one time.
// 		if (nSent != -1 && nSent == nSize && nExSize > 0)
// 		{
// 			int nSentSize = m_pCon->Write(pExBuffer, nExSize, 0);
// 			if( nSentSize < 0 )
// 			{
// 				return -1;
// 			}
// 			else
// 			{
// 				nSent = nSize + nSentSize;
// 			}
// 		}
// 	}
// 	m_pLog->Verbos(CUniversal::Format("Send Info : AllSize[%d],ExSize[%d],Sent[%d]", nPackSize, nExSize, nSent));

// 	// check send result.
// 	// send done, remove the is sent data and try send next package.
// 	if (nSent < nPackSize)
// 	{
// 		return 0;
// 	}
// 	else
// 	{
// 		m_pLog->Verbos(CUniversal::Format("Message package send succeed, remove from send list. All size[%d]", nSent));
// 		return 1;
// 	}
// }


bool Sloong::CDataTransPackage::RecvPackage(ULONG dtlen )
{

			char* data = new char[dtlen + 1];
			memset(data, 0, dtlen + 1);

            // TODO: 对于超时时间不需要这样频繁设置，可以直接在连接建立之后由该连接自己保存
			//nRecvSize = m_pCon->Read(data, dtlen, m_pConfig->m_nReceiveTimeout, true);//一次性接受所有消息
			int nRecvSize = m_pConn->Read(data, dtlen, 5, true);//一次性接受所有消息
			if (nRecvSize < 0)
			{
				return false;
			}
			else if(nRecvSize != dtlen )
			{
				//m_pLog->Warn(CUniversal::Format("Receive all data is timeout. recved lenght %d, data length %d",nRecvSize, dtlen));
				return false;
			}

            // TODO: 这里对于优先级的设置以及使用还是有一些问题的。目前优先级只是用在了发送，处理并未使用到优先级
			const char* pMsg = NULL;
			// check the priority level
			if (m_nPriorityLevel != 0)
			{
				char pLevel[2] = { 0 };
				pLevel[0] = data[0];
				int level = pLevel[0];
				if (level > m_nPriorityLevel || level < 0)
				{
					//m_pLog->Error(CUniversal::Format("Receive priority level error. the data is %d, the config level is %d. add this message to last list", level, m_nPriorityLevel));
                    return false;
				}
				else
				{
					nPriority = level;
				}
				pMsg = &data[1];
			}
			else
			{
				nPriority = 0;
				pMsg = data;
			}

            // TODO: 这里对于优先级，以及流水号和MD5这些的配置项有些不太合理，这里先暂时直接在构造函数中传进来，后面考虑怎么优化处理
			if (m_bEnableSwiftNumberSup)
			{
                // TODO: 接收长度信息这个可以直接在lConnect这个类里直接集成
                char pLongBuffer[s_llLen+1] = {0};
				memcpy(pLongBuffer, pMsg, s_llLen);
				nSwiftNumber = CUniversal::BytesToLong(pLongBuffer);
				pMsg += s_llLen;
			}

			if (m_bEnableMD5Check)
			{
				char tmd5[33] = { 0 };
				memcpy(tmd5, pMsg, 32);
				strMD5 = tmd5;
				pMsg += 32;
			}

			strMessage = string(pMsg);

			// Add the msg to the sock info list
			SAFE_DELETE_ARR(data);


			
			

}


string Sloong::CDataTransPackage::GetRecvMessage(){

}

void Sloong::CDataTransPackage::SetSendMessage(const string& msg, const char* exData, int exSize ){

}