#include "framework.h"
#include "Specific/Input/InputAction.h"

#include "Specific/clock.h"


namespace TEN::Input
{
	InputAction::InputAction(ActionID actionID)
	{
		ID = actionID;
	}

	ActionID InputAction::GetID() const
	{
		return ID;
	}

	float InputAction::GetValue() const
	{
		return Value;
	}

	float InputAction::GetTimeActive() const
	{
		return TimeActive;
	}

	float InputAction::GetTimeInactive() const
	{
		return TimeInactive;
	}

	bool InputAction::IsClicked() const
	{
		return ((Value != 0.0f) && (PrevValue == 0.0f));
	}

	bool InputAction::IsHeld(float delayInSec) const
	{
		float delayInFrameTime = (delayInSec == 0.0f) ? 0.0f : round(delayInSec / DELTA_TIME);
		return ((Value != 0.0f) && (TimeActive >= delayInFrameTime));
	}

	// NOTE: To avoid stutter on second pulse, ensure initialDelayInSec is multiple of delayInSec.
	bool InputAction::IsPulsed(float delayInSec, float initialDelayInSec) const
	{
		if (IsClicked())
			return true;

		if (!IsHeld() || PrevTimeActive == 0.0f || TimeActive == PrevTimeActive)
			return false;

		float activeDelayInFrameTime = (TimeActive > round(initialDelayInSec / DELTA_TIME)) ? round(delayInSec / DELTA_TIME) : round(initialDelayInSec / DELTA_TIME);
		float delayInFrameTime = std::floor(TimeActive / activeDelayInFrameTime) * activeDelayInFrameTime;
		if (delayInFrameTime > (std::floor(PrevTimeActive / activeDelayInFrameTime) * activeDelayInFrameTime))
			return true;

		// Keeping version counting real time for future reference. -- Sezz 2022.10.01
		/*float syncedTimeActive = TimeActive - std::fmod(TimeActive, DELTA_TIME);
		float activeDelay = (TimeActive > initialDelayInSec) ? delayInSeconds : initialDelayInSec;

		float delayTime = std::floor(syncedTimeActive / activeDelay) * activeDelay;
		if (delayTime >= PrevTimeActive)
			return true;*/

		return false;
	}

	bool InputAction::IsReleased(float maxDelayInSec) const
	{
		float maxDelayInFrameTime = (maxDelayInSec == INFINITY) ? INFINITY : round(maxDelayInSec / DELTA_TIME);
		return ((Value == 0.0f) && (PrevValue != 0.0f) && (TimeActive <= maxDelayInFrameTime));
	}

	void InputAction::Update(bool value)
	{
		Update(value ? 1.0f : 0.0f);
	}

	void InputAction::Update(float value)
	{
		UpdateValue(value);

		// TODO: Because our delta time is a placeholder constant and we cannot properly account for time drift,
		// count whole frames instead of actual time passed for now to avoid occasional stutter.
		// Inquiry methods take this into account. -- Sezz 2022.10.01
		constexpr auto FRAME_TIME = 1.0f;

		if (IsClicked())
		{
			PrevTimeActive = 0.0f;
			TimeActive = 0.0f;
			TimeInactive += FRAME_TIME;// DELTA_TIME;
		}
		else if (IsReleased())
		{
			PrevTimeActive = TimeActive;
			TimeActive += FRAME_TIME;// DELTA_TIME;
			TimeInactive = 0.0f;
		}
		else if (IsHeld())
		{
			PrevTimeActive = TimeActive;
			TimeActive += FRAME_TIME;// DELTA_TIME;
			TimeInactive = 0.0f;
		}
		else
		{
			PrevTimeActive = 0.0f;
			TimeActive = 0.0f;
			TimeInactive += FRAME_TIME;// DELTA_TIME;
		}
	}

	void InputAction::Clear()
	{
		Value = 0.0f;
		PrevValue = 0.0f;
		TimeActive = 0.0f;
		PrevTimeActive = 0.0f;
		TimeInactive = 0.0f;
	}

	void InputAction::DrawDebug() const
	{
		PrintDebugMessage("ID: %d", (int)ID);
		PrintDebugMessage("IsClicked: %d", IsClicked());
		PrintDebugMessage("IsHeld: %d", IsHeld());
		PrintDebugMessage("IsPulsed (.2s, .6s): %d", IsPulsed(0.2f, 0.6f));
		PrintDebugMessage("IsReleased: %d", IsReleased());
		PrintDebugMessage("");
		PrintDebugMessage("Value: %.3f", Value);
		PrintDebugMessage("PrevValue: %.3f", PrevValue);
		PrintDebugMessage("TimeActive: %.3f", TimeActive);
		PrintDebugMessage("PrevTimeActive: %.3f", PrevTimeActive);
		PrintDebugMessage("TimeInactive: %.3f", TimeInactive);
	}

	void InputAction::UpdateValue(float value)
	{
		PrevValue = Value;
		Value = value;
	}
}
