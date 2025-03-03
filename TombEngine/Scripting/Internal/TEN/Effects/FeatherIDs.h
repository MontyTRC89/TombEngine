#pragma once

#include "Game/effects/Streamer.h"

/// Constants for feather IDs.
// @enum Effects.FeatherID
// @pragma nostrip

/// Table of Effects.FeatherID constants.
// To be used with @{Effects.EmitStreamer} function.
//
// - `NONE`
// - `CENTER`
// - `LEFT`
// - `RIGHT`
//
// @table Effects.FeatherID

using namespace TEN::Effects::Streamer;

namespace TEN::Scripting::Effects
{
	static const auto FEATHER_IDS = std::unordered_map<std::string, StreamerFeatherType>
	{
		{ "NONE", StreamerFeatherType::None },
		{ "CENTER", StreamerFeatherType::Center },
		{ "LEFT", StreamerFeatherType::Left },
		{ "RIGHT",StreamerFeatherType::Right}
	};
}
