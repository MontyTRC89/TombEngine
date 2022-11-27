#include "framework.h"
#include "Specific/Input/Input.h"

#include <OISException.h>
#include <OISForceFeedback.h>
#include <OISInputManager.h>
#include <OISJoyStick.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include "Game/camera.h"
#include "Game/Gui.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Game/savegame.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"

using namespace OIS;
using namespace TEN::Gui;
using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

// Big TODO: Entire input system shouldn't be left exposed like this.

namespace TEN::Input
{
	constexpr auto AXIS_DEADZONE = 8000;

	const std::vector<std::string> g_KeyNames =
	{
			"<None>",		"Esc",			"1",			"2",			"3",			"4",			"5",			"6",
			"7",			"8",			"9",			"0",			"-",			"+",			"Back",			"Tab",
			"Q",			"W",			"E",			"R",			"T",			"Y",			"U",			"I",
			"O",			"P",			"<",			">",			"Enter",		"Ctrl",			"A",			"S",
			"D",			"F",			"G",			"H",			"J",			"K",			"L",			";",
			"'",			"`",			"Shift",		"#",			"Z",			"X",			"C",			"V",
			"B",			"N",			"M",			",",			".",			"/",			"Shift",		"Pad X",
			"Alt",			"Space",		"Caps Lock",	"",				"",				"",				"",				"",

			"",				"",				"",				"",				"",				"Num Lock",		"Scroll Lock",	"Pad 7",
			"Pad 8",		"Pad 9",		"Pad -",		"Pad 4",		"Pad 5",		"Pad 6",		"Pad +",		"Pad 1",
			"Pad 2",		"Pad 3",		"Pad 0",		"Pad .",		"",				"",				"\\",			"",
			"",				"",				"",				"",				"",				"",				"",				"",
			"",				"",				"",				"",				"",				"",				"",				"",
			"",				"",				"",				"",				"",				"",				"",				"",
			"",				"",				"",				"",				"",				"",				"",				"",
			"",				"",				"",				"",				"",				"",				"",				"",

			"",				"",				"",				"",				"",				"",				"",				"",
			"",				"",				"",				"",				"",				"",				"",				"",
			"",				"",				"",				"",				"",				"",				"",				"",
			"",				"",				"",				"",				"Pad Enter",	"Ctrl",			"",				"",
			"",				"",				"",				"",				"",				"",				"",				"",
			"",				"",				"Shift",		"",				"",				"",				"",				"",
			"",				"",				"",				"",				"",				"Pad /",		"",				"",
			"Alt",			"",				"",				"",				"",				"",				"",				"",

			"",				"",				"",				"",				"",				"",				"",				"Home",
			"Up",			"Page Up",		"",				"Left",			"",				"Right",		"",				"End",
			"Down",			"Page Down",	"Insert",		"Del",			"",				"",				"",				"",
			"",				"",				"",				"",				"",				"",				"",				"",
			"",				"",				"",				"",				"",				"",				"",				"",
			"",				"",				"",				"",				"",				"",				"",				"",
			"",				"",				"",				"",				"",				"",				"",				"",
			"",				"",				"",				"",				"",				"",				"",				"",

			"Left-Click",	"Right-Click",	"Middle-Click",	"Mouse 4",		"Mouse 5",		"Mouse 6",		"Mouse 7",		"Mouse 8",

			"Joy 1", 		"Joy 2",		"Joy 3",		"Joy 4", 		"Joy 5",		"Joy 6", 		"Joy 7",		"Joy 8",
			"Joy 9",		"Joy 10",		"Joy 11",		"Joy 12",		"Joy 13",		"Joy 14",		"Joy 15",		"Joy 16",

			"Mouse X-",		"Mouse X+",		"Mouse Y-",		"Mouse Y+",

			"X-",			"X+",			"Y-",			"Y+",			"Z-",			"Z+",			"W-",			"W+",
			"Joy LT",		"Joy LT",		"Joy RT",		"Joy RT",		"D-Pad Up",		"D-Pad Down",	"D-Pad Left",	"D-Pad Right"
	};

	// OIS interfaces
	InputManager*  OisInputManager = nullptr;
	Keyboard*	   OisKeyboard	   = nullptr;
	Mouse*		   OisMouse		   = nullptr;
	JoyStick*	   OisGamepad	   = nullptr;
	ForceFeedback* OisRumble	   = nullptr;
	Effect*		   OisEffect	   = nullptr;

