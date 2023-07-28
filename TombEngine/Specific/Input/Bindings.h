#pragma once

namespace TEN::Input
{
	enum class ActionID;

	using BindingMap = std::unordered_map<ActionID, int>;

	enum class BindingMapType
	{
		Keyboard,
		//Gamepad,
		//XBox,
		//Dualshock,
		//Dancepad,
		Custom,

		Count
	};

	class BindingManager
	{
	private:
		// Members
		std::unordered_map<BindingMapType, BindingMap> Bindings	 = {};
		std::unordered_map<ActionID, bool>			   Conflicts = {};

	public:
		// Constants
		static const BindingMap DEFAULT_KEYBOARD_BINDING_MAP;
		static const BindingMap DEFAULT_XBOX_BINDING_MAP;

		// Constructors
		BindingManager();

		// Getters
		int				  GetBoundKey(BindingMapType bindingType, ActionID actionID);
		const BindingMap& GetBindingMap(BindingMapType bindingType);

		// Setters
		void SetKeyBinding(BindingMapType bindingType, ActionID actionID, int key);
		void SetBindingMap(BindingMapType bindingType, const BindingMap& bindingMap);
		void SetDefaultBindingMap(BindingMapType bindingType);
		void SetConflict(ActionID actionID, bool value);

		// Inquirers
		bool TestConflict(ActionID actionID);
	};

	extern BindingManager g_Bindings;
}
