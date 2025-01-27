#pragma once
#include <string>
#include <unordered_map>

#include "Game/items.h"

/***
Constants for effect IDs.
@enum Effects.EffectID
@pragma nostrip
*/

/*** Table of Effects.EffectID constants.
To be used with @{Objects.Moveable.SetEffect} and @{Objects.Moveable.GetEffect} functions.

 - `NONE`
 - `FIRE`
 - `SPARKS`
 - `SMOKE`
 - `ELECTRICIGNITE`
 - `REDIGNITE`
 - `CADAVER`
 - `CUSTOM`

@table Effects.EffectID
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