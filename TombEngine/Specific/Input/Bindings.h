#pragma once

namespace TEN::Input
{
	enum class InputActionID;

	using BindingProfile = std::unordered_map<InputActionID, int>;

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
		std::unordered_map<InputActionID, bool>			  Conflicts = {};

	public:
		// Constants

		static const BindingProfile DEFAULT_KEYBOARD_MOUSE_BINDING_PROFILE;
		static const BindingProfile DEFAULT_XBOX_CONTROLLER_BINDING_PROFILE;

		// Constructors

		BindingManager();

		// Getters

		int					  GetBoundKey(InputDeviceID deviceID, InputActionID actionID);
		const BindingProfile& GetBindingProfile(InputDeviceID deviceID);

		// Setters

		void SetKeyBinding(InputDeviceID deviceID, InputActionID actionID, int key);
		void SetBindingProfile(InputDeviceID deviceID, const BindingProfile& profile);
		void SetDefaultBindingProfile(InputDeviceID deviceID);
		void SetConflict(InputActionID actionID, bool value);

		// Inquirers

		bool TestConflict(InputActionID actionID);
	};

	extern BindingManager g_Bindings;
}
