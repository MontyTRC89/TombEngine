#include "framework.h"
#include <iostream>

void TENLog(std::string_view str, LogLevel level, LogConfig config)
{
	if constexpr (!DebugBuild)
	{
		if (LogConfig::Debug == config)
		{
			return;
		}
	}

	switch (level)
	{
	case LogLevel::Error:
		std::cerr << "Error: " << str << "\n";
		// Monty code goes here
		break;
	case LogLevel::Warning:
		std::cout << "Warning: " << str << "\n";
		// Monty code goes here
		break;
	case LogLevel::Info:
		std::cout << "Info: " << str << "\n";
		// Monty code goes here
		break;
	}
}
