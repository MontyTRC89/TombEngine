#include "framework.h"
#include "Scripting/Internal/TEN/Effects/DisplaySprite/ScriptDisplaySprite.h"

#include "Game/effects/DisplaySprite.h"
#include "Game/Setup.h"
#include "Objects/game_object_ids.h"
#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/Effects/DisplaySprite/AlignModes.h"
#include "Scripting/Internal/TEN/Effects/DisplaySprite/ScaleModes.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"

using namespace TEN::Effects::DisplaySprite;

void ScriptDisplaySprite::Register(sol::table& parent)
{
	using ctors = sol::constructors<ScriptDisplaySprite(GAME_OBJECT_ID, int, const Vec2&, float, const Vec2&, const ScriptColor&)>;

	parent.new_usertype<ScriptDisplaySprite>(
		ScriptReserved_DisplaySprite,
		ctors(),
		sol::call_constructor, ctors(),

		/*** Draw the display sprite in display space for the current frame.
		@function DisplaySprite:Draw
		@tparam int[opt] priority Draw priority of the sprite. Can be thought of as a layer, with higher values having higher priority. Default is 0.
		@tparam DisplaySprite.AlignMode[opt] alignMode Align mode of the sprite. Default is DisplaySprite.AlignMode.CENTER.
		@tparam DisplaySprite.ScaleMode[opt] scaleMode Scale mode of the sprite. Default is DisplaySprite.ScaleMode.FIT.
		@tparam Effects.BlendID[opt] blendMode Blend mode of the sprite. Default is Effects.BlendID.ALPHABLEND.
		*/
		ScriptReserved_DisplaySpriteDraw, &ScriptDisplaySprite::Draw);

	// TODO: How???
	/*auto tableDisplaySprite = sol::table(state->lua_state(), sol::create);
	parent.set(ScriptReserved_DisplaySprite, tableDisplaySprite);

	auto handler = LuaHandler{ state };
	handler.MakeReadOnlyTable(tableDisplaySprite, ScriptReserved_DisplaySpriteAlignMode, DISPLAY_SPRITE_ALIGN_MODES);
	handler.MakeReadOnlyTable(tableDisplaySprite, ScriptReserved_DisplaySpriteScaleMode, DISPLAY_SPRITE_SCALE_MODES);*/
}

ScriptDisplaySprite::ScriptDisplaySprite(GAME_OBJECT_ID objectID, int spriteIndex, const Vec2& pos, float rot, const Vec2& scale,
										 const ScriptColor& color)
{
	ObjectID = objectID;
	SpriteIndex = spriteIndex;
	Position = pos;
	Rotation = rot;
	Scale = scale;
	Color = color;
}

void ScriptDisplaySprite::Draw(sol::optional<int> priority, sol::optional<DisplaySpriteAlignMode> alignMode,
							   sol::optional<DisplaySpriteScaleMode> scaleMode, sol::optional<BLEND_MODES> blendMode)
{
	// NOTE: Conversion from more intuitive 100x100 screen space resolution to internal 800x600 is required.
	// In a future refactor, everything will use 100x100 natively. -- Sezz 2023.08.31
	constexpr auto POS_CONVERSION_COEFF = Vector2(SCREEN_SPACE_RES.x / 100, SCREEN_SPACE_RES.y / 100);

	constexpr auto DEFAULT_PRIORITY	  = 0;
	constexpr auto DEFAULT_ALIGN_MODE = DisplaySpriteAlignMode::Center;
	constexpr auto DEFAULT_SCALE_MODE = DisplaySpriteScaleMode::Fit;
	constexpr auto DEFAULT_BLEND_MODE = BLENDMODE_ALPHABLEND;

	// Object is not a sprite object; return early.
	if (ObjectID < GAME_OBJECT_ID::ID_HORIZON || ObjectID >= GAME_OBJECT_ID::ID_NUMBER_OBJECTS)
	{
		TENLog("Attempted to draw display sprite from non-sprite object " + std::to_string(ObjectID), LogLevel::Warning);
		return;
	}

	// Sprite missing or sequence not found; return early.
	const auto& object = Objects[ObjectID];
	if (!object.loaded || SpriteIndex >= abs(object.nmeshes))
	{
		TENLog(
			"Attempted to draw missing display sprite " + std::to_string(SpriteIndex) +
			" from sprite object " + std::to_string(ObjectID),
			LogLevel::Warning);
		return;
	}

	auto convertedPos = Vector2(Position.x, Position.y) * POS_CONVERSION_COEFF;
	short convertedRot = ANGLE(Rotation);
	auto convertedScale = Vector2(Scale.x, Scale.y);
	auto convertedColor = Vector4(Color.GetR(), Color.GetG(), Color.GetB(), Color.GetA()) / UCHAR_MAX;

	AddDisplaySprite(
		ObjectID, SpriteIndex,
		convertedPos, convertedRot, convertedScale, convertedColor,
		priority.value_or(DEFAULT_PRIORITY),
		alignMode.value_or(DEFAULT_ALIGN_MODE),
		scaleMode.value_or(DEFAULT_SCALE_MODE),
		blendMode.value_or(DEFAULT_BLEND_MODE));
}
