#include "CmdProcess.h"
#include "version.h"
#include <univ/exception.h>

void Sloong::CCmdProcess::PrintVersion()
{
	cout << PRODUCT_TEXT << endl;
	cout << VERSION_TEXT << endl;
	cout << COPYRIGHT_TEXT << endl;
}


void Sloong::CCmdProcess::PrintHelp()
{
	cout << "-v/--v/version: show the version info." << endl;
	cout << "-r/--r/run [filepath] [ex config file path] : run with the config file." << endl;
	cout << "-rd : run with default config." << endl;
}

bool Sloong::CCmdProcess::Parser(int argc, char ** args, CServerConfig* config)
{
	if (argc >= 2)
	{
		string strCmd(args[1]);
		CUniversal::tolower(strCmd);

		if ((strCmd == "-r" || strCmd == "--r" || strCmd == "run") && (argc >= 3))
		{
			if (access(args[2], ACC_R) == 0)
			{
				try
				{
					if (argc > 3)
						config->Initialize(args[2], args[3]);
					else
						config->Initialize(args[2]);
					config->LoadConfig();
					return true;
				}
				catch (normal_except& e)
				{
					throw normal_except("Error when load config. error message is: " + string(e.what()));
				}
			}
			else
			{
				if (access(args[2], ACC_E) == 0)
					throw normal_except("cannot read config file. file path is : " + string(args[2]));
				else
					throw normal_except("config file not exist. file path is : " + string(args[2]));
			}
		}
		else if (strCmd == "-rd")
		{
			return true;
		}
		else if (strCmd == "-v" || strCmd == "--v" || strCmd == "version")
		{
			PrintVersion();
			return false;
		}
	}
	PrintHelp();
	return false;
}