	// Globals
	RumbleData				 RumbleInfo = {};
	MouseData				 MouseInfo	= {};
	std::vector<InputAction> ActionMap	= {};
	std::vector<bool>		 KeyMap		= {};
	std::vector<Vector2>	 AxisMap	= {};

	int DbInput = 0;
	int TrInput = 0;

	std::vector<int> DefaultBindings =
	{
		KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT, KC_PERIOD, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_RCONTROL, KC_SPACE, KC_COMMA, KC_NUMPAD0, KC_END, KC_ESCAPE, KC_P, KC_PGUP, KC_PGDOWN,
		/*KC_RCONTROL, KC_DOWN, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_SPACE,*/
		KC_F5, KC_F6, KC_RETURN, KC_ESCAPE, KC_NUMPAD0
	};

	// Input bindings. These are primitive mappings to actions.
	bool ConflictingKeys[KEY_COUNT];
	short KeyboardLayout[2][KEY_COUNT] =
	{
		{
			KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT, KC_PERIOD, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_RCONTROL, KC_SPACE, KC_COMMA, KC_NUMPAD0, KC_END, KC_ESCAPE, KC_P, KC_PGUP, KC_PGDOWN,
			/*KC_RCONTROL, KC_DOWN, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_SPACE*/
		},
		{
			KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT, KC_PERIOD, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_RCONTROL, KC_SPACE, KC_COMMA, KC_NUMPAD0, KC_END, KC_ESCAPE, KC_P, KC_PGUP, KC_PGDOWN,
			/*KC_RCONTROL, KC_DOWN, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_SPACE*/
		}
	};

	void InitialiseEffect()
	{
		OisEffect = new Effect(Effect::ConstantForce, Effect::Constant);
		OisEffect->direction = Effect::North;
		OisEffect->trigger_button = 0;
		OisEffect->trigger_interval = 0;
		OisEffect->replay_length = Effect::OIS_INFINITE;
		OisEffect->replay_delay = 0;
		OisEffect->setNumAxes(1);

		auto& pConstForce = *dynamic_cast<ConstantEffect*>(OisEffect->getForceEffect());
		pConstForce.level = 0;
		pConstForce.envelope.attackLength = 0;
		pConstForce.envelope.attackLevel = 0;
		pConstForce.envelope.fadeLength = 0;
		pConstForce.envelope.fadeLevel = 0;
	}

	void InitialiseInput(HWND handle)
	{
		TENLog("Initializing input system...", LogLevel::Info);

		for (int i = 0; i < (int)ActionID::Count; i++)
			ActionMap.push_back(InputAction((ActionID)i));

		KeyMap.resize(MAX_INPUT_SLOTS);
		AxisMap.resize((int)InputAxis::Count);

		RumbleInfo = {};

		try
		{
			OisInputManager = InputManager::createInputSystem((int)handle);
			OisInputManager->enableAddOnFactory(InputManager::AddOn_All);

			if (OisInputManager->getNumberOfDevices(OISKeyboard) == 0)
				TENLog("Keyboard not found!", LogLevel::Warning);
			else
				OisKeyboard = (Keyboard*)OisInputManager->createInputObject(OISKeyboard, true);

			if (OisInputManager->getNumberOfDevices(OISMouse) == 0)
			{
				TENLog("Mouse not found!", LogLevel::Warning);
			}
			else
			{
				OisMouse = (Mouse*)OisInputManager->createInputObject(OISMouse, true);

				auto& state = OisMouse->getMouseState();

				// TODO: Adapt dynamically to the set resolution.
				state.width = 1920;// g_Gui.GetCurrentSettings().Configuration.Width;
				state.height = 1080;// g_Gui.GetCurrentSettings().Configuration.Height;
			}
		}
		catch (OIS::Exception& ex)
		{
			TENLog("An exception occured during input system init: " + std::string(ex.eText), LogLevel::Error);
		}

		int numDevices = OisInputManager->getNumberOfDevices(OISJoyStick);
		if (numDevices > 0)
		{
			TENLog("Found " + std::to_string(numDevices) + " connected game controller" + ((numDevices > 1) ? "s." : "."), LogLevel::Info);

			try
			{
				OisGamepad = (JoyStick*)OisInputManager->createInputObject(OISJoyStick, true);
				TENLog("Using '" + OisGamepad->vendor() + "' device for input.", LogLevel::Info);

				// Try initializing vibration interface.
				OisRumble = (ForceFeedback*)OisGamepad->queryInterface(Interface::ForceFeedback);
				if (OisRumble != nullptr)
				{
					TENLog("Controller supports vibration.", LogLevel::Info);
					InitialiseEffect();
				}
			}
			catch (OIS::Exception& ex)
			{
				TENLog("An exception occured during game controller init: " + std::string(ex.eText), LogLevel::Error);
			}
		}
	}

