#include "framework.h"
#include "Game/Hud/Speedometer.h"

#include "Game/effects/DisplaySprite.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Specific/clock.h"

using namespace TEN::Effects::DisplaySprite;
using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

namespace TEN::Hud
{
	void SpeedometerController::UpdateValue(float value)
	{
		_hasValueUpdated = true;
		_value = std::clamp(value, 0.0f, 1.0f);
	}

	void SpeedometerController::Update()
	{
		constexpr auto DIAL_ANGLE_MAX		 = 270.0f;
		constexpr auto DIAL_ANGLE_LERP_ALPHA = 0.25f;
		constexpr auto FADE_TIME			 = 0.2f;

		if (_life <= 0.0f && _value <= 0.0f && _pointerAngle <= 0.0f)
			return;

		// Update life and updated value status.
		_life += _hasValueUpdated ? 1.0f : -1.0f;
		_life = std::clamp(_life, 0.0f, LIFE_MAX * FPS);
		_hasValueUpdated = false;

		// Ensure value alawys reaches 0.
		if (_life <= 0.0f)
			_value = 0.0f;

		// Update appearance.
		_pointerAngle = Lerp(_pointerAngle, DIAL_ANGLE_MAX * _value, DIAL_ANGLE_LERP_ALPHA);
		_opacity = std::clamp(_life / std::round(FADE_TIME * FPS), 0.0f, 1.0f);
	}

	void SpeedometerController::Draw() const
	{
		constexpr auto POS						 = Vector2(DISPLAY_SPACE_RES.x - (DISPLAY_SPACE_RES.x / 6), DISPLAY_SPACE_RES.y - (DISPLAY_SPACE_RES.y / 5));
		constexpr auto SCALE					 = Vector2(0.2f);
		constexpr auto SPRITE_SEQUENCE_OBJECT_ID = ID_SPEEDOMETER;
		constexpr auto DIAL_ELEMENT_SPRITE_ID	 = 0;
		constexpr auto POINTER_ELEMENT_SPRITE_ID = 1;
		constexpr auto DIAL_PRIORITY			 = 0;
		constexpr auto POINTER_PRIORITY			 = 1;
		constexpr auto SCALE_MODE				 = DisplaySpriteScaleMode::Fill;
		constexpr auto BLEND_MODE				 = BLEND_MODES::BLENDMODE_ALPHABLEND;

		DrawDebug();

		if (_life == 0.0f)
			return;

		auto color = Color(1.0f, 1.0f, 1.0f, _opacity);

		// Draw dial.
		AddDisplaySprite(
			SPRITE_SEQUENCE_OBJECT_ID, DIAL_ELEMENT_SPRITE_ID,
			POS, 0, SCALE, color,
			DIAL_PRIORITY, DisplaySpriteAlignMode::Center, SCALE_MODE, BLEND_MODE);

		// Draw pointer.
		AddDisplaySprite(
			SPRITE_SEQUENCE_OBJECT_ID, POINTER_ELEMENT_SPRITE_ID,
			POS, ANGLE(_pointerAngle), SCALE / 2, color,
			POINTER_PRIORITY, DisplaySpriteAlignMode::CenterTop, SCALE_MODE, BLEND_MODE);
	}

	void SpeedometerController::Clear()
	{
		*this = {};
	}

	void SpeedometerController::DrawDebug() const
	{
		g_Renderer.PrintDebugMessage("SPEEDOMETER DEBUG");
		g_Renderer.PrintDebugMessage("Value: %.3f", _value);
		g_Renderer.PrintDebugMessage("Pointer angle: %.3f", _pointerAngle);
		g_Renderer.PrintDebugMessage("Opacity: %.3f", _opacity);
		g_Renderer.PrintDebugMessage("Life: %.3f", _life / FPS);
	}
}
