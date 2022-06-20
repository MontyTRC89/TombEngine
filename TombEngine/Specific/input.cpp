#include "framework.h"
#include "Specific/input.h"

#include "Game/camera.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/savegame.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"

using namespace OIS;
using TEN::Renderer::g_Renderer;

namespace TEN::Input
{
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

	// OIS interfaces
	InputManager* g_InputManager = nullptr;
	Keyboard* g_Keyboard = nullptr;
	JoyStick* g_Joystick = nullptr;
	ForceFeedback* g_JoystickVibration = nullptr;
	
	int TrInput;
	int DbInput;
	int InputBusy;

	std::vector<bool>  KeyMap  = {};
	std::vector<float> AxisMap = {};

	int ConflictingKeys[NUM_CONTROLS];

	short KeyboardLayout[2][NUM_CONTROLS] =
	{
		{ KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT, KC_PERIOD, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_RCONTROL, KC_SPACE, KC_COMMA, KC_NUMPAD0, KC_END, KC_ESCAPE, KC_DELETE, KC_PGDOWN, KC_P, KC_RETURN },
		{ KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT, KC_PERIOD, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_RCONTROL, KC_SPACE, KC_COMMA, KC_NUMPAD0, KC_END, KC_ESCAPE, KC_DELETE, KC_PGDOWN, KC_P, KC_RETURN }
	};

	void InitialiseInput(HWND handle, HINSTANCE instance)
	{
		TENLog("Initializing input system...", LogLevel::Info);

		unsigned int v = g_InputManager->getVersionNumber();
		TENLog("OIS Version: " + std::to_string(v >> 16) + "." + std::to_string((v >> 8) & 0x000000FF) + "." + std::to_string(v & 0x000000FF), LogLevel::Info);

		KeyMap.resize(MAX_INPUT_SLOTS);
		AxisMap.resize(InputAxis::Count);

		try
		{
			g_InputManager = InputManager::createInputSystem((size_t)handle);
			g_InputManager->enableAddOnFactory(InputManager::AddOn_All);

			if (g_InputManager->getNumberOfDevices(OISKeyboard) == 0)
			{
				TENLog("Keyboard not found!", LogLevel::Warning);
			}
			else
			{
				g_Keyboard = (Keyboard*)g_InputManager->createInputObject(OISKeyboard, true);
			}
		}
		catch (OIS::Exception& ex)
		{
			TENLog("An exception occured during input system init: " + std::string(ex.eText), LogLevel::Error);
		}

		int numJoysticks = g_InputManager->getNumberOfDevices(OISJoyStick);
		if (numJoysticks > 0)
		{
			TENLog("Found " + std::to_string(numJoysticks) + " connected game controller" + (numJoysticks > 1 ? "s." : "."), LogLevel::Info);

			try
			{
				g_Joystick = (JoyStick*)g_InputManager->createInputObject(OISJoyStick, true);
				TENLog("Using '" + g_Joystick->vendor() + "' device for input.", LogLevel::Info);

				// Try to initialize vibration interface
				g_JoystickVibration = (ForceFeedback*)g_Joystick->queryInterface(Interface::ForceFeedback);
				if (g_JoystickVibration)
					TENLog("Controller supports vibration.", LogLevel::Info);
			}
			catch (OIS::Exception& ex)
			{
				TENLog("An exception occured during game controller init: " + std::string(ex.eText), LogLevel::Error);
			}
		}
	}

	void ClearInputData()
	{
		for (int key = 0; key < KeyMap.size(); key++)
			KeyMap[key] = false;

		for (int axis = 0; axis < InputAxis::Count; axis++)
			AxisMap[axis] = 0.0f;
	}

	void SetDiscreteAxisValues(unsigned int index)
	{
		for (int layout = 0; layout <= 1; layout++)
		{
			if (KeyboardLayout[layout][KEY_FORWARD] == index)
				AxisMap[(unsigned int)InputAxis::MoveVertical] = 1.0f;
			else if (KeyboardLayout[layout][KEY_BACK] == index)
				AxisMap[(unsigned int)InputAxis::MoveVertical] = -1.0f;
			else if (KeyboardLayout[layout][KEY_LEFT] == index)
				AxisMap[(unsigned int)InputAxis::MoveHorizontal] = -1.0f;
			else if (KeyboardLayout[layout][KEY_RIGHT] == index)
				AxisMap[(unsigned int)InputAxis::MoveHorizontal] = 1.0f;
		}
	}

