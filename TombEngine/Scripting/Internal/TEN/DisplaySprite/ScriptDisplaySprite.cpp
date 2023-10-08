#include "framework.h"
#include "Scripting/Internal/TEN/DisplaySprite/ScriptDisplaySprite.h"

#include "Game/effects/DisplaySprite.h"
#include "Game/Setup.h"
#include "Objects/game_object_ids.h"
#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/DisplaySprite/AlignModes.h"
#include "Scripting/Internal/TEN/DisplaySprite/ScaleModes.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"

using namespace TEN::Effects::DisplaySprite;

namespace TEN::Scripting::DisplaySprite
{
	void ScriptDisplaySprite::Register(sol::state& state, sol::table& parent)
	{
		using ctors = sol::constructors<
			ScriptDisplaySprite(GAME_OBJECT_ID, int, const Vec2&, float, const Vec2&, const ScriptColor&),
			ScriptDisplaySprite(GAME_OBJECT_ID, int, const Vec2&, float, const Vec2&)>;

		parent.new_usertype<ScriptDisplaySprite>(
			ScriptReserved_DisplaySprite,
			ctors(),
			sol::call_constructor, ctors(),

			/*** Draw the display sprite in display space for the current frame.
			@function DisplaySprite:Draw
			@tparam Objects.ObjID[opt] priority Draw priority of the sprite. Can be thought of as a layer, with higher values having higher priority. __Default: 0__
			@tparam DisplaySprite.AlignMode[opt] alignMode Align mode of the sprite. __Default: DisplaySprite.AlignMode.CENTER__
			@tparam DisplaySprite.ScaleMode[opt] scaleMode Scale mode of the sprite. __Default: DisplaySprite.ScaleMode.FIT__
			@tparam Effects.BlendID[opt] blendMode Blend mode of the sprite. __Default: Effects.BlendID.ALPHABLEND__
			*/
			ScriptReserved_DisplaySpriteDraw, &ScriptDisplaySprite::Draw,

			/// Get the position of the display sprite.
			// Coordinates in percentages are returned.
			// @function DisplaySprite:GetPosition()
			// @treturn Vec2 (x,y) x and y coordinates.
			ScriptReserved_DisplayStringGetPosition, &ScriptDisplaySprite::GetPosition,

			/// Set the position of the display sprite.
			// Coordinates in percentage are expected.
			// @function DisplaySprite:SetPosition(Vec2(x,y))
			// @tparam Vec2 (x,y)  new x and y coordinates.
			ScriptReserved_DisplayStringSetPosition, &ScriptDisplaySprite::SetPosition,

			/// Get the rotation of the display sprite.
			// Rotation in float is returned.
			// @function DisplaySprite:GetRotation()
			// @treturn float rotation rotation of the display sprite.
			ScriptReserved_DisplayStringGetRotation, &ScriptDisplaySprite::GetRotation,

			/// Set the rotation of the display sprite.
			// Rotation in float is expected.
			// @function DisplaySprite:SetRotation(float)
			// @tparam float rotation new rotation of the display sprite.
			ScriptReserved_DisplayStringSetRotation, &ScriptDisplaySprite::SetRotation,

			/// Get the color of the display sprite.
			// Get the display sprite's color
			// @function DisplaySprite:GetColor()
			// @treturn Color color a copy of the display sprite's color
			ScriptReserved_DisplayStringGetColor, &ScriptDisplaySprite::GetColor,

			ScriptReserved_DisplayStringSetColor, &ScriptDisplaySprite::SetColor,

			ScriptReserved_DisplayStringGetScale, &ScriptDisplaySprite::GetScale,

			ScriptReserved_DisplayStringSetScale, &ScriptDisplaySprite::SetScale,

			ScriptReserved_DisplayStringGetObjectID, &ScriptDisplaySprite::GetObjectID,

			ScriptReserved_DisplayStringSetObjectID, &ScriptDisplaySprite::SetObjectID,

			ScriptReserved_DisplayStringGetSpriteID, & ScriptDisplaySprite::GetSpriteID,

			ScriptReserved_DisplayStringSetSpriteID, & ScriptDisplaySprite::SetSpriteID);

		auto table = sol::table(state.lua_state(), sol::create);
		parent.set("DisplaySpriteEnum"/*ScriptReserved_DisplaySprite*/, table);
		
		auto handler = LuaHandler(&state);
		handler.MakeReadOnlyTable(table, ScriptReserved_DisplaySpriteTableAlignMode, DISPLAY_SPRITE_ALIGN_MODES);
		handler.MakeReadOnlyTable(table, ScriptReserved_DisplaySpriteTableScaleMode, DISPLAY_SPRITE_SCALE_MODES);
	}

