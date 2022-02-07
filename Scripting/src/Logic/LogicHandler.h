#pragma once
#include "Scripting/ScriptInterfaceGame.h"
#include "Game/items.h"
#include "Game/room.h"
#include "LuaHandler.h"
#include "Specific/trmath.h"
#include <unordered_set>
#include "Color/Color.h"
#include "GameScriptPosition.h"
#include "GameScriptRotation.h"
#include "Strings/StringsHandler.h"

struct LuaFunction {
	std::string Name;
	std::string Code;
	bool Executed;
};

struct GameScriptVector3 {
	float x;
	float y;
	float z;
};


class LuaVariables
{
public:
	std::unordered_map<std::string, sol::object>			variables;

	sol::object							GetVariable(sol::table tab, std::string key);
	void								SetVariable(sol::table tab, std::string key, sol::object value);
};

struct LuaVariable
{
	bool IsGlobal;
	std::string Name;
	int Type;
	float FloatValue;
	int IntValue;
	std::string StringValue;
	bool BoolValue;
};

class LogicHandler : public LuaHandler, public ScriptInterfaceGame
{
private:
	LuaVariables												m_globals{};
	LuaVariables												m_locals{};
	std::unordered_map<std::string, sol::protected_function>	m_levelFuncs{};
	sol::protected_function										m_onStart{};
	sol::protected_function										m_onLoad{};
	sol::protected_function										m_onControlPhase{};
	sol::protected_function										m_onSave{};
	sol::protected_function										m_onEnd{};

	void ResetLevelTables();

public:	
	LogicHandler(sol::state* lua, sol::table & parent);

	void								FreeLevelScripts() override;

	bool								SetLevelFunc(sol::table tab, std::string const& luaName, sol::object obj);
	sol::protected_function				GetLevelFunc(sol::table tab, std::string const& luaName);

	void								ExecuteScriptFile(const std::string& luaFilename) override;
	void								ExecuteFunction(std::string const & name) override;
	void								MakeItemInvisible(short id);


	// Variables
	template <typename T>
	void								GetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals);
	template <typename T>
	void								SetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals);
	void								ResetVariables();

	void								InitCallbacks() override;
	void								OnStart() override;
	void								OnLoad() override;
	void								OnControlPhase(float dt) override;
	void								OnSave() override;
	void								OnEnd() override;
};
