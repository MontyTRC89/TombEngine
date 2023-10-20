#pragma once
#include "Math/Math.h"
#include "Specific/Input/InputAction.h"

using namespace TEN::Math;

struct ItemInfo;

namespace TEN::Input
{
	constexpr auto MAX_KEYBOARD_KEYS	= 256;
	constexpr auto MAX_MOUSE_KEYS		= 8;
	constexpr auto MAX_GAMEPAD_KEYS		= 16;
	constexpr auto MAX_MOUSE_POV_AXES	= 6;
	constexpr auto MAX_GAMEPAD_AXES		= 6;
	constexpr auto MAX_GAMEPAD_POV_AXES = 4;

	constexpr auto MAX_INPUT_SLOTS = MAX_KEYBOARD_KEYS + MAX_MOUSE_KEYS + MAX_GAMEPAD_KEYS + MAX_MOUSE_POV_AXES + (MAX_GAMEPAD_AXES * 2) + MAX_GAMEPAD_POV_AXES;

	enum XInputButton
	{
		XB_START = MAX_KEYBOARD_KEYS + MAX_MOUSE_KEYS,
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
		XB_AXIS_X_POS = MAX_KEYBOARD_KEYS + MAX_GAMEPAD_KEYS + MAX_MOUSE_POV_AXES,
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

	enum class InputAxis
	{
		Move,
		Camera,
		Mouse,

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
	extern std::vector<Vector2>		AxisMap;

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

	Vector2 GetCursorDisplayPosition();

	// TODO: Move global query functions to player input object (not happening soon). -- Sezz 2023.08.07
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