	/*** Create a DisplaySprite.
	@function DisplaySprite
	@tparam Objects.ObjID ID of the sprite object.
	@tparam int int spriteID ID of the display sprite in the sprite object.
	@tparam Vec2 pos Display position of the display sprite.
	@tparam float rot Rotation of the display sprite in degrees.
	@tparam Vec2 scale Horizontal and vertical scale of the display sprite.
	@tparam Color color[opt] Color of the display sprite. __Default: Color(255, 255, 255, 255)__
	@treturn DisplaySprite A new DisplaySprite object.
	*/
	ScriptDisplaySprite::ScriptDisplaySprite(GAME_OBJECT_ID objectID, int spriteID, const Vec2& pos, float rot, const Vec2& scale, const ScriptColor& color)
	{
		ObjectID = objectID;
		SpriteID = spriteID;
		Position = pos;
		Rotation = rot;
		Scale = scale;
		Color = color;
	}

	ScriptDisplaySprite::ScriptDisplaySprite(GAME_OBJECT_ID objectID, int spriteID, const Vec2& pos, float rot, const Vec2& scale)
	{
		static const auto DEFAULT_COLOR = ScriptColor(255, 255, 255, 255);

		*this = ScriptDisplaySprite(objectID, spriteID, pos, rot, scale, DEFAULT_COLOR);
	}

	GAME_OBJECT_ID ScriptDisplaySprite::GetObjectID() const
	{
		return ObjectID;
	}

	int ScriptDisplaySprite::GetSpriteID() const
	{
		return SpriteID;
	}

	Vec2 ScriptDisplaySprite::GetPosition() const
	{
		return Position;
	}

	float ScriptDisplaySprite::GetRotation() const
	{
		return Rotation;
	}

	Vec2 ScriptDisplaySprite::GetScale() const
	{
		return Scale;
	}

	ScriptColor ScriptDisplaySprite::GetColor() const
	{
		return Color;
	}

	void ScriptDisplaySprite::SetObjectID(GAME_OBJECT_ID objectID)
	{
		ObjectID = objectID;
	}

	void ScriptDisplaySprite::SetSpriteID(int spriteID)
	{
		SpriteID = spriteID;
	}

	void ScriptDisplaySprite::SetPosition(const Vec2& pos)
	{
		Position = pos;
	}

	void ScriptDisplaySprite::SetRotation(float rot)
	{
		Rotation = rot;
	}

	void ScriptDisplaySprite::SetScale(const Vec2& scale)
	{
		Scale = scale;
	}

	void ScriptDisplaySprite::SetColor(const ScriptColor& color)
	{
		Color = color;
	}

	void ScriptDisplaySprite::Draw(sol::optional<int> priority, sol::optional<DisplaySpriteAlignMode> alignMode,
								   sol::optional<DisplaySpriteScaleMode> scaleMode, sol::optional<BLEND_MODES> blendMode)
	{
		// NOTE: Conversion from more intuitive 100x100 screen space resolution to internal 800x600 is required.
		// In a future refactor, everything will use 100x100 natively. -- Sezz 2023.08.31
		constexpr auto POS_CONVERSION_COEFF	  = Vector2(SCREEN_SPACE_RES.x / 100, SCREEN_SPACE_RES.y / 100);
		constexpr auto SCALE_CONVERSION_COEFF = 0.01f;

		constexpr auto DEFAULT_PRIORITY	  = 0;
		constexpr auto DEFAULT_ALIGN_MODE = DisplaySpriteAlignMode::Center;
		constexpr auto DEFAULT_SCALE_MODE = DisplaySpriteScaleMode::Fit;
		constexpr auto DEFAULT_BLEND_MODE = BLENDMODE_ALPHABLEND;

		// Object is not a sprite sequence object; return early.
		if (ObjectID < GAME_OBJECT_ID::ID_HORIZON || ObjectID >= GAME_OBJECT_ID::ID_NUMBER_OBJECTS)
		{
			TENLog("Attempted to draw display sprite from non-sprite sequence object " + std::to_string(ObjectID), LogLevel::Warning);
			return;
		}

		// Sprite missing or sequence not found; return early.
		const auto& object = Objects[ObjectID];
		if (!object.loaded || SpriteID >= abs(object.nmeshes))
		{
			TENLog(
				"Attempted to draw missing sprite " + std::to_string(SpriteID) +
				" from sprite sequence object " + std::to_string(ObjectID) +
				" as display sprite.",
				LogLevel::Warning);
			return;
		}

		auto convertedPos = Vector2(Position.x, Position.y) * POS_CONVERSION_COEFF;
		short convertedRot = ANGLE(Rotation);
		auto convertedScale = Vector2(Scale.x, Scale.y) * SCALE_CONVERSION_COEFF;
		auto convertedColor = Vector4(Color.GetR(), Color.GetG(), Color.GetB(), Color.GetA()) / UCHAR_MAX;

		AddDisplaySprite(
			ObjectID, SpriteID,
			convertedPos, convertedRot, convertedScale, convertedColor,
			priority.value_or(DEFAULT_PRIORITY),
			alignMode.value_or(DEFAULT_ALIGN_MODE),
			scaleMode.value_or(DEFAULT_SCALE_MODE),
			blendMode.value_or(DEFAULT_BLEND_MODE));
	}
}
