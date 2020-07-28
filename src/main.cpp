/*
 * @Author: WCB
 * @Date: 2020-04-26 17:33:59
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-17 15:55:00
 * @Description: Main function for apps. just for create and run gloabl apps.
 */
#include "main.h"
#include "base_service.h"
#include "utility.h"
#include "version.h"

void PrientHelp()
{
	cout << "sloongnet [<type>] [<address:port>] [--F=<ForceTargetTemplateID>]" << endl;
	cout << "sloongnet version" << endl;
	cout << "<type>: Manager|Worker" << endl;
	cout << "<address:port>: Listen port / Manager port" << endl;
	cout << "--F=<ForceTargetTemplateID>: If this node just for one templateid, set it." << endl;
	cout << "--T=<TypeName>: Set it for target module type." << endl;
	cout << "--?/--help/--h: Print help info." << endl;
	cout << "--v/--version: Print version info." << endl;
}

void PrintVersion()
{
	cout << PRODUCT_TEXT << endl;
	cout << VERSION_TEXT << endl;
	cout << COPYRIGHT_TEXT << endl;
}

typedef struct CmdInfo
{
	CmdInfo()
	{
		ManagerMode = false;
		Address = "";
		Port = 0;
		ForceTargetTemplateID = 0;
		ForceTargetType = "";
	}
	bool ManagerMode;
	string Address;
	int Port;
	int ForceTargetTemplateID;
	string ForceTargetType;
} CmdInfo;

int main(int argc, char **args)
{
	try
	{
		cout << "Command line:";
		for (int i = 1; i < argc; i++)
		{
			cout << args[i] << " ";
		}
		cout << endl;
		CmdInfo info;
		for (int i = 1; i < argc; i++)
		{
			auto item = string(args[i]);
			if (strcasecmp(args[i], "Manager") == 0)
			{
				info.ManagerMode = true;
				continue;
			}
			else if (item.find(":") != string::npos)
			{
				vector<string> addr = Helper::split(args[2], ':');
				info.Address = addr[0];
				if (!ConvertStrToInt(addr[1], &info.Port))
				{
					cout << "Convert [" << addr[1] << "] to int port fialed." << endl;
					return -1;
				}
			}
			else if (item.find("-F=") != string::npos)
			{
				auto tempid = item.substr(3);
				ConvertStrToInt(tempid, &info.ForceTargetTemplateID);
			}
			else if (item.find("-T=") != string::npos)
			{
				info.ForceTargetType = item.substr(3);
			}
			else if (strcasecmp(args[i], "--v") == 0 || strcasecmp(args[i], "--version") == 0)
			{
				PrintVersion();
				return 0;
			}
			else if (strcasecmp(args[i], "--?") == 0 || strcasecmp(args[i], "--help") == 0 || strcasecmp(args[i], "--h") == 0)
			{
				PrientHelp();
				return 0;
			}
		}

		if (info.Port == 0)
		{
			cout << "Port error, please check." << endl;
			return -2;
		}

		CResult res = CResult::Succeed();
		Sloong::CSloongBaseService::Instance = make_unique<Sloong::CSloongBaseService>();
		do
		{
			cout << "Initialize base service instance." << endl;
			res = Sloong::CSloongBaseService::Instance->Initialize(info.ManagerMode, info.Address, info.Port, info.ForceTargetTemplateID);
			if (!res.IsSucceed())
			{
				cout << "Initialize server error. Message: " << res.GetMessage() << endl;
				return -5;
			}

			
			cout << "Run base service instance." << endl;
			res = Sloong::CSloongBaseService::Instance->Run();
			
			cout << "Base service instance is end with result " << ResultType_Name(res.GetResult()) << endl;
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
