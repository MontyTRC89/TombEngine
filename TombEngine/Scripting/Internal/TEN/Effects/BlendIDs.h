#pragma once

// Last generated on 31/7/2022.

#include "Renderer11Enums.h"
#include <unordered_map>
#include <string>

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

@section Misc.BlendID
*/

/*** Table of blend mode constants (for use with particles).
@table CONSTANT_STRING_HERE
*/


static const std::unordered_map<std::string, BLEND_MODES> kBlendIDs{
	{"OPAQUE", BLENDMODE_OPAQUE},
	{"ALPHATEST", BLENDMODE_ALPHATEST},
	{"ADDITIVE", BLENDMODE_ADDITIVE},
	{"NOZTEST", BLENDMODE_NOZTEST},
	{"SUBTRACTIVE", BLENDMODE_SUBTRACTIVE},
	{"WIREFRAME", BLENDMODE_WIREFRAME},
	{"EXCLUDE", BLENDMODE_EXCLUDE},
	{"SCREEN", BLENDMODE_SCREEN},
	{"LIGHTEN", BLENDMODE_LIGHTEN},
	{"ALPHABLEND", BLENDMODE_ALPHABLEND}
};
