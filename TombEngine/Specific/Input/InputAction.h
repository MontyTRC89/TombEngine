#pragma once

using std::vector;

namespace TEN::Input
{
	typedef enum class ActionID
	{
		None = -1,

		// Basic control
		Forward,
		Back,
		Left,
		Right,
		Crouch,
		Sprint,
		Walk,
		Jump,
		Action,
		DrawWeapon,
		Flare, // Convert to generic Light button under Item hotkeys section.
		Look,
		Roll,
		Option, // Move to GUI control section.
		Pause, // Move to GUI control section.
		LeftStep,
		RightStep,

		// Vehicle control
		/*Accelerate,
		Reverse,
		Speed,
		Slow,
		Brake,
		Fire,*/

		// Item hotkeys
		/*Light, // Generic light button may be used for flares.
		Binoculars,
		SmallMedipack,
		BigMedipack,
		NextWeapon,
		PreviousWeapon,
		Weapon1,
		Weapon2,
		Weapon3,
		Weapon4,
		Weapon5,
		Weapon6,
		Weapon7,
		Weapon8,
		Weapon9,
		Weapon10,*/

		// GUI control
		/*Option,
		Pause,*/
		Save,
		Load,
		Select,
		Deselect,
		SwitchTarget, // Look -> SwitchTarget conversion must be handled differently.

		Count
	} In;

	// TODO: For analog triggers, use Value range [0.0f, 1.0f] with deadzone up to a quarter press.
	class InputAction
	{
	private:
		// Components
		ActionID ID				= In::None;
		float	 Value			= 0.0f;
		float	 PrevValue		= 0.0f;
		float	 TimeActive		= 0.0f;
		float	 PrevTimeActive = 0.0f;
		float	 TimeInactive	= 0.0f;

	public:
		// Constructors
		InputAction(ActionID actionID);

		// Getters
		ActionID GetID() const;
		float	 GetValue() const;
		float	 GetTimeActive() const;
		float	 GetTimeInactive() const;
		
		// Inquirers
		bool IsClicked() const;
		bool IsHeld(float delayInSec = 0.0f) const;
		bool IsPulsed(float delayInSec, float initialDelayInSec = 0.0f) const;
		bool IsReleased(float maxDelayInSec = INFINITY) const;

		// Utilities
		void Update(float value);
		void Clear();
		void PrintDebugInfo() const;

	private:
		// Helpers
		void UpdateValue(float value);
	};
}
