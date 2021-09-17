/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2015-11-12 15:56:50
 * @LastEditTime: 2021-09-17 17:40:45
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/main.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: Main function for apps. just for create and run gloabl apps.
 */

#include "main.h"
#include "base_service.h"
#include "utility.h"
#include "version.h"

void PrientHelp()
{
	cout << "sloongnet <type> <address:port> [--<assign|include|exclude>=<TypeName>]" << endl;
	cout << "sloongnet version" << endl;
	cout << "type: Manager|Worker" << endl;
	cout << "address:port: Listen port / Manager port" << endl;
	cout << "Note: the third parameter works only in Worker mode." << endl;
	cout << "--assign=<TargetTemplateID>: If this node just for one templateid, set it." << endl;
	cout << "--include=<TypeName>: Set it for target module type. If have multi type, use ',' to split. If assign is setted, this will be ignored." << endl;
	cout << "--exclude=<TypeName>: Set it for target is not support type. If have multi type, use ',' to split. If assign/include is setted, this will be ignored." << endl;
	cout << "-?/--help/-h: Print help info." << endl;
	cout << "-v/--version: Print version info." << endl;
}

void PrintVersion()
{
	cout << PRODUCT_TEXT << endl;
	cout << VERSION_TEXT << endl;
	cout << COPYRIGHT_TEXT << endl;
}

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main(int argc, char **args)
{
	// vector<string> argv;
	// for (int i = 1; i < argc; i++)
	// 	argv.push_back(args[i]);
	try
	{
		auto logger = spdlog::stdout_color_mt("console");
		
		if (argc < 2 || strcasecmp(args[1], "-v") == 0 || strcasecmp(args[1], "--version") == 0)
		{
			PrintVersion();
			return 0;
		}
		else if (strcasecmp(args[1], "-?") == 0 || strcasecmp(args[1], "--help") == 0 || strcasecmp(args[1], "-h") == 0)
		{
			PrientHelp();
			return 0;
		}

		string cmd = "Command line:";
		for (int i = 1; i < argc; i++)
		{
			cmd += string(args[i]) + " ";
		}
		logger->info(cmd);

		if (argc > 4)
		{
			logger->critical("Params error. See help with --help.");
			return -1;
		}
		NodeInfo info;

		if (strcasecmp(args[1], "Manager") == 0)
		{
			info.ManagerMode = true;
		}
		else if (strcasecmp(args[1], "Worker") == 0)
		{
			info.ManagerMode = false;
		}
		else
		{
			logger->critical("Unknown type. See help with --help.");
			return -1;
		}

		if (string(args[2]).find(":") != string::npos)
		{
			vector<string> addr = Helper::split(args[2], ':');
			info.Address = addr[0];
			if (!ConvertStrToInt(addr[1], &info.Port))
			{
				logger->critical(format("Convert [{}] to int port fialed.",  addr[1]));
				return -1;
			}
		}
		else
		{
			logger->critical( "Address port info error. See help with --help.");
			return -1;
		}

		// Stop support for custom parameters
		// for (int i = 3; i < argc; i++)
		if (argc == 4)
		{
			auto item = string(args[3]);
			if (item.find("--assign=") != string::npos)
			{
				info.AssignedTargetTemplateID = item.substr(9);
			}
			else if (item.find("--include=") != string::npos)
			{
				info.IncludeTargetType = item.substr(10);
			}
			else if (item.find("--exclude=") != string::npos)
			{
				info.ExcludeTargetType = item.substr(10);
			}
		}

		if (info.Port == 0)
		{
			logger->critical("Port error, please check.");
			return -2;
		}

		CResult res = CResult::Succeed;
		Sloong::CSloongBaseService::Instance = make_unique<Sloong::CSloongBaseService>();
		do
		{
			logger->info( "Initialize base service instance.");
			res = Sloong::CSloongBaseService::Instance->Initialize(info, logger.get());
			if (!res.IsSucceed())
			{
				logger->critical( format("Initialize server error. Message: {}",  res.GetMessage()));
				return -5;
			}

			logger->info("Run base service instance.");
			res = Sloong::CSloongBaseService::Instance->Run();

			logger->info(format("Base service instance is end with result {}. Message {}", ResultType_Name(res.GetResult()) , res.GetMessage()));
		} while (res.GetResult() == ResultType::Retry);

		logger->info( "Application exit.");
		Sloong::CSloongBaseService::Instance = nullptr;
		spdlog::shutdown();
		return 0;
	}
	catch (string &msg)
	{
		cout << "exception happened, message:" << msg << endl;
		return -4;
	}
	catch (exception &exc)
	{
		cout << "exception happened, message:" << exc.what() << endl;
		return -4;
	}
	catch (...)
	{
		cout << "Unhandle exception happened, system will shutdown. " << endl;
		cout << CUtility::GetCallStack();
		return -4;
	}
	cout << "Application exit with exception." << endl;
	return -5;
}
