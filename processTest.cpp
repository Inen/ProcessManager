// processTest.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include "processManager.h"

int _tmain(int argc, _TCHAR* argv[])
{
	printf("Start test!!!\n");
	getchar();
	
	processManager pr;
	DWORD id;
	std::cin >> id;
	pr.CatchProcess(id);



	//processManager pr(std::string("Calc.exe"));
	//pr.startProcess();

	//getchar();
	//std::cout << pr.GetHandle() << std::endl;
	//std::cout << pr.GetProcID() << std::endl;
	//processManager pr2;
	//pr2.CatchProcess(pr.GetProcID());
	//getchar();
	//std::cout << pr2.GetHandle() << std::endl;
	//std::cout << "-----------" << std::endl;
	//pr2.stopProcess();
	//getchar();

	//pr.GetProcessInfo();
	//pr.startProcess();
	//getchar();
	//pr.GetProcessInfo();
	//pr.restartProcess();
	//getchar();
	//pr.GetProcessInfo();
	//pr.stopProcess();
	//pr.GetProcessInfo();
	//getchar();
	
	return 0;
}

