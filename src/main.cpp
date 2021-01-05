/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2015-11-12 15:56:50
 * @LastEditTime: 2021-01-05 15:24:32
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

int main(int argc, char **args)
{
	try
	{
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

		cout << "Command line:";
		for (int i = 1; i < argc; i++)
		{
			cout << args[i] << " ";
		}
		cout << endl;

		if (argc > 4)
		{
			cout << "Params error. See help with --help." << endl;
			return -1;
		}
		RunInfo info;

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
			cout << "Unknown type. See help with --help." << endl;
			return -1;
		}

		if (string(args[2]).find(":") != string::npos)
		{
			vector<string> addr = Helper::split(args[2], ':');
			info.Address = addr[0];
			if (!ConvertStrToInt(addr[1], &info.Port))
			{
				cout << "Convert [" << addr[1] << "] to int port fialed." << endl;
				return -1;
			}
		}
		else
		{
			cout << "Address port info error. See help with --help." << endl;
			return -1;
		}

		// Stop support for custom parameters
		// for (int i = 3; i < argc; i++)
		if (argc == 4)
		{
			auto item = string(args[3]);
			if (item.find("--assign=") != string::npos)
			{
				info.AssignedTargetTemplateID = item.substr(8);
			}
			else if (item.find("--include=") != string::npos)
			{
				info.IncludeTargetType = item.substr(9);
			}
			else if (item.find("--exclude=") != string::npos)
			{
				info.ExcludeTargetType = item.substr(9);
			}
		}

		if (info.Port == 0)
		{
			cout << "Port error, please check." << endl;
			return -2;
		}

		CResult res = CResult::Succeed;
		Sloong::CSloongBaseService::Instance = make_unique<Sloong::CSloongBaseService>();
		do
		{
			cout << "Initialize base service instance." << endl;
			res = Sloong::CSloongBaseService::Instance->Initialize(move(info));
			if (!res.IsSucceed())
			{
				cout << "Initialize server error. Message: " << res.GetMessage() << endl;
				return -5;
			}

			cout << "Run base service instance." << endl;
			res = Sloong::CSloongBaseService::Instance->Run();

			cout << "Base service instance is end with result " << ResultType_Name(res.GetResult()) << ". Message: " << res.GetMessage() << endl;
		} while (res.GetResult() == ResultType::Retry);

		cout << "Application exit." << endl;
		Sloong::CSloongBaseService::Instance = nullptr;
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
}
