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

	// Time in game frames.
	unsigned long InputAction::GetTimeActive() const
	{
		return _timeActive;
	}

	// Time in game frames.
	unsigned long InputAction::GetTimeInactive() const
	{
		return _timeInactive;
	}

	bool InputAction::IsClicked() const
	{
		return (_value != 0.0f && _prevValue == 0.0f);
	}

	bool InputAction::IsHeld(float delaySecs) const
	{
		unsigned long delayGameFrames = (delaySecs == 0.0f) ? 0 : (unsigned long)round(delaySecs / DELTA_TIME);
		return (_value != 0.0f && _timeActive >= delayGameFrames);
	}

	// NOTE: To avoid stutter on second pulse, ensure initialDelaySecs is multiple of delaySecs.
	bool InputAction::IsPulsed(float delaySecs, float initialDelaySecs) const
	{
		if (IsClicked())
			return true;

		if (!IsHeld() || _prevTimeActive == 0 || _timeActive == _prevTimeActive)
			return false;

		float activeDelaySecs = (_timeActive > (unsigned long)round(initialDelaySecs / DELTA_TIME)) ? delaySecs : initialDelaySecs;
		unsigned long activeDelayGameFrames = (unsigned long)round(activeDelaySecs / DELTA_TIME);
		unsigned long delayGameFrames = (unsigned long)floor(_timeActive / activeDelayGameFrames) * activeDelayGameFrames;
		if (delayGameFrames > ((unsigned long)floor(_prevTimeActive / activeDelayGameFrames) * activeDelayGameFrames))
			return true;

		return false;
	}

	bool InputAction::IsReleased(float delaySecsMax) const
	{
		unsigned long delayGameFramesMax = (delaySecsMax == INFINITY) ? ULONG_MAX : (unsigned long)round(delaySecsMax / DELTA_TIME);
		return (_value == 0.0f && _prevValue != 0.0f && _timeActive <= delayGameFramesMax);
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