	void ReadJoystick()
	{
		if (!g_Joystick)
			return;

		try
		{
			// Poll joystick
			g_Joystick->capture();
			const JoyStickState& joy = g_Joystick->getJoyStickState();

			// Scan buttons
			for (int key = 0; key < joy.mButtons.size(); key++)
				KeyMap[MAX_KEYBOARD_KEYS + key] = joy.mButtons[key];

			// Scan axes
			for (int axis = 0; axis < joy.mAxes.size(); axis++)
			{
				// We don't support anything above 6 existing XBOX/PS controller axes (two sticks plus triggers)
				if (axis >= MAX_JOYSTICK_AXES)
					break;
				
				// Filter out deadzone
				if (abs(joy.mAxes[axis].abs) < JOY_AXIS_DEADZONE)
					continue;

				// Calculate normalized analog value to be used in game later
				float normalizedValue = (float)(joy.mAxes[axis].abs + (joy.mAxes[axis].abs > 0 ? -JOY_AXIS_DEADZONE : JOY_AXIS_DEADZONE)) / (float)std::numeric_limits<short>::max();

				// Calculate and reset discrete input slots
				unsigned int negKeyIndex = MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS + (axis * 2);
				unsigned int posKeyIndex = MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS + (axis * 2) + 1;
				KeyMap[negKeyIndex] = false;
				KeyMap[posKeyIndex] = false;

				// Decide on the discrete input registering based on analog value
				unsigned int usedIndex = normalizedValue > 0 ? negKeyIndex : posKeyIndex;
				KeyMap[usedIndex] = true;

				// Register analog input in certain direction.
				// If axis is bound as directional controls, register axis as directional input.
				// Otherwise, register as camera movement input (for future).
				// NOTE: abs() operations are needed to avoid issues with inverted axes on different controllers.

				if (KeyboardLayout[1][KEY_FORWARD] == usedIndex)
					AxisMap[InputAxis::MoveVertical] = abs(normalizedValue);
				else if (KeyboardLayout[1][KEY_BACK] == usedIndex)
					AxisMap[InputAxis::MoveVertical] = -abs(normalizedValue);
				else if (KeyboardLayout[1][KEY_LEFT] == usedIndex)
					AxisMap[InputAxis::MoveHorizontal] = -abs(normalizedValue);
				else if (KeyboardLayout[1][KEY_RIGHT] == usedIndex)
					AxisMap[InputAxis::MoveHorizontal] = abs(normalizedValue);
				else
				{
					unsigned int camAxisIndex = (unsigned int)std::clamp((unsigned int)InputAxis::CameraVertical + axis % 2, 
																		 (unsigned int)InputAxis::CameraVertical, 
																		 (unsigned int)InputAxis::CameraHorizontal);
					AxisMap[camAxisIndex] = normalizedValue;
				}
			}

			// Scan POVs (controller usually have one, but let's scan all of them for paranoia)
			for (int pov = 0; pov < 4; pov++)
			{
				if (joy.mPOV[pov].direction == Pov::Centered)
					continue;
				
				// Do 4 passes, every pass checks ever POV direction. For every direction,
				// separate keypress is registered. This is needed to allow multiple directions
				// pressed at the same time.

				for (int pass = 0; pass < 4; pass++)
				{
					unsigned int index = MAX_KEYBOARD_KEYS + MAX_JOYSTICK_KEYS + MAX_JOYSTICK_AXES * 2;

					switch (pass)
					{
					case 0: 
						if (!(joy.mPOV[pov].direction & Pov::North))
							continue;
						break;

					case 1:
						if (!(joy.mPOV[pov].direction & Pov::South))
							continue;
						break;

					case 2:
						if (!(joy.mPOV[pov].direction & Pov::West))
							continue;
						break;

					case 3:
						if (!(joy.mPOV[pov].direction & Pov::East))
							continue;
						break;							
					}

					index += pass;
					KeyMap[index] = true;
					SetDiscreteAxisValues(index);
				}
			}
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Unable to poll joystick input: " + std::string(ex.eText), LogLevel::Warning);
		}
	}

	void ReadKeyboard()
	{
		if (!g_Keyboard)
			return;

		try
		{
			g_Keyboard->capture();

			for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
			{
				if (!g_Keyboard->isKeyDown((KeyCode)i))
				{
					KeyMap[i] = false;
					continue;
				}

				KeyMap[i] = true;

				// Register directional discrete keypresses as max analog axis values
				SetDiscreteAxisValues(i);
			}
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Unable to poll keyboard input: " + std::string(ex.eText), LogLevel::Warning);
		}
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

	void SolveInputCollisions(int& lInput)
	{
		// TODO: Make FORWARD+BACK to turn 180 degrees a user option.
		if (lInput & IN_FORWARD && lInput & IN_BACK)
			lInput |= IN_ROLL;

		// Block roll in binocular mode (TODO: Is it needed?)
		if (lInput & IN_ROLL && BinocularRange)
			lInput &= ~IN_ROLL;

		// Block simultaneous LEFT+RIGHT input.
		if (lInput & IN_LEFT && lInput & IN_RIGHT)
			lInput &= ~(IN_LEFT | IN_RIGHT);

		if (Lara.Inventory.IsBusy)
		{
			lInput &= (IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT | IN_OPTION | IN_LOOK | IN_PAUSE);

			if (lInput & IN_FORWARD && lInput & IN_BACK)
				lInput &= ~IN_BACK;
		}
	}

	void HandleLaraHotkeys(int& lInput)
	{
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

		// Handle flares

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

		// Handle look timeout

		static int lookTimeout = 0;

		if (Lara.Control.HandStatus == HandStatus::WeaponReady)
		{
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

		// Handle weapon hotkeys

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

		// Handle medipack hotkeys

		static int medipackTimeout = 0;

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

		// Load / save hotkeys

		if (KeyMap[KC_F5])
			lInput |= IN_SAVE;

		if (KeyMap[KC_F6])
			lInput |= IN_LOAD;
	}

	bool UpdateInput(bool debounce)
	{
		ClearInputData();
		ReadKeyboard();
		ReadJoystick();

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

		if (KeyMap[KC_ESCAPE] || Key(KEY_DRAW))
			lInput |= IN_DESELECT;

		HandleLaraHotkeys(lInput);
		SolveInputCollisions(lInput);

		if (debounce)
			DbInput = InputBusy;

		InputBusy = lInput;
		TrInput = lInput;

		if (debounce)
			DbInput = TrInput & (DbInput ^ TrInput);

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
}
