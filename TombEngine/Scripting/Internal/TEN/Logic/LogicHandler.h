#pragma once
#include "ScriptInterfaceGame.h"
#include "Game/items.h"
#include "LuaHandler.h"
#include <unordered_set>
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

class LogicHandler : public ScriptInterfaceGame
{
private:
	std::unordered_map<std::string, sol::protected_function>	m_levelFuncs{};
	sol::protected_function										m_onStart{};
	sol::protected_function										m_onLoad{};
	sol::protected_function										m_onControlPhase{};
	sol::protected_function										m_onSave{};
	sol::protected_function										m_onEnd{};

	void ResetLevelTables();
	void ResetGameTables();
	LuaHandler m_handler;

public:	
	LogicHandler(sol::state* lua, sol::table & parent);

	void								FreeLevelScripts() override;

	void								LogPrint(sol::variadic_args va);
	bool								SetLevelFunc(sol::table tab, std::string const& luaName, sol::object value);
	void								ResetScripts(bool clearGameVars) override;

	sol::protected_function				GetLevelFunc(sol::table tab, std::string const& luaName);

	void								ExecuteScriptFile(const std::string& luaFilename) override;
	void								ExecuteFunction(std::string const& name, TEN::Control::Volumes::VolumeTriggerer, std::string const& arguments) override;

	void								ExecuteFunction(std::string const& name, short idOne, short idTwo) override;

	void								GetVariables(std::vector<SavedVar>& vars) override;
	void								ResetVariables();

	void								SetVariables(std::vector<SavedVar> const& vars) override;
	void								InitCallbacks() override;
	void								OnStart() override;
	void								OnLoad() override;
	void								OnControlPhase(float dt) override;
	void								OnSave() override;
	void								OnEnd() override;
};
