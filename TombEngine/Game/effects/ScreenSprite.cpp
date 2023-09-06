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

	void AddScreenSprite(GAME_OBJECT_ID objectID, int spriteIndex, const Vector2& pos, const Vector2& size, short orient,
						 const Vector4& color, int priority, BLEND_MODES blendMode)
	{
		auto screenSprite = ScreenSprite{};

		screenSprite.ObjectID = objectID;
		screenSprite.SpriteIndex = spriteIndex;
		screenSprite.Position = pos;
		screenSprite.Size = size;
		screenSprite.Orientation = orient;
		screenSprite.Color = color;
		screenSprite.Priority = priority;
		screenSprite.BlendMode = blendMode;

		ScreenSprites.push_back(screenSprite);
	}

	void ClearScreenSprites()
	{
		ScreenSprites.clear();
	}
}