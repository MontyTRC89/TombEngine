#include "framework.h"
#include "Scripting/Include/ScriptInterfaceState.h"

#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/TEN/DisplaySprite/ScriptDisplaySprite.h"
#include "Scripting/Internal/TEN/Effects/EffectsFunctions.h"
#include "Scripting/Internal/TEN/Flow/FlowHandler.h"
#include "Scripting/Internal/TEN/Inventory/InventoryHandler.h"
#include "Scripting/Internal/TEN/Logic/LogicHandler.h"
#include "Scripting/Internal/TEN/Misc/Miscellaneous.h"
#include "Scripting/Internal/TEN/Objects/ObjectsHandler.h"
#include "Scripting/Internal/TEN/Strings/StringsHandler.h"

using namespace TEN::Scripting::DisplaySprite;

static sol::state SolState;
static sol::table RootTable;

int lua_exception_handler(lua_State* luaStatePtr, sol::optional<const std::exception&> exception, sol::string_view description)
{
	return luaL_error(luaStatePtr, description.data());
}

ScriptInterfaceGame* ScriptInterfaceState::CreateGame()
{
	return new LogicHandler(&SolState, RootTable);
}

ScriptInterfaceFlowHandler* ScriptInterfaceState::CreateFlow()
{
	return new FlowHandler(&SolState, RootTable);
}

ScriptInterfaceObjectsHandler* ScriptInterfaceState::CreateObjectsHandler()
{
	return new ObjectsHandler(&SolState, RootTable);
}

ScriptInterfaceStringsHandler* ScriptInterfaceState::CreateStringsHandler()
{
	return new StringsHandler(&SolState, RootTable);
}

void ScriptInterfaceState::Init(const std::string& assetsDir)
{
	SolState.open_libraries(
		sol::lib::base, sol::lib::math, sol::lib::package, sol::lib::coroutine,
		sol::lib::table, sol::lib::string, sol::lib::debug);

	SolState.script("package.path=\"" + assetsDir + "Scripts/?.lua\"");
	SolState.set_exception_handler(lua_exception_handler);

	RootTable = sol::table(SolState.lua_state(), sol::create);
	SolState.set(ScriptReserved_TEN, RootTable);

	// Misc. handlers not assigned above.
	InventoryHandler::Register(&SolState, RootTable);
	Misc::Register(&SolState, RootTable);
	Effects::Register(&SolState, RootTable);
}
