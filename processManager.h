#include <Windows.h>
#include <iostream>
#include <functional>

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
	void runProcess();
	bool isNeedToContinue();
	HANDLE GetHandle();
	DWORD GetProcID();
	void GetProcessInfo();
	void SetOnProcStartCallback( std::function<void()> f);
	void SetOnProcRestartCallback(std::function<void()> f);
	void SetOnProcManuallyStoppedCallback(std::function<void()>  f);
	void CatchProcess(DWORD dwProcessID);
	void SetCmdLine(std::string & CLine);

private:
	LPCTSTR lpApplicationName;
	std::string CmdLine;
	bool continueFlag = false;
	STARTUPINFO StartUpInfo;
	PROCESS_INFORMATION ProcInfo;
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
};

