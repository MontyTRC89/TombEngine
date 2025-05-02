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
		{ In::Forward,		  KC_UP },
		{ In::Back,			  KC_DOWN },
		{ In::Left,			  KC_LEFT },
		{ In::Right,		  KC_RIGHT },
		{ In::StepLeft,		  KC_DELETE },
		{ In::StepRight,	  KC_PGDOWN },
		{ In::Action,		  KC_RCONTROL },
		{ In::Walk,			  KC_RSHIFT },
		{ In::Sprint,		  KC_SLASH },
		{ In::Crouch,		  KC_PERIOD },
		{ In::Jump,			  KC_RMENU },
		{ In::Roll,			  KC_END },
		{ In::Draw,			  KC_SPACE },
		{ In::Look,			  KC_NUMPAD0 },

		{ In::Accelerate,	  KC_RCONTROL },
		{ In::Reverse,		  KC_DOWN },
		{ In::Faster,		  KC_SLASH },
		{ In::Slower,		  KC_RSHIFT },
		{ In::Brake,		  KC_RMENU },
		{ In::Fire,			  KC_SPACE },

		{ In::Flare,		  KC_COMMA },
		{ In::SmallMedipack,  KC_MINUS },
		{ In::LargeMedipack,  KC_EQUALS },
		{ In::PreviousWeapon, KC_LBRACKET },
		{ In::NextWeapon,	  KC_RBRACKET },
		{ In::Weapon1,		  KC_1 },
		{ In::Weapon2,		  KC_2 },
		{ In::Weapon3,		  KC_3 },
		{ In::Weapon4,		  KC_4 },
		{ In::Weapon5,		  KC_5 },
		{ In::Weapon6,		  KC_6 },
		{ In::Weapon7,		  KC_7 },
		{ In::Weapon8,		  KC_8 },
		{ In::Weapon9,		  KC_9 },
		{ In::Weapon10,		  KC_0 },

		{ In::Select,		  KC_RETURN },
		{ In::Deselect,		  KC_ESCAPE },
		{ In::Pause,		  KC_P },
		{ In::Inventory,	  KC_ESCAPE },
		{ In::Save,			  KC_F5 },
		{ In::Load,			  KC_F6 }
	};

	const BindingProfile BindingManager::DEFAULT_GAMEPAD_BINDING_PROFILE =
	{
		{ In::Forward, XK_AXIS_X_NEG },
		{ In::Back, XK_AXIS_X_POS },
		{ In::Left, XK_AXIS_Y_NEG },
		{ In::Right, XK_AXIS_Y_POS },
		{ In::StepLeft, XK_L_STICK },
		{ In::StepRight, XK_R_STICK },
		{ In::Action, XK_A },
		{ In::Walk, XK_R_SHIFT },
		{ In::Sprint, XK_AXIS_R_TRIGGER_NEG },
		{ In::Crouch, XK_AXIS_L_TRIGGER_NEG },
		{ In::Jump, XK_X },
		{ In::Roll, XK_B },
		{ In::Draw, XK_Y },
		{ In::Look, XK_L_SHIFT },

		{ In::Accelerate, XK_A },
		{ In::Reverse, XK_AXIS_X_POS },
		{ In::Faster, XK_AXIS_R_TRIGGER_NEG },
		{ In::Slower, XK_R_SHIFT },
		{ In::Brake, XK_X },
		{ In::Fire, XK_AXIS_L_TRIGGER_NEG },

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
		{ In::Weapon10, KC_0 }/*,

		{ In::Select, KC_RETURN },
		{ In::Deselect, XK_SELECT },
		{ In::Pause, XK_START },
		{ In::Inventory, XK_SELECT },
		{ In::Save, KC_F5 },
		{ In::Load, KC_F6 }*/
	};

	const BindingProfile BindingManager::RAW_EVENT_BINDING_PROFILE
	{
		{ In::A,			KC_A },
		{ In::B,			KC_B },
		{ In::C,			KC_C },
		{ In::D,			KC_D },
		{ In::E,			KC_E },
		{ In::F,			KC_F },
		{ In::G,			KC_G },
		{ In::H,			KC_H },
		{ In::I,			KC_I },
		{ In::J,			KC_J },
		{ In::K,			KC_K },
		{ In::L,			KC_L },
		{ In::M,			KC_M },
		{ In::N,			KC_N },
		{ In::O,			KC_O },
		{ In::P,			KC_P },
		{ In::Q,			KC_Q },
		{ In::R,			KC_R },
		{ In::S,			KC_S },
		{ In::T,			KC_T },
		{ In::U,			KC_U },
		{ In::V,			KC_V },
		{ In::W,			KC_W },
		{ In::X,			KC_X },
		{ In::Y,			KC_Y },
		{ In::Z,			KC_Z },
		{ In::Num0,			KC_0 },
		{ In::Num1,			KC_1 },
		{ In::Num2,			KC_2 },
		{ In::Num3,			KC_3 },
		{ In::Num4,			KC_4 },
		{ In::Num5,			KC_5 },
		{ In::Num6,			KC_6 },
		{ In::Num7,			KC_7 },
		{ In::Num8,			KC_8 },
		{ In::Num9,			KC_9 },
		{ In::Minus,		KC_MINUS },
		{ In::Equals,		KC_EQUALS },
		{ In::Esc,			KC_ESCAPE },
		{ In::Tab, 			KC_TAB },
		{ In::Shift, 		KC_LSHIFT },
		{ In::Ctrl, 		KC_LCONTROL },
		{ In::Alt, 			KC_LMENU },
		{ In::Space, 		KC_SPACE },
		{ In::Enter, 		KC_RETURN },
		{ In::Backspace, 	KC_BACK },
		{ In::BracketLeft, 	KC_LBRACKET },
		{ In::BracketRight, KC_RBRACKET },
		{ In::Backslash, 	KC_BACKSLASH },
		{ In::Semicolon, 	KC_SEMICOLON },
		{ In::Apostrophe, 	KC_APOSTROPHE },
		{ In::Comma, 		KC_COMMA },
		{ In::Period, 		KC_PERIOD },
		{ In::Slash, 		KC_SLASH },
		{ In::ArrowUp, 		KC_UP },
		{ In::ArrowDown, 	KC_DOWN },
		{ In::ArrowLeft, 	KC_LEFT },
		{ In::ArrowRight, 	KC_RIGHT },

		{ In::ClickLeft, 	MK_LCLICK },
		{ In::ClickMiddle, 	MK_MCLICK },
		{ In::ClickRight, 	MK_RCLICK },
		{ In::ScrollUp, 	MK_AXIS_X_NEG }, // TODO: Check.
		{ In::ScrollDown, 	MK_AXIS_X_POS }	 // TODO: Check.
	};

	const BindingProfile& BindingManager::GetBindingProfile(BindingProfileID profileID)
	{
		// Find binding profile.
		auto bindingProfileIt = _bindings.find(profileID);
		TENAssert(bindingProfileIt != _bindings.end(), "Attempted to get missing binding profile " + std::to_string((int)profileID) + ".");

		// Return binding profile.
		const auto& [keyProfileID, bindingProfile] = *bindingProfileIt;
		return bindingProfile;
	}

	int BindingManager::GetBoundKeyID(BindingProfileID profileID, ActionID actionID)
	{
		// Find binding profile.
		auto bindingProfileIt = _bindings.find(profileID);
		if (bindingProfileIt == _bindings.end())
			return KC_UNASSIGNED;

		// Get binding profile.
		const auto& [inputDeviceID, bindingProfile] = *bindingProfileIt;

		// Find key-action binding.
		auto keyIt = bindingProfile.find(actionID);
		if (keyIt == bindingProfile.end())
			return KC_UNASSIGNED;

		// Return key binding.
		auto [keyActionID, keyID] = *keyIt;
		return keyID;
	}

	void BindingManager::SetKeyBinding(BindingProfileID profileID, ActionID actionID, int keyID)
	{
		// Overwrite or add key-action binding.
		_bindings[profileID][actionID] = keyID;
	}

	void BindingManager::SetBindingProfile(BindingProfileID profileID, const BindingProfile& bindingProfile)
	{
		// Overwrite or create binding profile.
		_bindings[profileID] = bindingProfile;
	}

	void BindingManager::SetDefaultBindingProfile(BindingProfileID profileID)
	{
		// Reset binding profile defaults.
		switch (profileID)
		{
			case BindingProfileID::Default:
				_bindings[profileID] = DEFAULT_KEYBOARD_MOUSE_BINDING_PROFILE;
				break;

			case BindingProfileID::Custom:
				_bindings[profileID] = DEFAULT_KEYBOARD_MOUSE_BINDING_PROFILE;
				break;

			default:
				TENLog("Failed to reset defaults for binding profile " + std::to_string((int)profileID) + ".", LogLevel::Warning);
				return;
		}
	}

	void BindingManager::SetConflict(ActionID actionID, bool value)
	{
		_conflicts[actionID] = value;
	}

	bool BindingManager::TestConflict(ActionID actionID)
	{
		return _conflicts.at(actionID);
	}

	void BindingManager::Initialize()
	{
		// Initialize default bindings.
		_bindings =
		{
			{ BindingProfileID::Default, DEFAULT_KEYBOARD_MOUSE_BINDING_PROFILE },
			{ BindingProfileID::Custom, DEFAULT_KEYBOARD_MOUSE_BINDING_PROFILE }
		};

		// Initialize conflicts.
		for (int i = 0; i < (int)ActionID::Count; i++)
		{
			auto actionID = (ActionID)i;
			_conflicts.insert({ actionID, false });
		}
	}

	BindingManager g_Bindings;
}
