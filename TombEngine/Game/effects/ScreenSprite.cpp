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

	void AddScreenSprite(GAME_OBJECT_ID objectID, int spriteIndex, const Vector2& pos, short orient, const Vector2& scale,
						 const Vector4& color, int priority, BLEND_MODES blendMode, ScreenSpriteScaleMode scaleMode)
	{
		auto screenSprite = ScreenSprite{};

		screenSprite.ObjectID = objectID;
		screenSprite.SpriteIndex = spriteIndex;
		screenSprite.Position = pos;
		screenSprite.Orientation = orient;
		screenSprite.Scale = scale;
		screenSprite.Color = color;
		screenSprite.Priority = priority;
		screenSprite.BlendMode = blendMode;
		screenSprite.ScaleMode = scaleMode;

		ScreenSprites.push_back(screenSprite);
	}

	void ClearScreenSprites()
	{
		ScreenSprites.clear();
	}
}
