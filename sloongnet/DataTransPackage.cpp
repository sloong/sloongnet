#include "DataTransPackage.h"

void Sloong::CDataTransPackage::Initialize(SmartConnect conn, CLog* pLog)
{
	m_pCon = conn;	
	m_pLog = pLog;
}

void Sloong::CDataTransPackage::PrepareSendPackageData()
{
	if (m_pLog!= nullptr)
	{
		m_pLog->Verbos(CUniversal::Format("SEND<<<[%d][%d]<<<%s&&&EXDATA<<<[%d]",m_pTransPackage->prioritylevel(),
										m_pTransPackage->serialnumber(),m_pTransPackage->content(), m_pTransPackage->extend().length()));
	}
	m_pTransPackage->SerializeToString(&m_strPackageData);
	m_nPackageSize = (int)m_strPackageData.length();
}

void Sloong::CDataTransPackage::RequestPackage( shared_ptr<MessagePackage> pack )
{
	m_pTransPackage = pack;
	PrepareSendPackageData();
}


void Sloong::CDataTransPackage::ResponsePackage( shared_ptr<MessagePackage> pack )
{
	m_pTransPackage = pack;
	PrepareSendPackageData();
}


void Sloong::CDataTransPackage::ResponsePackage(const string &msg, const string& exdata)
{
	m_pTransPackage->set_content(msg);
	m_pTransPackage->set_extend(exdata);
	PrepareSendPackageData();
}


/**
 * @Remarks: 
 * @Params: 
 * @Return: if send succeed, return true. need listen read message. 
 *          else return false, need listen write message.
 */
NetworkResult Sloong::CDataTransPackage::SendPackage()
{
	int nSentSize = m_pCon->SendPackage(m_strPackageData, m_nSent);
	if (nSentSize < 0){
		return NetworkResult::Error;
	}else{
		m_nSent += nSentSize;
	}

	if( m_pLog )
		m_pLog->Verbos(CUniversal::Format("Send Info : AllSize[%d],Sent[%d]", m_nPackageSize, m_nSent));

	if (m_nSent < m_nPackageSize){
		return NetworkResult::Retry;
	}else{
		if( m_pLog )
			m_pLog->Verbos(CUniversal::Format("Message package send succeed, remove from send list. All size[%d]", m_nSent));
		return NetworkResult::Succeed;
	}
}

NetworkResult Sloong::CDataTransPackage::RecvPackage(int timeout)
{
	string result;
	auto net_res = m_pCon->RecvPackage(result,timeout);
	if( net_res != NetworkResult::Succeed )
		return net_res;

	m_pTransPackage = make_shared<MessagePackage>();
	if(!m_pTransPackage->ParseFromString(result))
	{
		if( m_pLog )
			m_pLog->Error("Parser receive data error.");
		return NetworkResult::Error;
	}

	if (m_pTransPackage->prioritylevel() > s_PriorityLevel || m_pTransPackage->prioritylevel() < 0)
	{
		if( m_pLog )
			m_pLog->Error(CUniversal::Format("Receive priority level error. the data is %d, the config level is %d. add this message to last list", m_pTransPackage->prioritylevel(), s_PriorityLevel));
		return NetworkResult::Error;
	}
	
	if( m_pLog )
		m_pLog->Verbos(CUniversal::Format("RECV<<<[%d][%d]<<<%s",m_pTransPackage->prioritylevel(),m_pTransPackage->serialnumber(),m_pTransPackage->content()));

	if( m_pTransPackage->checkstring().length() > 0 ){
		string rmd5 = CMD5::Encode(m_pTransPackage->content());
		if ( strcasecmp(m_pTransPackage->checkstring().c_str(),rmd5.c_str()) != 0)
		{
			// handle error.
			if( m_pLog )
				m_pLog->Warn(CUniversal::Format("MD5 check fialed.Message:[%s].recv MD5:[%s].local md5[%s]",m_pTransPackage->content(), rmd5, m_pTransPackage->checkstring() ));
			string strSend = CUniversal::Format("{\"errno\": \"-1\",\"errmsg\" : \"package check error\",\"server_md5\":\"%s\",\"client_md5\":\"%s\",\"check_string\":\"%s\"}", rmd5, m_pTransPackage->checkstring(), m_pTransPackage->content());
			ResponsePackage(strSend);
			return NetworkResult::Invalid;
		}
	}


	return NetworkResult::Succeed;
}