	void DeinitialiseInput()
	{
		if (OisKeyboard != nullptr)
			OisInputManager->destroyInputObject(OisKeyboard);

		if (OisMouse != nullptr)
			OisInputManager->destroyInputObject(OisMouse);

		if (OisGamepad != nullptr)
			OisInputManager->destroyInputObject(OisGamepad);

		if (OisEffect != nullptr)
		{
			delete OisEffect;
			OisEffect = nullptr;
		}

		InputManager::destroyInputSystem(OisInputManager);
	}

	void ClearInputData()
	{
		for (auto& key : KeyMap)
			key = false;

		for (auto& axis : AxisMap)
			axis = Vector2::Zero;

		MouseInfo.Absolute = Vector2i::Zero;
		MouseInfo.Relative = Vector2i::Zero;

		DbInput = 0;
		TrInput = 0;
	}

	bool LayoutContainsIndex(int index)
	{
		for (int l = 0; l < 2; l++)
		{
			for (int i = 0; i < KEY_COUNT; i++)
			{
				if (KeyboardLayout[l][i] == index)
					return true;
			}
		}

		return false;
	}

	void DefaultConflict()
	{
		for (int i = 0; i < KEY_COUNT; i++)
		{
			short key = KeyboardLayout[0][i];

			ConflictingKeys[i] = false;

			for (int j = 0; j < KEY_COUNT; j++)
			{
				if (key != KeyboardLayout[1][j])
					continue;

				ConflictingKeys[i] = true;
				break;
			}
		}
	}

	void SetDiscreteAxisValues(int index)
	{
		for (int layout = 0; layout <= 1; layout++)
		{
			if (KeyboardLayout[layout][KEY_FORWARD] == index)
			{
				AxisMap[(int)InputAxis::Move].y = 1.0f;
			}
			else if (KeyboardLayout[layout][KEY_BACK] == index)
			{
				AxisMap[(int)InputAxis::Move].y = -1.0f;
			}
			else if (KeyboardLayout[layout][KEY_LEFT] == index)
			{
				AxisMap[(int)InputAxis::Move].x = -1.0f;
			}
			else if (KeyboardLayout[layout][KEY_RIGHT] == index)
			{
				AxisMap[(int)InputAxis::Move].x = 1.0f;
			}
		}
	}

