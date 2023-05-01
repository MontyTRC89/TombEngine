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

	ERROR
	WARNING
	INFO

@section Misc.LogLeve
*/

/*** Table of LogLevel ID constants (for use with Log command).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, LogLevel> kLogLevel
{
	{"ERROR", LogLevel::Error},
	{"WARNING", LogLevel::Warning},
	{"INFO", LogLevel::Info},
};
