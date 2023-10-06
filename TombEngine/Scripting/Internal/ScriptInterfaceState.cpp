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

static sol::state s_solState;
static sol::table s_rootTable;

int lua_exception_handler(lua_State* luaStatePtr, sol::optional<const std::exception&> exception, sol::string_view description)
{
	return luaL_error(luaStatePtr, description.data());
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

void ScriptInterfaceState::Init(const std::string& assetsDir)
{
	s_solState.open_libraries(
		sol::lib::base, sol::lib::math, sol::lib::package, sol::lib::coroutine,
		sol::lib::table, sol::lib::string, sol::lib::debug);

	s_solState.script("package.path=\"" + assetsDir + "Scripts/?.lua\"");
	s_solState.set_exception_handler(lua_exception_handler);

	s_rootTable = sol::table{ s_solState.lua_state(), sol::create };
	s_solState.set(ScriptReserved_TEN, s_rootTable);

	// Misc. handlers not assigned above.
	InventoryHandler::Register(&s_solState, s_rootTable);
	Misc::Register(&s_solState, s_rootTable);
	Effects::Register(&s_solState, s_rootTable);

	ScriptDisplaySprite::RegisterTables(&s_solState, s_rootTable);
}
