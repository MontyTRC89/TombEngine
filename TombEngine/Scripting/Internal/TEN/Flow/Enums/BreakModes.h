#pragma once
#include <string>
#include <unordered_map>

#include "Game/control/control.h"

/***
Constants for break modes.
@enum Flow.BreakMode
@pragma nostrip
*/

/*** Flow.BreakMode constants.

The following constants are inside Flow.BreakMode.

	NONE
	FULL
	SPECTATOR
	PLAYER

@section Flow.BreakMode
*/

/*** Table of break modes.
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, BreakMode> BREAK_MODES
{
	{ "NONE", BreakMode::None },
	{ "FULL", BreakMode::Full },
	{ "SPECTATOR", BreakMode::Spectator },
	{ "PLAYER", BreakMode::Player }
};
