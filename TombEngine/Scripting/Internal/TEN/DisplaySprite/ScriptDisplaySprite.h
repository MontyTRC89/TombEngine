#pragma once
#include "Game/effects/DisplaySprite.h"
#include "Objects/game_object_ids.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"

enum BLEND_MODE;

using namespace TEN::Effects::DisplaySprite;

// TODO:
// Check if align modes are applied correctly.
// Check if ScriptDisplaySprite object has been registered correctly.
// Check if its methods have been registered correctly.
// Register DISPLAY_SPRITE_SCALE_MODES and DISPLAY_SPRITE_ORIGIN_TYPES under the correct sub-thing.
// Test nested sprites in various modes and how they behave when the aspect ratio changes.
// Check with everyone if the mySprite.Draw(blah, blah, blah) syntax is sensible.

namespace TEN::Scripting::DisplaySprite
{
	class ScriptDisplaySprite
	{
	public:
		static void Register(sol::table& parent);
		static void RegisterTables(sol::state* state, sol::table& parent);

		// Members
		GAME_OBJECT_ID ObjectID = GAME_OBJECT_ID::ID_DEFAULT_SPRITES;
		int			   SpriteID = 0;

		Vec2		Position = Vec2(0.0f, 0.0f);
		float		Rotation = 0.0f;
		Vec2		Scale = Vec2(0.0f, 0.0f);
		ScriptColor Color = ScriptColor(255, 255, 255, 255);

		// Constructors
		ScriptDisplaySprite(GAME_OBJECT_ID objectID, int spriteID, const Vec2& pos, float rot, const Vec2& scale,
							sol::optional<const ScriptColor&> color);

		// Utilities
		void Draw(sol::optional<int> priority, sol::optional<DisplaySpriteAlignMode> alignMode,
				  sol::optional<DisplaySpriteScaleMode> scaleMode, sol::optional<BLEND_MODES> blendMode);
	};
}
