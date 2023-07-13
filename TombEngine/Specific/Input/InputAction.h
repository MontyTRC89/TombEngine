#pragma once

namespace TEN::Input
{
	typedef enum class ActionID
	{
		// General actions
		Forward,
		Back,
		Left,
		Right,
		StepLeft,
		StepRight,
		Action,
		Jump,
		Walk,
		Sprint,
		Crouch,
		Roll,
		Draw,
		Look,

		// Vehicle actions
		/*Accelerate,
		Reverse,
		Speed,
		Slow,
		Brake,
		Fire,*/

		// Quick actions
		Flare,
		SmallMedipack,
		LargeMedipack,
		PreviousWeapon,
		NextWeapon,
		Weapon1,
		Weapon2,
		Weapon3,
		Weapon4,
		Weapon5,
		Weapon6,
		Weapon7,
		Weapon8,
		Weapon9,
		Weapon10,
		SayNo,

		// Menu actions
		Select,
		Deselect,
		Option,
		Pause,
		Save,
		Load,
		SwitchTarget, // TODO: Look -> SwitchTarget conversion must be handled differently.

		Count
	} In;

	// TODO: For analog triggers, use Value range [0.0f, 1.0f] with deadzone up to a quarter press.
	class InputAction
	{
	private:
		// Members
		ActionID ID				= In::Forward;
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
		void Update(bool value);
		void Update(float value);
		void Clear();

		void DrawDebug() const;

	private:
		// Helpers
		void UpdateValue(float value);
	};
}
