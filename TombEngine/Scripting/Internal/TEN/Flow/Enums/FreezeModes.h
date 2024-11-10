#pragma once
#include <string>
#include <unordered_map>

#include "Game/control/control.h"

/***
Constants for freeze modes.
@enum Flow.FreezeMode
@pragma nostrip
*/

/*** Flow.FreezeMode constants.

The following constants are inside Flow.FreezeMode.

	NONE - Normal in-game operation.
	FULL - Game is completely frozen, as in pause or inventory menu.
	SPECTATOR - As above, but with ability to control camera.
	PLAYER - As above, but with ability to control player. Experimental mode.

@section Flow.FreezeMode
*/

/*** Table of freeze modes.
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, FreezeMode> FREEZE_MODES
{
	{ "NONE", FreezeMode::None },
	{ "FULL", FreezeMode::Full },
	{ "SPECTATOR", FreezeMode::Spectator },
	{ "PLAYER", FreezeMode::Player }
};
