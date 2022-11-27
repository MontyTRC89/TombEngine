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
	constexpr auto MAX_MOUSE_POV_AXES	= 4;
	constexpr auto MAX_GAMEPAD_AXES		= 6;
	constexpr auto MAX_GAMEPAD_POV_AXES = 4;

	constexpr auto MAX_INPUT_SLOTS = MAX_KEYBOARD_KEYS + MAX_MOUSE_KEYS + MAX_GAMEPAD_KEYS + MAX_MOUSE_POV_AXES + (MAX_GAMEPAD_AXES * 2) + MAX_GAMEPAD_POV_AXES;

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

	enum class InputAxis
	{
		Move,
		Camera,
		Count
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

	struct MouseData
	{
		Vector2i Absolute = Vector2i::Zero;
		Vector2i Relative = Vector2i::Zero;
	};

	extern const std::vector<std::string> g_KeyNames;

	extern MouseData				MouseInfo;
	extern std::vector<InputAction> ActionMap;
	extern std::vector<bool>		KeyMap;
	extern std::vector<Vector2>		AxisMap;

	// Legacy input bit fields.
	extern int DbInput; // Debounce: is input clicked?
	extern int TrInput; // Throttle: is input held?

	extern short KeyboardLayout[2][KEY_COUNT];

	void InitialiseInput(HWND handle);
	void DeinitialiseInput();
	void DefaultConflict();
	void UpdateInputActions(ItemInfo* item);
	void ClearAllActions();
	void Rumble(float power, float delayInSec = 0.3f, RumbleMode mode = RumbleMode::Both);
	void StopRumble();

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
