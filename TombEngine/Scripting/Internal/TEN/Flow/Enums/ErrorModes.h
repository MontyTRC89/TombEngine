#pragma once

#include "Game/control/control.h"
#include "Scripting/Internal/ScriptAssert.h"

/***
Constants for error modes.
@enum Flow.ErrorMode
@pragma nostrip
*/

/*** Flow.ErrorMode constants.

The following constants are inside Flow.ErrorMode: <br>

`ErrorMode.TERMINATE` - print to the log file and return to the title level when any script error is hit.
This is the one you will want to go for if you want to know IMMEDIATELY if something has gone wrong.

`ErrorMode.WARN` - print to the log file and continue running the application when a recoverable script error is hit.
Choose this one if booting to the title level is too much for you.

`ErrorMode.SILENT` - do nothing when a recoverable script error is hit.
Think __very__ carefully before using this setting. These error modes are here to help you to keep your scripts
working properly, but if you opt to ignore errors, you won't be alerted if you've misused a function or passed
an invalid argument.

In all of these modes, an *unrecoverable* error will boot you to the title level. If the title level itself
has an unrecoverable error, the game will close.

@section Flow.ErrorMode
*/

/*** Table of error modes.
@table ErrorMode
*/

static const std::unordered_map<std::string, ErrorMode> ERROR_MODES
{
	{ "SILENT", ErrorMode::Silent },
	{ "WARN", ErrorMode::Warn },
	{ "TERMINATE", ErrorMode::Terminate }
};

