#include "framework.h"
#include <sol.hpp>
#include "GameScriptSettings.h"

/***
Settings that will be run on game startup.
@pregameclass Settings
@pragma nostrip
*/

void GameScriptSettings::Register(sol::state* lua)
{
	lua->new_usertype<GameScriptSettings>("Settings",
		"screenWidth", &GameScriptSettings::ScreenWidth,
		"screenHeight", &GameScriptSettings::ScreenHeight,
		"enableDynamicShadows", &GameScriptSettings::EnableDynamicShadows,
		"windowed", &GameScriptSettings::Windowed,
		"enableWaterCaustics", &GameScriptSettings::EnableWaterCaustics,
		"drawingDistance", &GameScriptSettings::DrawingDistance,
		"showRendererSteps", &GameScriptSettings::ShowRendererSteps,
		"showDebugInfo", &GameScriptSettings::ShowDebugInfo,

/*** How should the application respond to script errors?
Must be one of the following:
`ErrorMode.TERMINATE` - print to the log file and terminate the application when any script error is hit.
This is the one you will want to go for if you want to know IMMEDIATELY if something has gone wrong.

`ErrorMode.WARN` - print to the log file and continue running the application when a recoverable script error is hit.
Choose this one if terminating the application is too much for you. Note that unrecoverable errors will still terminate
the application.

`ErrorMode.SILENT` - do nothing when a recoverable script error is hit.
Think __very__ carefully before using this setting. These error modes are here to help you to keep your scripts
working properly, but if you opt to ignore errors, you won't be alerted if you've misused a function or passed
an invalid argument.

As with `ErrorMode.WARN`, unrecoverable errors will still terminate the application.
@mem errorMode
*/
		"errorMode", &GameScriptSettings::ErrorMode
		);
}
