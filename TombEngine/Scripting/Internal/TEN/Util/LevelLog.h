#pragma once
#include <string>
#include <unordered_map>

#include "Game/debug/debug.h"
#include "Scripting/Internal/ReservedScriptNames.h"

/***
Constants for LogLevel IDs.
@enum Util.LogLevel
@pragma nostrip
*/

/*** Util.LogLevel constants.
The following constants are inside LogLevel.

	INFO
	WARNING
	ERROR

@section Util.LogLevel
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
