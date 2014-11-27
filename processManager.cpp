#include "stdafx.h"
#include "processManager.h"
#include "logger.h"

#include <Aclapi.h>
#include <Winternl.h>

processManager::processManager()
{
	logger::getInstance()->message(logger::LogInfo, "Created processManager without cmdline.");

}
processManager::processManager(std::string & CmdL)
{
	if (!CmdL.empty() && Handle == NULL)
	{
		CmdLine = CmdL;
		logger::getInstance()->message(logger::LogInfo, "Command line saved.");
	}
	else {
		logger::getInstance()->message(logger::LogError, "Command line empty.");
	}
}

processManager::~processManager()
{
	continueFlag = false;
	stopProcess();
}

DWORD WINAPI threadFunction(LPVOID lpParam)
{
	logger::getInstance()->message(logger::LogInfo, "Thread created.");
	auto pm = static_cast<processManager*> (lpParam);

	pm->runProcess();

	logger::getInstance()->message(logger::LogInfo, "Exit the thread.");
	ExitThread(0);
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
	if (!threadHandle)
		threadHandle = CreateThread(nullptr, 0, &threadFunction, this, 0, nullptr);
}

void processManager::runProcess()
{
	ZeroMemory(&StartUpInfo, sizeof(STARTUPINFO));
	LPSTR s = const_cast <char *> (CmdLine.c_str());
	if (CreateProcess(lpApplicationName, s,
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
		TerminateProcess(Handle, NO_ERROR);	// убрать процесс
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

DWORD WINAPI GetCmdLineThread(LPVOID lpParam)
{
	auto pm = static_cast<processManager*> (lpParam);
	std::string cml = GetCommandLine();
	pm->SetCmdLine(cml);
	ExitThread(0);
}

void processManager::CatchProcess(DWORD dwProcessID)
{

	if (dwProcessID)
	{
		procID = dwProcessID;
		Handle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwProcessID);
		if (!Handle)
			logger::getInstance()->message(logger::LogError, "Failed to catch the process.");
		else
		{
			logger::getInstance()->message(logger::LogInfo, "Handle is captured.");

			//			if ( !CreateRemoteThread(Handle, nullptr, 0, &GetCmdLineThread, this, 0, nullptr))
			//			logger::getInstance()->message(logger::LogError, "Failed to create thread for recieving CmdLine");

			PROCESS_BASIC_INFORMATION peb;
			DWORD tmp;

			typedef int (WINAPI* ZwQueryInformationProcess)(HANDLE, DWORD, PROCESS_BASIC_INFORMATION*, DWORD, DWORD*);
			ZwQueryInformationProcess MyZwQueryInformationProcess;

			HMODULE hMod = GetModuleHandle("ntdll.dll");
			MyZwQueryInformationProcess = (ZwQueryInformationProcess)GetProcAddress(hMod, "ZwQueryInformationProcess");

			MyZwQueryInformationProcess(Handle, 0, &peb, sizeof(PROCESS_BASIC_INFORMATION), &tmp);

			UNICODE_STRING commandLine;
			PVOID rtlUserProcParamsAddress;

			/* get the address of ProcessParameters */
			if (!ReadProcessMemory(Handle, (PCHAR)peb.PebBaseAddress + 0x10, &rtlUserProcParamsAddress, sizeof(PVOID), NULL))
			{
				logger::getInstance()->message(logger::LogError, "Could not read the address of ProcessParameters!\n");
				std::cout << GetLastError() << std::endl;
			}
			/* read the CommandLine UNICODE_STRING structure */
			if (!ReadProcessMemory(Handle, (PCHAR)rtlUserProcParamsAddress + 0x40, &commandLine, sizeof(commandLine), NULL))
				logger::getInstance()->message(logger::LogError, "Could not read CommandLine!\n");

			WCHAR *commandLineContents;

			/* allocate memory to hold the command line */
			commandLineContents = (WCHAR *)malloc(commandLine.Length);
			memset(commandLineContents, 0, commandLine.Length);

			/* read the command line */
			if (!ReadProcessMemory(Handle, commandLine.Buffer, commandLineContents, commandLine.Length, NULL))
				logger::getInstance()->message(logger::LogError, "Could not read the command line string!\n");

			char buf[256];
			sprintf_s(buf, "%.*S", commandLine.Length / 2, commandLineContents);
			CmdLine = buf;

			free(commandLineContents);

			continueFlag = true;
			statusFlag = IsRunning;
			if (!monitorHandle)
			{
				statusFlag = IsRestarting;
				monitorHandle = CreateThread(nullptr, 0, &monitorFunction, this, 0, nullptr);
			}
			else
				logger::getInstance()->message(logger::LogError, "Unabled to monitor this process!\n");

		}
	}
}

void processManager::SetCmdLine(std::string & CLine)
{
	CmdLine = CLine;
	std::cout << &CmdLine << std::endl;
}