#pragma once
#include <string>
#include <unordered_map>

#include "Game/Effects/DisplaySprite.h"

using namespace TEN::Effects::DisplaySprite;

/***
Constants for display sprite origin types.
@enum DisplaySprite.OriginType
@pragma nostrip
*/

/*** DisplaySprite.OriginType constants.

The following constants are inside OriginType.

CENTER
CENTER_TOP
CENTER_BOTTOM
CENTER_LEFT
CENTER_RIGHT
TOP_LEFT
TOP_RIGHT
BOTTOM_LEFT
BOTTOM_RIGHT

@section DisplaySprite.OriginType
*/

/*** Table of display sprite origin types.
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, DisplaySpriteOriginType> DISPLAY_SPRITE_ORIGIN_TYPES
{
	{ "CENTER", DisplaySpriteOriginType::Center },
	{ "CENTER_TOP", DisplaySpriteOriginType::CenterTop },
	{ "CENTER_BOTTOM", DisplaySpriteOriginType::CenterBottom },
	{ "CENTER_LEFT", DisplaySpriteOriginType::CenterLeft },
	{ "CENTER_RIGHT", DisplaySpriteOriginType::CenterRight },
	{ "TOP_LEFT", DisplaySpriteOriginType::TopLeft },
	{ "TOP_RIGHT", DisplaySpriteOriginType::TopRight },
	{ "BOTTOM_LEFT", DisplaySpriteOriginType::BottomLeft },
	{ "BOTTOM_RIGHT", DisplaySpriteOriginType::BottomRight }
};
