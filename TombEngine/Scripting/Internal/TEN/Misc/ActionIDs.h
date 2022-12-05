#pragma once

#include "Specific/Input/InputAction.h"
#include <unordered_map>
#include <string>

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

static const std::unordered_map<std::string, ActionID> kActionIDs
{
	{"FORWARD", ActionID::Forward},
	{"BACK", ActionID::Back},
	{"LEFT", ActionID::Left},
	{"RIGHT", ActionID::Right},
	{"CROUCH", ActionID::Crouch},
	{"SPRINT", ActionID::Sprint},
	{"WALK", ActionID::Walk},
	{"JUMP", ActionID::Jump},
	{"ACTION", ActionID::Action},
	{"DRAW", ActionID::DrawWeapon},
	{"FLARE", ActionID::Flare},
	{"LOOK", ActionID::Look},
	{"ROLL", ActionID::Roll},
	{"INVENTORY", ActionID::Option},
	{"PAUSE", ActionID::Pause},
	{"STEPLEFT", ActionID::LeftStep},
	{"STEPRIGHT", ActionID::RightStep}
};
