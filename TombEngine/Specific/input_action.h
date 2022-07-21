#pragma once

namespace TEN::Input
{
	typedef enum class ActionID
	{
		None = -1,

		// General control
		Forward,
		Back,
		Left,
		Right,
		Crouch,
		Sprint,
		Walk,
		Jump,
		Action,
		Draw,
		Flare,
		Look,
		Roll,
		Option,
		Pause,
		LeftStep,
		RightStep,

		// Vehicle control
		/*Accelerate,
		Reverse,
		Speed,
		Slow,
		Brake,
		Fire,*/

		// Hotkeys
		Save,
		Load,
		Select,
		Deselect,
		SwitchTarget,

		Count
	} In;

	// TODO: Each action should hold its own input mappping?
	// TODO: For use with analog triggers, use Value range [0.0f, 1.0f] with deadzone up to a quarter press.
	class InputAction
	{
	public:
		InputAction(ActionID actionID);

		void Update(float value);
		void Clear();
		void PrintDebugInfo();

		bool IsClicked();
		bool IsPulsed(float delayInSeconds, float initialDelayInSeconds = 0.0f);
		bool IsHeld();
		bool IsReleased(float maxDelayInSeconds = std::numeric_limits<float>::max());

		ActionID GetID();
		float GetValue();
		float GetTimeActive();
		float GetTimeInactive();

	private:
		ActionID ID			 = In::None;
		float Value			 = 0.0f;
		float PrevValue		 = 0.0f;
		float TimeActive	 = 0.0f;
		float PrevTimeActive = 0.0f;
		float TimeInactive	 = 0.0f;

		void UpdateValue(float value);
	};
}
