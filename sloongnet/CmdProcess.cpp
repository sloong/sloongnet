#include "CmdProcess.h"
#include "version.h"
#include "main.h"

void Sloong::CCmdProcess::Parser(string strCmd)
{
	if (strCmd == "-v" || strCmd == "--v" || strCmd == "version")
	{
		cout << PRODUCT_TEXT << endl;
		cout << VERSION_TEXT << endl;
		cout << COPYRIGHT_TEXT << endl;
		return;
	}
	else 
	{
		cout << "-v/--v/version: show the version info." << endl;
		cout << "-r/--r/run [filepath] [ex config file path] : run with the config file." << endl;
		cout << "-rd : run with default config." << endl;
		return;
	}
}
