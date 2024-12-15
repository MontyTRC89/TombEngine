#pragma once
#include "Renderer/RendererEnums.h"
#include <string>

/***
Constants for the post-process effects to apply.
@enum View.PostProcessMode
@pragma nostrip
*/

/*** View.PostProcessMode constants.

The following constants are inside PostProcessMode.

	NONE
	MONOCHROME
	NEGATIVE
	EXCLUSION

@section View.PostProcessMode
*/

/*** Table of post-process effect constants (for use with SetPostProcessMode() function).
@table PostProcessMode
*/

static const std::unordered_map<std::string, PostProcessMode> POSTPROCESS_MODES
{
	{ "NONE", PostProcessMode::None },
	{ "MONOCHROME", PostProcessMode::Monochrome },
	{ "NEGATIVE", PostProcessMode::Negative },
	{ "EXCLUSION", PostProcessMode::Exclusion }
};
