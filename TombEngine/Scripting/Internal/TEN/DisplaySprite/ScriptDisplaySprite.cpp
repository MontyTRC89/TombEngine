#include "framework.h"
#include "Scripting/Internal/TEN/DisplaySprite/ScriptDisplaySprite.h"

#include "Game/effects/DisplaySprite.h"
#include "Game/Setup.h"
#include "Objects/game_object_ids.h"
#include "Renderer/Renderer11.h"
#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/DisplaySprite/AlignModes.h"
#include "Scripting/Internal/TEN/DisplaySprite/ScaleModes.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"

using namespace TEN::Effects::DisplaySprite;
using TEN::Renderer::g_Renderer;

namespace TEN::Scripting::DisplaySprite
{
	void ScriptDisplaySprite::Register(sol::state& state, sol::table& parent)
	{
		using ctors = sol::constructors<
			ScriptDisplaySprite(GAME_OBJECT_ID, int, const Vec2&, float, const Vec2&, const ScriptColor&),
			ScriptDisplaySprite(GAME_OBJECT_ID, int, const Vec2&, float, const Vec2&)>;

		// Register type.
		parent.new_usertype<ScriptDisplaySprite>(
			ScriptReserved_DisplaySprite,
			ctors(),
			sol::call_constructor, ctors(),

			/*** Get the object ID of the sprite sequence object used by the display sprite.
			@function DisplaySprite:GetObjectID()
			@treturn Objects.ObjID Sprite sequence object ID.
			*/
			ScriptReserved_DisplayStringGetObjectID, &ScriptDisplaySprite::GetObjectID,

			/*** Get the sprite ID in the sprite sequence object used by the display sprite.
			@function DisplaySprite:GetSpriteID()
			@treturn int Sprite ID in the sprite sequence object.
			*/
			ScriptReserved_DisplayStringGetSpriteID, &ScriptDisplaySprite::GetSpriteID,

			/*** Get the display position of the display sprite in percent.
			@function DisplaySprite:GetPosition()
			@treturn Vec2 Display position in percent.
			*/
			ScriptReserved_DisplayStringGetPosition, &ScriptDisplaySprite::GetPosition,

			/*** Get the rotation of the display sprite in degrees.
			@function DisplaySprite:GetRotation()
			@treturn float Rotation in degrees.
			*/
			ScriptReserved_DisplayStringGetRotation, &ScriptDisplaySprite::GetRotation,

			/*** Get the horizontal and vertical scale of the display sprite in percent.
			@function DisplaySprite:GetScale()
			@treturn Vec2 Horizontal and vertical scale in percent.
			*/
			ScriptReserved_DisplayStringGetScale, &ScriptDisplaySprite::GetScale,

			/*** Get the color of the display sprite.
			@function DisplaySprite:GetColor()
			@treturn Color Color.
			*/
			ScriptReserved_DisplayStringGetColor, &ScriptDisplaySprite::GetColor,

			/*** Set the sprite sequence object ID used by the display sprite.
			@function DisplaySprite:SetObjectID(Objects.ObjID)
			@tparam Objects.ObjID New sprite sequence object ID.
			*/
			ScriptReserved_DisplayStringSetObjectID, &ScriptDisplaySprite::SetObjectID,

			/*** Set the sprite ID in the sprite sequence object used by the display sprite.
			@function DisplaySprite:SetSpriteID(int)
			@tparam int New sprite ID in the sprite sequence object.
			*/
			ScriptReserved_DisplayStringSetSpriteID, &ScriptDisplaySprite::SetSpriteID,

			/*** Set the display position of the display sprite in percent.
			@function DisplaySprite:SetPosition(Vec2)
			@tparam Vec2 New display position in percent.
			*/
			ScriptReserved_DisplayStringSetPosition, &ScriptDisplaySprite::SetPosition,

			/*** Set the rotation of the display sprite in degrees.
			@function DisplaySprite:SetRotation(float)
			@tparam float New rotation in degrees.
			*/
			ScriptReserved_DisplayStringSetRotation, &ScriptDisplaySprite::SetRotation,

			/*** Set the horizontal and vertical scale of the display sprite in percent.
			@function DisplaySprite:SetScale(Vec2)
			@tparam float New horizontal and vertical scale in percent.
			*/
			ScriptReserved_DisplayStringSetScale, &ScriptDisplaySprite::SetScale,

			/*** Set the color of the display sprite.
			@function DisplaySprite:SetColor(Color)
			@tparam float New color.
			*/
			ScriptReserved_DisplayStringSetColor, &ScriptDisplaySprite::SetColor,
			
			/*** Draw the display sprite in display space for the current frame.
			@function DisplaySprite:Draw
			@tparam Objects.ObjID[opt] priority Draw priority. Can be thought of as a layer, with higher values having precedence. __Default: 0__
			@tparam DisplaySprite.AlignMode[opt] alignMode Align mode interpreting an offset from the sprite's position. __Default: DisplaySprite.AlignMode.CENTER__
			@tparam DisplaySprite.ScaleMode[opt] scaleMode Scale mode interpreting the display sprite's horizontal and vertical scale. __Default: DisplaySprite.ScaleMode.FIT__
			@tparam Effects.BlendID[opt] blendMode Blend mode. __Default: Effects.BlendID.ALPHABLEND__
			*/
			ScriptReserved_DisplaySpriteDraw, &ScriptDisplaySprite::Draw);

		auto table = sol::table(state.lua_state(), sol::create);
		parent.set(ScriptReserved_DisplaySpriteEnum, table);
		
		// Register enums.
		auto handler = LuaHandler(&state);
		handler.MakeReadOnlyTable(table, ScriptReserved_DisplaySpriteEnumAlignMode, DISPLAY_SPRITE_ALIGN_MODES);
		handler.MakeReadOnlyTable(table, ScriptReserved_DisplaySpriteEnumScaleMode, DISPLAY_SPRITE_SCALE_MODES);
	}

