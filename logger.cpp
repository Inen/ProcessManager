#include "stdafx.h"
#include "logger.h"
#include <mutex>

logger::logger()
{
	InitializeCriticalSection(&mes_cs);
}

logger::~logger()
{
	DeleteCriticalSection(&mes_cs);
}

logger *logger::p_instance = 0;
static std::mutex loggerInstanceMutex;

logger * logger::getInstance()
{
	std::lock_guard<std::mutex> lock(loggerInstanceMutex);
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

		if (logTp <= logLevel) {
			switch (logTp)
			{
			case LogError:
				sprintf_s(messageBuffer, MAX_BUFFER_LENGTH, "ERROR:%s\n", message);
				break;
			case LogWarning:
				sprintf_s(messageBuffer, MAX_BUFFER_LENGTH, "WARNING:%s\n", message);
				break;
			case LogInfo:
				sprintf_s(messageBuffer, MAX_BUFFER_LENGTH, "INFO:%s\n", message);
				break;
			default:
				sprintf_s(messageBuffer, MAX_BUFFER_LENGTH, "UNNOWN:%s\n", message);
				break;
			}
		}

		printf(messageBuffer);

		if (writeToFile)
		{
			if (!fileHandle)
				fileHandle = CreateFile("Log.txt", GENERIC_WRITE, 0, 0,
				OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

			WriteFile(fileHandle, messageBuffer, (DWORD)strlen(messageBuffer),
				&dwBytesWritten, NULL);
		}

	}
	LeaveCriticalSection(&mes_cs);
}

void logger::SetWriteToFile(bool fileFlag)
{
	writeToFile = fileFlag;
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