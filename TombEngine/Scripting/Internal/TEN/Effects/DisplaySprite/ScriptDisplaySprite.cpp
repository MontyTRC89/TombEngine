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

		ScriptReserved_DisplaySpriteDraw, &ScriptDisplaySprite::Draw,

		/// (int) ID of the sprite object.
		//@mem ObjectID
		"ObjectID", &ScriptDisplaySprite::ObjectID,

		/// (int) ID of the display sprite in the sprite object.
		//@mem SpriteID
		"SpriteID", &ScriptDisplaySprite::SpriteID,
		
		/// (Vec2) Display position of the display sprite.
		//@mem Position
		"Position", &ScriptDisplaySprite::Position,
		
		/// (float) Rotation of the display sprite in degrees.
		//@mem Rotation
		"Rotation", &ScriptDisplaySprite::Rotation,
		
		/// (Vec2) Horizontal and vertical scale of the display sprite.
		//@mem Scale
		"Scale", &ScriptDisplaySprite::Scale,
		
		/// (Color) Color of the display sprite.
		//@mem Color
		"Color", &ScriptDisplaySprite::Color);

	// TODO: How???
	/*auto tableDisplaySprite = sol::table(state->lua_state(), sol::create);
	parent.set(ScriptReserved_DisplaySprite, tableDisplaySprite);

	auto handler = LuaHandler{ state };
	handler.MakeReadOnlyTable(tableDisplaySprite, ScriptReserved_DisplaySpriteAlignMode, DISPLAY_SPRITE_ALIGN_MODES);
	handler.MakeReadOnlyTable(tableDisplaySprite, ScriptReserved_DisplaySpriteScaleMode, DISPLAY_SPRITE_SCALE_MODES);*/
}

/*** 
@int objectID ID of the sprite object.
@int spriteID ID of the display sprite in the sprite object.
@Vec2 pos Display position of the display sprite.
@float rot Rotation of the display sprite in degrees.
@Vec2 scale Horizontal and vertical scale of the display sprite.
@Color color[opt] Color of the display sprite. __Default: Color(255, 255, 255, 255)__
@function Vec3
*/
ScriptDisplaySprite::ScriptDisplaySprite(GAME_OBJECT_ID objectID, int spriteID, const Vec2& pos, float rot, const Vec2& scale,
										 sol::optional<ScriptColor> color)
{
	static const auto DEFAULT_COLOR = ScriptColor(255, 255, 255, 255);

	ObjectID = objectID;
	SpriteID = spriteID;
	Position = pos;
	Rotation = rot;
	Scale = scale;
	Color = color.value_or(DEFAULT_COLOR);
}

/*** Draw the display sprite in display space for the current frame.
@function DisplaySprite:Draw
@tparam int[opt] priority Draw priority of the sprite. Can be thought of as a layer, with higher values having higher priority. __Default: 0__
@tparam DisplaySprite.AlignMode[opt] alignMode Align mode of the sprite. __Default: DisplaySprite.AlignMode.CENTER__
@tparam DisplaySprite.ScaleMode[opt] scaleMode Scale mode of the sprite. __Default: DisplaySprite.ScaleMode.FIT__
@tparam Effects.BlendID[opt] blendMode Blend mode of the sprite. __Default: Effects.BlendID.ALPHABLEND__
*/
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
	if (!object.loaded || SpriteID >= abs(object.nmeshes))
	{
		TENLog(
			"Attempted to draw missing display sprite " + std::to_string(SpriteID) +
			" from sprite object " + std::to_string(ObjectID),
			LogLevel::Warning);
		return;
	}

	auto convertedPos = Vector2(Position.x, Position.y) * POS_CONVERSION_COEFF;
	short convertedRot = ANGLE(Rotation);
	auto convertedScale = Vector2(Scale.x, Scale.y);
	auto convertedColor = Vector4(Color.GetR(), Color.GetG(), Color.GetB(), Color.GetA()) / UCHAR_MAX;

	AddDisplaySprite(
		ObjectID, SpriteID,
		convertedPos, convertedRot, convertedScale, convertedColor,
		priority.value_or(DEFAULT_PRIORITY),
		alignMode.value_or(DEFAULT_ALIGN_MODE),
		scaleMode.value_or(DEFAULT_SCALE_MODE),
		blendMode.value_or(DEFAULT_BLEND_MODE));
}
