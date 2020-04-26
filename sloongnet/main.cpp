﻿/*
 * @Author: WCB
 * @Date: 2020-04-26 17:33:59
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-26 18:15:45
 * @Description: Main function for apps. just for create and run gloabl apps.
 */
#include "main.h"
#include "base_service.h"
#include "version.h"

void PrientHelp()
{
	cout << "sloongnet [<type>] [<address:port>]" << endl;
	cout << "sloongnet version" << endl;
	cout << "<type>: Manager|Worker" << endl;
}


void PrintVersion()
{
	cout << PRODUCT_TEXT << endl;
	cout << VERSION_TEXT << endl;
	cout << COPYRIGHT_TEXT << endl;
}


int main(int argc, char** args)
{
	try
	{
		// 参数共有2个，类型和地址信息
		// 1 类型，为指定值
		// 2 地址信息：格式->地址：端口。如果类型为control，表示为监听地址
		if (argc != 3)
		{
			PrientHelp();
			return -2;
		}

		unique_ptr<RunTimeData> data = make_unique<RunTimeData>();
		data->ManagerMode = true;
		if (strcasecmp(args[1], "Worker") == 0)
		{
			data->ManagerMode = false;
		}
		else if (strcasecmp(args[1], "Manager") == 0)
		{
			data->ManagerMode = true;
		}
		else if (strcasecmp(args[1], "version") == 0)
		{
			PrintVersion();
			return 0;
		}
		else
		{
			cout << "Parse module type error." << endl;
			PrientHelp();
			return -1;
		}

		vector<string> addr = CUniversal::split(args[2], ':');
		if (addr.size() != 2)
		{
			cout << "Address info format error. Format [addr]:[port]" << endl;
			return -3;
		}

		data->ManagerAddress = addr[0];
		int port = atoi(addr[1].c_str());
		if (port == 0)
		{
			cout << "Convert [" << addr[1] << "] to int port fialed." << endl;
			return -3;
		}
		data->ManagerPort = port;
	
		CResult res = CResult::Succeed();
		Sloong::CSloongBaseService::Instance = make_unique<Sloong::CSloongBaseService>();
		do
		{
			res = Sloong::CSloongBaseService::Instance->Initialize(data.get());
			if (!res.IsSucceed()) {
				cout << "Initialize server error. Message: " << res.Message() << endl;
				return -5;
			}

			res = Sloong::CSloongBaseService::Instance->Run();
		} while (res.Result() == ResultType::Retry);
		return 0;
	}
	catch (string & msg)
	{
		cout << "exception happened, message:" << msg << endl;
		return -4;
	}
	catch (exception & exc)
	{
		cout << "exception happened, message:" << exc.what() << endl;
		return -4;
	}
	catch (...)
	{
		cout << "Unhandle exception happened, system will shutdown. " << endl;
		CUtility::write_call_stack();
		return -4;
	}
}
