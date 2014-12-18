#include "stdafx.h"
#include "processManager.h"
#include "logger.h"

#include <Winternl.h>

void processManager::Init()
{
	InitializeCriticalSection(&onProcStart_cs);
	InitializeCriticalSection(&onProcManuallyStopped_cs);
}
processManager::processManager()
{
	Init();
	logger::getInstance()->message(logger::LogInfo, "Created processManager without cmdline.");

}
processManager::processManager(std::string & CmdL)
{
	Init();
	if (!CmdL.empty())
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
	DeleteCriticalSection(&onProcStart_cs);
	DeleteCriticalSection(&onProcManuallyStopped_cs);
	continueFlag = false;
	stopProcess();
}

DWORD WINAPI threadFunction(LPVOID lpParam)
{
	logger::getInstance()->message(logger::LogInfo, "Thread created.");
	auto pm = static_cast<processManager*> (lpParam);

	pm->threadFunc();
	ExitThread(0);
}

void processManager::threadFunc()
{
	STARTUPINFO StartUpInfo;
	PROCESS_INFORMATION ProcInfo;

	ZeroMemory(&StartUpInfo, sizeof(STARTUPINFO));
	LPSTR s = const_cast <char *> (CmdLine.c_str());
	if (!CmdLine.empty() && CreateProcess(lpApplicationName, s,
		NULL, NULL, FALSE, NULL, NULL, NULL, &StartUpInfo, &ProcInfo))
	{
		logger::getInstance()->message(logger::LogInfo, "Process successfully started.");
		continueFlag = true;
		statusFlag = IsRunning;

		processHandleMutex.lock();
		Handle = ProcInfo.hProcess;
		processHandleMutex.unlock();

		procID = ProcInfo.dwProcessId;
	}
	else
	{
		logger::getInstance()->message(logger::LogError, "Process can not be started!");
	}
	WaitForSingleObject(Handle, INFINITE);
}

DWORD WINAPI monitorFunction(LPVOID lpParam)
{
	auto pm = static_cast<processManager*> (lpParam);
	pm->monitorFunc();

	logger::getInstance()->message(logger::LogInfo, "Exit the monitor thread.");
	ExitThread(0);
}

void processManager::monitorFunc()
{
	continueFlag = true;

	while (continueFlag)
	{
		logger::getInstance()->message(logger::LogInfo, "Process is not active.");
		logger::getInstance()->message(logger::LogInfo, "Trying to restart process...");
		runProcess();

		WaitForSingleObject(threadHandle, INFINITE);
	}
}

void processManager::startProcess()
{
	threadHandleMutex.lock();
	if (threadHandle)
		TerminateThread(threadHandle, NO_ERROR);
	threadHandleMutex.unlock();

	monitorHandleMutex.lock();
	if (!monitorHandle)
	{
		statusFlag = IsRunning;
		monitorHandle = CreateThread(nullptr, 0, &monitorFunction, this, 0, nullptr);
	}
	monitorHandleMutex.unlock();

	if (OnProcStart)
	{
		EnterCriticalSection(&onProcStart_cs);
		OnProcStart();
		LeaveCriticalSection(&onProcStart_cs);
	}
}

void processManager::runProcess()
{
	std::lock_guard<std::mutex> l(threadHandleMutex);
	if (threadHandle)
		TerminateThread(threadHandle, NO_ERROR);

	threadHandle = CreateThread(nullptr, 0, &threadFunction, this, 0, nullptr);
}

void processManager::restartProcess()
{
	logger::getInstance()->message(logger::LogInfo, "Restarting process.");
	continueFlag = false;
	statusFlag = IsRestarting;

	stopProcess();
	startProcess();
}


void processManager::stopProcess()
{
	continueFlag = false;

	monitorHandleMutex.lock();
	if (monitorHandle && TerminateThread(monitorHandle, NO_ERROR))
	{
		logger::getInstance()->message(logger::LogInfo, "Monitor thread terminated.");
		monitorHandle = 0;
	}
	monitorHandleMutex.unlock();

	processHandleMutex.lock();
	if (Handle && TerminateProcess(Handle, NO_ERROR))	// убрать процесс
	{
		Handle = 0;

		if (OnProcManuallyStopped)
		{
			EnterCriticalSection(&onProcManuallyStopped_cs);
			OnProcManuallyStopped();
			LeaveCriticalSection(&onProcManuallyStopped_cs);
		}
		logger::getInstance()->message(logger::LogInfo, "Process terminated.");
	}
	processHandleMutex.unlock();

	threadHandleMutex.lock();
	if (threadHandle && TerminateThread(threadHandle, NO_ERROR))
	{
		logger::getInstance()->message(logger::LogInfo, "Thread terminated.");
		threadHandle = 0;
	}
	threadHandleMutex.unlock();
	statusFlag = IsStopped;

}

void processManager::SetOnProcStartCallback(std::function<void()> f)
{
	OnProcStart = f;
	logger::getInstance()->message(logger::LogInfo, "Added OnProcStartCallback.");
}

void processManager::SetOnProcManuallyStoppedCallback(std::function<void()> f)
{
	OnProcManuallyStopped = f;
	logger::getInstance()->message(logger::LogInfo, "Added OnProcManuallyStoppedCallback.");
}

DWORD processManager::GetProcID()
{
	return procID;
}

void processManager::GetProcessInfo()
{
	processHandleMutex.lock();
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
	processHandleMutex.unlock();
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

		stopProcess();

		processHandleMutex.lock();
		Handle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwProcessID);
		if (!Handle)
		{
			logger::getInstance()->message(logger::LogError, "Failed to catch the process.");
		}
		else
		{
			logger::getInstance()->message(logger::LogInfo, "Handle is captured.");

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
		}
		processHandleMutex.unlock();

		if (continueFlag)
		{
			monitorHandleMutex.lock();
			if (!monitorHandle)
			{
				statusFlag = IsRestarting;
				monitorHandle = CreateThread(nullptr, 0, &monitorFunction, this, 0, nullptr);
			}
			else
			{
				logger::getInstance()->message(logger::LogError, "Unabled to monitor this process!\n");
			}
			monitorHandleMutex.unlock();
		}
	}
}

void processManager::SetCmdLine(std::string & CLine)
{
	CmdLine = CLine;
	std::cout << &CmdLine << std::endl;
}