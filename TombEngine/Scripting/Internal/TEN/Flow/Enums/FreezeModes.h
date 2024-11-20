#pragma once

#include "Game/control/control.h"

/***
Constants for freeze modes.
@enum Flow.FreezeMode
@pragma nostrip
*/

/*** Flow.FreezeMode constants.

The following constants are inside Flow.FreezeMode.

	NONE - Normal in-game operation.
	FULL - Game is completely frozen, as in pause or inventory menus.
	SPECTATOR - Game is completely frozen, but with ability to control camera.
	PLAYER - Game is completely frozen, but with ability to control player. Experimental.

@section Flow.FreezeMode
*/

/*** Table of freeze modes.
@table CONSTANT_STRING_HERE
*/

static const auto FREEZE_MODES = std::unordered_map<std::string, FreezeMode>
{
	{ "NONE", FreezeMode::None },
	{ "FULL", FreezeMode::Full },
	{ "SPECTATOR", FreezeMode::Spectator },
	{ "PLAYER", FreezeMode::Player }
};
