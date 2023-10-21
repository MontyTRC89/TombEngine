#pragma once
#include "Specific/Input/InputAction.h"

#include <string>
#include <unordered_map>

using namespace TEN::Input;

/***
Constants for action key IDs.
@enum Input.ActionID
@pragma nostrip
*/

/*** Input.ActionID constants.

The following constants are inside ActionID.

	FORWARD
	BACK
	LEFT
	RIGHT
	STEP_LEFT
	STEP_RIGHT
	WALK
	SPRINT
	CROUCH
	JUMP
	ROLL
	ACTION
	DRAW
	LOOK

	ACCELERATE
	REVERSE
	SPEED
	SLOW
	BRAKE
	FIRE

	FLARE
	SMALL_MEDIPACK
	LARGE_MEDIPACK
	PREVIOUS_WEAPON
	NEXT_WEAPON
	WEAPON_1
	WEAPON_2
	WEAPON_3
	WEAPON_4
	WEAPON_5
	WEAPON_6
	WEAPON_7
	WEAPON_8
	WEAPON_9
	WEAPON_10

	SELECT
	DESELECT
	PAUSE
	INVENTORY
	SAVE
	LOAD

@section Input.ActionID
*/

/*** Table of action ID constants (for use with KeyIsHeld / KeyIsHit / etc commands).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, ActionID> ACTION_IDS
{
	{ "FORWARD", In::Forward },
	{ "BACK", In::Back },
	{ "LEFT", In::Left },
	{ "RIGHT", In::Right },
	{ "STEP_LEFT", In::StepLeft },
	{ "STEP_RIGHT", In::StepRight },
	{ "WALK", In::Walk },
	{ "SPRINT", In::Sprint },
	{ "CROUCH", In::Crouch },
	{ "JUMP", In::Jump },
	{ "ROLL", In::Roll },
	{ "ACTION", In::Action },
	{ "DRAW", In::Draw },
	{ "LOOK", In::Look },

	{ "ACCELERATE", In::Accelerate },
	{ "REVERSE", In::Reverse },
	{ "SPEED", In::Speed },
	{ "SLOW", In::Slow },
	{ "BRAKE", In::Brake },
	{ "FIRE", In::Fire },

	{ "FLARE", In::Flare },
	{ "SMALL_MEDIPACK", In::SmallMedipack },
	{ "LARGE_MEDIPACK", In::LargeMedipack },
	{ "PREVIOUS_WEAPON", In::PreviousWeapon },
	{ "NEXT_WEAPON", In::NextWeapon },
	{ "WEAPON_1", In::Weapon1 },
	{ "WEAPON_2", In::Weapon2 },
	{ "WEAPON_3", In::Weapon3 },
	{ "WEAPON_4", In::Weapon4 },
	{ "WEAPON_5", In::Weapon5 },
	{ "WEAPON_6", In::Weapon6 },
	{ "WEAPON_7", In::Weapon7 },
	{ "WEAPON_8", In::Weapon8 },
	{ "WEAPON_9", In::Weapon9 },
	{ "WEAPON_10", In::Weapon10 },

	{ "SELECT", In::Select },
	{ "DESELECT", In::Deselect },
	{ "PAUSE", In::Pause },
	{ "INVENTORY", In::Inventory },
	{ "SAVE", In::Save },
	{ "LOAD", In::Load }
};
