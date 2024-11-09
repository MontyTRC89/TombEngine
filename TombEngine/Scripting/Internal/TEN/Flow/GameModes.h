#pragma once
#include <string>
#include <unordered_map>

#include "Game/control/control.h"

/***
Constants for game modes.
@enum Flow.GameMode
@pragma nostrip
*/

/*** Flow.GameMode constants.

The following constants are inside Flow.GameMode.

	NORMAL
	FROZEN
	MENU

@section Flow.GameMode
*/

/*** Table of game modes.
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, GameMode> GAME_MODES
{
	{ "NORMAL", GameMode::Normal },
	{ "FROZEN", GameMode::Frozen },
	{ "MENU", GameMode::Menu }
};
