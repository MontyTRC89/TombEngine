#pragma once
#include <string>
#include <unordered_map>

#include "Game/Debug/Debug.h"
#include "Scripting/Internal/ReservedScriptNames.h"

/***
Constants for LogLevel IDs.
@enum Util.LogLevel
@pragma nostrip
*/

/*** Table of Util.LogLevel constants. To be used with @{Util.PrintLog} function.

 - `INFO`
 - `WARNING`
 - `ERROR`

@table Util.LogLevel
*/

static const std::unordered_map<std::string, LogLevel> LOG_LEVEL
{
	{ ScriptReserved_LogLevelInfo, LogLevel::Info },
	{ ScriptReserved_LogLevelWarning, LogLevel::Warning },
	{ ScriptReserved_LogLevelError, LogLevel::Error }
};
