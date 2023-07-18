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
		KEY_LEFT_STEP,
		KEY_RIGHT_STEP,
		KEY_WALK,
		KEY_SPRINT,
		KEY_CROUCH,
		KEY_JUMP,
		KEY_ROLL,
		KEY_ACTION,
		KEY_DRAW,
		KEY_LOOK,

		KEY_ACCELERATE,
		KEY_REVERSE,
		KEY_SPEED,
		KEY_SLOW,
		KEY_BRAKE,
		KEY_FIRE,

		KEY_FLARE,
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

		KEY_SELECT,
		KEY_DESELECT,
		KEY_PAUSE,
		KEY_INVENTORY,
		KEY_SAVE,
		KEY_LOAD,

		KEY_COUNT
	};

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
		float	   Power	 = 0.0f;
		RumbleMode Mode		 = RumbleMode::None;
		float	   LastPower = 0.0f;
		float	   FadeSpeed = 0.0f;
	};

	extern std::vector<InputAction> ActionMap;
	extern std::vector<QueueState>	ActionQueue;
	extern std::vector<bool>		KeyMap;
	extern std::vector<float>		AxisMap;

	extern const std::vector<std::string>	   g_KeyNames;
	extern		 std::vector<std::vector<int>> Bindings;

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

	bool IsDirectionalActionHeld();
	bool IsWakeActionHeld();
	bool IsOpticActionHeld();
}
