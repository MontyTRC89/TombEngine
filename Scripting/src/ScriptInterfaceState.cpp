#include "frameworkandsol.h"
#include "ScriptInterfaceState.h"
#include "Logic/LogicHandler.h"
#include "Flow/FlowHandler.h"
#include "Objects/ObjectsHandler.h"
#include "Strings/StringsHandler.h"
#include "Inventory/InventoryHandler.h"
#include "ReservedScriptNames.h"

static sol::state s_solState;
static sol::table s_rootTable;

int lua_exception_handler(lua_State* L, sol::optional<std::exception const &> maybe_exception, sol::string_view description)
{
	return luaL_error(L, description.data());
}

ScriptInterfaceGame* ScriptInterfaceState::CreateGame()
{
	return new LogicHandler(&s_solState, s_rootTable);
}

ScriptInterfaceFlowHandler* ScriptInterfaceState::CreateFlow()
{
	return new FlowHandler(&s_solState, s_rootTable);
}

ScriptInterfaceObjectsHandler* ScriptInterfaceState::CreateObjectsHandler()
{
	return new ObjectsHandler(&s_solState, s_rootTable);
}

ScriptInterfaceStringsHandler* ScriptInterfaceState::CreateStringsHandler()
{
	return new StringsHandler(&s_solState, s_rootTable);
}

void ScriptInterfaceState::Init()
{
	s_solState.open_libraries(sol::lib::base, sol::lib::math);
	s_solState.set_exception_handler(lua_exception_handler);

	s_rootTable = sol::table{ s_solState.lua_state(), sol::create };
	s_solState.set(ScriptReserved_TEN, s_rootTable);

	// Misc handlers not assigned above
	InventoryHandler::Register(&s_solState, s_rootTable);
}

