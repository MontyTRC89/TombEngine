#pragma once
#include "ScriptInterfaceGame.h"
#include "Game/items.h"
#include "LuaHandler.h"
#include <unordered_set>
#include "Strings/StringsHandler.h"

enum class CallbackPoint;

class LogicHandler : public ScriptInterfaceGame
{
private:
	std::unordered_map<std::string, sol::protected_function>	m_levelFuncs{};
	sol::protected_function										m_onStart{};
	sol::protected_function										m_onLoad{};
	sol::protected_function										m_onControlPhase{};
	sol::protected_function										m_onSave{};
	sol::protected_function										m_onEnd{};

	std::unordered_set<std::string> m_callbacksPreControl;
	std::unordered_set<std::string> m_callbacksPostControl;

	void ResetLevelTables();
	void ResetGameTables();
	LuaHandler m_handler;

public:	
	LogicHandler(sol::state* lua, sol::table & parent);

	void								FreeLevelScripts() override;

	void								LogPrint(sol::variadic_args va);
	bool								SetLevelFunc(sol::table tab, std::string const& luaName, sol::object value);

	void								AddCallback(CallbackPoint point, std::string const& name);
	void								RemoveCallback(CallbackPoint point, std::string const & name);

	void								ResetScripts(bool clearGameVars) override;

	sol::protected_function				GetLevelFunc(sol::table tab, std::string const& luaName);

	void								ExecuteScriptFile(const std::string& luaFilename) override;
	void								ExecuteFunction(std::string const& name, TEN::Control::Volumes::VolumeTriggerer, std::string const& arguments) override;

	void								ExecuteFunction(std::string const& name, short idOne, short idTwo) override;

	void								GetVariables(std::vector<SavedVar>& vars) override;
	void								SetVariables(std::vector<SavedVar> const& vars) override;
	void								ResetVariables();

	void								SetCallbackStrings(std::vector<std::string> const& preControl, std::vector<std::string> const& postControl) override;
	void								GetCallbackStrings(std::vector<std::string>& preControl, std::vector<std::string>& postControl) const override;

	void								InitCallbacks() override;
	void								OnStart() override;
	void								OnLoad() override;
	void								OnControlPhase(float dt) override;
	void								OnSave() override;
	void								OnEnd() override;

};
