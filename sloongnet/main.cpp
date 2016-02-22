#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "userv.h"
#include "univ/log.h"
#include "serverconfig.h"
#include "version.h"
#include "jpeg.h"
#include "univ/exception.h"
using namespace std;
using namespace Sloong;
int main( int argc, char** args )
{
	CServerConfig config;
	if (argc >= 2)
	{
		string strCmd(args[1]);
		CUniversal::tolower(strCmd);
		if (strCmd == "-v" || strCmd == "--v" || strCmd == "version")
		{
			cout << PRODUCT_TEXT << endl;
			cout << VERSION_TEXT << endl;
			cout << COPYRIGHT_TEXT << endl;
			return 0;
		}
		else if ((strCmd == "-r" || strCmd == "--r" || strCmd == "run") && (argc >= 3))
		{
			if (access(args[2], ACC_R) == 0)
			{
				try
				{
					config.LoadConfigFile(args[2]);
				}
				catch (normal_except e)
				{
					cout << "Error when load config. error meesage is: " << e.what() << endl;
					cout << "if you want run continue with default config, input 'r'." << endl;
					char key;
					cin >> key;
					if (key != 'r' && key != 'R')
						return 0;
				}
			}
			else
			{
				if (access(args[2], ACC_E) == 0)
					cerr << "cannot read file. file path is : " << args[2] << endl;
				else
					cerr << "file not exist. file path is : " << args[2] << endl;
				return -1;
			}
		}
		else if(strCmd != "-rd")
		{
			cout << "-v/--v/version: show the version info." << endl;
			cout << "-r/--r/run [filepath] : run with the config file." << endl;
			cout << "-rd : run with default config." << endl;
			return 0;
		}
	}
		
    SloongWallUS us;
	us.Initialize(&config);
    us.Run();
	return 0;
}

