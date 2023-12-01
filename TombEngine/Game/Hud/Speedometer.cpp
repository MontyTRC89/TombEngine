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
		_value = std::clamp(value, 0.0f, 1.0f);
		_hasValueUpdated = true;
	}

	void SpeedometerController::Update()
	{
		constexpr auto DIAL_ANGLE_MAX		 = ANGLE(120.0f);
		constexpr auto DIAL_ANGLE_LERP_ALPHA = 0.25f;
		constexpr auto FADE_TIME			 = 0.2f;

		if (!_hasValueUpdated && _life <= 0.0f &&
			_value <= 0.0f && _pointerAngle <= 0.0f)
		{
			return;
		}

		// Update life and updated value status.
		_life = std::clamp(_life + (_hasValueUpdated ? 1.0f : -1.0f), 0.0f, LIFE_MAX * FPS);
		_hasValueUpdated = false;

		// Ensure value resets to 0.
		if (_life <= 0.0f)
			_value = 0.0f;

		// Update appearance.
		_pointerAngle = Lerp(_pointerAngle, DIAL_ANGLE_MAX * _value, DIAL_ANGLE_LERP_ALPHA);
		_opacity = std::clamp(_life / std::round(FADE_TIME * FPS), 0.0f, 1.0f);
	}

	void SpeedometerController::Draw() const
	{
		constexpr auto POS						 = Vector2(DISPLAY_SPACE_RES.x - (DISPLAY_SPACE_RES.x / 6), DISPLAY_SPACE_RES.y - (DISPLAY_SPACE_RES.y / 10));
		constexpr auto ORIENT_OFFSET			 = ANGLE(90.0f);
		constexpr auto SCALE					 = Vector2(0.35f);
		constexpr auto DIAL_ELEMENT_SPRITE_ID	 = 0;
		constexpr auto POINTER_ELEMENT_SPRITE_ID = 1;
		constexpr auto DIAL_PRIORITY			 = 0;
		constexpr auto POINTER_PRIORITY			 = 1;

		//DrawDebug();

		if (_life <= 0.0f)
			return;

		auto color = Color(1.0f, 1.0f, 1.0f, _opacity);

		// Draw dial.
		AddDisplaySprite(
			ID_SPEEDOMETER, DIAL_ELEMENT_SPRITE_ID,
			POS, 0, SCALE, color,
			DIAL_PRIORITY, DisplaySpriteAlignMode::Center, DisplaySpriteScaleMode::Fit, BLEND_MODES::BLENDMODE_ALPHABLEND);

		// Draw pointer.
		AddDisplaySprite(
			ID_SPEEDOMETER, POINTER_ELEMENT_SPRITE_ID,
			POS, _pointerAngle + ORIENT_OFFSET, SCALE, color,
			POINTER_PRIORITY, DisplaySpriteAlignMode::Center, DisplaySpriteScaleMode::Fit, BLEND_MODES::BLENDMODE_ALPHABLEND);
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
