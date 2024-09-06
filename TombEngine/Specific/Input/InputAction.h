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
		Walk,
		Sprint,
		Crouch,
		Jump,
		Roll,
		Action,
		Draw,
		Look,

		// Vehicle actions

		Accelerate,
		Reverse,
		Faster,
		Slower,
		Brake,
		Fire,

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

		// Menu actions

		Select,
		Deselect,
		Pause,
		Inventory,
		Save,
		Load,

		Count
	} In;

	class InputAction
	{
	private:
		// Members

		ActionID _id			 = In::Forward;
		float	 _value			 = 0.0f;
		float	 _prevValue		 = 0.0f;
		float	 _timeActive	 = 0.0f;
		float	 _prevTimeActive = 0.0f;
		float	 _timeInactive	 = 0.0f;

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
