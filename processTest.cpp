#include "stdafx.h"
#include <iostream>
#include "processManager.h"
#include "logger.h"

void OnStart()
{
	printf("Hello from OnProcStartCallback!\n");
}

void OnManuallyStopped()
{
	printf("Hello from OnProcManuallyStoppedCallback!\n");
}


int _tmain(int argc, _TCHAR* argv[])
{
	printf("Press enter to start test!!!\n");
	getchar();
	int loglevel = 3;
	logger::getInstance()->SetLogLevel(loglevel);
	logger::getInstance()->SetWriteToFile(true);
	printf("From now, evry action with loglevel lower than %i \nwill be logged to logfile and displayed on the screen.\n\n", loglevel);
	printf("-------------------------------------\n");
	processManager pr1(std::string("Calc.exe"));
	
	pr1.SetOnProcStartCallback(OnStart);
	pr1.SetOnProcManuallyStoppedCallback(OnManuallyStopped);

	pr1.startProcess();
	Sleep(100);
	printf("\nNow you can close process and it will be automatically started again.\n");
	printf("Press ENTER to restart process manually.\n");
	getchar();
	pr1.restartProcess();

	Sleep(100);
	printf("\nPress ENTER to obtain process info.\n");
	getchar();
	pr1.GetProcessInfo();

	Sleep(100);
	printf("\nPress ENTER to create another instance of class \nand catch already running process.\n");
	processManager pr2;
	pr2.CatchProcess(pr1.GetProcID());

	Sleep(100);
	printf("\nNow, first instant will close the process, but second will start it again.\n");
	pr1.stopProcess();

	Sleep(100);
	printf("You can close the process and it will be automatically started \nagain by second instance.\n");
	printf("Press ENTER to end the test.\n");
	getchar();	
	
	pr2.stopProcess();
	return 0;
}

