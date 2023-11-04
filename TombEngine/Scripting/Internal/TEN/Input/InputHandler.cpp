#include "framework.h"
#include "Scripting/Internal/TEN/Input/InputHandler.h"

#include "Renderer/Renderer11Enums.h"
#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Input/ActionIDs.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"
#include "Specific/Input/Input.h"

using namespace TEN::Input;

/// Functions for input management.
// @tentable Input
// @pragma nostrip

namespace Input
{
	/// Vibrate the game controller if the function is available and the setting is on.
	// @function Vibrate
	// @tparam float strength Vibration strength.
	// @tparam float time __(default 0.3)__ Vibration time in seconds.
	static void Vibrate(float strength, sol::optional<float> time)
	{
		Rumble(strength, time.value_or(0.3f), RumbleMode::Both);
	}

	static bool CheckInput(int actionID)
	{
		if (actionID > (int)ActionID::Count)
		{
			ScriptAssertF(false, "Input action {} does not exist.", actionID);
			return false;
		}

		return true;
	}

	/// Check if an action key is being held.
	// @function KeyIsHeld
	// @tparam Input.ActionID action Action ID to check.
	static bool KeyIsHeld(int actionID)
	{
		if (!CheckInput(actionID))
			return false;

		if (IsHeld((ActionID)actionID))
			return true;

		return false;
	}

	/// Check if an action key is being hit or clicked.
	// @function KeyIsHit
	// @tparam Input.ActionID action Action ID to check.
	static bool KeyIsHit(int actionID)
	{
		if (!CheckInput(actionID))
			return false;

		if (IsClicked((ActionID)actionID))
			return true;

		return false;
	}

	/// Simulate an action key push.
	// @function KeyPush
	// @tparam Input.ActionID action Action ID to push.
	static void KeyPush(int actionID)
	{
		if (!CheckInput(actionID))
			return;

		ActionQueue[actionID] = QueueState::Push;
	}

	/// Clear an action key.
	// @function KeyClear
	// @tparam Input.ActionID action Action ID to clear.
	static void KeyClear(int actionID)
	{
		if (!CheckInput(actionID))
			return;

		ActionQueue[actionID] = QueueState::Clear;
	}

	/// Get the display position of the cursor in percent.
	// @function GetCursorDisplayPosition()
	// @treturn Vec2 Cursor display position in percent.
	static Vec2 GetCursorDisplayPosition()
	{
		// NOTE: Conversion from internal 800x600 to more intuitive 100x100 display space resolution is required.
		// In a future refactor, everything will use 100x100 natively. -- Sezz 2023.10.20

		auto cursorPos = TEN::Input::GetCursorDisplayPosition();
		cursorPos = Vector2(cursorPos.x / SCREEN_SPACE_RES.x, cursorPos.y / SCREEN_SPACE_RES.y) * 100;
		return Vec2(cursorPos);
	}

	void Register(sol::state* state, sol::table& parent)
	{
		auto table = sol::table(state->lua_state(), sol::create);

		parent.set(ScriptReserved_Input, table);
		table.set_function(ScriptReserved_Vibrate, &Vibrate);
		table.set_function(ScriptReserved_KeyIsHeld, &KeyIsHeld);
		table.set_function(ScriptReserved_KeyIsHit, &KeyIsHit);
		table.set_function(ScriptReserved_KeyPush, &KeyPush);
		table.set_function(ScriptReserved_KeyClear, &KeyClear);

		table.set_function(ScriptReserved_GetCursorDisplayPosition, &GetCursorDisplayPosition);

		auto handler = LuaHandler(state);
		handler.MakeReadOnlyTable(table, ScriptReserved_ActionID, ACTION_IDS);
	}
}
