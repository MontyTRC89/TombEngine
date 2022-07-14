#include "framework.h"
#include "Specific/input_action.h"

#include "Game/control/control.h"
#include "Renderer/Renderer11.h"

using TEN::Renderer::g_Renderer;

namespace TEN::Input
{
	InputAction::InputAction(InputActionID ID)
	{
		this->ID = ID;
	}

	void InputAction::Update(float value)
	{
		static const float frameTime = 1.0f / FPS;

		this->UpdateValue(value);

		if (this->IsClicked())
		{
			this->PrevTimeHeld = 0.0f;
			this->TimeHeld = frameTime;
			this->TimeInactive = 0.0f;
		}
		else if (this->IsReleased())
		{
			this->PrevTimeHeld = 0.0f;
			this->TimeHeld = 0.0f;
			this->TimeInactive = frameTime;
		}
		else if (this->IsHeld())
		{
			this->PrevTimeHeld = TimeHeld;
			this->TimeHeld += frameTime;
			this->TimeInactive = 0.0f;
		}
		else
		{
			this->PrevTimeHeld = 0.0f;
			this->TimeHeld = 0.0f;
			this->TimeInactive += frameTime;
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
		g_Renderer.PrintDebugMessage("IsPulsed (0.2f, 0.6f): %d", this->IsPulsed(0.2f, 0.6f));
		g_Renderer.PrintDebugMessage("IsHeld: %d", this->IsHeld());
		g_Renderer.PrintDebugMessage("IsReleased: %d", this->IsReleased());
		g_Renderer.PrintDebugMessage("");
		g_Renderer.PrintDebugMessage("Value: %f", Value);
		g_Renderer.PrintDebugMessage("PrevValue: %f", PrevValue);
		g_Renderer.PrintDebugMessage("TimeHeld: %f", TimeHeld);
		g_Renderer.PrintDebugMessage("PrevTimeHeld: %f", PrevTimeHeld);
		g_Renderer.PrintDebugMessage("TimeInactive: %f", TimeInactive);
	}

	bool InputAction::IsClicked()
	{
		return (Value && !PrevValue);
	}

	// To avoid desync on the second pulse, ensure initialDelayInSeconds is a multiple of delayInSeconds.
	bool InputAction::IsPulsed(float delayInSeconds, float initialDelayInSeconds)
	{
		if (!this->IsHeld() || TimeHeld == PrevTimeHeld)
			return false;

		static const float frameTime = 1.0f / FPS;
		float syncedTimeHeld = TimeHeld - std::fmod(TimeHeld, frameTime);
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
