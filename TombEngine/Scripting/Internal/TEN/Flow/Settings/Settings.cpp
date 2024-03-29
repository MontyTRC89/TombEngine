#include "framework.h"
#include "Settings.h"

/***
Settings that will be run on game startup.
@tenclass Flow.Settings
@pragma nostrip
*/

void Settings::Register(sol::table & parent)
{
	parent.new_usertype<Settings>("Settings",
		sol::constructors<Settings()>(),
		sol::call_constructor, sol::constructors<Settings>(),

/*** How should the application respond to script errors?
Must be one of the following:
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

@mem errorMode
*/
		"errorMode", &Settings::ErrorMode
		);
}
