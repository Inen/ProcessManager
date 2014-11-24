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
		switch (logTp)
		{
		case LogError:
			if (logLevel >= LogError)
			{
				sprintf_s(messageBuffer, "ERROR:%s\n", message);
				printf(messageBuffer);
				WriteFile(fileHandle, messageBuffer, (DWORD)strlen(messageBuffer),
					&dwBytesWritten, NULL);
			}
			break;
		case LogWarning:
			if (logLevel >= LogWarning)
			{
				sprintf_s(messageBuffer, "WARNING:%s\n", message);
				printf(messageBuffer);
				WriteFile(fileHandle, messageBuffer, (DWORD)strlen(messageBuffer),
					&dwBytesWritten, NULL);
			}
			break;
		case LogInfo:
			if (logLevel == LogInfo)
			{
				sprintf_s(messageBuffer, "INFO:%s\n", message);
				printf(messageBuffer);
				WriteFile(fileHandle, messageBuffer, (DWORD)strlen(messageBuffer),
					&dwBytesWritten, NULL);
			}
			break;
		default:
			break;
		}
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