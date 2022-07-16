#include "framework.h"
#include "Specific/input_action.h"

#include "Game/control/control.h"
#include "Renderer/Renderer11.h"
#include "Specific/clock.h"

using TEN::Renderer::g_Renderer;

namespace TEN::Input
{
	InputAction::InputAction(InputActionID ID)
	{
		this->ID = ID;
	}

	void InputAction::Update(float value)
	{
		this->UpdateValue(value);

		if (this->IsClicked())
		{
			this->PrevTimeHeld = 0.0f;
			this->TimeHeld = 0.0f;
			this->TimeInactive += DELTA_TIME;
		}
		else if (this->IsReleased())
		{
			this->PrevTimeHeld = TimeHeld;
			this->TimeHeld += DELTA_TIME;
			this->TimeInactive = 0.0f;
		}
		else if (this->IsHeld())
		{
			this->PrevTimeHeld = TimeHeld;
			this->TimeHeld += DELTA_TIME;
			this->TimeInactive = 0.0f;
		}
		else
		{
			this->PrevTimeHeld = 0.0f;
			this->TimeHeld = 0.0f;
			this->TimeInactive += DELTA_TIME;
		}
	}

	void InputAction::Clear()
	{
		this->Value = 0.0f;
		this->PrevValue = 0.0f;
		this->TimeHeld = 0.0f;
		this->PrevTimeHeld = 0.0f;
		this->TimeInactive = 0.0f;
	}

	void InputAction::PrintDebugInfo()
	{
		g_Renderer.PrintDebugMessage("IsClicked: %d", this->IsClicked());
		g_Renderer.PrintDebugMessage("IsPulsed (.2s, .6s): %d", this->IsPulsed(0.2f, 0.6f));
		g_Renderer.PrintDebugMessage("IsHeld: %d", this->IsHeld());
		g_Renderer.PrintDebugMessage("IsReleased: %d", this->IsReleased());
		g_Renderer.PrintDebugMessage("");
		g_Renderer.PrintDebugMessage("Value: %.3f", Value);
		g_Renderer.PrintDebugMessage("PrevValue: %.3f", PrevValue);
		g_Renderer.PrintDebugMessage("TimeHeld: %.3f", TimeHeld);
		g_Renderer.PrintDebugMessage("PrevTimeHeld: %.3f", PrevTimeHeld);
		g_Renderer.PrintDebugMessage("TimeInactive: %.3f", TimeInactive);
	}

	bool InputAction::IsClicked()
	{
		return (Value && !PrevValue);
	}

	// To avoid desync on the second pulse, ensure initialDelayInSeconds is a multiple of delayInSeconds.
	bool InputAction::IsPulsed(float delayInSeconds, float initialDelayInSeconds)
	{
		if (this->IsClicked())
			return true;

		if (!this->IsHeld() || !PrevTimeHeld || TimeHeld == PrevTimeHeld)
			return false;

		float syncedTimeHeld = TimeHeld - std::fmod(TimeHeld, DELTA_TIME);
		float activeDelay = (TimeHeld > initialDelayInSeconds) ? delayInSeconds : initialDelayInSeconds;
		float delayTime = std::floor(syncedTimeHeld / activeDelay) * activeDelay;
		if (delayTime >= PrevTimeHeld)
			return true;

		return false;
	}

	bool InputAction::IsHeld()
	{
		return Value;
	}

	bool InputAction::IsReleased()
	{
		return (!Value && PrevValue);
	}

	InputActionID InputAction::GetID()
	{
		return ID;
	}

	float InputAction::GetValue()
	{
		return Value;
	}

	float InputAction::GetTimeHeld()
	{
		return TimeHeld;
	}

	float InputAction::GetTimeInactive()
	{
		return TimeInactive;
	}

	void InputAction::UpdateValue(float value)
	{
		this->PrevValue = Value;
		this->Value = value;
	}
}
