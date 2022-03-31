#pragma once
#include "Scripting/ScriptInterfaceGame.h"
#include "Game/items.h"
#include "Game/room.h"
#include "LuaHandler.h"
#include "Specific/trmath.h"
#include <unordered_set>
#include "Color/Color.h"
#include "Position/Position.h"
#include "Rotation/Rotation.h"
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

class LogicHandler : public LuaHandler, public ScriptInterfaceGame
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

public:	
	LogicHandler(sol::state* lua, sol::table & parent);

	void								FreeLevelScripts() override;

	bool								SetLevelFunc(sol::table tab, std::string const& luaName, sol::object obj);
	sol::protected_function				GetLevelFunc(sol::table tab, std::string const& luaName);

	void								ExecuteScriptFile(const std::string& luaFilename) override;
	void								ExecuteFunction(std::string const & name, TEN::Control::Volumes::VolumeTriggerer) override;

	void								GetVariables(std::vector<SavedVar>& vars) const override;
	void								ResetVariables();

	void								SetVariables(std::vector<SavedVar> const& vars) override;
	void								InitCallbacks() override;
	void								OnStart() override;
	void								OnLoad() override;
	void								OnControlPhase(float dt) override;
	void								OnSave() override;
	void								OnEnd() override;
};
