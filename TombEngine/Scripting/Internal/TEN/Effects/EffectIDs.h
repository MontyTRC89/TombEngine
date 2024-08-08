#pragma once
#include <string>
#include <unordered_map>

#include "Game/items.h"

/***
Constants for effect IDs.
@enum Effects.EffectID
@pragma nostrip
*/

/*** Effects.EffectID constants.

The following constants are inside EffectID.

	NONE
	FIRE
	SPARKS
	SMOKE
	ELECTRICIGNITE
	REDIGNITE
	CADAVER
	CUSTOM

@section Effects.EffectID
*/

/*** Table of effect ID constants (for use Moveable:SetEffect / Moveable:GetEffect functions).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, EffectType> EFFECT_IDS
{
	{ "NONE", EffectType::None },
	{ "FIRE", EffectType::Fire },
	{ "SPARKS", EffectType::Sparks },
	{ "SMOKE", EffectType::Smoke},
	{ "ELECTRICIGNITE", EffectType::ElectricIgnite },
	{ "REDIGNITE", EffectType::RedIgnite },
	{ "CUSTOM", EffectType::Custom }
};
