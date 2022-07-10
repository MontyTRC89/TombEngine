#pragma once

namespace TEN::Input
{
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

		IN_SAVE		  = (1 << KEY_COUNT + 0),
		IN_LOAD		  = (1 << KEY_COUNT + 1),
		IN_SELECT	  = (1 << KEY_COUNT + 2),
		IN_DESELECT   = (1 << KEY_COUNT + 3),
		IN_LOOKSWITCH = (1 << KEY_COUNT + 4)
	};

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
		Both,
		Left,
		Right
	};

	struct RumbleData
	{
		RumbleMode Mode;
		float Power;
		float LastPower;
		float FadeSpeed;
	};

	constexpr int MAX_KEYBOARD_KEYS    = 256;
	constexpr int MAX_GAMEPAD_KEYS     = 16;
	constexpr int MAX_GAMEPAD_AXES     = 6;
	constexpr int MAX_GAMEPAD_POV_AXES = 4;

	constexpr int MAX_INPUT_SLOTS = MAX_KEYBOARD_KEYS + MAX_GAMEPAD_KEYS + MAX_GAMEPAD_POV_AXES + MAX_GAMEPAD_AXES * 2;

	// Temporary input constants for use with vehicles.
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
	constexpr int VEHICLE_IN_DISMOUNT	= IN_JUMP | IN_ROLL; // TODO: Not needed since BRAKE is explicitly assosiated with dismounts anyway.

	constexpr int IN_OPTIC_CONTROLS = IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT | IN_ACTION | IN_CROUCH | IN_SPRINT;
	constexpr int IN_WAKE			= IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT | IN_LSTEP | IN_RSTEP | IN_WALK | IN_JUMP | IN_SPRINT | IN_ROLL | IN_CROUCH | IN_DRAW | IN_FLARE | IN_ACTION;
	constexpr int IN_DIRECTION		= IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT;

	extern const char* g_KeyNames[];

	// Legacy input bitfields.
	extern int DbInput;  // Debounce: is input clicked?
	extern int TrInput;  // Throttle: is input held?
	extern int RelInput; // Release: is input released?
	extern int RawInput; // Throttle for binocular input.

	extern std::vector<bool>  KeyMap;
	extern std::vector<float> AxisMap;

	extern short KeyboardLayout[2][KEY_COUNT];

	void InitialiseInput(HWND handle);
	void DeInitialiseInput();
	bool UpdateInput(bool debounce = true);
	void DefaultConflict();
	void Rumble(float power, float delayInSeconds = 0.3f, RumbleMode mode = RumbleMode::Both);
	void StopRumble();

	// ---------------------------------------------------------------

	typedef enum class InputID
	{
		None = -1,
		Forward,
		Back,
		Left,
		Right,
		Crouch,
		Sprint,
		Walk,
		Jump,
		Action,
		Draw,
		Flare,
		Look,
		Roll,
		Option,
		Pause,
		LeftStep,
		RightStep,
		/*Accelerate,
		Reverse,
		Speed,
		Slow,
		Brake,
		Fire,*/

		Save,
		Load,
		Select,
		Deselect,
		LookSwitch,
	} In;

	class InputAction
	{
	public:
		InputID ID			 = In::None;
		float	TimeHeld	 = 0.0f;
		float	PrevTimeHeld = 0.0f;
		float	TimeReleased = 0.0f;

		bool IsClicked();
		bool IsPulsed(float interval, float initialInterval = 0.0f);
		bool IsHeld();
		bool IsReleased();

	//private:
		bool IsActive	  = false;
		bool PrevIsActive = false;
	};

	extern std::vector<InputAction> Actions;

	bool IsClicked(InputID input);
	bool IsPulsed(InputID input, float interval, float initialInterval = 0.0f);
	bool IsHeld(InputID input);
	bool IsReleased(InputID input);
}
