#include "frameworkandsol.h"
#include "LogicHandler.h"

#if TEN_OPTIONAL_LUA
#include "ScriptAssert.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/Lara/lara.h"
#include "Game/savegame.h"
#include "Game/control/lot.h"
#include "Sound/sound.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/effects.h"
#include "Game/pickup/pickup.h"
#include "Game/gui.h"
#include "ReservedScriptNames.h"
#include "Game/camera.h"
#include <Renderer/Renderer11Enums.h>
#include "Game/effects/lightning.h"
#include "ScriptUtil.h"

using namespace TEN::Effects::Lightning;

/***
Functions and callbacks for level-specific logic scripts.
@tentable Logic 
@pragma nostrip
*/

#endif

LogicHandler::LogicHandler(sol::state* lua, sol::table & parent) : LuaHandler{ lua }
{
#if TEN_OPTIONAL_LUA
	ResetLevelTables();

	MakeSpecialTable(m_lua, ScriptReserved_GameVars, &LuaVariables::GetVariable, &LuaVariables::SetVariable, &m_globals);
	m_lua->new_enum<GAME_OBJECT_ID>("Object", {
		{"LARA", ID_LARA}
		});
#endif
}

void LogicHandler::ResetLevelTables()
{
#if TEN_OPTIONAL_LUA
	MakeSpecialTable(m_lua, ScriptReserved_LevelFuncs, &LogicHandler::GetLevelFunc, &LogicHandler::SetLevelFunc, this);
	MakeSpecialTable(m_lua, ScriptReserved_LevelVars, &LuaVariables::GetVariable, &LuaVariables::SetVariable, &m_locals);
#endif
}

sol::protected_function LogicHandler::GetLevelFunc(sol::table tab, std::string const& luaName)
{
#if TEN_OPTIONAL_LUA
	if (m_levelFuncs.find(luaName) == m_levelFuncs.end())
		return sol::lua_nil;

	return m_levelFuncs.at(luaName);
#else
	return sol::nil;
#endif
}

bool LogicHandler::SetLevelFunc(sol::table tab, std::string const& luaName, sol::object value)
{
#if TEN_OPTIONAL_LUA
	switch (value.get_type())
	{
	case sol::type::lua_nil:
		m_levelFuncs.erase(luaName);
		break;
	case sol::type::function:
		m_levelFuncs.insert_or_assign(luaName, value.as<sol::protected_function>());
		break;
	default:
		//todo When we save the game, do we save the functions or just the names?
		//todo It may be better just to save the names so that we can load the callbacks
		//todo from the level script each time (vital if the builder updates their
		//todo scripts after release -- squidshire, 31/08/2021
		std::string error{ "Could not assign LevelFuncs." };
		error += luaName + "; it must be a function (or nil).";
		return ScriptAssert(false, error);
	}
	return true;
#else
	return true;
#endif
}

void LogicHandler::FreeLevelScripts()
{
#if TEN_OPTIONAL_LUA
	m_levelFuncs.clear();
	m_locals = LuaVariables{};
	ResetLevelTables();
	m_onStart = sol::nil;
	m_onLoad = sol::nil;
	m_onControlPhase = sol::nil;
	m_onSave = sol::nil;
	m_onEnd = sol::nil;
	m_lua->collect_garbage();
#endif
}

void JumpToLevel(int levelNum)
{
#if TEN_OPTIONAL_LUA
	if (levelNum >= g_GameFlow->GetNumLevels())
		return;

	LevelComplete = levelNum;
#endif
}

int GetSecretsCount()
{
#if TEN_OPTIONAL_LUA
	return Statistics.Level.Secrets;
#else
	return 0;
#endif
}

void SetSecretsCount(int secretsNum)
{
#if TEN_OPTIONAL_LUA
	if (secretsNum > 255)
		return;
	Statistics.Level.Secrets = secretsNum;
#endif
}

