#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "userv.h"
#include "univ/log.h"
#include "serverconfig.h"
using namespace std;
using namespace Sloong;
int main( int argc, char** args )
{
	CServerConfig config;
    // just have one param is the config file path.
	if (argc >= 2)
	{
		// check the param 1 is not a file.
		if (access(args[1], R_OK) == 0)
		{
			// load config from file.
			config.LoadConfigFile(args[1]);
		}
		else
		{
			if (access(args[1], F_OK) == 0)
				cerr << "cannot read file." << args[1] << endl;
			else
				cerr << "file not exist" << args[1] << endl;
			return -1;
		}
	}
		
    SloongWallUS us;
	us.Initialize(&config);
    us.Run();
	return 0;
}

