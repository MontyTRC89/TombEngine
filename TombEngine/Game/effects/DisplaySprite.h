#pragma once
#include "Objects/game_object_ids.h"
#include "Renderer/Renderer11Enums.h"

namespace TEN::Effects::DisplaySprite
{
	enum class DisplaySpriteOriginType
	{
		Center,
		CenterTop,
		CenterBottom,
		CenterLeft,
		CenterRight,
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight
	};

	enum class DisplaySpriteScaleMode
	{
		Fit,
		Fill,
		Stretch,
		Tile
	};
	
	struct DisplaySprite
	{
		GAME_OBJECT_ID ObjectID	   = ID_DEFAULT_SPRITES;
		int			   SpriteIndex = 0;

		Vector2 Position	= Vector2::Zero;
		short	Orientation = 0;
		Vector2 Scale		= Vector2::One;
		Vector4 Color		= Vector4::One;

		int						Priority   = 0;
		DisplaySpriteOriginType OriginType = DisplaySpriteOriginType::Center;
		DisplaySpriteScaleMode	ScaleMode  = DisplaySpriteScaleMode::Fit;
		BLEND_MODES				BlendMode  = BLENDMODE_ALPHABLEND;
	};

	extern std::vector<DisplaySprite> DisplaySprites;
	
	void AddDisplaySprite(GAME_OBJECT_ID objectID, int spriteIndex, const Vector2& pos, short orient, const Vector2& scale, const Vector4& color,
						  int priority, DisplaySpriteOriginType originType, DisplaySpriteScaleMode scaleMode, BLEND_MODES blendMode);
	void ClearDisplaySprites();
}
