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
		// Fields

		InputActionID _id			  = In::Forward;
		float		  _value		  = 0.0f;
		float		  _prevValue	  = 0.0f;
		unsigned int  _timeActive	  = 0;
		unsigned int  _prevTimeActive = 0;
		unsigned int  _timeInactive	  = 0;

	public:
		// Constructors

		InputAction() = default;
		InputAction(InputActionID actionID);

		// Getters

		InputActionID GetID() const;
		float		  GetValue() const;
		unsigned int  GetTimeActive() const;
		unsigned int  GetTimeInactive() const;
		
		// Inquirers

		bool IsClicked() const;
		bool IsHeld(float delaySecs = 0.0f) const;
		bool IsPulsed(float delaySecs, float initialDelaySecs = 0.0f) const;
		bool IsReleased(float delaySecsMax = INFINITY) const;

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