	/*** Create a DisplaySprite object.
	@function DisplaySprite
	@tparam Objects.ObjID ID of the sprite sequence object.
	@tparam int int spriteID ID of the sprite in the sequence.
	@tparam Vec2 pos Display position in percent.
	@tparam float rot Rotation in degrees.
	@tparam Vec2 scale Horizontal and vertical scale in percent. Scaling is interpreted by the DisplaySpriteEnum.ScaleMode passed to the Draw() function call.
	@tparam Color color[opt] Color. __Default: Color(255, 255, 255, 255)__
	@treturn DisplaySprite A new DisplaySprite object.
	*/
	ScriptDisplaySprite::ScriptDisplaySprite(GAME_OBJECT_ID objectID, int spriteID, const Vec2& pos, float rot, const Vec2& scale, const ScriptColor& color)
	{
		_objectID = objectID;
		_spriteID = spriteID;
		_position = pos;
		_rotation = rot;
		_scale = scale;
		_color = color;
	}

	ScriptDisplaySprite::ScriptDisplaySprite(GAME_OBJECT_ID objectID, int spriteID, const Vec2& pos, float rot, const Vec2& scale)
	{
		static const auto DEFAULT_COLOR = ScriptColor(255, 255, 255, 255);

		*this = ScriptDisplaySprite(objectID, spriteID, pos, rot, scale, DEFAULT_COLOR);
	}

	GAME_OBJECT_ID ScriptDisplaySprite::GetObjectID() const
	{
		return _objectID;
	}

	int ScriptDisplaySprite::GetSpriteID() const
	{
		return _spriteID;
	}

	Vec2 ScriptDisplaySprite::GetPosition() const
	{
		return _position;
	}

	float ScriptDisplaySprite::GetRotation() const
	{
		return _rotation;
	}

	Vec2 ScriptDisplaySprite::GetScale() const
	{
		return _scale;
	}

	ScriptColor ScriptDisplaySprite::GetColor() const
	{
		return _color;
	}

	void ScriptDisplaySprite::SetObjectID(GAME_OBJECT_ID objectID)
	{
		_objectID = objectID;
	}

	void ScriptDisplaySprite::SetSpriteID(int spriteID)
	{
		_spriteID = spriteID;
	}

	void ScriptDisplaySprite::SetPosition(const Vec2& pos)
	{
		_position = pos;
	}

	void ScriptDisplaySprite::SetRotation(float rot)
	{
		_rotation = rot;
	}

	void ScriptDisplaySprite::SetScale(const Vec2& scale)
	{
		_scale = scale;
	}

	void ScriptDisplaySprite::SetColor(const ScriptColor& color)
	{
		_color = color;
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
		if (_objectID < GAME_OBJECT_ID::ID_HORIZON || _objectID >= GAME_OBJECT_ID::ID_NUMBER_OBJECTS)
		{
			TENLog("Attempted to draw display sprite from non-sprite sequence object " + std::to_string(_objectID), LogLevel::Warning);
			return;
		}

		// Sprite missing or sequence not found; return early.
		const auto& object = Objects[_objectID];
		if (!object.loaded || _spriteID >= abs(object.nmeshes))
		{
			TENLog(
				"Attempted to draw missing sprite " + std::to_string(_spriteID) +
				" from sprite sequence object " + std::to_string(_objectID) +
				" as display sprite.",
				LogLevel::Warning);
			return;
		}

		auto convertedPos = Vector2(_position.x, _position.y) * POS_CONVERSION_COEFF;
		short convertedRot = ANGLE(_rotation);
		auto convertedScale = Vector2(_scale.x, _scale.y) * SCALE_CONVERSION_COEFF;
		auto convertedColor = Vector4(_color.GetR(), _color.GetG(), _color.GetB(), _color.GetA()) / UCHAR_MAX;

		AddDisplaySprite(
			_objectID, _spriteID,
			convertedPos, convertedRot, convertedScale, convertedColor,
			priority.value_or(DEFAULT_PRIORITY),
			alignMode.value_or(DEFAULT_ALIGN_MODE),
			scaleMode.value_or(DEFAULT_SCALE_MODE),
			blendMode.value_or(DEFAULT_BLEND_MODE));
	}
}
