#pragma once

namespace TEN::Input
{
	enum class ActionID;

	using BindingProfile = std::unordered_map<ActionID, int>;

	// TODO: These don't represent devices yet, it's still the legacy way.
	enum class InputDeviceID
	{
		KeyboardMouse,
		//Gamepad,
		//XBox,
		//Dualshock,
		//Dancepad,
		Custom,

		Count
	};

	// TODO: Allow different binding profiles for each device. Default, Custom1, Custom2.
	class BindingManager
	{
	private:
		// Members
		std::unordered_map<InputDeviceID, BindingProfile> Bindings	= {};
		std::unordered_map<ActionID, bool>				  Conflicts = {};

	public:
		// Constants
		static const BindingProfile DEFAULT_KEYBOARD_MOUSE_BINDING_PROFILE;
		static const BindingProfile DEFAULT_XBOX_BINDING_PROFILE;

		// Constructors
		BindingManager();

		// Getters
		int					  GetBoundKey(InputDeviceID deviceID, ActionID actionID);
		const BindingProfile& GetBindingProfile(InputDeviceID deviceID);

		// Setters
		void SetKeyBinding(InputDeviceID deviceID, ActionID actionID, int key);
		void SetBindingProfile(InputDeviceID deviceID, const BindingProfile& profile);
		void SetDefaultBindingProfile(InputDeviceID deviceID);
		void SetConflict(ActionID actionID, bool value);

		// Inquirers
		bool TestConflict(ActionID actionID);
	};

	extern BindingManager g_Bindings;
}
