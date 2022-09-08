#pragma once
#include "ScriptInterfaceGame.h"
#include "Game/items.h"
#include "LuaHandler.h"
#include <unordered_set>

class LevelFunc;
enum class CallbackPoint;

class LogicHandler : public ScriptInterfaceGame
{
private:
	// Hierarchy of tables.
	//
	// For example:
	// LevelFuncs
	// |-	Engine
	//		|-	Timer
	//		|-	Util
	// |-	External
	//		|-	EnemiesBySteve
	//		|-	BetterEnemiesByChris
	//			|-	SubTable
	//
	// Each of these tables can only contain other tables as well as a string with their "path".
	// For example, the SubTable table will have a path of "LevelFuncs.Ext.MySecondLib.SubTable".
	// It uses this to construct the full path name of any functions that end up in m_levelFuncs_luaFunctions.
	//
	// Each of these has a metatable whose __index metamethod looks in m_levelFuncsTables, using the path
	// as the key, for the full name of the function. It then gets the FuncNameHolder from m_levelFuncsFakeFuncs,
	// and that FuncNameHolder's __call metamethod looks in m_levelFuncs_luaFunctions for the real function.
	sol::table m_levelFuncs{};

	// Maps full function paths into Lua functions.
	std::unordered_map<std::string, sol::protected_function> m_levelFuncs_luaFunctions{};

	// Maps full function paths to LevelFunc objects.
	// This is a table instead of a C++ container to more easily interface with Sol.
	sol::table m_levelFuncs_levelFuncObjects{};

	// Contains tables; each table refers to a table in the LevelFuncs hierarchy, and contains the full names
	// of the functions to index in m_levelFuncs_luaFunctions.
	// Tables are non-nested, so the following are all at the base level of m_levelFuncsTables.
	// "LevelFuncs"
	// "LevelFuncs.Engine"
	// "LevelFuncs.Engine.Util"
	// "LevelFuncs.MyLevel"
	// "LevelFuncs.MyLevel.CoolFuncs"
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_levelFuncs_tablesOfNames{};

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

	void								CallLevelFunc(std::string const &, sol::variadic_args);
	void								CallLevelFunc(std::string const &, float dt);

	void								FreeLevelScripts() override;

	void								LogPrint(sol::variadic_args va);
	bool								SetLevelFuncsMember(sol::table tab, std::string const& luaName, sol::object value);

	void								AddCallback(CallbackPoint point, LevelFunc const & lf);
	void								RemoveCallback(CallbackPoint point, LevelFunc const & lf);

	void								ResetScripts(bool clearGameVars) override;

	sol::object							GetLevelFuncsMember(sol::table tab, std::string const& luaName);

	void								ExecuteScriptFile(const std::string& luaFilename) override;
	void								ExecuteString(const std::string& command) override;
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
