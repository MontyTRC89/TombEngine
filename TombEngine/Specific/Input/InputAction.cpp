#include "framework.h"
#include "Specific/Input/InputAction.h"

#include "Specific/clock.h"

namespace TEN::Input
{
	InputAction::InputAction(InputActionID actionID)
	{
		_id = actionID;
	}

	InputActionID InputAction::GetID() const
	{
		return _id;
	}

	float InputAction::GetValue() const
	{
		return _value;
	}

	unsigned long InputAction::GetTimeActive() const
	{
		return _timeActive;
	}

	unsigned long InputAction::GetTimeInactive() const
	{
		return _timeInactive;
	}

	bool InputAction::IsClicked() const
	{
		return (_value != 0.0f && _prevValue == 0.0f);
	}

	bool InputAction::IsHeld(float delayInSec) const
	{
		unsigned long delayInFrameTime = (delayInSec == 0.0f) ? 0 : (unsigned long)round(delayInSec / DELTA_TIME);
		return (_value != 0.0f && _timeActive >= delayInFrameTime);
	}

	// NOTE: To avoid stutter on second pulse, ensure initialDelayInSec is multiple of delayInSec.
	bool InputAction::IsPulsed(float delayInSec, float initialDelayInSec) const
	{
		if (IsClicked())
			return true;

		if (!IsHeld() || _prevTimeActive == 0 || _timeActive == _prevTimeActive)
			return false;

		float activeDelayInSec = (_timeActive > (unsigned long)round(initialDelayInSec / DELTA_TIME)) ? delayInSec : initialDelayInSec;
		unsigned long activeDelayInFrameTime = (unsigned long)round(activeDelayInSec / DELTA_TIME);
		unsigned long delayInFrameTime = (unsigned long)std::floor(_timeActive / activeDelayInFrameTime) * activeDelayInFrameTime;
		if (delayInFrameTime > ((unsigned long)std::floor(_prevTimeActive / activeDelayInFrameTime) * activeDelayInFrameTime))
			return true;

		return false;
	}

	bool InputAction::IsReleased(float delayInSecMax) const
	{
		unsigned long delayInFrameTimeMax = (delayInSecMax == INFINITY) ? ULONG_MAX : (unsigned long)round(delayInSecMax / DELTA_TIME);
		return (_value == 0.0f && _prevValue != 0.0f && _timeActive <= delayInFrameTimeMax);
	}

	void InputAction::Update(bool value)
	{
		Update(value ? 1.0f : 0.0f);
	}

	void InputAction::Update(float value)
	{
		UpdateValue(value);

		if (IsClicked())
		{
			_prevTimeActive = 0;
			_timeActive = 0;
			_timeInactive++;
		}
		else if (IsReleased())
		{
			_prevTimeActive = _timeActive;
			_timeActive++;
			_timeInactive = 0;
		}
		else if (IsHeld())
		{
			_prevTimeActive = _timeActive;
			_timeActive++;
			_timeInactive = 0;
		}
		else
		{
			_prevTimeActive = 0;
			_timeActive = 0;
			_timeInactive++;
		}
	}

	void InputAction::Clear()
	{
		_value = 0.0f;
		_prevValue = 0.0f;
		_timeActive = 0;
		_prevTimeActive = 0;
		_timeInactive = 0;
	}

	void InputAction::DrawDebug() const
	{
		PrintDebugMessage("INPUT ACTION DEBUG");
		PrintDebugMessage("ID: %d", (int)_id);
		PrintDebugMessage("IsClicked: %d", IsClicked());
		PrintDebugMessage("IsHeld: %d", IsHeld());
		PrintDebugMessage("IsPulsed (.2s, .6s): %d", IsPulsed(0.2f, 0.6f));
		PrintDebugMessage("IsReleased: %d", IsReleased());
		PrintDebugMessage("");
		PrintDebugMessage("Value: %.3f", _value);
		PrintDebugMessage("PrevValue: %.3f", _prevValue);
		PrintDebugMessage("TimeActive: %d", _timeActive);
		PrintDebugMessage("PrevTimeActive: %d", _prevTimeActive);
		PrintDebugMessage("TimeInactive: %d", _timeInactive);
	}

	void InputAction::UpdateValue(float value)
	{
		_prevValue = _value;
		_value = value;
	}
}
