#include "stdafx.h"
#include "processManager.h"
#include "logger.h"

processManager::processManager(TCHAR *CmdL)
{
	if (CmdL != NULL && Handle == NULL)
	{
		CmdLine = CmdL;
		logger::getInstance()->message(logger::LogInfo, "Command line saved.");
	}
}

processManager::~processManager()
{
	continueFlag = false;
}

DWORD WINAPI threadFunction(LPVOID lpParam)
{
	logger::getInstance()->message(logger::LogInfo, "Thread created.");
	auto pm = static_cast<processManager*> (lpParam);
	pm -> runProcess();
	ExitThread(0);
	logger::getInstance()->message(logger::LogInfo, "Exit the thread.");
}

DWORD WINAPI monitorFunction(LPVOID lpParam)
{
	auto pm = static_cast<processManager*> (lpParam);
	while (pm->isNeedToContinue())
	{
		DWORD dwExitCode = 0;
		GetExitCodeProcess(pm->GetHandle(), &dwExitCode);
		if (dwExitCode != STILL_ACTIVE)
		{
			
			logger::getInstance()->message(logger::LogInfo, "Process is not active.");
			logger::getInstance()->message(logger::LogInfo, "Trying to restart process...");
			pm->runProcess();
		}
		Sleep(100);
	}

	pm->stopProcess();

	ExitThread(0);
	logger::getInstance()->message(logger::LogInfo, "Exit the thread.");
}

void processManager::startProcess()
{
	if (OnProcStart)
		OnProcStart();
	if ( !threadHandle )
		threadHandle = CreateThread(nullptr, 0, &threadFunction, this, 0, nullptr);
}

void processManager::runProcess()
{
	ZeroMemory(&StartUpInfo, sizeof(STARTUPINFO));
	if (CreateProcess(lpApplicationName, CmdLine,
		NULL, NULL, FALSE, NULL, NULL, NULL, &StartUpInfo, &ProcInfo))
	{
		logger::getInstance()->message(logger::LogInfo, "Process successfully started.");
		continueFlag = true;
		statusFlag = IsRunning;
		Handle = ProcInfo.hProcess;
		procID = ProcInfo.dwProcessId;
		if (!monitorHandle)
		{
			statusFlag = IsRestarting;
			monitorHandle = CreateThread(nullptr, 0, &monitorFunction, this, 0, nullptr);
		}
	}
	else
	{
		logger::getInstance()->message(logger::LogError, "Process can not be started!");
		TerminateThread(threadHandle, NO_ERROR);
	}
}

void processManager::restartProcess()
{
	statusFlag = IsRestarting;
	if (OnProcRestart)
		OnProcRestart();
	if (Handle != 0)
	{
		TerminateProcess(ProcInfo.hProcess, NO_ERROR);
		logger::getInstance()->message(logger::LogInfo, "Process terminated.");
		Handle = NULL;
	}
	startProcess();
}


void processManager::stopProcess()
{	
	if (OnProcManuallyStopped)
		OnProcManuallyStopped();
	if (Handle != 0)
	{
		continueFlag = false;
		TerminateThread(monitorHandle, NO_ERROR);
		TerminateProcess(ProcInfo.hProcess, NO_ERROR);	// убрать процесс
		TerminateThread(threadHandle, NO_ERROR);
		statusFlag = IsStopped;
		logger::getInstance()->message(logger::LogInfo, "Process terminated.");
		Handle = 0;
	}
}

HANDLE processManager::GetHandle()
{
	return Handle;
}

bool processManager::isNeedToContinue()
{
	return continueFlag;
}

void processManager::SetOnProcStartCallback(std::function<void()> f)
{
	OnProcStart = f;
}

void processManager::SetOnProcRestartCallback(std::function<void()> f)
{
	OnProcRestart = f;
}

void processManager::SetOnProcManuallyStoppedCallback(std::function<void()> f)
{
	OnProcManuallyStopped = f;
}

DWORD processManager::GetProcID()
{
	return procID;
}

void processManager::GetProcessInfo()
{
	std::cout << "Process handle = " << Handle << std::endl;
	std::cout << "Process ID = " << procID << std::endl;

	switch (statusFlag)
	{
	case IsStopped:
		std::cout << "Status: Process is stopped" << std::endl;
		break;
	case IsRunning:
		std::cout << "Status: Process is running" << std::endl;
		break;
	case IsRestarting:
		std::cout << "Status: Process is restarting" << std::endl;
		break;
	default:
		std::cout << "Status is UNKNOWN" << std::endl;
		break;
 	}
}