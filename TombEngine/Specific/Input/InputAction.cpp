#include "Specific/Input/InputAction.h"

#include "Specific/clock.h"

namespace TEN::Input
{
	InputAction::InputAction(ActionID actionID)
	{
		_id = actionID;
	}

	ActionID InputAction::GetID() const
	{
		return _id;
	}

	float InputAction::GetValue() const
	{
		return _value;
	}

	float InputAction::GetTimeActive() const
	{
		return _timeActive;
	}

	float InputAction::GetTimeInactive() const
	{
		return _timeInactive;
	}

	bool InputAction::IsClicked() const
	{
		return ((_value != 0.0f) && (_prevValue == 0.0f));
	}

	bool InputAction::IsHeld(float delayInSec) const
	{
		float delayInFrameTime = (delayInSec == 0.0f) ? 0.0f : round(delayInSec / DELTA_TIME);
		return ((_value != 0.0f) && (_timeActive >= delayInFrameTime));
	}

	// NOTE: To avoid stutter on second pulse, ensure initialDelayInSec is multiple of delayInSec.
	bool InputAction::IsPulsed(float delayInSec, float initialDelayInSec) const
	{
		if (IsClicked())
			return true;

		if (!IsHeld() || _prevTimeActive == 0.0f || _timeActive == _prevTimeActive)
			return false;

		float activeDelayInFrameTime = (_timeActive > round(initialDelayInSec / DELTA_TIME)) ? round(delayInSec / DELTA_TIME) : round(initialDelayInSec / DELTA_TIME);
		float delayInFrameTime = std::floor(_timeActive / activeDelayInFrameTime) * activeDelayInFrameTime;
		if (delayInFrameTime > (std::floor(_prevTimeActive / activeDelayInFrameTime) * activeDelayInFrameTime))
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
		return ((_value == 0.0f) && (_prevValue != 0.0f) && (_timeActive <= maxDelayInFrameTime));
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
			_prevTimeActive = 0.0f;
			_timeActive = 0.0f;
			_timeInactive += FRAME_TIME;// DELTA_TIME;
		}
		else if (IsReleased())
		{
			_prevTimeActive = _timeActive;
			_timeActive += FRAME_TIME;// DELTA_TIME;
			_timeInactive = 0.0f;
		}
		else if (IsHeld())
		{
			_prevTimeActive = _timeActive;
			_timeActive += FRAME_TIME;// DELTA_TIME;
			_timeInactive = 0.0f;
		}
		else
		{
			_prevTimeActive = 0.0f;
			_timeActive = 0.0f;
			_timeInactive += FRAME_TIME;// DELTA_TIME;
		}
	}

	void InputAction::Clear()
	{
		_value = 0.0f;
		_prevValue = 0.0f;
		_timeActive = 0.0f;
		_prevTimeActive = 0.0f;
		_timeInactive = 0.0f;
	}

	void InputAction::DrawDebug() const
	{
		PrintDebugMessage("ID: %d", (int)_id);
		PrintDebugMessage("IsClicked: %d", IsClicked());
		PrintDebugMessage("IsHeld: %d", IsHeld());
		PrintDebugMessage("IsPulsed (.2s, .6s): %d", IsPulsed(0.2f, 0.6f));
		PrintDebugMessage("IsReleased: %d", IsReleased());
		PrintDebugMessage("");
		PrintDebugMessage("Value: %.3f", _value);
		PrintDebugMessage("PrevValue: %.3f", _prevValue);
		PrintDebugMessage("TimeActive: %.3f", _timeActive);
		PrintDebugMessage("PrevTimeActive: %.3f", _prevTimeActive);
		PrintDebugMessage("TimeInactive: %.3f", _timeInactive);
	}

	void InputAction::UpdateValue(float value)
	{
		_prevValue = _value;
		_value = value;
	}
}
