#pragma once

#include <OISInputManager.h>
#include <OISException.h>
#include <OISKeyboard.h>
#include <OISJoyStick.h>
#include <OISEvents.h>
#include <OISForceFeedback.h>

namespace TEN::Input
{
	constexpr int NUM_CONTROLS = 16;

	constexpr int JOY_AXIS_DEADZONE  = 2500;

	constexpr int MAX_KEYBOARD_KEYS  = 256;
	constexpr int MAX_JOYSTICK_KEYS  = 16;
	constexpr int MAX_JOYSTICK_AXES  = 6;
	constexpr int MAX_POV_AXES       = 4;
	constexpr int LAST_KEYBOARD_KEY  = MAX_KEYBOARD_KEYS - 1;
	constexpr int LAST_JOYSTICK_KEY  = MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS - 1;

	constexpr int MAX_INPUT_SLOTS = MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS + MAX_JOYSTICK_AXES * 2 + MAX_POV_AXES;

	enum INPUT_BUTTONS
	{
		IN_NONE = 0,								// 0x00000000
		IN_FORWARD = (1 << 0),						// 0x00000001
		IN_BACK = (1 << 1),							// 0x00000002
		IN_LEFT = (1 << 2),							// 0x00000004
		IN_RIGHT = (1 << 3),						// 0x00000008
		IN_JUMP = (1 << 4),							// 0x00000010
		IN_DRAW = (1 << 5), // Space / Triangle		// 0x00000020
		IN_ACTION = (1 << 6), // Ctrl / X			// 0x00000040
		IN_WALK = (1 << 7), // Shift / R1			// 0x00000080
		IN_OPTION = (1 << 8),						// 0x00000100
		IN_LOOK = (1 << 9),							// 0x00000200
		IN_LSTEP = (1 << 10),						// 0x00000400
		IN_RSTEP = (1 << 11),						// 0x00000800
		IN_ROLL = (1 << 12), // End / O				// 0x00001000
		IN_PAUSE = (1 << 13),						// 0x00002000
		IN_A = (1 << 14),							// 0x00004000
		IN_B = (1 << 15),							// 0x00008000
		IN_CHEAT = (1 << 16),						// 0x00010000
		IN_D = (1 << 17),							// 0x00020000
		IN_E = (1 << 18),							// 0x00040000
		IN_FLARE = (1 << 19),						// 0x00080000
		IN_SELECT = (1 << 20),						// 0x00100000
		IN_DESELECT = (1 << 21),					// 0x00200000
		IN_SAVE = (1 << 22), // F5					// 0x00400000
		IN_LOAD = (1 << 23),  // F6					// 0x00800000
		IN_STEPSHIFT = (1 << 24),					// 0x01000000
		IN_LOOKLEFT = (1 << 25),					// 0x02000000
		IN_LOOKRIGHT = (1 << 26),					// 0x04000000
		IN_LOOKFORWARD = (1 << 27),					// 0x08000000
		IN_LOOKBACK = (1 << 28),					// 0x10000000
		IN_CROUCH = (1 << 29),						// 0x20000000
		IN_SPRINT = (1 << 30),						// 0x40000000
		IN_LOOKSWITCH = (1 << 31),					// 0x80000000
		IN_ALL = ~0,								// 0xFFFFFFFF (-1)
	};

	constexpr int IN_OPTIC_CONTROLS = (IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT | IN_ACTION | IN_SELECT | IN_CROUCH | IN_SPRINT);
	constexpr int IN_WAKE = (IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT | IN_LSTEP | IN_RSTEP | IN_WALK | IN_JUMP | IN_SPRINT | IN_ROLL | IN_CROUCH | IN_DRAW | IN_FLARE | IN_ACTION);
	constexpr int IN_DIRECTION = (IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT);

	enum IKEYS
	{
		KEY_FORWARD = 0,
		KEY_BACK = 1,
		KEY_LEFT = 2,
		KEY_RIGHT = 3,
		KEY_CROUCH = 4,
		KEY_SPRINT = 5,
		KEY_WALK = 6,
		KEY_JUMP = 7,
		KEY_ACTION = 8,
		KEY_DRAW = 9,
		KEY_FLARE = 10,
		KEY_LOOK = 11,
		KEY_ROLL = 12,
		KEY_OPTION = 13,
		KEY_LSTEP = 14,
		KEY_RSTEP = 15,
		KEY_PAUSE = 16,
		KEY_SELECT = 17
	};

	extern const char* g_KeyNames[];
	extern int TrInput;
	extern int DbInput;
	extern int InputBusy;

	extern short KeyboardLayout[2][18];

	extern std::vector<bool>   KeyMap;
	extern std::vector<float>  AxisMap;

	void InitialiseInput(HWND handle, HINSTANCE instance);
	bool UpdateInput(bool debounce = true);
	bool Key(int number);
	void DefaultConflict();
}