void AddOneSecret()
{
#if TEN_OPTIONAL_LUA
	if (Statistics.Level.Secrets >= 255)
		return;
	Statistics.Level.Secrets++;
	PlaySecretTrack();
#endif
}


void LogicHandler::SetVariables(std::map<std::string, VarSaveType> const & locals, std::map<std::string, VarSaveType> const & globals)
{
#if TEN_OPTIONAL_LUA
	m_locals.variables.clear();
	for (const auto& it : locals)
	{
		m_locals.variables.insert(std::pair<std::string, sol::object>(it.first, sol::object(m_lua->lua_state(), sol::in_place, it.second)));
	}
	m_globals.variables.clear();
	for (const auto& it : globals)
	{
		m_globals.variables.insert(std::pair<std::string, sol::object>(it.first, sol::object(m_lua->lua_state(), sol::in_place, it.second)));
	}
#endif
}

void LogicHandler::GetVariables(std::map<std::string, VarSaveType>& locals, std::map<std::string, VarSaveType>& globals) const
{
#if TEN_OPTIONAL_LUA
	for (const auto& it : m_locals.variables)
	{
		locals.insert(std::pair<std::string, VarSaveType>(it.first, it.second.as<VarSaveType>()));
	}
	for (const auto& it : m_globals.variables)
	{
		globals.insert(std::pair<std::string, VarSaveType>(it.first, it.second.as<VarSaveType>()));
	}
#endif
}


template <typename R, char const * S, typename mapType>
std::unique_ptr<R> GetByName(std::string const & type, std::string const & name, mapType const & map)
{
#if TEN_OPTIONAL_LUA
	ScriptAssert(map.find(name) != map.end(), std::string{ type + " name not found: " + name }, ERROR_MODE::TERMINATE);
	return std::make_unique<R>(map.at(name), false);
#endif
}

/*** Special objects
@section specialobjects
*/

/*** An @{ItemInfo} representing Lara herself.
@table Lara
*/
void LogicHandler::ResetVariables()
{
#if TEN_OPTIONAL_LUA
	(*m_lua)["Lara"] = NULL;
#endif
}



sol::object LuaVariables::GetVariable(sol::table tab, std::string key)
{
#if TEN_OPTIONAL_LUA
	if (variables.find(key) == variables.end())
		return sol::lua_nil;

	return variables[key];
#else
	return sol::nil;
#endif
}

void LuaVariables::SetVariable(sol::table tab, std::string key, sol::object value)
{
#if TEN_OPTIONAL_LUA
	switch (value.get_type())
	{
	case sol::type::lua_nil:
		variables.erase(key);
		break;
	case sol::type::boolean:
	case sol::type::number:
	case sol::type::string:
		variables[key] = value;
		break;
	default:
		ScriptAssert(false, "Variable " + key + " has an unsupported type.", ERROR_MODE::TERMINATE);
		break;
	}
#endif
}

void LogicHandler::ExecuteScriptFile(const std::string & luaFilename)
{
#if TEN_OPTIONAL_LUA
	ExecuteScript(luaFilename);
#endif
}

void LogicHandler::ExecuteFunction(std::string const & name)
{
#if TEN_OPTIONAL_LUA
	sol::protected_function func = (*m_lua)["LevelFuncs"][name.c_str()];
	auto r = func();
	if (!r.valid())
	{
		sol::error err = r;
		ScriptAssertF(false, "Could not execute function {}: {}", name, err.what());
	}
#endif
}

#if TEN_OPTIONAL_LUA
static void doCallback(sol::protected_function const & func, std::optional<float> dt = std::nullopt)  {
	auto r = dt.has_value() ? func(dt) : func();

	if (!r.valid())
	{
		sol::error err = r;
		ScriptAssert(false, err.what(), ERROR_MODE::TERMINATE);
	}
}
#endif

void LogicHandler::OnStart()
{
#if TEN_OPTIONAL_LUA
	if (m_onStart.valid())
		doCallback(m_onStart);
#endif
}

