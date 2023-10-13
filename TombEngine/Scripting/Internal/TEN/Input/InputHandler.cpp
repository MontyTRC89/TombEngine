#include "framework.h"
#include "Scripting/Internal/TEN/Input/InputHandler.h"

#include "Specific/Input/Input.h"
#include "Scripting/Internal/TEN/Input/ActionIDs.h"
#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"

using namespace TEN::Input;

namespace Input
{
	static void Vibrate(float strength, sol::optional<float> time)
	{
		Rumble(strength, time.value_or(0.3f), RumbleMode::Both);
	}

	static bool CheckInput(int actionIndex)
	{
		if (actionIndex > (int)ActionID::Count)
		{
			ScriptAssertF(false, "Input action {} does not exist.", actionIndex);
			return false;
		}

		return true;
	}

	static bool KeyIsHeld(int actionIndex)
	{
		if (!CheckInput(actionIndex))
			return false;

		if (IsHeld((ActionID)actionIndex))
			return true;

		return false;
	}

	static bool KeyIsHit(int actionIndex)
	{
		if (!CheckInput(actionIndex))
			return false;

		if (IsClicked((ActionID)actionIndex))
			return true;

		return false;
	}

	static void KeyPush(int actionIndex)
	{
		if (!CheckInput(actionIndex))
			return;

		ActionQueue[actionIndex] = QueueState::Push;
	}

	static void KeyClear(int actionIndex)
	{
		if (!CheckInput(actionIndex))
			return;

		ActionQueue[actionIndex] = QueueState::Clear;
	}

	void Register(sol::state* state, sol::table& parent)
	{
		sol::table tableInput{ state->lua_state(), sol::create };
		parent.set(ScriptReserved_Input, tableInput);

		///Vibrate game controller, if function is available and setting is on.
		//@function Vibrate
		//@tparam float strength Strength of the vibration
		//@tparam float time __(default 0.3)__ Time of the vibration, in seconds
		tableInput.set_function(ScriptReserved_Vibrate, &Vibrate);

		/// Check if particular action key is held
		//@function KeyIsHeld
		//@tparam Input.ActionID action action mapping index to check
		tableInput.set_function(ScriptReserved_KeyIsHeld, &KeyIsHeld);

		/// Check if particular action key was hit (once)
		//@function KeyIsHit
		//@tparam Input.ActionID action action mapping index to check
		tableInput.set_function(ScriptReserved_KeyIsHit, &KeyIsHit);

		/// Emulate pushing of a certain action key
		//@function KeyPush
		//@tparam Input.ActionID action action mapping index to push
		tableInput.set_function(ScriptReserved_KeyPush, &KeyPush);

		/// Clears particular input from action key
		//@function KeyClear
		//@tparam Input.ActionID action action mapping index to clear
		tableInput.set_function(ScriptReserved_KeyClear, &KeyClear);

		LuaHandler handler{ state };
		handler.MakeReadOnlyTable(tableInput, ScriptReserved_ActionID, ACTION_IDS);
	}
}