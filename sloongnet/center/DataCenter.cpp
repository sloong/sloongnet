#include "DataCenter.h"

using namespace Sloong;

CDataCenter::CDataCenter()
{

}


CDataCenter::~CDataCenter()
{
}

bool CDataCenter::Add(DATA_ITEM item, void * object)
{
	auto it = m_oDataList.find(item);
	if (it != m_oDataList.end())
	{
		return false;
	}
	m_oDataList.insert(make_pair(item, object));
	return false;
}

void * CDataCenter::Get(DATA_ITEM item)
{
	auto data = m_oDataList.find(item);
	if (data == m_oDataList.end())
	{
		return nullptr;
	}
	return (*data).second;
}

bool CDataCenter::Remove(DATA_ITEM item)
{
	m_oDataList.erase(item);
}

bool CDataCenter::AddTemp(string name, void * object)
{
	m_oTempDataList[name] = object;
	return true;
}

void * CDataCenter::GetTemp(string name)
{
	auto item = m_oTempDataList.find(name);
	if (item == m_oTempDataList.end())
	{
		return nullptr;
	}
	m_oTempDataList.erase(name);
	return (*item).second;
}