	void ReadGameController()
	{
		if (OisGamepad == nullptr)
			return;

		try
		{
			OisGamepad->capture();
			const auto& state = OisGamepad->getJoyStickState();

			// Poll buttons.
			for (int key = 0; key < state.mButtons.size(); key++)
				KeyMap[MAX_KEYBOARD_KEYS + MAX_MOUSE_KEYS + key] = state.mButtons[key];

			// Poll axes.
			for (int axis = 0; axis < state.mAxes.size(); axis++)
			{
				// NOTE: We don't support anything above 6 existing XBOX/PS controller axes (two sticks plus two triggers = 6).
				if (axis >= MAX_GAMEPAD_AXES)
					break;

				// Filter out deadzone.
				if (abs(state.mAxes[axis].abs) < AXIS_DEADZONE)
					continue;

				// Calculate raw normalized analog value for camera.
				float axisValue = (state.mAxes[axis].abs > 0) ? -AXIS_DEADZONE : AXIS_DEADZONE;
				float normalizedValue = float(state.mAxes[axis].abs + axisValue) / float(SHRT_MAX - AXIS_DEADZONE);

				// Calculate scaled analog value for movement.
				// NOTE: [0.2f, 1.7f] range gives the most organic rates.
				float scaledValue = abs(normalizedValue) * 1.5f + 0.2f;

				// Calculate and reset discrete input slots.
				int negKeyIndex = MAX_KEYBOARD_KEYS + MAX_MOUSE_KEYS + MAX_GAMEPAD_KEYS + (axis * 2);
				int posKeyIndex = MAX_KEYBOARD_KEYS + MAX_MOUSE_KEYS + MAX_GAMEPAD_KEYS + (axis * 2) + 1;
				KeyMap[negKeyIndex] = false;
				KeyMap[posKeyIndex] = false;

				// Decide on the discrete input registering based on analog value.
				int usedIndex = normalizedValue > 0 ? negKeyIndex : posKeyIndex;
				KeyMap[usedIndex] = true;

				// Register analog input in certain direction.
				// If axis is bound as directional controls, register axis as directional input.
				// Otherwise, register as camera movement input (for future).
				// NOTE: abs() operations are needed to avoid issues with inverted axes on different controllers.

				if (KeyboardLayout[1][KEY_FORWARD] == usedIndex)
				{
					AxisMap[(int)InputAxis::Move].y = abs(scaledValue);
				}
				else if (KeyboardLayout[1][KEY_BACK] == usedIndex)
				{
					AxisMap[(int)InputAxis::Move].y = -abs(scaledValue);
				}
				else if (KeyboardLayout[1][KEY_LEFT] == usedIndex)
				{
					AxisMap[(int)InputAxis::Move].x = -abs(scaledValue);
				}
				else if (KeyboardLayout[1][KEY_RIGHT] == usedIndex)
				{
					AxisMap[(int)InputAxis::Move].x = abs(scaledValue);
				}
				else if (!LayoutContainsIndex(usedIndex))
				{
					if ((axis % 2) == 0)
						AxisMap[(int)InputAxis::Camera].y = normalizedValue;
					else
						AxisMap[(int)InputAxis::Camera].x = normalizedValue;
				}
			}

			// Poll POVs.
			// NOTE: Controllers usually have one, but scan all just in case.
			for (int pov = 0; pov < 4; pov++)
			{
				if (state.mPOV[pov].direction == Pov::Centered)
					continue;

				// Register multiple directional keypresses mapped to analog axes.
				int index = MAX_KEYBOARD_KEYS + MAX_MOUSE_KEYS + MAX_GAMEPAD_KEYS + MAX_MOUSE_POV_AXES + (MAX_GAMEPAD_AXES * 2);
				for (int pass = 0; pass < 4; pass++)
				{
					switch (pass)
					{
					case 0:
						if ((state.mPOV[pov].direction & Pov::North) == 0)
							continue;
						break;

					case 1:
						if ((state.mPOV[pov].direction & Pov::South) == 0)
							continue;
						break;

					case 2:
						if ((state.mPOV[pov].direction & Pov::West) == 0)
							continue;
						break;

					case 3:
						if ((state.mPOV[pov].direction & Pov::East) == 0)
							continue;
						break;
					}

					KeyMap[index + pass] = true;
					SetDiscreteAxisValues(index + pass);
				}
			}
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Unable to poll game controller input: " + std::string(ex.eText), LogLevel::Warning);
		}
	}

	void ReadKeyboard()
	{
		if (OisKeyboard == nullptr)
			return;

		try
		{
			OisKeyboard->capture();

			// Poll keys.
			for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
			{
				if (OisKeyboard->isKeyDown((KeyCode)i))
				{
					KeyMap[i] = true;
					SetDiscreteAxisValues(i); // Interpret discrete directional keypresses as max analog axis values.
					continue;
				}

				KeyMap[i] = false;
			}
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Unable to poll keyboard input: " + std::string(ex.eText), LogLevel::Warning);
		}
	}

	void ReadMouse()
	{
		if (OisMouse == nullptr)
			return;

		try
		{
			OisMouse->capture();
			const auto& state = OisMouse->getMouseState();

			// Poll buttons.
			for (int i = 0; i < MAX_MOUSE_KEYS; i++)
				KeyMap[MAX_KEYBOARD_KEYS + i] = state.buttonDown((MouseButtonID)i);

			// Register multiple directional keypresses mapped to mouse axes.
			int index = MAX_KEYBOARD_KEYS + MAX_MOUSE_KEYS + MAX_GAMEPAD_KEYS;
			for (int pass = 0; pass < 4; pass++)
			{
				switch (pass)
				{
				// Mouse X-
				case 0:
					if (state.X.rel >= 0)
						continue;
					break;

				// Mouse X+
				case 1:
					if (state.X.rel <= 0)
						continue;
					break;

				// Mouse Y-
				case 2:
					if (state.Y.rel >= 0)
						continue;
					break;

				// Mouse Y+
				case 3:
					if (state.Y.rel <= 0)
						continue;
					break;
				}

				KeyMap[index + pass] = true;
				SetDiscreteAxisValues(index + pass);
			}

			// Poll axes.
			MouseInfo.Absolute.x = state.X.abs;
			MouseInfo.Absolute.y = state.Y.abs;
			MouseInfo.Relative.x = state.X.rel;
			MouseInfo.Relative.y = state.Y.rel;
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Unable to poll mouse input: " + std::string(ex.eText), LogLevel::Warning);
		}
	}

