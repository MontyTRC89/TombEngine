#pragma once

#include <unordered_map>
#include <string>
#include "Game/items.h"

/***
Constants for effect IDs.
@enum Effects.EffectID
@pragma nostrip
*/

/*** Effects.EffectID constants.

The following constants are inside EffectID.

	BURN
	ELECTRIC
	SMOKE

@section Misc.EffectID
*/

/*** Table of effect ID constants (for use Moveable:SetEffect / Moveable:GetEffect functions).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, EffectType> kEffectIDs
{
	{"NONE", EffectType::None},
	{"BURN", EffectType::Burn},
	{"ELECTRIC", EffectType::Electric},
	{"SMOKE", EffectType::Smoke}
};
