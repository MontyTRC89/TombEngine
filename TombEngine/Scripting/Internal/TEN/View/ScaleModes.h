#pragma once

#include "Game/effects/DisplaySprite.h"

using namespace TEN::Effects::DisplaySprite;

namespace TEN::Scripting::View
{

	/// Constants for display sprite scale modes.
	// @enum View.ScaleMode
	// @pragma nostrip

	/// Table of View.ScaleMode constants. To be used with @{View.DisplaySprite} class.
	// 
	// - `FIT`
	// - `FILL`
	// - `STRETCH`
	// 
	// @table View.ScaleMode

	static const std::unordered_map<std::string, DisplaySpriteScaleMode> SCALE_MODES
	{
		{ "FIT", DisplaySpriteScaleMode::Fit },
		{ "FILL", DisplaySpriteScaleMode::Fill },
		{ "STRETCH", DisplaySpriteScaleMode::Stretch }
	};
}