	bool Key(int number)
	{
		for (int layout = 1; layout >= 0; layout--)
		{
			short key = KeyboardLayout[layout][number];

			if (KeyMap[key])
				return true;

			// Mirror Ctrl, Shift, and Alt.
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
		}

		return false;
	}

	void SolveActionCollisions()
	{
		// Block simultaneous LEFT+RIGHT actions.
		if (IsHeld(In::Left) && IsHeld(In::Right))
		{
			ClearAction(In::Left);
			ClearAction(In::Right);
		}
	}

	void HandlePlayerHotkeys(ItemInfo* item)
	{
		static const vector<int> unavailableFlareStates =
		{
			LS_CRAWL_FORWARD,
			LS_CRAWL_TURN_LEFT,
			LS_CRAWL_TURN_RIGHT,
			LS_CRAWL_BACK,
			LS_CRAWL_TO_HANG,
			LS_CRAWL_TURN_180
		};

		auto& lara = *GetLaraInfo(item);

		// Handle hardcoded action-to-key mappings.
		ActionMap[(int)In::Save].Update(KeyMap[KC_F5] ? true : false);
		ActionMap[(int)In::Load].Update(KeyMap[KC_F6] ? true : false);
		ActionMap[(int)In::Select].Update((KeyMap[KC_RETURN] || Key(KEY_ACTION)) ? true : false);
		ActionMap[(int)In::Deselect].Update((KeyMap[KC_ESCAPE] || Key(KEY_DRAW)) ? true : false);

		// Handle target switch when locked on to an entity.
		if (lara.Control.HandStatus == HandStatus::WeaponReady &&
			lara.TargetEntity != nullptr)
		{
			if (IsClicked(In::Look))
			{
				ActionMap[(int)In::SwitchTarget].Update(true);
				//ActionMap[(int)In::Look].Clear();
			}
			else
				ClearAction(In::SwitchTarget);
		}
		else
			ClearAction(In::SwitchTarget);

		// Handle flares.
		if (IsClicked(In::Flare))
		{
			if (TestState(item->Animation.ActiveState, unavailableFlareStates))
				SoundEffect(SFX_TR4_LARA_NO_ENGLISH, nullptr, SoundEnvironment::Always);
		}

		// Handle weapon hotkeys.
		if (KeyMap[KC_1] && lara.Weapons[(int)LaraWeaponType::Pistol].Present)
			lara.Control.Weapon.RequestGunType = LaraWeaponType::Pistol;

		if (KeyMap[KC_2] && lara.Weapons[(int)LaraWeaponType::Shotgun].Present)
			lara.Control.Weapon.RequestGunType = LaraWeaponType::Shotgun;

		if (KeyMap[KC_3] && lara.Weapons[(int)LaraWeaponType::Uzi].Present)
			lara.Control.Weapon.RequestGunType = LaraWeaponType::Uzi;

		if (KeyMap[KC_4] && lara.Weapons[(int)LaraWeaponType::Revolver].Present)
			lara.Control.Weapon.RequestGunType = LaraWeaponType::Revolver;

		if (KeyMap[KC_5] && lara.Weapons[(int)LaraWeaponType::GrenadeLauncher].Present)
			lara.Control.Weapon.RequestGunType = LaraWeaponType::GrenadeLauncher;

		if (KeyMap[KC_6] && lara.Weapons[(int)LaraWeaponType::Crossbow].Present)
			lara.Control.Weapon.RequestGunType = LaraWeaponType::Crossbow;

		if (KeyMap[KC_7] && lara.Weapons[(int)LaraWeaponType::HarpoonGun].Present)
			lara.Control.Weapon.RequestGunType = LaraWeaponType::HarpoonGun;

		if (KeyMap[KC_8] && lara.Weapons[(int)LaraWeaponType::HK].Present)
			lara.Control.Weapon.RequestGunType = LaraWeaponType::HK;

		if (KeyMap[KC_9] && lara.Weapons[(int)LaraWeaponType::RocketLauncher].Present)
			lara.Control.Weapon.RequestGunType = LaraWeaponType::RocketLauncher;

		// Handle medipack hotkeys.
		static bool dbMedipack = true;
		if ((KeyMap[KC_MINUS] || KeyMap[KC_EQUALS]) && dbMedipack)
		{
			if ((item->HitPoints > 0 && item->HitPoints < LARA_HEALTH_MAX) ||
				lara.PoisonPotency)
			{
				bool hasUsedMedipack = false;

				if (KeyMap[KC_MINUS] &&
					lara.Inventory.TotalSmallMedipacks != 0)
				{
					hasUsedMedipack = true;

					item->HitPoints += LARA_HEALTH_MAX / 2;
					if (item->HitPoints > LARA_HEALTH_MAX)
						item->HitPoints = LARA_HEALTH_MAX;

					if (lara.Inventory.TotalSmallMedipacks != -1)
						lara.Inventory.TotalSmallMedipacks--;
				}
				else if (KeyMap[KC_EQUALS] &&
					lara.Inventory.TotalLargeMedipacks != 0)
				{
					hasUsedMedipack = true;
					item->HitPoints = LARA_HEALTH_MAX;

					if (lara.Inventory.TotalLargeMedipacks != -1)
						lara.Inventory.TotalLargeMedipacks--;
				}

				if (hasUsedMedipack)
				{
					lara.PoisonPotency = 0;
					SoundEffect(SFX_TR4_MENU_MEDI, nullptr, SoundEnvironment::Always);
					Statistics.Game.HealthUsed++;
				}
			}
		}
		dbMedipack = (KeyMap[KC_MINUS] || KeyMap[KC_EQUALS]) ? false : true;

		// Handle saying "no".
		static bool dbNo = true;
		if (KeyMap[KC_N] && dbNo)
			SayNo();
		dbNo = KeyMap[KC_N] ? false : true;

		// Toggle fullscreen.
		static bool dbFullscreen = true;
		if ((KeyMap[KC_LMENU] || KeyMap[KC_RMENU]) && KeyMap[KC_RETURN] && dbFullscreen)
		{
			g_Configuration.Windowed = !g_Configuration.Windowed;
			SaveConfiguration();
			g_Renderer.ToggleFullScreen();
		}
		dbFullscreen = ((KeyMap[KC_LMENU] || KeyMap[KC_RMENU]) && KeyMap[KC_RETURN]) ? false : true;

		// Handle debug page switch.
		static bool dbDebugPage = true;
		if ((KeyMap[KC_F10] || KeyMap[KC_F11]) && dbDebugPage)
			g_Renderer.SwitchDebugPage(KeyMap[KC_F10]);
		dbDebugPage = (KeyMap[KC_F10] || KeyMap[KC_F11]) ? false : true;
	}

