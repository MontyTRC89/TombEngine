#pragma once
#include <unordered_map>
#include <string>

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

/*** Table of LogLevel ID constants (for use with Log command).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, LogLevel> kLevelLog
{
	{ScriptReserved_LogLevelInfo, LogLevel::Info},
	{ScriptReserved_LogLevelWarning, LogLevel::Warning},
	{ScriptReserved_LogLevelError,LogLevel::Error}
};
