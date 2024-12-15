#pragma once

#include "Game/control/control.h"

namespace TEN::Scripting
{
/***
Constants for freeze modes.
@enum Flow.FreezeMode
@pragma nostrip
*/

/*** Flow.FreezeMode constants.

The following constants are inside Flow.FreezeMode.

	FreezeMode.NONE - Normal in-game operation.
	FreezeMode.FULL - Game is completely frozen, as in pause or inventory menus.
	FreezeMode.SPECTATOR - Game is completely frozen, but with ability to control camera.
	FreezeMode.PLAYER - Game is completely frozen, but with ability to control player. Experimental.

@section Flow.FreezeMode
*/

/*** Table of freeze modes.
@table FreezeMode
*/

	static const auto FREEZE_MODES = std::unordered_map<std::string, FreezeMode>
	{
		{ "NONE", FreezeMode::None },
		{ "FULL", FreezeMode::Full },
		{ "SPECTATOR", FreezeMode::Spectator },
		{ "PLAYER", FreezeMode::Player }
	};
}