	void UpdateRumble()
	{
		if (!OisRumble || !OisEffect || !RumbleInfo.Power)
			return;

		RumbleInfo.Power -= RumbleInfo.FadeSpeed;

		// Don't update effect too frequently if its value hasn't changed much.
		if (RumbleInfo.Power >= 0.2f &&
			RumbleInfo.LastPower - RumbleInfo.Power < 0.1f)
			return;

		if (RumbleInfo.Power <= 0.0f)
		{
			StopRumble();
			return;
		}

		try
		{
			auto& force = *dynamic_cast<ConstantEffect*>(OisEffect->getForceEffect());
			force.level = RumbleInfo.Power * 10000;

			switch (RumbleInfo.Mode)
			{
			case RumbleMode::Left:
				OisEffect->direction = Effect::EDirection::West;
				break;

			case RumbleMode::Right:
				OisEffect->direction = Effect::EDirection::East;
				break;

			case RumbleMode::Both:
				OisEffect->direction = Effect::EDirection::North;
				break;
			}

			OisRumble->upload(OisEffect);
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Error updating vibration effect: " + std::string(ex.eText), LogLevel::Error);
		}

		RumbleInfo.LastPower = RumbleInfo.Power;
	}

	void UpdateInputActions(ItemInfo* item)
	{
		ClearInputData();
		UpdateRumble();
		ReadKeyboard();
		ReadMouse();
		ReadGameController();

		// Update action map (mappable actions only).
		for (int i = 0; i < KEY_COUNT; i++)
			ActionMap[i].Update(Key(i) ? true : false); // TODO: Poll analog value of key. Potentially, any can be a trigger.

		// Additional handling.
		HandlePlayerHotkeys(item);
		SolveActionCollisions();

		// Port actions back to legacy bit fields.
		for (const auto& action : ActionMap)
		{
			int actionBit = 1 << (int)action.GetID();

			DbInput |= action.IsClicked() ? actionBit : 0;
			TrInput |= action.IsHeld()	  ? actionBit : 0;
		}

		// TEMP: Mouse debug.
		g_Renderer.PrintDebugMessage("Width: %d", OisMouse->getMouseState().width);
		g_Renderer.PrintDebugMessage("Height: %d", OisMouse->getMouseState().height);
		g_Renderer.PrintDebugMessage("Mouse Abs. X: %d", MouseInfo.Absolute.x);
		g_Renderer.PrintDebugMessage("Mouse Abs. Y: %d", MouseInfo.Absolute.y);
		g_Renderer.PrintDebugMessage("Mouse Rel. X: %d", MouseInfo.Relative.x);
		g_Renderer.PrintDebugMessage("Mouse Rel. Y: %d", MouseInfo.Relative.y);
	}

