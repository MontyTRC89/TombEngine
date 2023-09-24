#pragma once
#include <string>
#include <unordered_map>

#include "Game/effects/DisplaySprite.h"

using namespace TEN::Effects::DisplaySprite;

/***
Constants for display sprite scale modes.
@enum DisplaySprite.DisplaySpriteScaleMode
@pragma nostrip
*/

/*** DisplaySprite.DisplaySpriteScaleMode constants.

The following constants are inside DisplaySpriteScaleMode.

FIT
FILL
STRETCH
TILE

@section DisplaySprite.DisplaySpriteScaleMode
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
