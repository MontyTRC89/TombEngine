#include "frameworkandsol.h"
#include "ScriptInterfaceState.h"
#include "GameLogicScript.h"
#include "Flow/Flow.h"
#include "Objects/ObjectsHandler.h"
#include "Strings/StringsHandler.h"
#include "ReservedScriptNames.h"

static sol::state g_solState;
static sol::table s_rootTable;

int lua_exception_handler(lua_State* L, sol::optional<std::exception const &> maybe_exception, sol::string_view description)
{
	return luaL_error(L, description.data());
}

ScriptInterfaceGame* ScriptInterfaceState::CreateGame()
{
	return new GameScript(&g_solState);
}

ScriptInterfaceFlow* ScriptInterfaceState::CreateFlow()
{
	return new Flow(&g_solState, s_rootTable);
}

ScriptInterfaceObjectsHandler* ScriptInterfaceState::CreateObjectsHandler()
{
	return new ObjectsHandler(&g_solState, s_rootTable);
}

ScriptInterfaceStringsHandler* ScriptInterfaceState::CreateStringsHandler()
{
	return new StringsHandler(&g_solState);
}

void ScriptInterfaceState::Init()
{
	g_solState.open_libraries(sol::lib::base, sol::lib::math);
	g_solState.set_exception_handler(lua_exception_handler);

	s_rootTable = sol::table{ g_solState.lua_state(), sol::create };
	g_solState.set(ScriptReserved_TEN, s_rootTable);
}

