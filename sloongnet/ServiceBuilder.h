#pragma once

#include "main.h"
#include "base_service.h"
class ServiceBuilder
{
public:
	static unique_ptr<CSloongBaseService> Build(GLOBAL_CONFIG* config);
};

