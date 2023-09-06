#include "framework.h"
#include "Game/effects/ScreenSprite.h"

#include "Game/effects/effects.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/objectslist.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Math;

namespace TEN::Effects::ScreenSprite
{
	std::vector<ScreenSprite> ScreenSprites = {};

	void AddScreenSprite(GAME_OBJECT_ID objectNumber, short spriteIndex, const Vector2& pos, const Vector2& size, const Vector3& color,
		BLEND_MODES blendMode, short angle, float opacity, int priority)
	{
		ScreenSprite screenSprite;

		screenSprite.ObjectNumber = objectNumber;
		screenSprite.SpriteIndex = spriteIndex;
		screenSprite.Position = pos;
		screenSprite.Size = size;
		screenSprite.Color = color;
		screenSprite.Opacity = opacity;
		screenSprite.Angle = angle;
		screenSprite.Priority = priority;
		screenSprite.BlendMode = blendMode;

		ScreenSprites.push_back(screenSprite);
	}

	void ClearScreenSprites()
	{
		ScreenSprites.clear();
	}
}