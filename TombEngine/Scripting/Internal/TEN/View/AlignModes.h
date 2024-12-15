#pragma once
#include <string>
#include <unordered_map>

#include "Game/Effects/DisplaySprite.h"

using namespace TEN::Effects::DisplaySprite;

namespace TEN::Scripting::View
{
/***
Constants for sprite align modes.
@enum View.AlignMode
@pragma nostrip
*/

/*** View.AlignMode constants.

The following constants are inside View.AlignMode.

CENTER
CENTER_TOP
CENTER_BOTTOM
CENTER_LEFT
CENTER_RIGHT
TOP_LEFT
TOP_RIGHT
BOTTOM_LEFT
BOTTOM_RIGHT

@section View.AlignMode
*/

/*** Table of align modes.
@table AlignMode
*/

	static const std::unordered_map<std::string, DisplaySpriteAlignMode> ALIGN_MODES
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