void LogicHandler::OnLoad()
{
#if TEN_OPTIONAL_LUA
	if(m_onLoad.valid())
		doCallback(m_onLoad);
#endif
}

void LogicHandler::OnControlPhase(float dt)
{
#if TEN_OPTIONAL_LUA
	if(m_onControlPhase.valid())
		doCallback(m_onControlPhase, dt);
#endif
}

void LogicHandler::OnSave()
{
#if TEN_OPTIONAL_LUA
	if(m_onSave.valid())
		doCallback(m_onSave);
#endif
}

void LogicHandler::OnEnd()
{
#if TEN_OPTIONAL_LUA
	if(m_onEnd.valid())
		doCallback(m_onEnd);
#endif
}

/*** Special tables

TombEngine uses the following tables for specific things.

@section levelandgametables
*/

/*** A table with level-specific data which will be saved and loaded.
This is for level-specific information that you want to store in saved games.

For example, you may have a level with a custom puzzle where Lara has
to kill exactly seven enemies to open a door to a secret. You could use
the following line each time an enemy is killed:

	LevelVars.enemiesKilled = LevelVars.enemiesKilled + 1

If the player saves the level after killing three, saves, and then reloads the save
some time later, the values `3` will be put back into `LevelVars.enemiesKilled.`

__This table is emptied when a level is finished.__ If the player needs to be able
to return to the level (like in the Karnak and Alexandria levels in *The Last Revelation*),
you will need to use the @{GameVars} table, below.
@table LevelVars
*/

/*** A table with game data which will be saved and loaded.
This is for information not specific to any level, but which concerns your whole
levelset or game, that you want to store in saved games.

For example, you may wish to have a final boss say a specific voice line based on
a choice the player made in a previous level. In the level with the choice, you could
write:

	GameVars.playerSnoopedInDraws = true

And in the script file for the level with the boss, you could write:

	if GameVars.playerSnoopedInDrawers then
		PlayAudioTrack("how_dare_you.wav")
	end

Unlike @{LevelVars}, this table will remain intact for the entirety of the game.
@table GameVars
*/

/*** A table with level-specific functions.

This serves two purposes: it holds the level callbacks (listed below) as well as
any trigger functions you might have specified. For example, if you give a trigger
a Lua name of "my_trigger" in Tomb Editor, you will have to implement it as a member
of this table:

	LevelFuncs.my_trigger = function() 
		-- implementation goes here
	end

The following are the level callbacks. They are optional; if your level has no special
behaviour for a particular scenario, you do not need to implement the function. For
example, if your level does not need any special initialisation when it is loaded,
you can just leave out `LevelFuncs.OnStart`.

@tfield function OnStart Will be called when a level is loaded
@tfield function OnLoad Will be called when a saved game is loaded
@tfield function(float) OnControlPhase Will be called during the game's update loop,
and provides the delta time (a float representing game time since last call) via its argument.
@tfield function OnSave Will be called when the player saves the game
@tfield function OnEnd Will be called when leaving a level. This includes finishing it, exiting to the menu, or loading a save in a different level. 
@table LevelFuncs
*/

void LogicHandler::InitCallbacks()
{
#if TEN_OPTIONAL_LUA
	auto assignCB = [this](sol::protected_function& func, std::string const & luaFunc) {
		std::string fullName = "LevelFuncs." + luaFunc;
		func = (*m_lua)["LevelFuncs"][luaFunc];
		std::string err{ "Level's script does not define callback " + fullName};
		if (!ScriptAssert(func.valid(), err)) {
			ScriptWarn("Defaulting to no " + fullName + " behaviour.");
		}
	};

	assignCB(m_onStart, "OnStart");
	assignCB(m_onLoad, "OnLoad");
	assignCB(m_onControlPhase, "OnControlPhase");
	assignCB(m_onSave, "OnSave");
	assignCB(m_onEnd, "OnEnd");
#endif
}
