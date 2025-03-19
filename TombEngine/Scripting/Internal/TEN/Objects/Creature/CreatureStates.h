#pragma once

#include "Game/itemdata/creature_info.h"
#include <unordered_map>
#include <string>

/// Constants for feather modes.
// @enum Effects.FeatherMode
// @pragma nostrip

/// Table of Effects.FeatherMode constants.
// To be used with @{Effects.EmitStreamer} function.
//
// - `NONE`
// - `CENTER`
// - `LEFT`
// - `RIGHT`
//
// @table Effects.FeatherMode

namespace TEN::Scripting::Creature
{
	static const auto CREATURE_MOOD = std::unordered_map<std::string, MoodType>
	{
       
        {"Bored", MoodType::Bored},
        {"Attack", MoodType::Attack},
        {"Escape", MoodType::Escape},
        {"Stalk", MoodType::Stalk}
	};
}
