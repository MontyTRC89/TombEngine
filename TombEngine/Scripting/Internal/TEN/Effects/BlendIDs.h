#pragma once

#include "Renderer/RendererEnums.h"

namespace TEN::Scripting::Effects
{
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

	static const auto BLEND_IDS = std::unordered_map<std::string, BlendMode>
	{
		{ "OPAQUE", BlendMode::Opaque },
		{ "ALPHA_TEST", BlendMode::AlphaTest },
		{ "ADDITIVE", BlendMode::Additive },
		{ "NO_DEPTH_TEST", BlendMode::NoDepthTest },
		{ "SUBTRACTIVE", BlendMode::Subtractive },
		{ "WIREFRAME", BlendMode::Wireframe },
		{ "EXCLUDE", BlendMode::Exclude },
		{ "SCREEN", BlendMode::Screen },
		{ "LIGHTEN", BlendMode::Lighten },
		{ "ALPHA_BLEND", BlendMode::AlphaBlend },

		// COMPATIBILITY
		{ "ALPHATEST", BlendMode::AlphaTest },
		{ "NOZTEST", BlendMode::NoDepthTest },
		{ "ALPHABLEND", BlendMode::AlphaBlend }
	};
}
