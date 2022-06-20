#include "framework.h"
#include "Specific/input.h"

#include "Game/camera.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/savegame.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Specific/winmain.h"

using namespace OIS;
using TEN::Renderer::g_Renderer;

//namespace TEN::Input
//{
	const char* g_KeyNames[] =
	{
			NULL,		"ESC",		"1",		"2",		"3",		"4",		"5",		"6",
			"7",		"8",		"9",		"0",		"-",		"+",		"BKSP",		"TAB",
			"Q",		"W",		"E",		"R",		"T",		"Y",		"U",		"I",
			"O",		"P",		"<",		">",		"RET",		"CTRL",		"A",		"S",
			"D",		"F",		"G",		"H",		"J",		"K",		"L",		";",
			"'",		"`",		"SHIFT",	"#",		"Z",		"X",		"C",		"V",
			"B",		"N",		"M",		",",		".",		"/",		"SHIFT",	"PADx",
			"ALT",		"SPACE",	"CAPS",		NULL,		NULL,		NULL,		NULL,		NULL,

			NULL,		NULL,		NULL,		NULL,		NULL,		"NMLK",		NULL,		"PAD7",
			"PAD8",		"PAD9",		"PAD-",		"PAD4",		"PAD5",		"PAD6",		"PAD+",		"PAD1",
			"PAD2",		"PAD3",		"PAD0",		"PAD.",		NULL,		NULL,		"\\",		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,

			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		"ENTER",	"CTRL",		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		"SHIFT",	NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		"PAD/",		NULL,		NULL,
			"ALT",		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,

			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		"HOME",
			"UP",		"PGUP",		NULL,		"LEFT",		NULL,		"RIGHT",	NULL,		"END",
			"DOWN",		"PGDN",		"INS",		"DEL",		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,
			NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,

			"JOY1", 	"JOY2",		"JOY3",		"JOY4", 	"JOY5",		"JOY6", 	"JOY7",		"JOY8",
			"JOY9",		"JOY10",	"JOY11",	"JOY12",	"JOY13",	"JOY14",	"JOY15",	"JOY16",

			"X-",		"X+",		"Y-",		"Y+",		"Z+",		"Z-",		"W+",		"W-", 

			"U+",		"U-",		"V+",		"V-",		"POV_UP",	"POV_DOWN", "POV_LEFT", "POV_RIGHT"
	};

	InputManager* g_InputManager = nullptr;
	Keyboard* g_Keyboard = nullptr;
	JoyStick* g_Joystick = nullptr;

	int TrInput;
	int DbInput;
	int InputBusy;
	bool SetDebounce = false;

	std::vector<bool>  KeyMap  = {};
	std::vector<float> AxisMap = {};

	int ConflictingKeys[18];

	short KeyboardLayout[2][18] =
	{
		{ KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT, KC_PERIOD, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_RCONTROL, KC_SPACE, KC_COMMA, KC_NUMPAD0, KC_END, KC_ESCAPE, KC_DELETE, KC_PGDOWN, KC_P, KC_RETURN },
		{ KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT, KC_PERIOD, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_RCONTROL, KC_SPACE, KC_COMMA, KC_NUMPAD0, KC_END, KC_ESCAPE, KC_DELETE, KC_PGDOWN, KC_P, KC_RETURN }
	};

	class EventHandler : public KeyListener, public JoyStickListener
	{
	public:
		EventHandler() { }
		~EventHandler() { }

		bool keyPressed(const KeyEvent& arg)
		{
			if (arg.key >= KeyMap.size())
				return true;

			KeyMap[arg.key] = true;
			return true;
		}

		bool keyReleased(const KeyEvent& arg)
		{
			if (arg.key >= KeyMap.size())
				return true;

			KeyMap[arg.key] = false;
			return true;
		}

		bool buttonPressed(const JoyStickEvent& arg, int button)
		{
			if (button + MAX_KEYBOARD_KEYS >= KeyMap.size())
				return true;

			KeyMap[button + MAX_KEYBOARD_KEYS] = true;
			return true;
		}

		bool buttonReleased(const JoyStickEvent& arg, int button)
		{
			if (button + MAX_KEYBOARD_KEYS >= KeyMap.size())
				return true;

			KeyMap[button + MAX_KEYBOARD_KEYS] = false;
			return true;
		}

		bool axisMoved(const JoyStickEvent& arg, int axis)
		{
			return true;
		}

		bool sliderMoved(const JoyStickEvent& arg, int index)
		{
			std::cout << std::endl
				<< arg.device->vendor() << ". Slider # " << index
				<< " X Value: " << arg.state.mSliders[index].abX
				<< " Y Value: " << arg.state.mSliders[index].abY;
			return true;
		}

		bool povMoved(const JoyStickEvent& arg, int pov)
		{
			std::cout << std::endl
				<< arg.device->vendor() << ". POV" << pov << " ";

			if (arg.state.mPOV[pov].direction & Pov::North) //Going up
				std::cout << "North";
			else if (arg.state.mPOV[pov].direction & Pov::South) //Going down
				std::cout << "South";

			if (arg.state.mPOV[pov].direction & Pov::East) //Going right
				std::cout << "East";
			else if (arg.state.mPOV[pov].direction & Pov::West) //Going left
				std::cout << "West";

			if (arg.state.mPOV[pov].direction == Pov::Centered) //stopped/centered out
				std::cout << "Centered";
			return true;
		}

		bool vector3Moved(const JoyStickEvent& arg, int index)
		{
			std::cout.precision(2);
			std::cout.flags(std::ios::fixed | std::ios::right);
			std::cout << std::endl
				<< arg.device->vendor() << ". Orientation # " << index
				<< " X Value: " << arg.state.mVectors[index].x
				<< " Y Value: " << arg.state.mVectors[index].y
				<< " Z Value: " << arg.state.mVectors[index].z;
			std::cout.precision();
			std::cout.flags();
			return true;
		}
	};

	//Create a global instance
	EventHandler handler;

	void InitialiseInput(HWND handle, HINSTANCE instance)
	{
		TENLog("Initializing input system...", LogLevel::Info);

		KeyMap.resize(MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS + MAX_POV_AXES + MAX_JOYSTICK_AXES * 2);
		AxisMap.resize(MAX_JOYSTICK_AXES);

		try
		{
			g_InputManager = InputManager::createInputSystem((size_t)handle);

			TENLog("System: " + g_InputManager->inputSystemName());

			auto v = g_InputManager->getVersionNumber();
			TENLog("OIS library version: " + std::to_string(v), LogLevel::Info);

			if (g_InputManager->getNumberOfDevices(OISKeyboard) == 0)
			{
				TENLog("Keyboard not found!", LogLevel::Warning);
			}
			else
			{
				g_Keyboard = (Keyboard*)g_InputManager->createInputObject(OISKeyboard, false, "");
				g_Keyboard->setEventCallback(&handler);
			}
		}
		catch (OIS::Exception& ex)
		{
			TENLog("An exception occured during input system init: " + std::string(ex.eText), LogLevel::Error);
		}

		int numJoysticks = g_InputManager->getNumberOfDevices(OISJoyStick);
		if (numJoysticks > 0)
		{
			TENLog("Found " + std::to_string(numJoysticks) + " connected game devices.", LogLevel::Info);

			try
			{
				g_Joystick = (JoyStick*)g_InputManager->createInputObject(OISJoyStick, false, "");
				g_Joystick->setEventCallback(&handler);
			}
			catch(OIS::Exception& ex)
			{
				TENLog("An exception occured during joystick init: " + std::string(ex.eText), LogLevel::Error);
			}
		}
	}

	void ReadJoystick()
	{
		if (!g_Joystick)
			return;

		g_Joystick->capture();
		const JoyStickState& joy = g_Joystick->getJoyStickState();

		// Zero all joy keys, axis and POVs
		for (unsigned int key = MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS; key < KeyMap.size(); key++)
			KeyMap[key] = false;

		for (unsigned int key = 0; key < joy.mButtons.size(); key++)
			KeyMap[MAX_KEYBOARD_KEYS + key] = joy.mButtons[key];

		for (unsigned int axis = 0; axis < joy.mAxes.size(); axis++)
		{
			if (axis >= AxisMap.size())
				break;

			if (abs(joy.mAxes[axis].abs) < JOY_AXIS_DEADZONE)
				continue;

			float normalizedValue = (float)(joy.mAxes[axis].abs + (joy.mAxes[axis].abs > 0 ? -JOY_AXIS_DEADZONE : JOY_AXIS_DEADZONE)) / 65536.0f;
			AxisMap[axis] = normalizedValue;

			int negKeyIndex = MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS + (axis * 2);
			int posKeyIndex = MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS + (axis * 2) + 1;

			KeyMap[negKeyIndex] = false;
			KeyMap[posKeyIndex] = false;

			if (normalizedValue > 0)
				KeyMap[negKeyIndex] = true;
			else
				KeyMap[posKeyIndex] = true;
		}

		for (unsigned int pov = 0; pov < 4; pov++)
		{
			if (joy.mPOV[pov].direction & Pov::North)
				KeyMap[MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS + MAX_JOYSTICK_AXES * 2 + 0] = true;

			if (joy.mPOV[pov].direction & Pov::South)
				KeyMap[MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS + MAX_JOYSTICK_AXES * 2 + 1] = true;

			if (joy.mPOV[pov].direction & Pov::West)
				KeyMap[MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS + MAX_JOYSTICK_AXES * 2 + 2] = true;

			if (joy.mPOV[pov].direction & Pov::East)
				KeyMap[MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS + MAX_JOYSTICK_AXES * 2 + 3] = true;
		}
	}

	void ReadKeyboard()
	{
		if (!g_Keyboard)
			return;

		g_Keyboard->capture();

		for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
			KeyMap[i] = g_Keyboard->isKeyDown((KeyCode)i);

		// Empty the numeric pad keys
		KeyMap[KC_DECIMAL] = 0;
		KeyMap[KC_NUMPAD0] = 0;
		KeyMap[KC_NUMPAD1] = 0;
		KeyMap[KC_NUMPAD2] = 0;
		KeyMap[KC_NUMPAD3] = 0;
		KeyMap[KC_NUMPAD4] = 0;
		KeyMap[KC_NUMPAD5] = 0;
		KeyMap[KC_NUMPAD6] = 0;
		KeyMap[KC_NUMPAD7] = 0;
		KeyMap[KC_NUMPAD8] = 0;
		KeyMap[KC_NUMPAD9] = 0;
	}

	bool Key(int number)
	{
		short key = KeyboardLayout[1][number];

		if (KeyMap[key])
			return true;

		switch (key)
		{
		case KC_RCONTROL:
			return KeyMap[KC_LCONTROL];

		case KC_LCONTROL:
			return KeyMap[KC_RCONTROL];

		case KC_RSHIFT:
			return KeyMap[KC_LSHIFT];

		case KC_LSHIFT:
			return KeyMap[KC_RSHIFT];

		case KC_RMENU:
			return KeyMap[KC_LMENU];

		case KC_LMENU:
			return KeyMap[KC_RMENU];
		}

		if (ConflictingKeys[number])
			return false;

		key = KeyboardLayout[0][number];

		if (KeyMap[key])
			return true;

		switch (key)
		{
		case KC_RCONTROL:
			return KeyMap[KC_LCONTROL];

		case KC_LCONTROL:
			return KeyMap[KC_RCONTROL];

		case KC_RSHIFT:
			return KeyMap[KC_LSHIFT];

		case KC_LSHIFT:
			return KeyMap[KC_RSHIFT];

		case KC_RMENU:
			return KeyMap[KC_LMENU];

		case KC_LMENU:
			return KeyMap[KC_RMENU];
		}

		return false;
	}

	bool UpdateInput()
	{
		ReadKeyboard();
		//ReadJoystick();

		static int lInput;

		lInput = 0;

		if (Key(KEY_FORWARD))
			lInput |= IN_FORWARD;

		if (Key(KEY_BACK))
			lInput |= IN_BACK;

		if (Key(KEY_LEFT))
			lInput |= IN_LEFT;

		if (Key(KEY_RIGHT))
			lInput |= IN_RIGHT;

		if (Key(KEY_CROUCH))
			lInput |= IN_CROUCH;

		if (Key(KEY_SPRINT))
			lInput |= IN_SPRINT;

		if (Key(KEY_WALK))
			lInput |= IN_WALK;

		if (Key(KEY_JUMP))
			lInput |= IN_JUMP;

		if (Key(KEY_ACTION))
			lInput |= IN_ACTION | IN_SELECT;

		if (Key(KEY_DRAW))
			lInput |= IN_DRAW;

		bool flare = false;
		static bool flareNo = false;

		// TODO: Better flare handling in crawl states.
		if (Key(KEY_FLARE) || flare)
		{
			if (!flareNo)
			{
				if (LaraItem->Animation.ActiveState == LS_CRAWL_FORWARD ||
					LaraItem->Animation.ActiveState == LS_CRAWL_TURN_LEFT ||
					LaraItem->Animation.ActiveState == LS_CRAWL_TURN_RIGHT ||
					LaraItem->Animation.ActiveState == LS_CRAWL_BACK ||
					LaraItem->Animation.ActiveState == LS_CRAWL_TO_HANG)
				{
					SoundEffect(SFX_TR4_LARA_NO_ENGLISH, nullptr, SoundEnvironment::Always);
					flareNo = true;
				}
				else
				{
					flareNo = false;
					lInput |= IN_FLARE;
				}
			}
		}
		else
			flareNo = false;

		if (Key(KEY_LOOK))
			lInput |= IN_LOOK;

		if (Key(KEY_ROLL))
			lInput |= IN_ROLL;

		if (Key(KEY_OPTION))
			lInput |= IN_OPTION;

		if (Key(KEY_LSTEP))
			lInput |= IN_LSTEP;

		if (Key(KEY_RSTEP))
			lInput |= IN_RSTEP;

		if (Key(KEY_PAUSE))
			lInput |= IN_PAUSE;

		if (Key(KEY_SELECT))
			lInput |= IN_SELECT;

		/*if (opt_ControlMethod == CM_JOYSTICK)
		{
			if (Key(opt_JLok + 256))
				lInput |= IN_LOOK;

			if (Key(opt_JRol + 256))
				lInput |= IN_ROLL;

			if (Key(opt_JInv + 256))
				lInput |= IN_OPTION;
		}*/

		// CHECK
		if (KeyMap[1])
			lInput |= IN_OPTION | IN_DESELECT;

		// Switch debug pages

		static int debugTimeout = 0;
		if (KeyMap[KC_F10] || KeyMap[KC_F11])
		{
			if (debugTimeout == 0)
			{
				debugTimeout = 1;
				g_Renderer.SwitchDebugPage(KeyMap[KC_F10]);
			}
		}
		else
			debugTimeout = 0;

		static int lookTimeout = 0;

		if (Lara.Control.HandStatus == HandStatus::WeaponReady)
		{
			// TODO: AutoTarget Not Saved
			//Savegame.AutoTarget = 1;

			if (lInput & IN_LOOK)
			{
				if (lookTimeout >= 6)
					lookTimeout = 100;
				else
				{
					lInput &= ~IN_LOOK;
					lookTimeout++;
				}
			}
			else
			{
				if (lookTimeout != 0 && lookTimeout != 100)
					lInput |= IN_LOOKSWITCH;

				lookTimeout = 0;
			}
		}

		static int medipackTimeout = 0;

		/***************************WEAPON HOTKEYS***************************/
		if (KeyMap[KC_1] && Lara.Weapons[(int)LaraWeaponType::Pistol].Present == true)
			Lara.Control.Weapon.RequestGunType = LaraWeaponType::Pistol;

		if (KeyMap[KC_2] && Lara.Weapons[(int)LaraWeaponType::Shotgun].Present == true)
			Lara.Control.Weapon.RequestGunType = LaraWeaponType::Shotgun;

		if (KeyMap[KC_3] && Lara.Weapons[(int)LaraWeaponType::Revolver].Present == true)
			Lara.Control.Weapon.RequestGunType = LaraWeaponType::Revolver;

		if (KeyMap[KC_4] && Lara.Weapons[(int)LaraWeaponType::Uzi].Present == true)
			Lara.Control.Weapon.RequestGunType = LaraWeaponType::Uzi;

		if (KeyMap[KC_5] && Lara.Weapons[(int)LaraWeaponType::HarpoonGun].Present == true)
			Lara.Control.Weapon.RequestGunType = LaraWeaponType::HarpoonGun;

		if (KeyMap[KC_6] && Lara.Weapons[(int)LaraWeaponType::HK].Present == true)
			Lara.Control.Weapon.RequestGunType = LaraWeaponType::HK;

		if (KeyMap[KC_7] && Lara.Weapons[(int)LaraWeaponType::RocketLauncher].Present == true)
			Lara.Control.Weapon.RequestGunType = LaraWeaponType::RocketLauncher;

		if (KeyMap[KC_8] && Lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Present == true)
			Lara.Control.Weapon.RequestGunType = LaraWeaponType::GrenadeLauncher;
		/*------------------------------------------------------------------*/

		if (KeyMap[KC_0])
		{
			if (medipackTimeout == 0)
			{
				if (LaraItem->HitPoints > 0 && LaraItem->HitPoints < LARA_HEALTH_MAX || Lara.PoisonPotency)
				{
					if (Lara.Inventory.TotalSmallMedipacks != 0)
					{
						if (Lara.Inventory.TotalSmallMedipacks != -1)
							Lara.Inventory.TotalSmallMedipacks--;

						Lara.PoisonPotency = 0;
						LaraItem->HitPoints += LARA_HEALTH_MAX / 2;
						SoundEffect(SFX_TR4_MENU_MEDI, nullptr, SoundEnvironment::Always); // TODO: Fix heal sound not triggering if small medi doesn't top off Lara's health. original tr4/5 issue

						if (LaraItem->HitPoints > LARA_HEALTH_MAX)
						{
							LaraItem->HitPoints = LARA_HEALTH_MAX;
							SoundEffect(SFX_TR4_MENU_MEDI, nullptr, SoundEnvironment::Always);
							Statistics.Game.HealthUsed++;
						}
					}

					medipackTimeout = 15;
				}
			}
		}
		else if (KeyMap[KC_9])
		{
			if (medipackTimeout == 0)
			{
				if (LaraItem->HitPoints > 0 && LaraItem->HitPoints < LARA_HEALTH_MAX || Lara.PoisonPotency)
				{
					if (Lara.Inventory.TotalLargeMedipacks != 0)
					{
						if (Lara.Inventory.TotalLargeMedipacks != -1)
							Lara.Inventory.TotalLargeMedipacks--;

						Lara.PoisonPotency = 0;
						LaraItem->HitPoints += LARA_HEALTH_MAX;

						if (LaraItem->HitPoints > LARA_HEALTH_MAX)
						{
							LaraItem->HitPoints = LARA_HEALTH_MAX;
							SoundEffect(SFX_TR4_MENU_MEDI, nullptr, SoundEnvironment::Always);
							Statistics.Game.HealthUsed++;
						}
					}

					medipackTimeout = 15;
				}
			}
		}
		else if (medipackTimeout != 0)
			medipackTimeout--;

		if (KeyMap[KC_F10])
			lInput |= IN_E;

		// TODO: Make FORWARD+BACK to turn 180 degrees a user option.
		/*if (lInput & IN_FORWARD && lInput & IN_BACK)
			lInput |= IN_ROLL;*/
		if (lInput & IN_ROLL && BinocularRange)
			lInput &= ~IN_ROLL;

		// Block simultaneous LEFT+RIGHT input.
		if (lInput & IN_LEFT && lInput & IN_RIGHT)
			lInput &= ~(IN_LEFT | IN_RIGHT);

		if (SetDebounce)
			DbInput = InputBusy;

		if (KeyMap[KC_F5])
			lInput |= IN_SAVE;

		if (KeyMap[KC_F6])
			lInput |= IN_LOAD;

		InputBusy = lInput;
		TrInput = lInput;

		if (Lara.Inventory.IsBusy)
		{
			lInput &= (IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT | IN_OPTION | IN_LOOK | IN_PAUSE);

			if (lInput & IN_FORWARD && lInput & IN_BACK)
				lInput &= ~IN_BACK;
		}

		if (SetDebounce)
			DbInput = TrInput & (DbInput ^ TrInput);

		if (KeyMap[KC_F])
			lInput |= IN_FORWARD;
		else
			lInput &= ~IN_FORWARD;

		return true;
	}

	void DefaultConflict()
	{
		for (int i = 0; i < NUM_CONTROLS; i++)
		{
			short key = KeyboardLayout[0][i];

			ConflictingKeys[i] = false;

			for (int j = 0; j < NUM_CONTROLS; j++)
			{
				if (key == KeyboardLayout[1][j])
				{
					ConflictingKeys[i] = true;
					break;
				}
			}
		}
	}
//}
