// processTest.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include "processManager.h"

int _tmain(int argc, _TCHAR* argv[])
{
	printf("Hello World!!!\n");
	getchar();
	processManager pr("Calc.exe");

	pr.GetProcessInfo();
	pr.startProcess();
	getchar();
	pr.GetProcessInfo();
	pr.restartProcess();
	getchar();
	pr.GetProcessInfo();
	pr.stopProcess();
	pr.GetProcessInfo();
	getchar();
	return 0;
}

