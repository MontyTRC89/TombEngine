#pragma once
#include <string>
#include <unordered_map>

#include "Game/effects/ScreenSprite.h"

using namespace TEN::Effects::DisplaySprite;

/***
Constants for display sprite scale modes.
@enum Effects.DisplaySpriteScaleMode
@pragma nostrip
*/

/*** Effects.DisplaySpriteScaleMode constants.

The following constants are inside DisplaySpriteScaleMode.

FIT
FILL
STRETCH
TILE

@section Effects.DisplaySpriteScaleMode
*/

/*** Table of display sprite scale modes.
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, DisplaySpriteScaleMode> DISPLAY_SPRITE_SCALE_MODES
{
	{ "FIT", DisplaySpriteScaleMode::Fit },
	{ "FILL", DisplaySpriteScaleMode::Fill },
	{ "STRETCH", DisplaySpriteScaleMode::Stretch },
	{ "TILE", DisplaySpriteScaleMode::Tile }
};
