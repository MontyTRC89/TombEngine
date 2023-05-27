#pragma once
#include <unordered_map>
#include <string>

/***
Constants for LevelLog IDs.
@enum Misc.LevelLog
@pragma nostrip
*/

/*** Misc.LevelLog constants.

The following constants are inside LevelLog.

	INFO
	WARNING
	ERROR

@section Misc.LevelLog
*/

/*** Table of LevelLog ID constants (for use with PrintLog() command).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, LogLevel> kLevelLog
{
	{ScriptReserved_LogLevelInfo, LogLevel::Info},
	{ScriptReserved_LogLevelWarning, LogLevel::Warning},
	{ScriptReserved_LogLevelError, LogLevel::Error}
};
