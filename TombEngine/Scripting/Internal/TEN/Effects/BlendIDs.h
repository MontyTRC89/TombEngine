#pragma once
#include <string>
#include <unordered_map>

#include "Renderer/RendererEnums.h"

/***
Constants for blend mode IDs.
@enum Effects.BlendID
@pragma nostrip
*/

/*** Effects.BlendID constants.

The following constants are inside BlendID.

	OPAQUE  
	ALPHATEST  
	ADDITIVE  
	NOZTEST  
	SUBTRACTIVE  
	WIREFRAME  
	EXCLUDE  
	SCREEN  
	LIGHTEN  
	ALPHABLEND  

@section Effects.BlendID
*/

/*** Table of blend mode constants (for use with particles).
@table BlendID
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
