#pragma once
#include <string>
#include <unordered_map>

#include "Game/control/control.h"

/***
Constants for game statuses.
@enum Flow.GameStatus
@pragma nostrip
*/

/*** Flow.GameStatus constants.

The following constants are inside Flow.GameStatus.

	NORMAL
	NEW_GAME
	LOAD_GAME
	EXIT_TO_TITLE
	EXIT_GAME
	LARA_DEAD
	LEVEL_COMPLETE

@section Flow.GameStatus
*/

/*** Table of game statuses.
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, GameStatus> GAME_STATUSES
{
	{ "NORMAL", GameStatus::Normal },
	{ "NEW_GAME", GameStatus::NewGame },
	{ "LOAD_GAME", GameStatus::LoadGame },
	{ "EXIT_TO_TITLE", GameStatus::ExitToTitle },
	{ "EXIT_GAME", GameStatus::ExitGame },
	{ "LARA_DEAD", GameStatus::LaraDead },
	{ "LEVEL_COMPLETE", GameStatus::LevelComplete }
};
