#pragma once
#include "Objects/game_object_ids.h"
#include "Renderer/Renderer11Enums.h"

namespace TEN::Effects::ScreenSprite
{
	struct ScreenSprite
	{
		GAME_OBJECT_ID ObjectID	   = ID_DEFAULT_SPRITES;
		int			   SpriteIndex = 0;

		Vector2 Position	= Vector2::Zero;
		short	Orientation = 0;
		Vector2 Size		= Vector2::One;
		Vector4 Color		= Vector4::One;

		int			Priority  = 0;
		BLEND_MODES BlendMode = BLENDMODE_ALPHABLEND;
	};

	extern std::vector<ScreenSprite> ScreenSprites;

	void AddScreenSprite(GAME_OBJECT_ID objectID, int spriteIndex, const Vector2& pos, short orient, const Vector2& size,
						 const Vector4& color, int priority, BLEND_MODES blendMode);
	void ClearScreenSprites();
}
