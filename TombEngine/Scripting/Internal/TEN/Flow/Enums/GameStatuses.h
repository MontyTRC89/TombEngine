#pragma once

#include "Game/control/control.h"

namespace TEN::Scripting
{
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
@table GameStatus
*/

	static const auto GAME_STATUSES = std::unordered_map<std::string, GameStatus>
	{
		{ "NORMAL", GameStatus::Normal },
		{ "NEW_GAME", GameStatus::NewGame },
		{ "LOAD_GAME", GameStatus::LoadGame },
		{ "EXIT_TO_TITLE", GameStatus::ExitToTitle },
		{ "EXIT_GAME", GameStatus::ExitGame },
		{ "LARA_DEAD", GameStatus::LaraDead }, // TODO: Rename to PLAYER_DEAD
		{ "LEVEL_COMPLETE", GameStatus::LevelComplete }
	};
}
