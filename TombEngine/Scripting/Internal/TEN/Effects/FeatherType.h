#pragma once
#include "framework.h"
#include "Game/effects/Streamer.h"

/// Constants for Streamer Feather type IDs.
// @enum Effects.StreamerFeatherType
// @pragma nostrip

/// Table of Effects.StreamerFeatherType constants.
// To be used with @{Effects.EmitStreamer} function.
//
// - `NONE`
// - `CENTER`
// - `LEFT`
// - `RIGHT`
//
// @table Effects.StreamerFeatherType

using namespace TEN::Effects::Streamer;

namespace TEN::Scripting::Effects
{
	static const auto STREAMER_FEATHER_TYPE = std::unordered_map<std::string, StreamerFeatherType>
	{
		{ "NONE", StreamerFeatherType::None },
		{ "CENTER",StreamerFeatherType::Center },
		{ "LEFT", StreamerFeatherType::Left },
		{ "RIGHT",StreamerFeatherType::Right},
	};
}
