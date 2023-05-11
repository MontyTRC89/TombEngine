#pragma once
#include <string>
#include <unordered_map>

/***
Constants for LogLevel IDs.
@enum Misc.LogLevel
@pragma nostrip
*/

/*** Misc.LogLevel constants.
The following constants are inside LogLevel.

	INFO
	WARNING
	ERROR

@section Misc.LogLevel
*/

/*** Table of LogLevel ID constants (for use with PrintLog() command).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, LogLevel> LOG_LEVEL
{
	{ ScriptReserved_LogLevelInfo, LogLevel::Info },
	{ ScriptReserved_LogLevelWarning, LogLevel::Warning },
	{ ScriptReserved_LogLevelError, LogLevel::Error }
};
