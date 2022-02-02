#include "frameworkandsol.h"
#include "ScriptInterfaceState.h"
#include "GameLogicScript.h"
#include "GameFlowScript.h"
#include "Entity/Entity.h"

sol::state g_solState;

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
	return new GameFlow(&g_solState);
}

ScriptInterfaceEntity* CreateEntities()
{
	return new GameEntities(&g_solState);
}

void ScriptInterfaceState::Init()
{
	g_solState.open_libraries(sol::lib::base, sol::lib::math);
	g_solState.set_exception_handler(lua_exception_handler);
}

