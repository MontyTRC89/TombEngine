#pragma once
#include <string>
#include <unordered_map>

#include "Game/effects/DisplaySprite.h"

using namespace TEN::Effects::DisplaySprite;

namespace TEN::Scripting::View
{
/***
Constants for scale modes.
@enum View.ScaleMode
@pragma nostrip
*/

/*** View.ScaleMode constants.

The following constants are inside View.ScaleMode.

FIT
FILL
STRETCH

@section View.ScaleMode
*/

/*** Table of display sprite scale modes.
@table ScaleMode
*/

	static const std::unordered_map<std::string, DisplaySpriteScaleMode> SCALE_MODES
	{
		{ "FIT", DisplaySpriteScaleMode::Fit },
		{ "FILL", DisplaySpriteScaleMode::Fill },
		{ "STRETCH", DisplaySpriteScaleMode::Stretch }
	};
}
