#include "filecenter.h"

#include "protocol/manager.pb.h"
using namespace Manager;

using namespace Sloong;


unique_ptr<CFileCenter> Sloong::CFileCenter::Instance = nullptr;

extern "C" CResult RequestPackageProcesser(void *pEnv, CDataTransPackage *pack)
{
    auto pManager = TYPE_TRANS<FileManager *>(pEnv);
    if (pManager)
        return pManager->RequestPackageProcesser(pack);
    else
        return CResult::Make_Error("Environment convert error. cannot process message.");
}

extern "C" CResult ResponsePackageProcesser(void *pEnv, CDataTransPackage *pack)
{
    auto pManager = TYPE_TRANS<FileManager *>(pEnv);
    if (pManager)
        return pManager->ResponsePackageProcesser(pack);
    else
        return CResult::Make_Error("Environment convert error. cannot process message.");
}

extern "C" CResult EventPackageProcesser(CDataTransPackage *pack)
{
    CFileCenter::Instance->EventPackageProcesser(pack);
    return CResult::Succeed();
}

extern "C" CResult NewConnectAcceptProcesser(CSockInfo *info)
{
    return CResult::Succeed();
}

extern "C" CResult ModuleInitialization(GLOBAL_CONFIG *config)
{
    CFileCenter::Instance = make_unique<CFileCenter>();
    return CFileCenter::Instance->Initialization(config);
}

extern "C" CResult ModuleInitialized(SOCKET sock, IControl *iC)
{
    return CFileCenter::Instance->Initialized(iC);
}

extern "C" CResult CreateProcessEnvironment(void **out_env)
{
    return CFileCenter::Instance->CreateProcessEnvironmentHandler(out_env);
}

CResult CFileCenter::Initialization(GLOBAL_CONFIG *config)
{
    return CResult::Succeed();
}

CResult CFileCenter::Initialized(IControl *ic)
{
    IObject::Initialize(ic);
    m_iC->RegisterEventHandler(SocketClose, std::bind(&CFileCenter::OnSocketClose, this, std::placeholders::_1));
    return CResult::Succeed();
}

void Sloong::CFileCenter::OnSocketClose(SharedEvent event)
{
}

CResult Sloong::CFileCenter::CreateProcessEnvironmentHandler(void **out_env)
{
    auto item = make_unique<FileManager>();
	auto res = item->Initialize(m_iC);
	if (res.IsFialed())
		return res;
	(*out_env) = item.get();
	m_listManage.push_back(std::move(item));
	return CResult::Succeed();
}

void Sloong::CFileCenter::EventPackageProcesser(CDataTransPackage *trans_pack)
{
    auto event = Events_MIN;
    auto data_pack = trans_pack->GetDataPackage();
    if (!Manager::Events_Parse(data_pack->content(), &event))
    {
        m_pLog->Error(Helper::Format("Receive event but parse error. content:[%s]", data_pack->content().c_str()));
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
