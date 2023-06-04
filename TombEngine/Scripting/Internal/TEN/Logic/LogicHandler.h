#pragma once
#include <unordered_set>

#include "Game/items.h"
#include "Scripting/Include/ScriptInterfaceGame.h"
#include "Scripting/Internal/LuaHandler.h"

enum class CallbackPoint;
class LevelFunc;

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

	sol::protected_function	m_onStart{};
	sol::protected_function	m_onLoad{};
	sol::protected_function	m_onControlPhase{};
	sol::protected_function	m_onSave{};
	sol::protected_function	m_onEnd{};

	std::unordered_set<std::string> m_callbacksPreSave;
	std::unordered_set<std::string> m_callbacksPostSave;
	std::unordered_set<std::string> m_callbacksPreLoad;
	std::unordered_set<std::string> m_callbacksPostLoad;
	std::unordered_set<std::string> m_callbacksPreStart;
	std::unordered_set<std::string> m_callbacksPostStart;
	std::unordered_set<std::string> m_callbacksPreEnd;
	std::unordered_set<std::string> m_callbacksPostEnd;
	std::unordered_set<std::string> m_callbacksPreControl;
	std::unordered_set<std::string> m_callbacksPostControl;

	std::unordered_map<CallbackPoint, std::unordered_set<std::string> *> m_callbacks;

	std::vector<std::variant<std::string, uint32_t>> m_savedVarPath;

	bool m_shortenedCalls = false;

	std::string GetRequestedPath() const;

	void ResetLevelTables();
	void ResetGameTables();
	LuaHandler m_handler;

public:	
	LogicHandler(sol::state* lua, sol::table& parent);

	template <typename ... Ts> sol::protected_function_result CallLevelFuncBase(const sol::protected_function & func, Ts ... vs)
	{
		auto funcResult = func.call(vs...);
		return funcResult;
	}

	template <typename ... Ts> sol::protected_function_result CallLevelFuncByName(const std::string& name, Ts ... vs)
	{
		auto func = m_levelFuncs_luaFunctions[name];
		auto funcResult = CallLevelFuncBase(func, vs...);

		if (!funcResult.valid())
		{
			sol::error err = funcResult;
			ScriptAssertF(false, "Could not execute function {}: {}", name, err.what());
		}

		return funcResult;
	}

	template <typename ... Ts> sol::protected_function_result CallLevelFunc(const sol::protected_function & func, Ts ... vs)
	{
		auto funcResult = CallLevelFuncBase(func, vs...);

		if (!funcResult.valid())
		{
			sol::error err = funcResult;
			ScriptAssertF(false, "Could not execute function: {}", err.what());
		}

		return funcResult;
	}

	void FreeLevelScripts() override;

	void LogPrint(sol::variadic_args args);
	bool SetLevelFuncsMember(sol::table tab, const std::string& name, sol::object value);

	void AddCallback(CallbackPoint point, const LevelFunc& levelFunc);
	void RemoveCallback(CallbackPoint point, const LevelFunc& levelFunc);

	void ResetScripts(bool clearGameVars) override;
	void ShortenTENCalls() override;

	sol::object GetLevelFuncsMember(sol::table tab, const std::string& name);

	void ExecuteScriptFile(const std::string& luaFilename) override;
	void ExecuteString(const std::string& command) override;
	void ExecuteFunction(const std::string& name, TEN::Control::Volumes::VolumeActivator, const std::string& arguments) override;

	void ExecuteFunction(const std::string& name, short idOne, short idTwo) override;

	void GetVariables(std::vector<SavedVar>& vars) override;
	void SetVariables(const std::vector<SavedVar>& vars) override;
	void ResetVariables();

	void SetCallbackStrings(const std::vector<std::string>& preStart,
							const std::vector<std::string>& postStart,
							const std::vector<std::string>& preEnd,
							const std::vector<std::string>& postEnd,
							const std::vector<std::string>& preSave,
							const std::vector<std::string>& postSave, 
							const std::vector<std::string>& preLoad,   
							const std::vector<std::string>& postLoad, 
							const std::vector<std::string>& preControl,   
							const std::vector<std::string>& posControl) override;

	void GetCallbackStrings(std::vector<std::string>& preStart,
							std::vector<std::string>& postStart,
							std::vector<std::string>& preEnd,
							std::vector<std::string>& postEnd,
							std::vector<std::string>& preSave,
							std::vector<std::string>& postSave,
							std::vector<std::string>& preLoad,
							std::vector<std::string>& postLoad,
							std::vector<std::string>& preControl,
							std::vector<std::string>& postControl) const override;

	void InitCallbacks() override;
	void OnStart() override;
	void OnLoad() override;
	void OnControlPhase(float deltaTime) override;
	void OnSave() override;
	void OnEnd(GameStatus reason) override;
};
