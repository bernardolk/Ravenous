#pragma once

#include "engine/core/core.h"

enum RavenousLogLevel
{
	LOG_INFO    = 1,
	LOG_WARNING = 2,
	LOG_ERROR   = 3
};

inline void Log(RavenousLogLevel Level, const string& Message)
{
	string MessageHeader = "\n";
	switch (Level)
	{
		case LOG_INFO:
		{
			MessageHeader += "> INFO message: ";
			break;
		}
		case LOG_WARNING:
		{
			MessageHeader += "> WARNING message: ";
			break;
		}
		case LOG_ERROR:
		{
			MessageHeader += "> ERROR message: ";
			break;
		}
	}

	printf(MessageHeader.c_str());
	print(Message.c_str());
}
