#pragma once
#include "Renderer/RendererEnums.h"
#include <string>

/***
Constants for the post-process color effects to apply.
@enum View.PostProcessColorEffect
@pragma nostrip
*/

/*** View.PostProcessColorEffect constants.

The following constants are inside PostProcessColorEffect.

	NORMAL
	SEPIA
	MONOCHROME
	NEGATIVE

@section View.PostProcessColorEffect
*/

/*** Table of post-process color effects constants (for use with SetPostProcessColorEffect() function).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, PostProcessColorEffect> POSTPROCESS_COLOR_EFFECTS
{
	{ "NORMAL", PostProcessColorEffect::Normal },
	{ "SEPIA", PostProcessColorEffect::Sepia },
	{ "MONOCHROME", PostProcessColorEffect::Monochrome },
	{ "NEGATIVE", PostProcessColorEffect::Negative }
};
