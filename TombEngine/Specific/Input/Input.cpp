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
#include "Renderer/Renderer.h"
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

	const std::vector<std::string> g_KeyNames =
	{
			"<None>",		"Esc",			"1",			"2",			"3",			"4",			"5",			"6",
			"7",			"8",			"9",			"0",			"-",			"+",			"Back",			"Tab",
			"Q",			"W",			"E",			"R",			"T",			"Y",			"U",			"I",
			"O",			"P",			"[",			"]",			"Enter",		"Ctrl",			"A",			"S",
			"D",			"F",			"G",			"H",			"J",			"K",			"L",			";",
			"'",			"`",			"Shift",		"#",			"Z",			"X",			"C",			"V",
			"B",			"N",			"M",			",",			".",			"/",			"Shift",		"Pad X",
			"Alt",			"Space",		"Caps Lock",	"F1",			"F2",			"F3",			"F4",			"F5",

			"F6",			"F7",			"F8",			"F9",			"F10",			"Num Lock",		"Scroll Lock",	"Pad 7",
			"Pad 8",		"Pad 9",		"Pad -",		"Pad 4",		"Pad 5",		"Pad 6",		"Pad +",		"Pad 1",
			"Pad 2",		"Pad 3",		"Pad 0",		"Pad .",		"",				"",				"\\",			"F11",
			"F12",			"",				"",				"",				"",				"",				"",				"",
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

			"Joy 1", 		"Joy 2",		"Joy 3",		"Joy 4", 		"Joy 5",		"Joy 6", 		"Joy 7",		"Joy 8",
			"Joy 9",		"Joy 10",		"Joy 11",		"Joy 12",		"Joy 13",		"Joy 14",		"Joy 15",		"Joy 16",

			"X-",			"X+",			"Y-",			"Y+",			"Z-",			"Z+",			"W-",			"W+",
			"Joy LT",		"Joy LT",		"Joy RT",		"Joy RT",		"D-Pad Up",		"D-Pad Down",	"D-Pad Left",	"D-Pad Right",

			"Left-Click",	"Right-Click",	"Middle-Click",	"Mouse 4",		"Mouse 5",		"Mouse 6",		"Mouse 7",		"Mouse 8",
			"Mouse X-",		"Mouse X+",		"Mouse Y-",		"Mouse Y+",		"Mouse Z-",		"Mouse Z+"
	};

	// Binding rows:
	// 1. General actions
	// 2. Vehicle actions
	// 3. Quick actions
	// 4. Menu actions

	const auto DefaultGenericBindings = std::vector<int>
	{
		KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT, KC_DELETE, KC_PGDOWN, KC_RSHIFT, KC_SLASH, KC_PERIOD, KC_RMENU, KC_END, KC_RCONTROL, KC_SPACE, KC_NUMPAD0,
		KC_RCONTROL, KC_DOWN, KC_SLASH, KC_RSHIFT, KC_RMENU, KC_SPACE,
		KC_COMMA, KC_MINUS, KC_EQUALS, KC_LBRACKET, KC_RBRACKET, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0,
		KC_RETURN, KC_ESCAPE, KC_P, KC_ESCAPE, KC_F5, KC_F6
	};
	const auto DefaultXInputBindings = std::vector<int>
	{
		XB_AXIS_X_NEG, XB_AXIS_X_POS, XB_AXIS_Y_NEG, XB_AXIS_Y_POS, XB_LSTICK, XB_RSTICK, XB_RSHIFT, XB_AXIS_RTRIGGER_NEG, XB_AXIS_LTRIGGER_NEG, XB_X, XB_B, XB_A, XB_Y, XB_LSHIFT,
		XB_A, XB_B, XB_AXIS_RTRIGGER_NEG, XB_AXIS_LTRIGGER_NEG, XB_X, XB_RSHIFT,
		XB_DPAD_DOWN, XB_DPAD_RIGHT, XB_DPAD_LEFT, KC_LBRACKET, XB_DPAD_UP, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0,
		XB_A, XB_Y, XB_START, XB_SELECT, KC_F5, KC_F6
	};

	std::vector<std::vector<int>> Bindings =
	{
		DefaultGenericBindings,
		DefaultGenericBindings
	};

	auto ConflictingKeys = std::array<bool, (int)In::Count>{};

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
			// Use OIS ParamList since default behaviour blocks WIN key and steals mouse.
			auto paramList = ParamList{};
			auto wnd = std::ostringstream{};
			wnd << (size_t)handle;
			paramList.insert(std::make_pair(std::string("WINDOW"), wnd.str()));
			paramList.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
			paramList.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
			paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND")));
			paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));

			OisInputManager = InputManager::createInputSystem(paramList);
			OisInputManager->enableAddOnFactory(InputManager::AddOn_All);

			if (OisInputManager->getNumberOfDevices(OISKeyboard) == 0)
			{
				TENLog("Keyboard not found.", LogLevel::Warning);
			}
			else
			{
				OisKeyboard = (Keyboard*)OisInputManager->createInputObject(OISKeyboard, true);
			}

			if (OisInputManager->getNumberOfDevices(OISMouse) == 0)
			{
				TENLog("Mouse not found.", LogLevel::Warning);
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

		int deviceCount = OisInputManager->getNumberOfDevices(OISJoyStick);
		if (deviceCount > 0)
		{
			TENLog("Found " + std::to_string(deviceCount) + " connected game controller" + ((deviceCount > 1) ? "s." : "."), LogLevel::Info);

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
					g_Configuration.EnableThumbstickCamera = true;
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
	}

	void ApplyActionQueue()
	{
		for (int i = 0; i < (int)In::Count; i++)
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
			for (int i = 0; i < (int)In::Count; i++)
			{
				if (Bindings[layout][i] == index)
					return true;
			}
		}

		return false;
	}

	// Merge right and left Ctrl, Shift, and Alt keys.
	int WrapSimilarKeys(int source)
	{
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
		for (int i = 0; i < (int)In::Count; i++)
		{
			int key = Bindings[0][i];

			ConflictingKeys[i] = false;

			for (int j = 0; j < (int)In::Count; j++)
			{
				if (key != Bindings[1][j])
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
			if (Bindings[layout][(int)In::Forward] == index)
			{
				AxisMap[(int)InputAxis::Move].y = 1.0f;
			}
			else if (Bindings[layout][(int)In::Back] == index)
			{
				AxisMap[(int)InputAxis::Move].y = -1.0f;
			}
			else if (Bindings[layout][(int)In::Left] == index)
			{
				AxisMap[(int)InputAxis::Move].x = -1.0f;
			}
			else if (Bindings[layout][(int)In::Right] == index)
			{
				AxisMap[(int)InputAxis::Move].x = 1.0f;
			}
		}
	}

	static void ReadKeyboard()
	{
		if (OisKeyboard == nullptr)
			return;

		try
		{
			OisKeyboard->capture();

			// Poll keys.
			for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
			{
				if (!OisKeyboard->isKeyDown((KeyCode)i))
					continue;

				int key = WrapSimilarKeys(i);
				KeyMap[key] = true;

				// Interpret discrete directional keypresses as analog axis values.
				SetDiscreteAxisValues(key);
			}
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Unable to poll keyboard input: " + std::string(ex.eText), LogLevel::Warning);
		}
	}

	static void ReadMouse()
	{
		if (OisMouse == nullptr)
			return;

		try
		{
			OisMouse->capture();
			auto& state = OisMouse->getMouseState();

			// Update active area resolution.
			auto screenRes = g_Renderer.GetScreenResolution();
			state.width = screenRes.x;
			state.height = screenRes.y;

			// Poll buttons.
			for (int i = 0; i < MAX_MOUSE_KEYS; i++)
				KeyMap[ACTION_OFFSET_MOUSE + i] = state.buttonDown((MouseButtonID)i);

			// Register multiple directional keypresses mapped to mouse axes.
			int baseIndex = ACTION_OFFSET_MOUSE + MAX_MOUSE_KEYS;
			for (int pass = 0; pass < MAX_MOUSE_AXES; pass++)
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

			// TODO: Will need to take screen aspect ratio into account once mouse axes start being used. -- Sezz 2023.10.20
			// Normalize raw mouse axis values to range [-1.0f, 1.0f].
			auto rawAxes = Vector2(state.X.rel, state.Y.rel);
			auto normAxes = Vector2(
				(((rawAxes.x - -DISPLAY_SPACE_RES.x) * 2) / (DISPLAY_SPACE_RES.x - -DISPLAY_SPACE_RES.x)) - 1.0f,
				(((rawAxes.y - -DISPLAY_SPACE_RES.y) * 2) / (DISPLAY_SPACE_RES.y - -DISPLAY_SPACE_RES.y)) - 1.0f);

			// Apply sensitivity and smoothing.
			float sensitivity = (g_Configuration.MouseSensitivity * 0.1f) + 0.4f;
			float smoothing = 1.0f - (g_Configuration.MouseSmoothing * 0.1f);
			normAxes *= sensitivity * smoothing;

			// Set mouse axis values.
			AxisMap[(int)InputAxis::Mouse] = normAxes;
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Unable to poll mouse input: " + std::string(ex.eText), LogLevel::Warning);
		}
	}
	
	static void ReadGameController()
	{
		if (OisGamepad == nullptr)
			return;

		try
		{
			OisGamepad->capture();
			const auto& state = OisGamepad->getJoyStickState();

			// Poll buttons.
			for (int key = 0; key < state.mButtons.size(); key++)
				KeyMap[ACTION_OFFSET_GAMEPAD + key] = state.mButtons[key];

			// Poll axes.
			for (int axis = 0; axis < state.mAxes.size(); axis++)
			{
				// NOTE: We don't support anything above 6 existing XBOX/PS controller axes (2 sticks plus 2 triggers = 6).
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
				int negKeyIndex = ACTION_OFFSET_GAMEPAD + MAX_GAMEPAD_KEYS + (axis * 2);
				int posKeyIndex = ACTION_OFFSET_GAMEPAD + MAX_GAMEPAD_KEYS + (axis * 2) + 1;
				KeyMap[negKeyIndex] = false;
				KeyMap[posKeyIndex] = false;

				// Determine discrete input registering based on analog value.
				int usedIndex = (normalizedValue > 0) ? negKeyIndex : posKeyIndex;
				KeyMap[usedIndex] = true;

				// Register analog input in certain direction.
				// If axis is bound as directional controls, register axis as directional input.
				// Otherwise, register as camera movement input (for future).
				// NOTE: abs() operations are needed to avoid issues with inverted axes on different controllers.

				if (Bindings[1][(int)In::Forward] == usedIndex)
				{
					AxisMap[(int)InputAxis::Move].y = abs(scaledValue);
				}
				else if (Bindings[1][(int)In::Back] == usedIndex)
				{
					AxisMap[(int)InputAxis::Move].y = -abs(scaledValue);
				}
				else if (Bindings[1][(int)In::Left] == usedIndex)
				{
					AxisMap[(int)InputAxis::Move].x = -abs(scaledValue);
				}
				else if (Bindings[1][(int)In::Right] == usedIndex)
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
				int baseIndex = ACTION_OFFSET_GAMEPAD + MAX_GAMEPAD_KEYS + (MAX_GAMEPAD_AXES * 2);
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
			int key = Bindings[layout][number];
			
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
			g_Configuration.EnableWindowedMode = !g_Configuration.EnableWindowedMode;
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

		// Update action map.
		for (auto& action : ActionMap)
			action.Update(Key((int)action.GetID()));

		if (applyQueue)
			ApplyActionQueue();

		// Additional handling.
		HandleHotkeyActions();
		SolveActionCollisions();
	}

	void ClearAllActions()
	{
		for (auto& action : ActionMap)
			action.Clear();

		for (auto& queue : ActionQueue)
			queue = QueueState::None;
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
			if (i >= (int)In::Count)
				break;

			Bindings[1][i] = bindings[i];
		}
	}

	void ApplyDefaultBindings()
	{
		ApplyBindings(DefaultGenericBindings);
		ApplyDefaultXInputBindings();
	}

	bool ApplyDefaultXInputBindings()
	{
		if (!OisGamepad)
			return false;

		for (int i = 0; i < (int)In::Count; i++)
		{
			if (Bindings[1][i] != KC_UNASSIGNED && Bindings[1][i] != Bindings[0][i])
				return false;
		}

		auto vendor = TEN::Utils::ToLower(OisGamepad->vendor());
		if (vendor.find("xbox") != std::string::npos || vendor.find("xinput") != std::string::npos)
		{
			ApplyBindings(DefaultXInputBindings);

			for (int i = 0; i < (int)In::Count; i++)
				g_Configuration.Bindings[i] = Bindings[1][i];

			// Additionally turn on thumbstick camera and vibration.
			g_Configuration.EnableRumble = g_Configuration.EnableThumbstickCamera = true;

			return true;
		}
		else
		{
			return false;
		}
	}

	Vector2 GetMouse2DPosition()
	{
		const auto& state = OisMouse->getMouseState();

		auto areaRes = Vector2(state.width, state.height);
		auto areaPos = Vector2(state.X.abs, state.Y.abs);
		return (DISPLAY_SPACE_RES * (areaPos / areaRes));
	}

	void ClearAction(ActionID actionID)
	{
		ActionMap[(int)actionID].Clear();
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

	bool IsDirectionalActionHeld()
	{
		return (IsHeld(In::Forward) || IsHeld(In::Back) || IsHeld(In::Left) || IsHeld(In::Right));
	}

	bool IsWakeActionHeld()
	{
		if (IsDirectionalActionHeld() || IsHeld(In::StepLeft) || IsHeld(In::StepRight) ||
			IsHeld(In::Walk) || IsHeld(In::Jump) || IsHeld(In::Sprint) || IsHeld(In::Roll) || IsHeld(In::Crouch) ||
			IsHeld(In::Draw) || IsHeld(In::Flare) || IsHeld(In::Action))
		{
			return true;
		}

		return false;
	}

	bool IsOpticActionHeld()
	{
		return (IsDirectionalActionHeld() || IsHeld(In::Action) || IsHeld(In::Crouch) || IsHeld(In::Sprint));
	}
}
