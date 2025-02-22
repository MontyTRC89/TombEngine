#pragma once

#include "Scripting/Internal/TEN/Util/Collision.h"

#include "Game/collision/Point.h"

/// Constants for blend mode IDs.
// @enum Effects.BlendID
// @pragma nostrip

/// Table of Effects.BlendID constants.
//
// - `OPAQUE`
// - `ALPHA_TEST`
// - `ADDITIVE`
// - `NO_DEPTH_TEST`
// - `SUBTRACTIVE`
// - `EXCLUDE`
// - `SCREEN`
// - `LIGHTEN`
// - `ALPHA_BLEND`
//
// @table Effects.BlendID

namespace TEN::Scripting::Util
{
	static const auto FLOOR_MATERIAL = std::unordered_map<std::string, MaterialType>
	{
        { "Mud", MaterialType::Mud },
        { "Snow", MaterialType::Snow },
        { "Sand", MaterialType::Sand },
        { "Gravel", MaterialType::Gravel },
        { "Ice", MaterialType::Ice },
        { "Water", MaterialType::Water },
        { "Stone", MaterialType::Stone },
        { "Wood", MaterialType::Wood },
        { "Metal", MaterialType::Metal },
        { "Marble", MaterialType::Marble },
        { "Grass", MaterialType::Grass },
        { "Concrete", MaterialType::Concrete },
        { "OldWood", MaterialType::OldWood },
        { "OldMetal", MaterialType::OldMetal },
        { "Custom1", MaterialType::Custom1 },
        { "Custom2", MaterialType::Custom2 },
        { "Custom3", MaterialType::Custom3 },
        { "Custom4", MaterialType::Custom4 },
        { "Custom5", MaterialType::Custom5 },
        { "Custom6", MaterialType::Custom6 },
        { "Custom7", MaterialType::Custom7 },
        { "Custom8", MaterialType::Custom8 }
	};
}

