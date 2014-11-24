#include <Windows.h>
#pragma once

class logger
{
public:
	static logger * getInstance();
	enum
	{
		LogError = 1,
		LogWarning,
		LogInfo
	};
	void message(int logTp, const char* message);
	void SetLogLevel(int logLvl);
	void SetEnabled();
	void SetDisabled();
private:

	logger(const logger&);
	logger& operator=(logger&);

	static logger * p_instance;
	logger();
	~logger();
	int logLevel = LogInfo;
	int logType;
	bool enabled = true;
	HANDLE fileHandle = 0;
	char messageBuffer[1024];
	CRITICAL_SECTION mes_cs;
};

