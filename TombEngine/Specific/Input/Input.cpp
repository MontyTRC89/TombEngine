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
#include "Game/savegame.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Specific/clock.h"
#include "Specific/trutils.h"
#include "Specific/winmain.h"

using namespace OIS;
using namespace TEN::Gui;
using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

// Big TODO: Entire input system shouldn't be left exposed like this.

namespace TEN::Input
{
	constexpr auto AXIS_SCALE			 = 1.5f;
	constexpr auto AXIS_DEADZONE		 = 8000;
	constexpr auto AXIS_OFFSET			 = 0.2f;
	constexpr auto MOUSE_AXIS_CONSTRAINT = 100.0f;

	const std::vector<std::string> g_KeyNames =
	{
			"<None>",		"Esc",			"1",			"2",			"3",			"4",			"5",			"6",
			"7",			"8",			"9",			"0",			"-",			"+",			"Back",			"Tab",
			"Q",			"W",			"E",			"R",			"T",			"Y",			"U",			"I",
			"O",			"P",			"<",			">",			"Enter",		"Ctrl",			"A",			"S",
			"D",			"F",			"G",			"H",			"J",			"K",			"L",			";",
			"'",			"`",			"Shift",		"#",			"Z",			"X",			"C",			"V",
			"B",			"N",			"M",			",",			".",			"/",			"Shift",		"Pad X",
			"Alt",			"Space",		"Caps Lock",	"F1",			"F2",			"F3",			"F4",			"F5",

			"F6",			"F7",			"F8",			"F9",			"F10",			"Num Lock",		"Scroll Lock",	"Pad 7",
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

			"Mouse X-",		"Mouse X+",		"Mouse Y-",		"Mouse Y+",		"Mouse Z-",		"Mouse Z+",

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
	RumbleData				 RumbleInfo  = {};
	std::vector<InputAction> ActionMap	 = {};
	std::vector<QueueState>	 ActionQueue = {};
	std::vector<bool>		 KeyMap		 = {};
	std::vector<Vector2>	 AxisMap	 = {};

	int DbInput = 0;
	int TrInput = 0;

	// Rows:
	// 1. General actions
	// 2. Vehicle actions (TODO)
	// 3. Quick actions
	// 4. Menu controls
	const auto DefaultBindings = std::vector<int>
	{
		KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT, KC_PGUP, KC_PGDOWN, KC_RCONTROL, KC_RMENU, KC_RSHIFT, KC_SLASH, KC_PERIOD, KC_END, KC_SPACE, KC_NUMPAD0,
		/*KC_RCONTROL, KC_DOWN, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_SPACE,*/
		KC_COMMA, KC_MINUS, KC_EQUALS, KC_LBRACKET, KC_RBRACKET, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_N,
		KC_RETURN, KC_ESCAPE, KC_P, KC_ESCAPE, KC_F5, KC_F6, KC_NUMPAD0
	};
	const auto XInputBindings = std::vector<int>
	{
		XB_AXIS_X_NEG, XB_AXIS_X_POS, XB_AXIS_Y_NEG, XB_AXIS_Y_POS, XB_LSTICK, XB_RSTICK, XB_A, XB_X, XB_RSHIFT, XB_AXIS_RTRIGGER_NEG, XB_AXIS_LTRIGGER_NEG, XB_B, XB_Y, XB_LSHIFT,
		/*KC_RCONTROL, KC_DOWN, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_SPACE,*/
		XB_DPAD_DOWN, KC_MINUS, KC_EQUALS, KC_LBRACKET, KC_RBRACKET, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_N,
		KC_RETURN, XB_SELECT, XB_START, XB_SELECT, KC_F5, KC_F6, KC_NUMPAD0
	};

	// Input bindings. These are primitive mappings to actions.
	bool ConflictingKeys[KEY_COUNT];
	short KeyboardLayout[2][KEY_COUNT] =
	{
		{
			KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT, KC_PGUP, KC_PGDOWN, KC_RCONTROL, KC_RMENU, KC_RSHIFT, KC_SLASH, KC_PERIOD, KC_END, KC_SPACE, KC_NUMPAD0,
			/*KC_RCONTROL, KC_DOWN, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_SPACE,*/
			KC_COMMA, KC_MINUS, KC_EQUALS, KC_LBRACKET, KC_RBRACKET, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_N,
			KC_RETURN, KC_ESCAPE, KC_P, KC_ESCAPE, KC_F5, KC_F6
		},
		{
			KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT, KC_PGUP, KC_PGDOWN, KC_RCONTROL, KC_RMENU, KC_RSHIFT, KC_SLASH, KC_PERIOD, KC_END, KC_SPACE, KC_NUMPAD0,
			/*KC_RCONTROL, KC_DOWN, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_SPACE,*/
			KC_COMMA, KC_MINUS, KC_EQUALS, KC_LBRACKET, KC_RBRACKET, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_N,
			KC_RETURN, KC_ESCAPE, KC_P, KC_ESCAPE, KC_F5, KC_F6
		}
	};

	void InitializeEffect()
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

	void InitializeInput(HWND handle)
	{
		TENLog("Initializing input system...", LogLevel::Info);

		// Initialize maps.
		for (int i = 0; i < (int)ActionID::Count; i++)
		{
			ActionMap.push_back(InputAction((ActionID)i));
			ActionQueue.push_back(QueueState::None);
		}

		KeyMap.resize(MAX_INPUT_SLOTS);
		AxisMap.resize((int)InputAxis::Count);

		RumbleInfo = {};

		try
		{
			OisInputManager = InputManager::createInputSystem((size_t)handle);
			OisInputManager->enableAddOnFactory(InputManager::AddOn_All);

			if (OisInputManager->getNumberOfDevices(OISKeyboard) == 0)
			{
				TENLog("Keyboard not found!", LogLevel::Warning);
			}
			else
			{
				OisKeyboard = (Keyboard*)OisInputManager->createInputObject(OISKeyboard, true);
			}

			if (OisInputManager->getNumberOfDevices(OISMouse) == 0)
			{
				TENLog("Mouse not found!", LogLevel::Warning);
			}
			else
			{
				OisMouse = (Mouse*)OisInputManager->createInputObject(OISMouse, true);
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

				// Try to initialize vibration interface.
				OisRumble = (ForceFeedback*)OisGamepad->queryInterface(Interface::ForceFeedback);
				if (OisRumble != nullptr)
				{
					TENLog("Controller supports vibration.", LogLevel::Info);
					InitializeEffect();
				}

				// If controller is XInput and default bindings were successfully assigned, save configuration.
				if (ApplyDefaultXInputBindings())
				{
					g_Configuration.EnableRumble = (OisRumble != nullptr);
					g_Configuration.EnableThumbstickCameraControl = true;
					SaveConfiguration();
				}
			}
			catch (OIS::Exception& ex)
			{
				TENLog("An exception occured during game controller init: " + std::string(ex.eText), LogLevel::Error);
			}
		}
	}

	void DeinitializeInput()
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

		DbInput = 0;
		TrInput = 0;
	}

