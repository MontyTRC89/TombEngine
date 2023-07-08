#pragma once
#include "Specific/Input/InputAction.h"

struct ItemInfo;

namespace TEN::Input
{
	constexpr int MAX_KEYBOARD_KEYS    = 256;
	constexpr int MAX_GAMEPAD_KEYS     = 16;
	constexpr int MAX_GAMEPAD_AXES     = 6;
	constexpr int MAX_GAMEPAD_POV_AXES = 4;

	constexpr int MAX_INPUT_SLOTS = MAX_KEYBOARD_KEYS + MAX_GAMEPAD_KEYS + MAX_GAMEPAD_POV_AXES + MAX_GAMEPAD_AXES * 2;

	enum XInputButton
	{
		XB_START = MAX_KEYBOARD_KEYS,
		XB_SELECT,
		XB_LSTICK,
		XB_RSTICK,
		XB_LSHIFT,
		XB_RSHIFT,
		XB_UNUSED1,
		XB_UNUSED2,
		XB_A,
		XB_B,
		XB_X,
		XB_Y,
		XB_LOGO,
		XB_AXIS_X_POS = MAX_KEYBOARD_KEYS + MAX_GAMEPAD_KEYS,
		XB_AXIS_X_NEG,
		XB_AXIS_Y_POS,
		XB_AXIS_Y_NEG,
		XB_AXIS_Z_POS,
		XB_AXIS_Z_NEG,
		XB_AXIS_W_POS,
		XB_AXIS_W_NEG,
		XB_AXIS_LTRIGGER_NEG,
		XB_AXIS_LTRIGGER_POS,
		XB_AXIS_RTRIGGER_NEG,
		XB_AXIS_RTRIGGER_POS,
		XB_DPAD_UP,
		XB_DPAD_DOWN,
		XB_DPAD_LEFT,
		XB_DPAD_RIGHT
	};

	// Deprecated.
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
		KEY_LOOK,
		KEY_ROLL,
		KEY_LSTEP,
		KEY_RSTEP,

		KEY_LIGHT,
		KEY_SMALL_MEDIPACK,
		KEY_LARGE_MEDIPACK,
		KEY_PREVIOUS_WEAPON,
		KEY_NEXT_WEAPON,
		KEY_WEAPON_1,
		KEY_WEAPON_2,
		KEY_WEAPON_3,
		KEY_WEAPON_4,
		KEY_WEAPON_5,
		KEY_WEAPON_6,
		KEY_WEAPON_7,
		KEY_WEAPON_8,
		KEY_WEAPON_9,
		KEY_WEAPON_10,
		KEY_SAY_NO,

		KEY_OPTION,
		KEY_PAUSE,
		KEY_SAVE,
		KEY_LOAD,
		KEY_SELECT,
		KEY_DESELECT,

		KEY_COUNT
	};

	// Deprecated.
	enum InputActions
	{
		IN_NONE = 0,

		IN_FORWARD = (1 << KEY_FORWARD),
		IN_BACK	   = (1 << KEY_BACK),
		IN_LEFT	   = (1 << KEY_LEFT),
		IN_RIGHT   = (1 << KEY_RIGHT),
		IN_CROUCH  = (1 << KEY_CROUCH),
		IN_SPRINT  = (1 << KEY_SPRINT),
		IN_WALK	   = (1 << KEY_WALK),
		IN_JUMP	   = (1 << KEY_JUMP),
		IN_ACTION  = (1 << KEY_ACTION),
		IN_DRAW	   = (1 << KEY_DRAW),
		IN_LOOK	   = (1 << KEY_LOOK),
		IN_ROLL	   = (1 << KEY_ROLL),
		IN_LSTEP   = (1 << KEY_LSTEP),
		IN_RSTEP   = (1 << KEY_RSTEP),
	};
	
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

	// TODO: Not needed since BRAKE is explicitly associated with dismounts anyway.
	constexpr int VEHICLE_IN_DISMOUNT	= IN_JUMP | IN_ROLL;

	enum InputAxis
	{
		MoveVertical,
		MoveHorizontal,
		CameraVertical,
		CameraHorizontal,
		Count
	};

	enum class QueueState
	{
		None,
		Push,
		Clear
	};

	enum class RumbleMode
	{
		None,
		Left,
		Right,
		Both
	};

	struct RumbleData
	{
		RumbleMode Mode		 = RumbleMode::None;
		float	   Power	 = 0.0f;
		float	   LastPower = 0.0f;
		float	   FadeSpeed = 0.0f;
	};

	extern const char* g_KeyNames[];

	extern std::vector<InputAction> ActionMap;
	extern std::vector<QueueState>	ActionQueue;
	extern std::vector<bool>		KeyMap;
	extern std::vector<float>		AxisMap;

	// Legacy input bit fields.
	extern int DbInput; // Debounce input.
	extern int TrInput; // Throttle input.

	extern int KeyboardLayout[2][KEY_COUNT];

	void InitializeInput(HWND handle);
	void DeinitializeInput();
	void DefaultConflict();
	void UpdateInputActions(ItemInfo* item, bool applyQueue = false);
	void ApplyActionQueue();
	void ClearActionQueue();
	void ClearAllActions();
	void Rumble(float power, float delayInSec = 0.3f, RumbleMode mode = RumbleMode::Both);
	void StopRumble();
    void ApplyDefaultBindings();
    bool ApplyDefaultXInputBindings();

	// TODO: Later, all these global action accessor functions should be tied to a specific controller/player.
	// Having them loose like this is very inelegant, but since this is only the first iteration, they will do for now. -- Sezz 2022.10.12
	void  ClearAction(ActionID actionID);
	bool  NoAction();
	bool  IsClicked(ActionID actionID);
	bool  IsHeld(ActionID actionID, float delayInSec = 0.0f);
	bool  IsPulsed(ActionID actionID, float delayInSec, float initialDelayInSec = 0.0f);
	bool  IsReleased(ActionID actionID, float maxDelayInSec = INFINITY);
	float GetActionValue(ActionID actionID);
	float GetActionTimeActive(ActionID actionID);
	float GetActionTimeInactive(ActionID actionID);

	bool IsDirectionActionHeld();
	bool IsWakeActionHeld();
	bool IsOpticActionHeld();
}
