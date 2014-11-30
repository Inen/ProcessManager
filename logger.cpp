#include "stdafx.h"
#include "logger.h"

logger::logger()
{
	InitializeCriticalSection(&mes_cs);
}

logger::~logger()
{
	DeleteCriticalSection(&mes_cs);
}

logger* logger::p_instance = 0;

logger * logger::getInstance()
{
	if (!p_instance)
		p_instance = new logger();
	return p_instance;
};

void logger::message(int logTp, const char* message)
{
	EnterCriticalSection(&mes_cs);
	DWORD dwBytesWritten = 0;
	if (enabled)
	{
		if (!fileHandle)
			fileHandle = CreateFile("Log.txt", GENERIC_WRITE, 0, 0,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (logTp <= logLevel) {
			switch (logTp)
			{
			case LogError:
				sprintf_s(messageBuffer, "ERROR:%s\n", message);
				break;
			case LogWarning:
				sprintf_s(messageBuffer, "WARNING:%s\n", message);
				break;
			case LogInfo:
				sprintf_s(messageBuffer, "INFO:%s\n", message);
				break;
			default:
				sprintf_s(messageBuffer, "UNNOWN:%s\n", message);
				break;
			}
		}

		printf(messageBuffer);
		WriteFile(fileHandle, messageBuffer, (DWORD)strlen(messageBuffer),
			&dwBytesWritten, NULL);

	}
	LeaveCriticalSection(&mes_cs);
}

void logger::SetLogLevel(int logLvl)
{
	if (logLvl >= LogInfo) 
		logLevel = LogInfo;
	else if (logLvl <= LogError) 
		logLevel = LogError;
	else 
		logLevel = LogWarning;
}

void logger::SetEnabled()
{
	enabled = true;
}

void logger::SetDisabled()
{
	enabled = false;
}