	void ClearAllActions()
	{
		for (auto& action : ActionMap)
			action.Clear();

		DbInput = 0;
		TrInput = 0;
	}

	void Rumble(float power, float delayInSec, RumbleMode mode)
	{
		if (!g_Configuration.EnableRumble)
			return;

		power = std::clamp(power, 0.0f, 1.0f);

		if (power == 0.0f || RumbleInfo.Power)
			return;

		RumbleInfo.FadeSpeed = power / (delayInSec * FPS);
		RumbleInfo.Power = power + RumbleInfo.FadeSpeed;
		RumbleInfo.LastPower = RumbleInfo.Power;
	}

	void StopRumble()
	{
		if (!OisRumble || !OisEffect)
			return;

		try
		{
			OisRumble->remove(OisEffect);
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Error when stopping vibration effect: " + std::string(ex.eText), LogLevel::Error);
		}

		RumbleInfo = {};
	}

	void ClearAction(ActionID actionID)
	{
		ActionMap[(int)actionID].Clear();

		int actionBit = 1 << (int)actionID;
		DbInput &= ~actionBit;
		TrInput &= ~actionBit;
	}

	bool NoAction()
	{
		for (const auto& action : ActionMap)
		{
			if (action.IsHeld())
				return false;
		}

		return true;
	}

	bool IsClicked(ActionID actionID)
	{
		return ActionMap[(int)actionID].IsClicked();
	}

	bool IsHeld(ActionID actionID, float delayInSec)
	{
		return ActionMap[(int)actionID].IsHeld(delayInSec);
	}

	bool IsPulsed(ActionID actionID, float delayInSec, float initialDelayInSec)
	{
		return ActionMap[(int)actionID].IsPulsed(delayInSec, initialDelayInSec);
	}

	bool IsReleased(ActionID actionID, float maxDelayInSec)
	{
		return ActionMap[(int)actionID].IsReleased(maxDelayInSec);
	}

	float GetActionValue(ActionID actionID)
	{
		return ActionMap[(int)actionID].GetValue();
	}

	float GetActionTimeActive(ActionID actionID)
	{
		return ActionMap[(int)actionID].GetTimeActive();
	}

	float GetActionTimeInactive(ActionID actionID)
	{
		return ActionMap[(int)actionID].GetTimeInactive();
	}

	bool IsDirectionActionHeld()
	{
		return (IsHeld(In::Forward) || IsHeld(In::Back) || IsHeld(In::Left) || IsHeld(In::Right));
	}

	bool IsWakeActionHeld()
	{
		if (IsDirectionActionHeld() || IsHeld(In::LeftStep) || IsHeld(In::RightStep) ||
			IsHeld(In::Walk) || IsHeld(In::Jump) || IsHeld(In::Sprint) || IsHeld(In::Roll) || IsHeld(In::Crouch) ||
			IsHeld(In::DrawWeapon) || IsHeld(In::Flare) || IsHeld(In::Action))
		{
			return true;
		}

		return false;
	}

	bool IsOpticActionHeld()
	{
		return (IsDirectionActionHeld() || IsHeld(In::Action) || IsHeld(In::Crouch) || IsHeld(In::Sprint));
	}
}
