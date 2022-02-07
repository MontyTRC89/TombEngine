#include "frameworkandsol.h"
#include "LogicHandler.h"
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
#include "GameScriptFreeFunctions.h"
#include "ReservedScriptNames.h"
#include "Game/camera.h"
#include "ScriptUtil.h"

/***
Functions and callbacks for level-specific logic scripts.
@tentable Misc 
@pragma nostrip
*/


LogicHandler::LogicHandler(sol::state* lua, sol::table & parent) : LuaHandler{ lua }
{
	sol::table table_logic{ m_lua->lua_state(), sol::create };
	parent.set(ScriptReserved_Strings, table_logic);

	MakeReadOnlyTable(ScriptReserved_DisplayStringOption, kDisplayStringOptionNames);

	ResetLevelTables();

	MakeSpecialTable(m_lua, ScriptReserved_GameVars, &LuaVariables::GetVariable, &LuaVariables::SetVariable, &m_globals);

	m_lua->new_enum<GAME_OBJECT_ID>("Object", {
		{"LARA", ID_LARA}
		});
}

void LogicHandler::ResetLevelTables()
{
	MakeSpecialTable(m_lua, ScriptReserved_LevelFuncs, &LogicHandler::GetLevelFunc, &LogicHandler::SetLevelFunc, this);
	MakeSpecialTable(m_lua, ScriptReserved_LevelVars, &LuaVariables::GetVariable, &LuaVariables::SetVariable, &m_locals);
}

sol::protected_function LogicHandler::GetLevelFunc(sol::table tab, std::string const& luaName)
{
	if (m_levelFuncs.find(luaName) == m_levelFuncs.end())
		return sol::lua_nil;

	return m_levelFuncs.at(luaName);
}

bool LogicHandler::SetLevelFunc(sol::table tab, std::string const& luaName, sol::object value)
{
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
}

void LogicHandler::FreeLevelScripts()
{
	m_levelFuncs.clear();
	m_locals = LuaVariables{};
	ResetLevelTables();
	m_onStart = sol::nil;
	m_onLoad = sol::nil;
	m_onControlPhase = sol::nil;
	m_onSave = sol::nil;
	m_onEnd = sol::nil;
	m_lua->collect_garbage();
}

void JumpToLevel(int levelNum)
{
	if (levelNum >= g_GameFlow->GetNumLevels())
		return;
	LevelComplete = levelNum;
}

int GetSecretsCount()
{
	return Statistics.Level.Secrets;
}

void SetSecretsCount(int secretsNum)
{
	if (secretsNum > 255)
		return;
	Statistics.Level.Secrets = secretsNum;
}

void AddOneSecret()
{
	if (Statistics.Level.Secrets >= 255)
		return;
	Statistics.Level.Secrets++;
	PlaySecretTrack();
}

/*
void LogicHandler::MakeItemInvisible(short id)
{
	if (m_itemsMap.find(id) == m_itemsMap.end())
		return;

	short itemNum = m_itemsMap[id];

	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (item->active)
	{
		if (Objects[item->objectNumber].intelligent)
		{
			if (item->status == ITEM_ACTIVE)
			{
				item->touchBits = 0;
				item->status = ITEM_INVISIBLE;
				DisableBaddieAI(itemNum);
			}
		}
		else
		{
			item->touchBits = 0;
			item->status = ITEM_INVISIBLE;
		}
	}
}
*/
template <typename T>
void LogicHandler::GetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals)
{
	for (const auto& it : m_locals.variables)
	{
		if (it.second.is<T>())
			locals.insert(std::pair<std::string, T>(it.first, it.second.as<T>()));
	}
	for (const auto& it : m_globals.variables)
	{
		if (it.second.is<T>())
			globals.insert(std::pair<std::string, T>(it.first, it.second.as<T>()));
	}
}

template void LogicHandler::GetVariables<bool>(std::map<std::string, bool>& locals, std::map<std::string, bool>& globals);
template void LogicHandler::GetVariables<float>(std::map<std::string, float>& locals, std::map<std::string, float>& globals);
template void LogicHandler::GetVariables<std::string>(std::map<std::string, std::string>& locals, std::map<std::string, std::string>& globals);

template <typename T>
void LogicHandler::SetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals)
{
	//TODO Look into serialising tables from these maps, too -- squidshire, 24/08/2021
	m_locals.variables.clear();
	for (const auto& it : locals)
	{
		m_locals.variables.insert(std::pair<std::string, sol::object>(it.first, sol::object(m_lua->lua_state(), sol::in_place, it.second)));
	}
	for (const auto& it : globals)
	{
		m_globals.variables.insert(std::pair<std::string, sol::object>(it.first, sol::object(m_lua->lua_state(), sol::in_place, it.second)));
	}
}

template void LogicHandler::SetVariables<bool>(std::map<std::string, bool>& locals, std::map<std::string, bool>& globals);
template void LogicHandler::SetVariables<float>(std::map<std::string, float>& locals, std::map<std::string, float>& globals);
template void LogicHandler::SetVariables<std::string>(std::map<std::string, std::string>& locals, std::map<std::string, std::string>& globals);

template <typename R, char const * S, typename mapType>
std::unique_ptr<R> GetByName(std::string const & type, std::string const & name, mapType const & map)
{
	ScriptAssert(map.find(name) != map.end(), std::string{ type + " name not found: " + name }, ERROR_MODE::TERMINATE);
	return std::make_unique<R>(map.at(name), false);
}


/*** Special objects
@section specialobjects
*/

/*** An @{ItemInfo} representing Lara herself.
@table Lara
*/
void LogicHandler::ResetVariables()
{
	(*m_lua)["Lara"] = NULL;
}


sol::object LuaVariables::GetVariable(sol::table tab, std::string key)
{
	if (variables.find(key) == variables.end())
		return sol::lua_nil;
	return variables[key];
}

void LuaVariables::SetVariable(sol::table tab, std::string key, sol::object value)
{
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
}

void LogicHandler::ExecuteScriptFile(const std::string & luaFilename)
{
	ExecuteScript(luaFilename);
}

void LogicHandler::ExecuteFunction(std::string const & name)
{
	sol::protected_function func = (*m_lua)["LevelFuncs"][name.c_str()];
	auto r = func();
	if (!r.valid())
	{
		sol::error err = r;
		ScriptAssertF(false, "Could not execute function {}: {}", name, err.what());
	}
}

static void doCallback(sol::protected_function const & func, std::optional<float> dt = std::nullopt)  {
	auto r = dt.has_value() ? func(dt) : func();

	if (!r.valid())
	{
		sol::error err = r;
		ScriptAssert(false, err.what(), ERROR_MODE::TERMINATE);
	}
}

void LogicHandler::OnStart()
{
	if (m_onStart.valid())
		doCallback(m_onStart);
}

void LogicHandler::OnLoad()
{
	if(m_onLoad.valid())
		doCallback(m_onLoad);
}

void LogicHandler::OnControlPhase(float dt)
{
	if(m_onControlPhase.valid())
		doCallback(m_onControlPhase, dt);
}

void LogicHandler::OnSave()
{
	if(m_onSave.valid())
		doCallback(m_onSave);
}

void LogicHandler::OnEnd()
{
	if(m_onEnd.valid())
		doCallback(m_onEnd);
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

	if GameVars.playerSnoopedInDraws then
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
}
