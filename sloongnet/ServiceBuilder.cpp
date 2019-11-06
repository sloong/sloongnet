#include "ServiceBuilder.h"

#include "control_service.h"
#include "data_service.h"
#include "firewall_service.h"
#include "process_service.h"
#include "proxy_service.h"

unique_ptr<CSloongBaseService> ServiceBuilder::Build(GLOBAL_CONFIG* config)
{
	switch (config->type())
	{
	case ModuleType::Control:
		return make_unique<SloongControlService>();
	case ModuleType::Data:
		return make_unique<SloongNetDataCenter>();
	case ModuleType::Firewall:
		return make_unique<SloongNetFirewall>();
	case ModuleType::Gateway:
		return make_unique<SloongNetProxy>();
	case ModuleType::Process:
		return make_unique<SloongNetProcess>();
	case ModuleType::DB:
		//return make_unique<SloongData
	default:
		return nullptr;
	}
}