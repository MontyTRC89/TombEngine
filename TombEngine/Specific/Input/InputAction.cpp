#include "framework.h"
#include "Specific/Input/InputAction.h"

#include "Renderer/Renderer11.h"
#include "Specific/clock.h"

using TEN::Renderer::g_Renderer;

namespace TEN::Input
{
	InputAction::InputAction(ActionID actionID)
	{
		this->ID = actionID;
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
		float delayInFrameTime = round(delayInSec / DELTA_TIME);
		return ((Value != 0.0f) && (TimeActive >= delayInFrameTime));
	}

	// To avoid desync on the second pulse, ensure initialDelayInSec is a multiple of delayInSec.
	bool InputAction::IsPulsed(float delayInSec, float initialDelayInSec) const
	{
		if (this->IsClicked())
			return true;

		if (!this->IsHeld() || PrevTimeActive == 0.0f || TimeActive == PrevTimeActive)
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
		float maxDelayInFrameTime = round(maxDelayInSec / DELTA_TIME);
		return ((Value == 0.0f) && (PrevValue != 0.0f) && (TimeActive <= maxDelayInFrameTime));
	}

	void InputAction::Update(float value)
	{
		this->UpdateValue(value);

		// TODO: Because our delta time is a placeholder constant and we cannot properly account for time drift,
		// count whole frames instead of actual time passed for now to avoid occasional stutter.
		// Inquiry methods take this into account. -- Sezz 2022.10.01
		static constexpr float frameTime = 1.0f;

		if (this->IsClicked())
		{
			this->PrevTimeActive = 0.0f;
			this->TimeActive = 0.0f;
			this->TimeInactive += frameTime;// DELTA_TIME;
		}
		else if (this->IsReleased())
		{
			this->PrevTimeActive = TimeActive;
			this->TimeActive += frameTime;// DELTA_TIME;
			this->TimeInactive = 0.0f;
		}
		else if (this->IsHeld())
		{
			this->PrevTimeActive = TimeActive;
			this->TimeActive += frameTime;// DELTA_TIME;
			this->TimeInactive = 0.0f;
		}
		else
		{
			this->PrevTimeActive = 0.0f;
			this->TimeActive = 0.0f;
			this->TimeInactive += frameTime;// DELTA_TIME;
		}
	}

	void InputAction::Clear()
	{
		this->Value = 0.0f;
		this->PrevValue = 0.0f;
		this->TimeActive = 0.0f;
		this->PrevTimeActive = 0.0f;
		this->TimeInactive = 0.0f;
	}

	void InputAction::PrintDebugInfo() const
	{
		g_Renderer.PrintDebugMessage("ID: %d", (int)ID);
		g_Renderer.PrintDebugMessage("IsClicked: %d", this->IsClicked());
		g_Renderer.PrintDebugMessage("IsPulsed (.2s, .6s): %d", this->IsPulsed(0.2f, 0.6f));
		g_Renderer.PrintDebugMessage("IsHeld: %d", this->IsHeld());
		g_Renderer.PrintDebugMessage("IsReleased: %d", this->IsReleased());
		g_Renderer.PrintDebugMessage("");
		g_Renderer.PrintDebugMessage("Value: %.3f", Value);
		g_Renderer.PrintDebugMessage("PrevValue: %.3f", PrevValue);
		g_Renderer.PrintDebugMessage("TimeActive: %.3f", TimeActive);
		g_Renderer.PrintDebugMessage("PrevTimeActive: %.3f", PrevTimeActive);
		g_Renderer.PrintDebugMessage("TimeInactive: %.3f", TimeInactive);
	}

	void InputAction::UpdateValue(float value)
	{
		this->PrevValue = Value;
		this->Value = value;
	}
}
