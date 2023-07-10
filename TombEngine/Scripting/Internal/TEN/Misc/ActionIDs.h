#pragma once
#include "Specific/Input/InputAction.h"

#include <string>
#include <unordered_map>

using namespace TEN::Input;

/***
Constants for action key IDs.
@enum Misc.ActionID
@pragma nostrip
*/

/*** Misc.ActionID constants.

The following constants are inside ActionID.

	FORWARD
	BACK
	LEFT
	RIGHT
	CROUCH
	SPRINT
	WALK
	JUMP
	ACTION
	DRAW
	FLARE
	LOOK
	ROLL
	INVENTORY
	PAUSE
	STEPLEFT
	STEPRIGHT

@section Misc.ActionID
*/

/*** Table of action ID constants (for use with KeyIsHeld / KeyIsHit / etc commands).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, ActionID> ACTION_IDS
{
	{ "FORWARD", ActionID::Forward },
	{ "BACK", ActionID::Back },
	{ "LEFT", ActionID::Left },
	{ "RIGHT", ActionID::Right },
	{ "STEP_LEFT", ActionID::StepLeft },
	{ "STEP_RIGHT", ActionID::StepRight },
	{ "ACTION", ActionID::Action },
	{ "JUMP", ActionID::Jump },
	{ "WALK", ActionID::Walk },
	{ "SPRINT", ActionID::Sprint },
	{ "CROUCH", ActionID::Crouch },
	{ "ROLL", ActionID::Roll },
	{ "DRAW", ActionID::Draw },
	{ "LOOK", ActionID::Look },

	{ "LIGHT", ActionID::Flare },
	{ "SMALL_MEDIPACK", ActionID::SmallMedipack },
	{ "LARGE_MEDIPACK", ActionID::LargeMedipack },
	{ "PREVIOUS_WEAPON", ActionID::PreviousWeapon },
	{ "NEXT_WEAPON", ActionID::NextWeapon },
	{ "WEAPON_1", ActionID::Weapon1 },
	{ "WEAPON_2", ActionID::Weapon2 },
	{ "WEAPON_3", ActionID::Weapon3 },
	{ "WEAPON_4", ActionID::Weapon4 },
	{ "WEAPON_5", ActionID::Weapon5 },
	{ "WEAPON_6", ActionID::Weapon6 },
	{ "WEAPON_7", ActionID::Weapon7 },
	{ "WEAPON_8", ActionID::Weapon8 },
	{ "WEAPON_9", ActionID::Weapon9 },
	{ "WEAPON_10", ActionID::Weapon10 },

	{ "SELECT", ActionID::Select },
	{ "DESELECT", ActionID::Deselect },
	{ "INVENTORY", ActionID::Option },
	{ "PAUSE", ActionID::Pause },
	{ "SAVE", ActionID::Save },
	{ "LOAD", ActionID::Load }
};