	void ApplyActionQueue()
	{
		for (int i = 0; i < KEY_COUNT; i++)
		{
			switch (ActionQueue[i])
			{
			default:
			case QueueState::None:
				break;

			case QueueState::Push:
				ActionMap[i].Update(true);
				break;

			case QueueState::Clear:
				ActionMap[i].Clear();
				break;
			}
		}
	}

	void ClearActionQueue()
	{
		for (auto& queue : ActionQueue)
			queue = QueueState::None;
	}

	bool LayoutContainsIndex(int index)
	{
		for (int layout = 1; layout >= 0; layout--)
		{
			for (int i = 0; i < KEY_COUNT; i++)
			{
				if (KeyboardLayout[layout][i] == index)
					return true;
			}
		}

		return false;
	}

	int WrapSimilarKeys(int source)
	{
		// Merge right/left Ctrl, Shift, Alt.

		switch (source)
		{
		case KC_LCONTROL:
			return KC_RCONTROL;

		case KC_LSHIFT:
			return KC_RSHIFT;

		case KC_LMENU:
			return KC_RMENU;
		}

		return source;
	}

	void DefaultConflict()
	{
		for (int i = 0; i < KEY_COUNT; i++)
		{
			int key = KeyboardLayout[0][i];

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

					// Interpret discrete directional keypresses as analog axis values.
					SetDiscreteAxisValues(i);
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
			int baseIndex = MAX_KEYBOARD_KEYS + MAX_MOUSE_KEYS + MAX_GAMEPAD_KEYS;
			for (int pass = 0; pass < MAX_MOUSE_POV_AXES; pass++)
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

				// Mouse Z-
				case 4:
					if (state.Z.rel >= 0)
						continue;
					break;

				// Mouse Z+
				case 5:
					if (state.Z.rel <= 0)
						continue;
					break;
				}

				KeyMap[baseIndex + pass] = true;

				// Interpret discrete directional keypresses as mouse axis values.
				SetDiscreteAxisValues(baseIndex + pass);
			}

