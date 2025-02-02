#include "framework.h"
#include "Specific/Input/Bindings.h"

#include "Specific/Input/InputAction.h"
#include "Specific/Input/Keys.h"

#include <OISKeyboard.h>

using namespace OIS;

namespace TEN::Input
{
	const BindingProfile BindingManager::DEFAULT_KEYBOARD_MOUSE_BINDING_PROFILE =
	{
		{ In::Forward, KC_UP },
		{ In::Back, KC_DOWN },
		{ In::Left, KC_LEFT },
		{ In::Right, KC_RIGHT },
		{ In::StepLeft, KC_DELETE },
		{ In::StepRight, KC_PGDOWN },
		{ In::Action, KC_RCONTROL },
		{ In::Walk, KC_RSHIFT },
		{ In::Sprint, KC_SLASH },
		{ In::Crouch, KC_PERIOD },
		{ In::Jump, KC_RMENU },
		{ In::Roll, KC_END },
		{ In::Draw, KC_SPACE },
		{ In::Look, KC_NUMPAD0 },

		{ In::Accelerate, KC_RCONTROL },
		{ In::Reverse, KC_DOWN },
		{ In::Faster, KC_SLASH },
		{ In::Slower, KC_RSHIFT },
		{ In::Brake, KC_RMENU },
		{ In::Fire, KC_SPACE },

		{ In::Flare, KC_COMMA },
		{ In::SmallMedipack, KC_MINUS },
		{ In::LargeMedipack, KC_EQUALS },
		{ In::PreviousWeapon, KC_LBRACKET },
		{ In::NextWeapon, KC_RBRACKET },
		{ In::Weapon1, KC_1 },
		{ In::Weapon2, KC_2 },
		{ In::Weapon3, KC_3 },
		{ In::Weapon4, KC_4 },
		{ In::Weapon5, KC_5 },
		{ In::Weapon6, KC_6 },
		{ In::Weapon7, KC_7 },
		{ In::Weapon8, KC_8 },
		{ In::Weapon9, KC_9 },
		{ In::Weapon10, KC_0 },

		{ In::Select, KC_RETURN },
		{ In::Deselect, KC_ESCAPE },
		{ In::Pause, KC_P },
		{ In::Inventory, KC_ESCAPE },
		{ In::Save, KC_F5 },
		{ In::Load, KC_F6 }
	};

	const BindingProfile BindingManager::DEFAULT_XBOX_CONTROLLER_BINDING_PROFILE =
	{
		{ In::Forward, XK_AXIS_X_NEG },
		{ In::Back, XK_AXIS_X_POS },
		{ In::Left, XK_AXIS_Y_NEG },
		{ In::Right, XK_AXIS_Y_POS },
		{ In::StepLeft, XK_LSTICK },
		{ In::StepRight, XK_RSTICK },
		{ In::Action, XK_A },
		{ In::Walk, XK_RSHIFT },
		{ In::Sprint, XK_AXIS_RTRIGGER_NEG },
		{ In::Crouch, XK_AXIS_LTRIGGER_NEG },
		{ In::Jump, XK_X },
		{ In::Roll, XK_B },
		{ In::Draw, XK_Y },
		{ In::Look, XK_LSHIFT },

		{ In::Accelerate, XK_A },
		{ In::Reverse, XK_AXIS_X_POS },
		{ In::Faster, XK_AXIS_RTRIGGER_NEG },
		{ In::Slower, XK_RSHIFT },
		{ In::Brake, XK_X },
		{ In::Fire, XK_AXIS_LTRIGGER_NEG },

		{ In::Flare, XK_DPAD_DOWN },
		{ In::SmallMedipack, KC_MINUS },
		{ In::LargeMedipack, KC_EQUALS },
		{ In::PreviousWeapon, KC_LBRACKET },
		{ In::NextWeapon, KC_RBRACKET },
		{ In::Weapon1, KC_1 },
		{ In::Weapon2, KC_2 },
		{ In::Weapon3, KC_3 },
		{ In::Weapon4, KC_4 },
		{ In::Weapon5, KC_5 },
		{ In::Weapon6, KC_6 },
		{ In::Weapon7, KC_7 },
		{ In::Weapon8, KC_8 },
		{ In::Weapon9, KC_9 },
		{ In::Weapon10, KC_0 },

		{ In::Select, KC_RETURN },
		{ In::Deselect, XK_SELECT },
		{ In::Pause, XK_START },
		{ In::Inventory, XK_SELECT },
		{ In::Save, KC_F5 },
		{ In::Load, KC_F6 }
	};

	BindingManager::BindingManager()
	{
		// Initialize default bindings.
		_bindings =
		{
			{ InputDeviceID::KeyboardMouse, DEFAULT_KEYBOARD_MOUSE_BINDING_PROFILE },
			{ InputDeviceID::Custom, DEFAULT_KEYBOARD_MOUSE_BINDING_PROFILE }
		};

		// Initialize conflicts.
		for (int i = 0; i < (int)InputActionID::Count; i++)
		{
			auto actionID = (InputActionID)i;
			_conflicts.insert({ actionID, false });
		}
	}

	const BindingProfile& BindingManager::GetBindingProfile(InputDeviceID deviceID)
	{
		// Find binding profile.
		auto bindingProfileIt = _bindings.find(deviceID);
		TENAssert(bindingProfileIt != _bindings.end(), "Attempted to get missing binding profile " + std::to_string((int)deviceID) + ".");

		// Get and return binding profile.
		const auto& [inputDeviceID, bindingProfile] = *bindingProfileIt;
		return bindingProfile;
	}

	int BindingManager::GetBoundKey(InputDeviceID deviceID, InputActionID actionID)
	{
		// Find binding profile.
		auto bindingProfileIt = _bindings.find(deviceID);
		if (bindingProfileIt == _bindings.end())
			return KC_UNASSIGNED;

		// Get binding profile.
		const auto& [inputDeviceID, bindingProfile] = *bindingProfileIt;

		// Find key binding.
		auto keyIt = bindingProfile.find(actionID);
		if (keyIt == bindingProfile.end())
			return KC_UNASSIGNED;

		// Get and return key binding.
		auto [inputActionID, key] = *keyIt;
		return key;
	}

	void BindingManager::SetKeyBinding(InputDeviceID deviceID, InputActionID actionID, int key)
	{
		// Overwrite or add key binding.
		_bindings[deviceID][actionID] = key;
	}

	void BindingManager::SetBindingProfile(InputDeviceID deviceID, const BindingProfile& bindingProfile)
	{
		// Overwrite or create binding profile.
		_bindings[deviceID] = bindingProfile;
	}

	void BindingManager::SetDefaultBindingProfile(InputDeviceID deviceID)
	{
		// Reset binding profile defaults.
		switch (deviceID)
		{
		case InputDeviceID::KeyboardMouse:
			_bindings[deviceID] = DEFAULT_KEYBOARD_MOUSE_BINDING_PROFILE;
			break;

		case InputDeviceID::Custom:
			_bindings[deviceID] = DEFAULT_KEYBOARD_MOUSE_BINDING_PROFILE;
			break;

		default:
			TENLog("Failed to reset defaults for binding profile " + std::to_string((int)deviceID) + ".", LogLevel::Warning);
			return;
		}
	}

	void BindingManager::SetConflict(InputActionID actionID, bool value)
	{
		_conflicts.insert({ actionID, value });
	}

	bool BindingManager::TestConflict(InputActionID actionID)
	{
		return _conflicts.at(actionID);
	}

	BindingManager g_Bindings;
}
