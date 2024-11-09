#pragma once

namespace TEN::Input
{
	typedef enum class InputActionID
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

		InputActionID ID			 = In::Forward;
		float		  Value			 = 0.0f;
		float		  PrevValue		 = 0.0f;
		float		  TimeActive	 = 0.0f;
		float		  PrevTimeActive = 0.0f;
		float		  TimeInactive	 = 0.0f;

	public:
		// Constructors

		InputAction() = default;
		InputAction(InputActionID actionID);

		// Getters

		InputActionID GetID() const;
		float		  GetValue() const;
		float		  GetTimeActive() const;
		float		  GetTimeInactive() const;
		
		// Inquirers

		bool IsClicked() const;
		bool IsHeld(float delayInSec = 0.0f) const;
		bool IsPulsed(float delayInSec, float initialDelayInSec = 0.0f) const;
		bool IsReleased(float delayInSecMax = INFINITY) const;

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
