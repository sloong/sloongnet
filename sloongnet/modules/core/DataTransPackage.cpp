#include "DataTransPackage.h"

CLog *Sloong::CDataTransPackage::g_pLog = nullptr;

void Sloong::CDataTransPackage::PrepareSendPackageData()
{
	m_pTransPackage.SerializeToString(&m_strPackageData);
	m_nPackageSize = (int)m_strPackageData.length();
	if( m_nPackageSize > g_max_package_size)
	{
		m_pTransPackage.set_result(ResultType::Error);
		m_pTransPackage.set_content("The package size is to bigger.");
		m_pTransPackage.clear_extend();
		if (g_pLog != nullptr)
			g_pLog->Assert("The package size is to bigger, this's returned and replaced with an error message package.")
		m_pTransPackage.SerializeToString(&m_strPackageData);	
	}
	if (g_pLog != nullptr)
		g_pLog->Debug(Helper::Format("SEND>>>[%d][%llu]>>>[%d]", m_pTransPackage.priority(), m_pTransPackage.id(), m_nPackageSize));
}

void Sloong::CDataTransPackage::RequestPackage()
{
	m_pTransPackage.set_status(DataPackage_StatusType::DataPackage_StatusType_Request);
	PrepareSendPackageData();
}

void Sloong::CDataTransPackage::RequestPackage(const DataPackage &pack)
{
	m_pTransPackage = pack;
	RequestPackage();
}

void Sloong::CDataTransPackage::ResponsePackage(const DataPackage &pack)
{
	m_pTransPackage = pack;
	m_pTransPackage.set_status(DataPackage_StatusType::DataPackage_StatusType_Response);
	PrepareSendPackageData();
}

void Sloong::CDataTransPackage::ResponsePackage(ResultType result)
{
	m_pTransPackage.set_result(result);
	m_pTransPackage.set_status(DataPackage_StatusType::DataPackage_StatusType_Response);
	PrepareSendPackageData();
}

void Sloong::CDataTransPackage::ResponsePackage(ResultType result, const string &message, const string *exdata /*=nullptr*/)
{
	m_pTransPackage.set_result(result);
	m_pTransPackage.set_content(message);
	m_pTransPackage.set_status(DataPackage_StatusType::DataPackage_StatusType_Response);
	if (exdata)
		m_pTransPackage.set_extend(*exdata);
	PrepareSendPackageData();
}

void Sloong::CDataTransPackage::ResponsePackage(const CResult &result)
{
	m_pTransPackage.set_result(result.Result());
	m_pTransPackage.set_content(result.Message());
	m_pTransPackage.set_status(DataPackage_StatusType::DataPackage_StatusType_Response);
	PrepareSendPackageData();
}

/**
 * @Remarks: 
 * @Params: 
 * @Return: if send succeed, return true. need listen read message. 
 *          else return false, need listen write message.
 */
ResultType Sloong::CDataTransPackage::SendPackage()
{
	if (m_nPackageSize == 0)
	{
		if (g_pLog)
			g_pLog->Error("Cannot send message. the send data is zero");
		return ResultType::Invalid;
	}
	if (g_pLog)
		g_pLog->Debug(Helper::Format("Start send package : AllSize[%d],Sent[%d]", m_nPackageSize, m_nSent));

	auto res = m_pCon->SendPackage(m_strPackageData, m_nSent);
	if (g_pLog)
		g_pLog->Debug(Helper::Format("Send package end: AllSize[%d],Sent[%d]", m_nPackageSize, m_nSent));
	if (res == ResultType::Succeed)
	{
		this->Record();
		if (g_pLog)
			g_pLog->Debug(Helper::Format("Send package succeed: %s", FormatRecord().c_str()));
		m_nPackageSize = 0;
		return ResultType::Succeed;
	}
	else if (res == ResultType::Retry)
	{
		return ResultType::Retry;
	}
	else if (res == ResultType::Error)
	{
		if (g_pLog)
			g_pLog->Warn(Helper::Format("Error when send data. Socket[%d][%s:%d].Errno[%d].", m_pCon->GetSocketID(), m_pCon->m_strAddress.c_str(), m_pCon->m_nPort, m_pCon->GetErrno()));
		return ResultType::Error;
	}
	return ResultType::Error;
}

ResultType Sloong::CDataTransPackage::RecvPackage(bool block)
{
	string result;
	auto net_res = m_pCon->RecvPackage(result, m_nPackageSize, m_nReceived, block);
	if (net_res == ResultType::Succeed)
	{
		return RecvPackageSucceedProcess(result);
	}
	if (net_res == ResultType::Warning)
	{
		// If the data length received 11 error, ignore it.
		return ResultType::Warning;
	}
	else if (net_res == ResultType::Retry)
	{
		if (g_pLog)
			g_pLog->Verbos(Helper::Format("Receive package returned [Retry]. Package size[%d], Received[%d]", m_nPackageSize, m_nReceived));
		return ResultType::Retry;
	}
	else if (net_res == ResultType::Error)
	{
		if (g_pLog)
			g_pLog->Warn(Helper::Format("Error when receive data. Socket[%d][%s:%d].Errno[%d].", m_pCon->GetSocketID(), m_pCon->m_strAddress.c_str(), m_pCon->m_nPort, m_pCon->GetErrno()));
		return ResultType::Error;
	}
	return ResultType::Error;
}

ResultType Sloong::CDataTransPackage::RecvPackageSucceedProcess(const string& result)
{
	if (!m_pTransPackage.ParseFromString(result))
	{
		if (g_pLog)
			g_pLog->Error("Parser receive data error.");
		return ResultType::Error;
	}

	if (m_pTransPackage.priority() > s_PriorityLevel || m_pTransPackage.priority() < 0)
	{
		if (g_pLog)
			g_pLog->Error(Helper::Format("Receive priority level error. the data is %d, the config level is %d. add this message to last list", m_pTransPackage.priority(), s_PriorityLevel));
		return ResultType::Error;
	}

	if (g_pLog)
		g_pLog->Debug(Helper::Format("RECV<<<[%d][%llu]<<<[%d]", m_pTransPackage.priority(), m_pTransPackage.id(), result.size()));

	if( result.size() > g_max_package_size )
	{
		ResponsePackage(ResultType::Error, "The package size is to bigger.");
		return ResultType::Invalid;
	}

	if (m_pTransPackage.hash().length() > 0)
	{
		ResponsePackage(ResultType::Error, "Now don't support hash check.");
		return ResultType::Invalid;
	}
	this->Record();
	return ResultType::Succeed;
}

string Sloong::CDataTransPackage::FormatRecord()
{
	string str;
	auto start = m_listClock.begin();
	for (auto item = ++m_listClock.begin(); item != m_listClock.end(); item++)
	{
		auto s = (*item).tv_sec - (*start).tv_sec;
		auto us = ((*item).tv_usec - (*start).tv_usec);
		double total = s * 1000 + us / 1000.0;
		str = Helper::Format("%s[%.2f]", str.c_str(), total);
	}
	return str;
}