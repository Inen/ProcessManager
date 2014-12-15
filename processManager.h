#include <Windows.h>
#include <iostream>
#include <functional>
#include <mutex>

#pragma once
class processManager
{
public:
	processManager();
	processManager(std::string & CmdLine);
	~processManager();
	void startProcess();
	void restartProcess();
	void stopProcess();
	void monitorFunc();
	void threadFunc();
	DWORD GetProcID();
	void GetProcessInfo();
	void SetOnProcStartCallback( std::function<void()> f);
	void SetOnProcManuallyStoppedCallback(std::function<void()>  f);
	void CatchProcess(DWORD dwProcessID);
	void SetCmdLine(std::string & CLine);

private:
	void runProcess();
	void Init();
	LPCTSTR lpApplicationName;
	std::string CmdLine;
	bool continueFlag = false;

	HANDLE Handle = 0;
	HANDLE threadHandle = 0;
	HANDLE monitorHandle = 0;
	DWORD procID = 0;
	enum 
	{
		IsStopped,
		IsRunning,
		IsRestarting
	};

	int statusFlag = IsStopped;

	std::function<void()> OnProcStart = nullptr;
	std::function<void()> OnProcRestart = nullptr;
	std::function<void()> OnProcManuallyStopped = nullptr;
	CRITICAL_SECTION onProcStart_cs;
	CRITICAL_SECTION onProcManuallyStopped_cs;
	std::mutex threadHandleMutex;
	std::mutex monitorHandleMutex;
	std::mutex processHandleMutex;
};

