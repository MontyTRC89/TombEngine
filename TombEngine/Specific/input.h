#pragma once

namespace TEN::Input
{
	constexpr int JOY_AXIS_DEADZONE  = 8000;

	constexpr int MAX_KEYBOARD_KEYS  = 256;
	constexpr int MAX_JOYSTICK_KEYS  = 16;
	constexpr int MAX_JOYSTICK_AXES  = 6;
	constexpr int MAX_POV_AXES       = 4;
	constexpr int LAST_KEYBOARD_KEY  = MAX_KEYBOARD_KEYS - 1;
	constexpr int LAST_JOYSTICK_KEY  = MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS - 1;

	constexpr int MAX_INPUT_SLOTS = MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS + MAX_POV_AXES + MAX_JOYSTICK_AXES * 2;

	enum InputAxis
	{
		MoveVertical,
		MoveHorizontal,
		CameraVertical,
		CameraHorizontal,
		Count
	};

	enum InputKeys
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
		KEY_COUNT
	};

	enum InputActions
	{
		IN_NONE = 0,				// 0x00000000
		IN_FORWARD = (1 << 0),		// 0x00000001
		IN_BACK = (1 << 1),			// 0x00000002
		IN_LEFT = (1 << 2),			// 0x00000004
		IN_RIGHT = (1 << 3),		// 0x00000008
		IN_JUMP = (1 << 4),			// 0x00000010
		IN_DRAW = (1 << 5), 		// 0x00000020
		IN_ACTION = (1 << 6), 		// 0x00000040
		IN_WALK = (1 << 7), 		// 0x00000080
		IN_OPTION = (1 << 8),		// 0x00000100
		IN_LOOK = (1 << 9),			// 0x00000200
		IN_LSTEP = (1 << 10),		// 0x00000400
		IN_RSTEP = (1 << 11),		// 0x00000800
		IN_ROLL = (1 << 12),		// 0x00001000
		IN_PAUSE = (1 << 13),		// 0x00002000
		IN_FLARE = (1 << 14),		// 0x00004000
		IN_SELECT = (1 << 15),		// 0x00008000
		IN_DESELECT = (1 << 16),	// 0x00010000
		IN_SAVE = (1 << 17), 		// 0x00020000
		IN_LOAD = (1 << 18),		// 0x00040000
		IN_CROUCH = (1 << 19),		// 0x00080000
		IN_SPRINT = (1 << 20),		// 0x00100000
		IN_LOOKSWITCH = (1 << 21)	// 0x00200000
	};

	constexpr int IN_OPTIC_CONTROLS = (IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT | IN_ACTION | IN_SELECT | IN_CROUCH | IN_SPRINT);
	constexpr int IN_WAKE = (IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT | IN_LSTEP | IN_RSTEP | IN_WALK | IN_JUMP | IN_SPRINT | IN_ROLL | IN_CROUCH | IN_DRAW | IN_FLARE | IN_ACTION);
	constexpr int IN_DIRECTION = (IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT);

	extern const char* g_KeyNames[];
	extern int TrInput;
	extern int DbInput;
	extern int InputBusy;

	extern std::vector<bool>   KeyMap;
	extern std::vector<float>  AxisMap;

	extern short KeyboardLayout[2][KEY_COUNT];

	void InitialiseInput(HWND handle);
	void DeInitialiseInput();
	bool UpdateInput(bool debounce = true);
	void DefaultConflict();
}