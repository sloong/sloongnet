/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-10-09 10:28:37
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/filecenter/filecenter.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
/*** 
 * @......................................&&.........................
 * @....................................&&&..........................
 * @.................................&&&&............................
 * @...............................&&&&..............................
 * @.............................&&&&&&..............................
 * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * @.....&&&&&&&&&&&&&&&&............................&&..............
 * @....&&&&&&&&&&&&&&&.................&&...........................
 * @...&&&&&&&&&&&&&&&.....................&&&&......................
 * @...&&&&&&&&&&.&&&........................&&&&&...................
 * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * @........&&...................&&&&&&.........................&&&..
 * @.........&.....................&&&&........................&&....
 * @...............................&&&.......................&&......
 * @................................&&......................&&.......
 * @.................................&&..............................
 * @..................................&..............................
 */

#include "filecenter.h"

#include "protocol/manager.pb.h"
using namespace Manager;

using namespace Sloong;


unique_ptr<CFileCenter> Sloong::CFileCenter::Instance = nullptr;

extern "C" PackageResult RequestPackageProcesser(void *pEnv, Package *pack)
{
    auto pManager = STATIC_TRANS<FileManager *>(pEnv);
    if (pManager)
        return pManager->RequestPackageProcesser(pack);
    else
        return PackageResult::Make_Error("Environment convert error. cannot process message.");
}

extern "C" PackageResult ResponsePackageProcesser(void *pEnv, Package *pack)
{
    auto pManager = STATIC_TRANS<FileManager *>(pEnv);
    if (pManager)
        return pManager->ResponsePackageProcesser(pack);
    else
        return PackageResult::Make_Error("Environment convert error. cannot process message.");
}

extern "C" CResult EventPackageProcesser(Package *pack)
{
    CFileCenter::Instance->EventPackageProcesser(pack);
    return CResult::Succeed;
}

extern "C" CResult NewConnectAcceptProcesser(SOCKET sock)
{
    return CResult::Succeed;
}

extern "C" CResult ModuleInitialization(IControl *ic)
{
    CFileCenter::Instance = make_unique<CFileCenter>();
    return CFileCenter::Instance->Initialization(ic);
}

extern "C" CResult ModuleInitialized()
{
    return CFileCenter::Instance->Initialized();
}

extern "C" CResult CreateProcessEnvironment(void **out_env)
{
    return CFileCenter::Instance->CreateProcessEnvironmentHandler(out_env);
}

CResult CFileCenter::Initialization(IControl *ic)
{
    IObject::Initialize(ic);
    ic->Add(FILECENTER_DATAITEM::UploadInfos, &m_mapTokenToUploadInfo);
    return CResult::Succeed;
}

CResult CFileCenter::Initialized()
{
    return CResult::Succeed;
}

CResult Sloong::CFileCenter::CreateProcessEnvironmentHandler(void **out_env)
{
    auto item = make_unique<FileManager>();
	auto res = item->Initialize(m_iC);
	if (res.IsFialed())
		return res;
	(*out_env) = item.get();
	m_listManage.push_back(std::move(item));
	return CResult::Succeed;
}

void Sloong::CFileCenter::EventPackageProcesser(Package *pack)
{
    auto event = Events_MIN;
    if (!Manager::Events_Parse(pack->content(), &event))
    {
        m_pLog->Error(Helper::Format("Receive event but parse error. content:[%s]", pack->content().c_str()));
        return;
    }

    switch (event)
    {
    case Manager::Events::ReferenceModuleOnline:
    {
        m_pLog->Info("Receive ReferenceModuleOnline event");
    }
    break;
    case Manager::Events::ReferenceModuleOffline:
    {
        m_pLog->Info("Receive ReferenceModuleOffline event");
    }
    break;
    default:
    {
    }
    break;
    }
}
