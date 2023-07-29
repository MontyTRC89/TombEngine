#include "framework.h"
#include "Specific/Input/Input.h"

#include <OISException.h>
#include <OISForceFeedback.h>
#include <OISInputManager.h>
#include <OISJoyStick.h>
#include <OISKeyboard.h>

#include "Game/items.h"
#include "Game/savegame.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Specific/clock.h"
#include "Specific/trutils.h"
#include "Specific/winmain.h"

using namespace OIS;
using TEN::Renderer::g_Renderer;

// Big TODO: Entire input system shouldn't be left exposed like this.

namespace TEN::Input
{
	constexpr int AXIS_DEADZONE = 8000;

	// OIS interfaces
	InputManager*  OisInputManager = nullptr;
	Keyboard*	   OisKeyboard	   = nullptr;
	JoyStick*	   OisGamepad	   = nullptr;
	ForceFeedback* OisRumble	   = nullptr;
	Effect*		   OisEffect	   = nullptr;

	// Globals
	RumbleData				 RumbleInfo  = {};
	std::vector<InputAction> ActionMap	 = {};
	std::vector<QueueState>	 ActionQueue = {};
	std::vector<bool>		 KeyMap		 = {};
	std::vector<float>		 AxisMap	 = {};

	//  Deprecated legacy input bit fields.
	int DbInput = 0;
	int TrInput = 0;

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
			"Joy LT",		"Joy LT",		"Joy RT",		"Joy RT",		"D-Pad Up",		"D-Pad Down",	"D-Pad Left",	"D-Pad Right"
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
		XB_A, XB_AXIS_X_POS, XB_AXIS_RTRIGGER_NEG, XB_RSHIFT, XB_X, XB_AXIS_LTRIGGER_NEG,
		XB_DPAD_DOWN, KC_MINUS, KC_EQUALS, KC_LBRACKET, KC_RBRACKET, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0,
		KC_RETURN, XB_SELECT, XB_START, XB_SELECT, KC_F5, KC_F6
	};

	std::vector<std::vector<int>> Bindings =
	{
		DefaultGenericBindings,
		DefaultGenericBindings
	};

	auto ConflictingKeys = std::array<bool, KEY_COUNT>{};

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

		for (int i = 0; i < (int)ActionID::Count; i++)
		{
			ActionMap.push_back(InputAction((ActionID)i));
			ActionQueue.push_back(QueueState::None);
		}

		KeyMap.resize(MAX_INPUT_SLOTS);
		AxisMap.resize(InputAxis::Count);

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
		}
		catch (OIS::Exception& ex)
		{
			TENLog("An exception occured during input system init: " + std::string(ex.eText), LogLevel::Error);
		}

		int numDevices = OisInputManager->getNumberOfDevices(OISJoyStick);
		if (numDevices > 0)
		{
			TENLog("Found " + std::to_string(numDevices) + " connected game controller" + (numDevices > 1 ? "s." : "."), LogLevel::Info);

			try
			{
				OisGamepad = (JoyStick*)OisInputManager->createInputObject(OISJoyStick, true);
				TENLog("Using '" + OisGamepad->vendor() + "' device for input.", LogLevel::Info);

				// Try to initialize vibration interface.
				OisRumble = (ForceFeedback*)OisGamepad->queryInterface(Interface::ForceFeedback);
				if (OisRumble)
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
		if (OisKeyboard)
			OisInputManager->destroyInputObject(OisKeyboard);

		if (OisGamepad)
			OisInputManager->destroyInputObject(OisGamepad);

		if (OisEffect)
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
			axis = 0.0f;

		// Clear legacy bit fields.
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

	bool LayoutContainsIndex(unsigned int index)
	{
		for (int layout = 1; layout >= 0; layout--)
		{
			for (int i = 0; i < KEY_COUNT; i++)
			{
				if (Bindings[layout][i] == index)
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
			int key = Bindings[0][i];

			ConflictingKeys[i] = false;

			for (int j = 0; j < KEY_COUNT; j++)
			{
				if (key != Bindings[1][j])
					continue;

				ConflictingKeys[i] = true;
				break;
			}
		}
	}

	void SetDiscreteAxisValues(unsigned int index)
	{
		for (int layout = 0; layout <= 1; layout++)
		{
			if (Bindings[layout][KEY_FORWARD] == index)
			{
				AxisMap[(unsigned int)InputAxis::MoveVertical] = 1.0f;
			}
			else if (Bindings[layout][KEY_BACK] == index)
			{
				AxisMap[(unsigned int)InputAxis::MoveVertical] = -1.0f;
			}
			else if (Bindings[layout][KEY_LEFT] == index)
			{
				AxisMap[(unsigned int)InputAxis::MoveHorizontal] = -1.0f;
			}
			else if (Bindings[layout][KEY_RIGHT] == index)
			{
				AxisMap[(unsigned int)InputAxis::MoveHorizontal] = 1.0f;
			}
		}
	}

	void ReadGameController()
	{
		if (!OisGamepad)
			return;

		try
		{
			// Poll gamepad.
			OisGamepad->capture();
			const JoyStickState& state = OisGamepad->getJoyStickState();

			// Scan buttons.
			for (int key = 0; key < state.mButtons.size(); key++)
				KeyMap[MAX_KEYBOARD_KEYS + key] = state.mButtons[key];

			// Scan axes.
			for (int axis = 0; axis < state.mAxes.size(); axis++)
			{
				// We don't support anything above 6 existing XBOX/PS controller axes (two sticks plus triggers).
				if (axis >= MAX_GAMEPAD_AXES)
					break;

				// Filter out deadzone.
				if (abs(state.mAxes[axis].abs) < AXIS_DEADZONE)
					continue;

				// Calculate raw normalized analog value (for camera).
				float normalizedValue = (float)(state.mAxes[axis].abs + (state.mAxes[axis].abs > 0 ? -AXIS_DEADZONE : AXIS_DEADZONE))
					/ (float)(std::numeric_limits<short>::max() - AXIS_DEADZONE);

				// Calculate scaled analog value (for movement).
				// Minimum value of 0.2f and maximum value of 1.7f is empirically the most organic rate from tests.
				float scaledValue = abs(normalizedValue) * 1.5f + 0.2f;

				// Calculate and reset discrete input slots.
				unsigned int negKeyIndex = MAX_KEYBOARD_KEYS + MAX_GAMEPAD_KEYS + (axis * 2);
				unsigned int posKeyIndex = MAX_KEYBOARD_KEYS + MAX_GAMEPAD_KEYS + (axis * 2) + 1;
				KeyMap[negKeyIndex] = false;
				KeyMap[posKeyIndex] = false;

				// Decide on the discrete input registering based on analog value.
				unsigned int usedIndex = normalizedValue > 0 ? negKeyIndex : posKeyIndex;
				KeyMap[usedIndex] = true;

				// Register analog input in certain direction.
				// If axis is bound as directional controls, register axis as directional input.
				// Otherwise, register as camera movement input (for future).
				// NOTE: abs() operations are needed to avoid issues with inverted axes on different controllers.

				if (Bindings[1][KEY_FORWARD] == usedIndex)
				{
					AxisMap[InputAxis::MoveVertical] = abs(scaledValue);
				}
				else if (Bindings[1][KEY_BACK] == usedIndex)
				{
					AxisMap[InputAxis::MoveVertical] = -abs(scaledValue);
				}
				else if (Bindings[1][KEY_LEFT] == usedIndex)
				{
					AxisMap[InputAxis::MoveHorizontal] = -abs(scaledValue);
				}
				else if (Bindings[1][KEY_RIGHT] == usedIndex)
				{
					AxisMap[InputAxis::MoveHorizontal] = abs(scaledValue);
				}
				else if (!LayoutContainsIndex(usedIndex))
				{
					unsigned int camAxisIndex = (unsigned int)std::clamp((unsigned int)InputAxis::CameraVertical + axis % 2,
						(unsigned int)InputAxis::CameraVertical,
						(unsigned int)InputAxis::CameraHorizontal);
					AxisMap[camAxisIndex] = normalizedValue;
				}
			}

			// Scan POVs (controllers usually have one, but scan all out of paranoia).
			for (int pov = 0; pov < 4; pov++)
			{
				if (state.mPOV[pov].direction == Pov::Centered)
					continue;

				// Do 4 passes; every pass checks every POV direction. For every direction,
				// separate keypress is registered.
				// This is needed to allow multiple directions  pressed at the same time.
				for (int pass = 0; pass < 4; pass++)
				{
					unsigned int index = MAX_KEYBOARD_KEYS + MAX_GAMEPAD_KEYS + MAX_GAMEPAD_AXES * 2;

					switch (pass)
					{
					case 0:
						if (!(state.mPOV[pov].direction & Pov::North))
							continue;
						break;

					case 1:
						if (!(state.mPOV[pov].direction & Pov::South))
							continue;
						break;

					case 2:
						if (!(state.mPOV[pov].direction & Pov::West))
							continue;
						break;

					case 3:
						if (!(state.mPOV[pov].direction & Pov::East))
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
			TENLog("Unable to poll game controller input: " + std::string(ex.eText), LogLevel::Warning);
		}
	}

	void ReadKeyboard()
	{
		if (!OisKeyboard)
			return;

		try
		{
			OisKeyboard->capture();

			for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
			{
				if (!OisKeyboard->isKeyDown((KeyCode)i))
				{
					continue;
				}

				int key = WrapSimilarKeys(i);
				KeyMap[key] = true;

				// Register directional discrete keypresses as max analog axis values.
				SetDiscreteAxisValues(key);
			}
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Unable to poll keyboard input: " + std::string(ex.eText), LogLevel::Warning);
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
		ReadGameController();
		DefaultConflict();

		// Update action map.
		for (int i = 0; i < KEY_COUNT; i++)
			ActionMap[i].Update(Key(i));

		if (applyQueue)
			ApplyActionQueue();

		// Additional handling.
		HandleHotkeyActions();
		SolveActionCollisions();

		// Port actions back to legacy bit fields.
		for (const auto& action : ActionMap)
		{
			// TEMP FIX: Only port up to 32 bits.
			auto actionID = action.GetID();
			if ((int)actionID >= 32)
				break;

			int actionBit = 1 << (int)actionID;

			DbInput |= action.IsClicked() ? actionBit : 0;
			TrInput |= action.IsHeld()	  ? actionBit : 0;
		}
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

		RumbleInfo.FadeSpeed = power / (delayInSec * (float)FPS);
		RumbleInfo.Power = power + RumbleInfo.FadeSpeed;
		RumbleInfo.LastPower = RumbleInfo.Power;
	}

	void StopRumble()
	{
		if (!OisRumble || !OisEffect)
			return;

		try { OisRumble->remove(OisEffect); }
		catch (OIS::Exception& ex) { TENLog("Error when stopping vibration effect: " + std::string(ex.eText), LogLevel::Error); }

		RumbleInfo = {};
	}

	static void ApplyBindings(const std::vector<int>& bindings)
	{
		for (int i = 0; i < bindings.size(); i++)
		{
			if (i >= KEY_COUNT)
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

		for (int i = 0; i < KEY_COUNT; i++)
		{
			if (Bindings[1][i] != KC_UNASSIGNED && Bindings[1][i] != Bindings[0][i])
				return false;
		}

		auto vendor = TEN::Utils::ToLower(OisGamepad->vendor());
		if (vendor.find("xbox") != std::string::npos || vendor.find("xinput") != std::string::npos)
		{
			ApplyBindings(DefaultXInputBindings);

			for (int i = 0; i < KEY_COUNT; i++)
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

	void ClearAction(ActionID actionID)
	{
		ActionMap[(int)actionID].Clear();

		// TEMP FIX: Only port up to 32 bits.
		if ((int)actionID >= 32)
			return;

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
