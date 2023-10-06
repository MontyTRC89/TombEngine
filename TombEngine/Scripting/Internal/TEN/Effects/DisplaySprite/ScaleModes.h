#pragma once
#include <string>
#include <unordered_map>

#include "Game/effects/DisplaySprite.h"

using namespace TEN::Effects::DisplaySprite;

namespace TEN::Scripting::DisplaySprite
{
	/***
	Constants for display sprite scale modes.
	@enum DisplaySprite.ScaleMode
	@pragma nostrip
	*/

	/*** DisplaySprite.ScaleMode constants.

	The following constants are inside DisplaySprite.ScaleMode.

	FIT
	FILL
	STRETCH

	@section DisplaySprite.ScaleMode
	*/

	/*** Table of display sprite scale modes.
	@table CONSTANT_STRING_HERE
	*/

	static const std::unordered_map<std::string, DisplaySpriteScaleMode> DISPLAY_SPRITE_SCALE_MODES
	{
		{ "FIT", DisplaySpriteScaleMode::Fit },
		{ "FILL", DisplaySpriteScaleMode::Fill },
		{ "STRETCH", DisplaySpriteScaleMode::Stretch }
	};
}