			// Poll axes.
			auto rawAxes = Vector2(state.X.rel, state.Y.rel);
			float sensitivity = (g_Configuration.MouseSensitivity * 0.1f) + 0.4f;
			float smoothing = 1.0f - (g_Configuration.MouseSmoothing * 0.1f);
			AxisMap[(int)InputAxis::Mouse] = (rawAxes * sensitivity) * smoothing;
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Unable to poll mouse input: " + std::string(ex.eText), LogLevel::Warning);
		}
	}
	
	void ReadGameController()
	{
		return;
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
				float normalizedValue = (state.mAxes[axis].abs + axisValue) / (SHRT_MAX - AXIS_DEADZONE);

				// Calculate scaled analog value for movement.
				// NOTE: [0.2f, 1.7f] range gives most organic rates.
				float scaledValue = (abs(normalizedValue) * AXIS_SCALE) + AXIS_OFFSET;

				// Calculate and reset discrete input slots.
				int negKeyIndex = MAX_KEYBOARD_KEYS + MAX_MOUSE_KEYS + MAX_GAMEPAD_KEYS + (axis * 2);
				int posKeyIndex = MAX_KEYBOARD_KEYS + MAX_MOUSE_KEYS + MAX_GAMEPAD_KEYS + (axis * 2) + 1;
				KeyMap[negKeyIndex] = false;
				KeyMap[posKeyIndex] = false;

				// Determine discrete input registering based on analog value.
				int usedIndex = (normalizedValue > 0) ? negKeyIndex : posKeyIndex;
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
					{
						AxisMap[(int)InputAxis::Camera].y = normalizedValue;
					}
					else
					{
						AxisMap[(int)InputAxis::Camera].x = normalizedValue;
					}
				}
			}

			// Poll POVs.
			// NOTE: Controllers usually have one, but scan all just in case.
			for (int pov = 0; pov < MAX_GAMEPAD_POV_AXES; pov++)
			{
				if (state.mPOV[pov].direction == Pov::Centered)
					continue;

				// Register multiple directional keypresses mapped to analog axes.
				int baseIndex = MAX_KEYBOARD_KEYS + MAX_MOUSE_KEYS + MAX_GAMEPAD_KEYS + MAX_MOUSE_POV_AXES + (MAX_GAMEPAD_AXES * 2);
				for (int pass = 0; pass < MAX_GAMEPAD_POV_AXES; pass++)
				{
					switch (pass)
					{
					// D-Pad Up
					case 0:
						if ((state.mPOV[pov].direction & Pov::North) == 0)
							continue;
						break;

					// D-Pad Down
					case 1:
						if ((state.mPOV[pov].direction & Pov::South) == 0)
							continue;
						break;

					// D-Pad Left
					case 2:
						if ((state.mPOV[pov].direction & Pov::West) == 0)
							continue;
						break;

					// D-Pad Right
					case 3:
						if ((state.mPOV[pov].direction & Pov::East) == 0)
							continue;
						break;
					}

					KeyMap[baseIndex + pass] = true;
					SetDiscreteAxisValues(baseIndex + pass);
				}
			}
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Unable to poll game controller input: " + std::string(ex.eText), LogLevel::Warning);
		}
	}

	bool Key(int number)
	{
		for (int layout = 1; layout >= 0; layout--)
		{
			int key = KeyboardLayout[layout][number];
			
			if (layout == 0 && ConflictingKeys[number])
				continue;

			if (KeyMap[key])
				return true;
		}

		return false;
	}

	void SolveActionCollisions()
	{
		// Block simultaneous Left+Right actions.
		if (IsHeld(In::Left) && IsHeld(In::Right))
		{
			ClearAction(In::Left);
			ClearAction(In::Right);
		}
	}

	static void HandleHotkeyActions()
	{
		// Save screenshot.
		static bool dbScreenshot = true;
		if (KeyMap[KC_SYSRQ] && dbScreenshot)
			g_Renderer.SaveScreenshot();
		dbScreenshot = !KeyMap[KC_SYSRQ];

		// Toggle fullscreen.
		static bool dbFullscreen = true;
		if ((KeyMap[KC_LMENU] || KeyMap[KC_RMENU]) && KeyMap[KC_RETURN] && dbFullscreen)
		{
			g_Configuration.Windowed = !g_Configuration.Windowed;
			SaveConfiguration();
			g_Renderer.ToggleFullScreen();
		}
		dbFullscreen = !((KeyMap[KC_LMENU] || KeyMap[KC_RMENU]) && KeyMap[KC_RETURN]);

		if (!DebugMode)
			return;

		// Switch debug page.
		static bool dbDebugPage = true;
		if ((KeyMap[KC_F10] || KeyMap[KC_F11]) && dbDebugPage)
			g_Renderer.SwitchDebugPage(KeyMap[KC_F10]);
		dbDebugPage = !(KeyMap[KC_F10] || KeyMap[KC_F11]);
	}

	static void UpdateRumble()
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

	void UpdateInputActions(ItemInfo* item, bool applyQueue)
	{
		ClearInputData();
		UpdateRumble();
		ReadKeyboard();
		ReadMouse();
		ReadGameController();
		DefaultConflict();

		// Update action map (mappable actions only).
		for (int i = 0; i < KEY_COUNT; i++)
		{
			// TODO: Poll analog value of key. Potentially, any can be a trigger.
			ActionMap[i].Update(Key(i) ? true : false);
		}

		if (applyQueue)
			ApplyActionQueue();

		// Additional handling.
		HandleHotkeyActions();
		SolveActionCollisions();

		// Port actions back to legacy bit fields.
		for (const auto& action : ActionMap)
		{
			int actionBit = 1 << (int)action.GetID();

			DbInput |= action.IsClicked() ? actionBit : 0;
			TrInput |= action.IsHeld()	  ? actionBit : 0;
		}

		// TEMP: Mouse debug.
		g_Renderer.PrintDebugMessage("Mouse X: %.3f", AxisMap[(int)InputAxis::Mouse].x);
		g_Renderer.PrintDebugMessage("Mouse Y: %.3f", AxisMap[(int)InputAxis::Mouse].y);
	}

	void ClearAllActions()
	{
		for (auto& action : ActionMap)
			action.Clear();

		for (auto& queue : ActionQueue)
			queue = QueueState::None;

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

	static void ApplyBindings(const std::vector<int>& bindings)
	{
		for (int i = 0; i < bindings.size(); i++)
		{
			if (i >= KEY_COUNT)
				break;

			KeyboardLayout[1][i] = bindings[i];
		}
	}

	void ApplyDefaultBindings()
	{
		ApplyBindings(DefaultBindings);
		ApplyDefaultXInputBindings();
	}

	bool ApplyDefaultXInputBindings()
	{
		if (!OisGamepad)
			return false;

		for (int i = 0; i < KEY_COUNT; i++)
		{
			if (KeyboardLayout[1][i] != KC_UNASSIGNED && KeyboardLayout[1][i] != KeyboardLayout[0][i])
				return false;
		}

		auto vendor = TEN::Utils::ToLower(OisGamepad->vendor());

		if (vendor.find("xbox") != std::string::npos || vendor.find("xinput") != std::string::npos)
		{
			ApplyBindings(XInputBindings);

			for (int i = 0; i < KEY_COUNT; i++)
				g_Configuration.KeyboardLayout[i] = KeyboardLayout[1][i];

			// Additionally turn on thumbstick camera and vibration.
			g_Configuration.EnableRumble = g_Configuration.EnableThumbstickCameraControl = true;

			return true;
		}
		else
		{
			return false;
		}
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
		if (IsDirectionActionHeld() || IsHeld(In::StepLeft) || IsHeld(In::StepRight) ||
			IsHeld(In::Walk) || IsHeld(In::Jump) || IsHeld(In::Sprint) || IsHeld(In::Roll) || IsHeld(In::Crouch) ||
			IsHeld(In::Draw) || IsHeld(In::Flare) || IsHeld(In::Action))
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
