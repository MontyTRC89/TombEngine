#include "framework.h"
#include "Specific/Input/Bindings.h"

#include "Specific/Input/InputAction.h"
#include "Specific/Input/Keys.h"

#include <OISKeyboard.h>

using namespace OIS;

namespace TEN::Input
{
	const BindingMap BindingManager::DEFAULT_KEYBOARD_BINDING_MAP =
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
		{ In::Speed, KC_SLASH },
		{ In::Slow, KC_RSHIFT },
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

	const BindingMap BindingManager::DEFAULT_XBOX_BINDING_MAP =
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
		{ In::Speed, XK_AXIS_RTRIGGER_NEG },
		{ In::Slow, XK_RSHIFT },
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
		Bindings =
		{
			{ BindingMapType::Keyboard, DEFAULT_KEYBOARD_BINDING_MAP },
			{ BindingMapType::Custom, DEFAULT_KEYBOARD_BINDING_MAP }
		};

		for (int i = 0; i < (int)ActionID::Count; i++)
		{
			auto actionID = (ActionID)i;
			Conflicts.insert({ actionID, false });
		}
	}

	const BindingMap& BindingManager::GetBindingMap(BindingMapType bindingType)
	{
		// Find binding map.
		auto it = Bindings.find(bindingType);
		assertion(it != Bindings.end(), ("Attempted to get missing binding map " + std::to_string((int)bindingType)).c_str());

		// Get and return binding map.
		const auto& bindingMap = it->second;
		return bindingMap;
	}

	int BindingManager::GetBoundKey(BindingMapType bindingType, ActionID actionID)
	{
		// Find binding map.
		auto bindingMapIt = Bindings.find(bindingType);
		if (bindingMapIt == Bindings.end())
			return KC_UNASSIGNED;

		// Get binding map.
		const auto& bindingMap = bindingMapIt->second;

		// Find key binding.
		auto keyIt = bindingMap.find(actionID);
		if (keyIt == bindingMap.end())
			return KC_UNASSIGNED;

		// Get and return key binding.
		int key = keyIt->second;
		return key;
	}

	void BindingManager::SetKeyBinding(BindingMapType bindingType, ActionID actionID, int key)
	{
		// Overwrite or add new key binding.
		Bindings[bindingType][actionID] = key;
	}

	void BindingManager::SetBindingMap(BindingMapType bindingType, const BindingMap& bindingMap)
	{
		// Overwrite or create new binding map.
		Bindings[bindingType] = bindingMap;
	}

	void BindingManager::SetDefaultBindingMap(BindingMapType bindingType)
	{
		// TODO: Figure out how this should work.
		// Reset binding map defaults.
		switch (bindingType)
		{
		case BindingMapType::Keyboard:
			Bindings[bindingType] = DEFAULT_KEYBOARD_BINDING_MAP;
			break;

		case BindingMapType::Custom:
			Bindings[bindingType] = DEFAULT_KEYBOARD_BINDING_MAP;
			break;

		default:
			TENLog("Cannot reset defaults for binding map " + std::to_string((int)bindingType), LogLevel::Warning);
			return;
		}
	}

	BindingManager g_Bindings;
}
