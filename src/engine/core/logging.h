#pragma once

#include "engine/core/core.h"

enum RavenousLogLevel
{
	LOG_INFO    = 1,
	LOG_WARNING = 2,
	LOG_ERROR   = 3
};

inline void Log(RavenousLogLevel level, const std::string& message)
{
	std::string message_header = "\n";
	switch (level)
	{
		case LOG_INFO:
		{
			message_header += "> INFO message: ";
			break;
		}
		case LOG_WARNING:
		{
			message_header += "> WARNING message: ";
			break;
		}
		case LOG_ERROR:
		{
			message_header += "> ERROR message: ";
			break;
		}
	}

	printf(message_header.c_str());
	print(message.c_str());
}
