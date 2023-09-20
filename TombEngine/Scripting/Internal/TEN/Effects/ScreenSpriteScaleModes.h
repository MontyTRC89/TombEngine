#pragma once
#include <string>
#include <unordered_map>

#include "Game/effects/ScreenSprite.h"

using namespace TEN::Effects::ScreenSprite;

/***
Constants for screen sprite scale modes.
@enum Effects.ScreenSpriteScaleMode
@pragma nostrip
*/

/*** Effects.ScreenSpriteScaleMode constants.

The following constants are inside ScreenSpriteScaleMode.

FIT
FILL
STRETCH
TILE

@section Effects.ScreenSpriteScaleMode
*/

/*** Table of screen sprite scale modes.
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, ScreenSpriteScaleMode> SCREEN_SPRITE_SCALE_MODES
{
	{ "FIT", ScreenSpriteScaleMode::Fit },
	{ "FILL", ScreenSpriteScaleMode::Fill },
	{ "STRETCH", ScreenSpriteScaleMode::Stretch },
	{ "TILE", ScreenSpriteScaleMode::Tile }
};
