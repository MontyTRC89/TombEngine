#pragma once
#include <string>
#include <unordered_map>

#include "Game/Effects/DisplaySprite.h"

using namespace TEN::Effects::DisplaySprite;

namespace TEN::Scripting::DisplaySprite
{
	/***
	Constants for display sprite align modes.
	@enum DisplaySprite.AlignMode
	@pragma nostrip
	*/

	/*** DisplaySprite.AlignMode constants.

	The following constants are inside AlignMode.

	CENTER
	CENTER_TOP
	CENTER_BOTTOM
	CENTER_LEFT
	CENTER_RIGHT
	TOP_LEFT
	TOP_RIGHT
	BOTTOM_LEFT
	BOTTOM_RIGHT

	@section DisplaySprite.AlignMode
	*/

	/*** Table of display sprite align modes.
	@table CONSTANT_STRING_HERE
	*/

	static const std::unordered_map<std::string, DisplaySpriteAlignMode> DISPLAY_SPRITE_ALIGN_MODES
	{
		{ "CENTER", DisplaySpriteAlignMode::Center },
		{ "CENTER_TOP", DisplaySpriteAlignMode::CenterTop },
		{ "CENTER_BOTTOM", DisplaySpriteAlignMode::CenterBottom },
		{ "CENTER_LEFT", DisplaySpriteAlignMode::CenterLeft },
		{ "CENTER_RIGHT", DisplaySpriteAlignMode::CenterRight },
		{ "TOP_LEFT", DisplaySpriteAlignMode::TopLeft },
		{ "TOP_RIGHT", DisplaySpriteAlignMode::TopRight },
		{ "BOTTOM_LEFT", DisplaySpriteAlignMode::BottomLeft },
		{ "BOTTOM_RIGHT", DisplaySpriteAlignMode::BottomRight }
	};
}
