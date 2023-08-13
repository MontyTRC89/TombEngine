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
	std::vector<float>		 KeyMap		 = {};
	std::vector<float>		 AxisMap	 = {};

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

		KeyMap.resize(KEY_SLOT_COUNT);
		AxisMap.resize(InputAxis::Count);

		RumbleInfo = {};

		try
		{
			// Use an OIS ParamList since the default behaviour blocks the WIN key.
			ParamList pl;
			std::ostringstream wnd;
			wnd << (size_t)handle;
			pl.insert(std::make_pair(std::string("WINDOW"), wnd.str()));
			pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
			pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));

			OisInputManager = InputManager::createInputSystem(pl);
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
			key = 0.0f;

		for (auto& axis : AxisMap)
			axis = 0.0f;
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

			case QueueState::Update:
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

	static bool TestBoundKey(int key)
	{
		for (int i = 1; i >= 0; i--)
		{
			auto deviceID = (InputDeviceID)i;
			for (int j = 0; j < (int)In::Count; j++)
			{
				auto actionID = (ActionID)j;
				if (g_Bindings.GetBoundKey(deviceID, actionID) != KC_UNASSIGNED)
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
		for (int i = 0; i < (int)In::Count; i++)
		{
			auto actionID = (ActionID)i;

			g_Bindings.SetConflict(actionID, false);

			int key = g_Bindings.GetBoundKey(InputDeviceID::KeyboardMouse, (ActionID)i);
			for (int j = 0; j < (int)In::Count; j++)
			{
				if (key != g_Bindings.GetBoundKey(InputDeviceID::Custom, (ActionID)j))
					continue;

				g_Bindings.SetConflict(actionID, true);
				break;
			}
		}
	}

	static void SetDiscreteAxisValues(unsigned int keyID)
	{
		for (int i = 0; i < (int)InputDeviceID::Count; i++)
		{
			auto deviceID = (InputDeviceID)i;
			if (g_Bindings.GetBoundKey(deviceID, In::Forward) == keyID)
			{
				AxisMap[(int)InputAxis::MoveVertical] = 1.0f;
			}
			else if (g_Bindings.GetBoundKey(deviceID, In::Back) == keyID)
			{
				AxisMap[(int)InputAxis::MoveVertical] = -1.0f;
			}
			else if (g_Bindings.GetBoundKey(deviceID, In::Left) == keyID)
			{
				AxisMap[(int)InputAxis::MoveHorizontal] = -1.0f;
			}
			else if (g_Bindings.GetBoundKey(deviceID, In::Right) == keyID)
			{
				AxisMap[(int)InputAxis::MoveHorizontal] = 1.0f;
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
				KeyMap[KEYBOARD_KEY_COUNT + MOUSE_BUTTON_COUNT + (MOUSE_AXIS_COUNT * 2) + key] = state.mButtons[key] ? 1.0f : 0.0f;

			// Scan axes.
			for (int axis = 0; axis < state.mAxes.size(); axis++)
			{
				// We don't support anything above 6 existing XBOX/PS controller axes (two sticks plus triggers).
				if (axis >= GAMEPAD_AXIS_COUNT)
					break;

				// Filter out deadzone.
				if (abs(state.mAxes[axis].abs) < AXIS_DEADZONE)
					continue;

				// Calculate raw normalized analog value (for camera).
				float normalizedValue = float(state.mAxes[axis].abs + (state.mAxes[axis].abs > 0 ? -AXIS_DEADZONE : AXIS_DEADZONE)) /
					float(SHRT_MAX - AXIS_DEADZONE);

				// Calculate scaled analog value (for movement).
				// Minimum value of 0.2f and maximum value of 1.7f is empirically the most organic rate from tests.
				float scaledValue = abs(normalizedValue) * 1.5f + 0.2f;

				// Calculate and reset discrete input slots.
				int negKey = KEYBOARD_KEY_COUNT + MOUSE_BUTTON_COUNT + (MOUSE_AXIS_COUNT * 2) + GAMEPAD_BUTTON_COUNT + (axis * 2);
				int posKey = KEYBOARD_KEY_COUNT + MOUSE_BUTTON_COUNT + (MOUSE_AXIS_COUNT * 2) + GAMEPAD_BUTTON_COUNT + (axis * 2) + 1;
				KeyMap[negKey] = (normalizedValue > 0) ? abs(normalizedValue) : 0.0f;
				KeyMap[posKey] = (normalizedValue < 0) ? abs(normalizedValue) : 0.0f;

				// Decide on the discrete input registering based on analog value.
				int usedKey = (normalizedValue > 0) ? negKey : posKey;

				// Register analog input in certain direction.
				// If axis is bound as directional controls, register axis as directional input.
				// Otherwise, register as camera movement input (for future).
				// NOTE: abs() operations are needed to avoid issues with inverted axes on different controllers.

				if (g_Bindings.GetBoundKey(InputDeviceID::Custom, In::Forward) == usedKey)
				{
					AxisMap[InputAxis::MoveVertical] = abs(scaledValue);
				}
				else if (g_Bindings.GetBoundKey(InputDeviceID::Custom, In::Back) == usedKey)
				{
					AxisMap[InputAxis::MoveVertical] = -abs(scaledValue);
				}
				else if (g_Bindings.GetBoundKey(InputDeviceID::Custom, In::Left)  == usedKey)
				{
					AxisMap[InputAxis::MoveHorizontal] = -abs(scaledValue);
				}
				else if (g_Bindings.GetBoundKey(InputDeviceID::Custom, In::Right) == usedKey)
				{
					AxisMap[InputAxis::MoveHorizontal] = abs(scaledValue);
				}
				else if (!TestBoundKey(usedKey))
				{
					unsigned int camAxisIndex = (int)std::clamp((int)InputAxis::CameraVertical + axis % 2,
						(int)InputAxis::CameraVertical,
						(int)InputAxis::CameraHorizontal);
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
					unsigned int index = KEYBOARD_KEY_COUNT + MOUSE_BUTTON_COUNT + (MOUSE_AXIS_COUNT * 2) + GAMEPAD_BUTTON_COUNT + (GAMEPAD_AXIS_COUNT * 2);

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
					KeyMap[index] = 1.0f;
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

			for (int i = 0; i < KEYBOARD_KEY_COUNT; i++)
			{
				if (!OisKeyboard->isKeyDown((KeyCode)i))
				{
					continue;
				}

				int key = WrapSimilarKeys(i);
				KeyMap[key] = 1.0f;

				// Register directional discrete keypresses as max analog axis values.
				SetDiscreteAxisValues(key);
			}
		}
		catch (OIS::Exception& ex)
		{
			TENLog("Unable to poll keyboard input: " + std::string(ex.eText), LogLevel::Warning);
		}
	}

	static float Key(ActionID actionID)
	{
		for (int i = (int)InputDeviceID::Count - 1; i >= 0; i--)
		{
			auto deviceID = (InputDeviceID)i;

			if (deviceID == InputDeviceID::KeyboardMouse && g_Bindings.TestConflict(actionID))
				continue;

			int key = g_Bindings.GetBoundKey((InputDeviceID)i, actionID);
			if (KeyMap[key] != 0.0f)
				return KeyMap[key];
		}

		return 0.0f;
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
		for (auto& action : ActionMap)
			action.Update(Key(action.GetID()));

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

	static void ApplyBindings(const BindingProfile& set)
	{
		g_Bindings.SetBindingProfile(InputDeviceID::Custom, set);
	}

	void ApplyDefaultBindings()
	{
		ApplyBindings(BindingManager::DEFAULT_XBOX_BINDING_PROFILE);
		ApplyDefaultXInputBindings();
	}

	bool ApplyDefaultXInputBindings()
	{
		if (!OisGamepad)
			return false;

		for (int i = 0; i < (int)In::Count; i++)
		{
			auto actionID = (ActionID)i;

			int defaultKey = g_Bindings.GetBoundKey(InputDeviceID::KeyboardMouse, actionID);
			int userKey = g_Bindings.GetBoundKey(InputDeviceID::Custom, actionID);

			if (userKey != KC_UNASSIGNED &&
				userKey != defaultKey)
			{
				return false;
			}
		}

		auto vendor = TEN::Utils::ToLower(OisGamepad->vendor());
		if (vendor.find("xbox") != std::string::npos || vendor.find("xinput") != std::string::npos)
		{
			ApplyBindings(BindingManager::DEFAULT_XBOX_BINDING_PROFILE);
			g_Configuration.Bindings = g_Bindings.GetBindingProfile(InputDeviceID::Custom);

			// Additionally turn on thumbstick camera and vibration.
			g_Configuration.EnableRumble = true;
			g_Configuration.EnableThumbstickCamera = true;

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
