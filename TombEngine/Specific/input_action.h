#pragma once

namespace TEN::Input
{
	typedef enum class InputActionID
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

	// TODO: For use with analog triggers, use Value range [0.0f, 1.0f] with deadzone up to a quarter press.
	class InputAction
	{
	public:
		InputAction(InputActionID ID);

		void Update(float value);
		void Clear();
		void PrintDebugInfo();

		bool IsClicked();
		bool IsPulsed(float delayInSeconds, float initialDelayInSeconds = 0.0f);
		bool IsHeld();
		bool IsReleased();

		InputActionID GetID();
		float GetValue();
		float GetTimeHeld();
		float GetTimeInactive();

	private:
		InputActionID ID   = In::None;
		float Value		   = 0.0f;
		float PrevValue	   = 0.0f;
		float TimeHeld	   = 0.0f;
		float PrevTimeHeld = 0.0f;
		float TimeInactive = 0.0f;

		void UpdateValue(float value);
	};
}
