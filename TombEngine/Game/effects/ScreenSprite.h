#pragma once

#include "Objects/game_object_ids.h"
#include "Renderer/Renderer11Enums.h"

namespace TEN::Effects::ScreenSprite
{
	struct ScreenSprite
	{
		Vector2 Position = Vector2::Zero;
		Vector2 Size = Vector2::One;
		short Angle = 0;
		int Priority = 0;
		GAME_OBJECT_ID ObjectNumber = ID_DEFAULT_SPRITES;
		short SpriteIndex = 0;
		Vector3 Color = Vector3::One;
		float Opacity = 0.0f;
		BLEND_MODES BlendMode = BLENDMODE_ALPHABLEND;
	};

	extern std::vector<ScreenSprite> ScreenSprites;

	void AddScreenSprite(GAME_OBJECT_ID objectNumber, short spriteIndex, const Vector2& pos, const Vector2& size, const Vector3& color,
		BLEND_MODES blendMode, short angle, float opacity, int priority);
	void ClearScreenSprites();
}
