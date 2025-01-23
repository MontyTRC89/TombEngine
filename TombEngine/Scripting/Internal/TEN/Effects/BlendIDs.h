#pragma once

#include "Renderer/RendererEnums.h"

/***
Constants for blend mode IDs.
@enum Effects.BlendID
@pragma nostrip
*/

/*** Table of Effects.BlendID constants (for use with particles).

 - `OPAQUE`
 - `ALPHATEST`
 - `ADDITIVE`
 - `SUBTRACTIVE`
 - `EXCLUDE`
 - `SCREEN`
 - `LIGHTEN`
 - `ALPHABLEND`

@table Effects.BlendID
*/

static const std::unordered_map<std::string, BlendMode> BLEND_IDS
{
	{ "OPAQUE", BlendMode::Opaque },
	{ "ALPHATEST", BlendMode::AlphaTest },
	{ "ADDITIVE", BlendMode::Additive },
	{ "NOZTEST", BlendMode::NoDepthTest },
	{ "SUBTRACTIVE", BlendMode::Subtractive },
	{ "WIREFRAME", BlendMode::Wireframe },
	{ "EXCLUDE", BlendMode::Exclude },
	{ "SCREEN", BlendMode::Screen },
	{ "LIGHTEN", BlendMode::Lighten },
	{ "ALPHABLEND", BlendMode::AlphaBlend }
};
