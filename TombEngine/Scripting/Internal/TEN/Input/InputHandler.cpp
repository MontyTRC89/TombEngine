#include "framework.h"
#include "Scripting/Internal/TEN/Input/InputHandler.h"

#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Input/ActionIDs.h"
#include "Specific/Input/Input.h"

using namespace TEN::Input;

/// Functions for input management.
// @tentable Input
// @pragma nostrip

namespace Input
{
	///Vibrate game controller, if function is available and setting is on.
	//@function Vibrate
	//@tparam float strength Strength of the vibration
	//@tparam float time __(default 0.3)__ Time of the vibration, in seconds
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

	/// Check if particular action key is held
	//@function KeyIsHeld
	//@tparam Input.ActionID action action mapping index to check
	static bool KeyIsHeld(int actionIndex)
	{
		if (!CheckInput(actionIndex))
			return false;

		if (IsHeld((ActionID)actionIndex))
			return true;

		return false;
	}

	/// Check if particular action key was hit (once)
	//@function KeyIsHit
	//@tparam Input.ActionID action action mapping index to check
	static bool KeyIsHit(int actionIndex)
	{
		if (!CheckInput(actionIndex))
			return false;

		if (IsClicked((ActionID)actionIndex))
			return true;

		return false;
	}

	/// Emulate pushing of a certain action key
	//@function KeyPush
	//@tparam Input.ActionID action action mapping index to push
	static void KeyPush(int actionIndex)
	{
		if (!CheckInput(actionIndex))
			return;

		ActionQueue[actionIndex] = QueueState::Push;
	}

	/// Clears particular input from action key
	//@function KeyClear
	//@tparam Input.ActionID action action mapping index to clear
	static void KeyClear(int actionIndex)
	{
		if (!CheckInput(actionIndex))
			return;

		ActionQueue[actionIndex] = QueueState::Clear;
	}

	void Register(sol::state* state, sol::table& parent)
	{
		auto tableInput = sol::table(state->lua_state(), sol::create);

		parent.set(ScriptReserved_Input, tableInput);
		tableInput.set_function(ScriptReserved_Vibrate, &Vibrate);
		tableInput.set_function(ScriptReserved_KeyIsHeld, &KeyIsHeld);
		tableInput.set_function(ScriptReserved_KeyIsHit, &KeyIsHit);
		tableInput.set_function(ScriptReserved_KeyPush, &KeyPush);
		tableInput.set_function(ScriptReserved_KeyClear, &KeyClear);

		auto handler = LuaHandler(state);
		handler.MakeReadOnlyTable(tableInput, ScriptReserved_ActionID, ACTION_IDS);
	}
}
