#pragma once
#include "Specific/Input/InputAction.h"

namespace TEN::Input
{
	constexpr int MAX_KEYBOARD_KEYS    = 256;
	constexpr int MAX_GAMEPAD_KEYS     = 16;
	constexpr int MAX_GAMEPAD_AXES     = 6;
	constexpr int MAX_GAMEPAD_POV_AXES = 4;

	constexpr int MAX_INPUT_SLOTS = MAX_KEYBOARD_KEYS + MAX_GAMEPAD_KEYS + MAX_GAMEPAD_POV_AXES + MAX_GAMEPAD_AXES * 2;

	enum InputKey
	{
		KEY_FORWARD,
		KEY_BACK,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_CROUCH,
		KEY_SPRINT,
		KEY_WALK,
		KEY_JUMP,
		KEY_ACTION,
		KEY_DRAW,
		KEY_FLARE,
		KEY_LOOK,
		KEY_ROLL,
		KEY_OPTION,
		KEY_PAUSE,
		KEY_LSTEP,
		KEY_RSTEP,
		/*KEY_ACCELERATE,
		KEY_REVERSE,
		KEY_SPEED,
		KEY_SLOW,
		KEY_BRAKE,
		KEY_FIRE,*/

		KEY_COUNT
	};

	enum InputActions
	{
		IN_NONE		  = 0,
		IN_FORWARD	  = (1 << KEY_FORWARD),
		IN_BACK		  = (1 << KEY_BACK),
		IN_LEFT		  = (1 << KEY_LEFT),
		IN_RIGHT	  = (1 << KEY_RIGHT),
		IN_CROUCH	  = (1 << KEY_CROUCH),
		IN_SPRINT	  = (1 << KEY_SPRINT),
		IN_WALK		  = (1 << KEY_WALK),
		IN_JUMP		  = (1 << KEY_JUMP),
		IN_ACTION	  = (1 << KEY_ACTION),
		IN_DRAW		  = (1 << KEY_DRAW),
		IN_FLARE	  = (1 << KEY_FLARE),
		IN_LOOK		  = (1 << KEY_LOOK),
		IN_ROLL		  = (1 << KEY_ROLL),
		IN_OPTION	  = (1 << KEY_OPTION),
		IN_PAUSE	  = (1 << KEY_PAUSE),
		IN_LSTEP	  = (1 << KEY_LSTEP),
		IN_RSTEP	  = (1 << KEY_RSTEP),
		/*IN_ACCELERATE = (1 << KEY_ACCELERATE),
		IN_REVERSE	  = (1 << KEY_REVERSE),
		IN_SPEED	  = (1 << KEY_SPEED),
		IN_SLOW		  = (1 << KEY_SLOW),
		IN_BRAKE	  = (1 << KEY_BRAKE),
		IN_FIRE		  = (1 << KEY_FIRE),*/

		// Additional input actions without direct key relation

		IN_SAVE		  = (1 << (KEY_COUNT + 0)),
		IN_LOAD		  = (1 << (KEY_COUNT + 1)),
		IN_SELECT	  = (1 << (KEY_COUNT + 2)),
		IN_DESELECT   = (1 << (KEY_COUNT + 3)),
		IN_LOOKSWITCH = (1 << (KEY_COUNT + 4))
	};
	
	#define ACTION_HELD_DIRECTION	  (IsHeld(In::Forward) || IsHeld(In::Back) || IsHeld(In::Left) || IsHeld(In::Right))
	#define ACTION_HELD_WAKE		  (ACTION_HELD_DIRECTION || IsHeld(In::LeftStep) || IsHeld(In::RightStep) || IsHeld(In::Walk) || IsHeld(In::Jump) || IsHeld(In::Sprint) || IsHeld(In::Roll) || IsHeld(In::Crouch) || IsHeld(In::DrawWeapon) || IsHeld(In::Flare) || IsHeld(In::Action))
	#define ACTION_HELD_OPTIC_CONTROL (ACTION_HELD_DIRECTION || IsHeld(In::Action) || IsHeld(In::Crouch) || IsHeld(In::Sprint))

	// Temporary input constants for use with vehicles:

	// TODO: Not needed. Thought too far ahead.
	constexpr int VEHICLE_IN_UP			= IN_FORWARD;
	constexpr int VEHICLE_IN_DOWN		= IN_BACK;
	constexpr int VEHICLE_IN_LEFT		= IN_LEFT;
	constexpr int VEHICLE_IN_RIGHT		= IN_RIGHT;

	constexpr int VEHICLE_IN_ACCELERATE = IN_ACTION;
	constexpr int VEHICLE_IN_REVERSE	= IN_BACK;
	constexpr int VEHICLE_IN_SPEED		= IN_SPRINT;
	constexpr int VEHICLE_IN_SLOW		= IN_WALK;
	constexpr int VEHICLE_IN_BRAKE		= IN_JUMP;
	constexpr int VEHICLE_IN_FIRE		= IN_DRAW | IN_CROUCH;

	// TODO: Not needed since BRAKE is explicitly assosiated with dismounts anyway.
	constexpr int VEHICLE_IN_DISMOUNT	= IN_JUMP | IN_ROLL;

	enum InputAxis
	{
		MoveVertical,
		MoveHorizontal,
		CameraVertical,
		CameraHorizontal,
		Count
	};

	enum class RumbleMode
	{
		Left,
		Right,
		Both
	};

	struct RumbleData
	{
		RumbleMode Mode;
		float Power;
		float LastPower;
		float FadeSpeed;
	};

	extern const char* g_KeyNames[];

	extern vector<InputAction>	ActionMap;
	extern vector<bool>			KeyMap;
	extern vector<float>		AxisMap;

	// Legacy input bitfields
	extern int DbInput; // Debounce: is input clicked?
	extern int TrInput; // Throttle: is input held?

	extern short KeyboardLayout[2][KEY_COUNT];

	void InitialiseInput(HWND handle);
	void DeInitialiseInput();
	void DefaultConflict();
	void UpdateInputActions();
	void ClearAllActions();
	void Rumble(float power, float delayInSec= 0.3f, RumbleMode mode = RumbleMode::Both);
	void StopRumble();

	void  ClearAction(ActionID actionID);
	bool  NoAction();
	bool  IsClicked(ActionID actionID);
	bool  IsPulsed(ActionID actionID, float delayInSec, float initialDelayInSec= 0.0f);
	bool  IsHeld(ActionID actionID);
	bool  IsReleased(ActionID actionID);
	float GetActionValue(ActionID actionID);
	float GetActionTimeActive(ActionID actionID);
	float GetActionTimeInactive(ActionID actionID);
}
