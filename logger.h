#include <Windows.h>


#pragma once

#define MAX_BUFFER_LENGTH 1024

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
	void SetWriteToFile(bool fileFlag);

private:
	logger(const logger&);
	logger& operator=(logger&);
	static logger * p_instance;
	logger();
	~logger();

	int logLevel = LogInfo;
	int logType;
	bool enabled = true;
	bool writeToFile = false;
	HANDLE fileHandle = 0;
	char messageBuffer[MAX_BUFFER_LENGTH];
	CRITICAL_SECTION mes_cs;

};

