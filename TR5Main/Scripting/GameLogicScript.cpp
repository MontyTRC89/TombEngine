#include "framework.h"
#include "GameLogicScript.h"
#include "ScriptAssert.h"
#include "items.h"
#include "box.h"
#include "lara.h"
#include "savegame.h"
#include "lot.h"
#include "sound.h"
#include "setup.h"
#include "level.h"
#include "tomb4fx.h"
#include "effect2.h"
#include "pickup.h"
#include "newinv2.h"
#include "InventorySlots.h"
#include "ObjectIDs.h"

#ifndef _DEBUG
#include <iostream>
#endif

/***
functions and callbacks for game specific scripts
@module gamelogic
@pragma nostrip
*/

extern GameFlow* g_GameFlow;

static void PlayAudioTrack(std::string const & trackName, bool looped)
{
	S_CDPlay(trackName, looped);
}

static void PlaySoundEffect(int id, GameScriptPosition p, int flags)
{
	PHD_3DPOS pos;

	pos.xPos = p.x;
	pos.yPos = p.y;
	pos.zPos = p.z;
	pos.xRot = 0;
	pos.yRot = 0;
	pos.zRot = 0;

	SoundEffect(id, &pos, flags);
}

static void PlaySoundEffect(int id, int flags)
{
	SoundEffect(id, NULL, flags);
}

static void SetAmbientTrack(std::string const & trackName)
{
	CurrentAtmosphere = trackName;
	S_CDPlay(CurrentAtmosphere, 1);
}

static int FindRoomNumber(GameScriptPosition pos)
{
	return 0;
}

static void AddLightningArc(GameScriptPosition src, GameScriptPosition dest, GameScriptColor color, int lifetime, int amplitude, int beamWidth, int segments, int flags)
{
	PHD_VECTOR p1;
	p1.x = src.x;
	p1.y = src.y;
	p1.z = src.z;

	PHD_VECTOR p2;
	p2.x = dest.x;
	p2.y = dest.y;
	p2.z = dest.z;

	TriggerLightning(&p1, &p2, amplitude, color.GetR(), color.GetG(), color.GetB(), lifetime, flags, beamWidth, segments);
}

static void AddShockwave(GameScriptPosition pos, int innerRadius, int outerRadius, GameScriptColor color, int lifetime, int speed, int angle, int flags)
{
	PHD_3DPOS p;
	p.xPos = pos.x;
	p.yPos = pos.y;
	p.zPos = pos.z;
	
	TriggerShockwave(&p, innerRadius, outerRadius, speed, color.GetR(), color.GetG(), color.GetB(), lifetime, FROM_DEGREES(angle), flags);
}

static void AddDynamicLight(GameScriptPosition pos, GameScriptColor color, int radius, int lifetime)
{
	TriggerDynamicLight(pos.x, pos.y, pos.z, radius, color.GetR(), color.GetG(), color.GetB());
}

static void AddBlood(GameScriptPosition pos, int num)
{
	TriggerBlood(pos.x, pos.y, pos.z, -1, num);
}

static void AddFireFlame(GameScriptPosition pos, int size)
{
	AddFire(pos.x, pos.y, pos.z, size, FindRoomNumber(pos), true);
}

static void Earthquake(int strength)
{
	Camera.bounce = -strength;
}

// Inventory
static void InventoryAdd(GAME_OBJECT_ID slot, sol::optional<int> count)
{
	PickedUpObject(slot, count.value_or(0));
}

static void InventoryRemove(GAME_OBJECT_ID slot, sol::optional<int> count)
{
	RemoveObjectFromInventory(static_cast<GAME_OBJECT_ID>(inventry_objects_list[slot].object_number), count.value_or(0));
}

static int InventoryGetCount(GAME_OBJECT_ID slot)
{
	return GetInventoryCount(slot);
}

static void InventorySetCount(GAME_OBJECT_ID slot, int count)
{
	// add the amount we'd need to add to get to count
	int currAmt = GetInventoryCount(slot);
	InventoryAdd(slot, count - currAmt);
}

static void InventoryCombine(int slot1, int slot2)
{
	
}

static void InventorySeparate(int slot)
{

}

// Misc
static void PrintString(std::string key, GameScriptPosition pos, GameScriptColor color, int lifetime, int flags)
{

}

GameScript::GameScript(sol::state* lua) : LuaHandler{ lua }, m_itemsMapId{}, m_itemsMapName{}, m_meshesMapName{}
{
/*** Ambience and music
@section Music
*/

/***
Set the named track as the ambient track, and start playing it
@function SetAmbientTrack
@tparam string name of track (without file extension) to play
*/
	m_lua->set_function("SetAmbientTrack", &SetAmbientTrack);

/***
Start playing the named track.
@function PlayAudioTrack
@tparam string name of track (without file extension) to play
@tparam bool loop if true, the track will loop; if false, it won't
*/
	m_lua->set_function("PlayAudioTrack", &PlayAudioTrack);

/*** Player inventory management
@section Inventory
*/

/***
Add x of a certain item to the inventory.
@function GiveInvItem
@tparam @{InvItem} item the item to be added
@tparam int count the number of items to add
*/
	m_lua->set_function("GiveInvItem", &InventoryAdd);

/***
Remove x of a certain item from the inventory.
@function TakeInvItem
@tparam @{InvItem} item the item to be removed
@tparam int count the number of items to remove
*/
	m_lua->set_function("TakeInvItem", &InventoryRemove);

/***
Get the amount the player holds of an item.
@function GetInvItemCount
@tparam @{InvItem} item the item to check
@return the amount of the item the player has in the inventory
*/
	m_lua->set_function("GetInvItemCount", &InventoryGetCount);

/***
Set the amount of a certain item the player has in the inventory.
Similar to @{GiveInvItem} but replaces with the new amount instead of adding it.
@function SetInvItemCount
@tparam @{InvItem} item the item to be set
@tparam int count the number of items the player will have 
*/
	m_lua->set_function("SetInvItemCount", &InventorySetCount);

/*** Game entity getters.
All Lua variables created with these functions will be non-owning.
This means that the actual in-game entity (object/camera/sink/whatever)
will _not_ be removed from the game if the Lua variable goes out of scope
or is destroyed in some other way.
@section getters
*/

/***
Get an ItemInfo by its name.
@function GetItemByName
@tparam string name the unique name of the item as set in, or generated by, Tomb Editor
@return a non-owning ItemInfo referencing the item.
*/
	m_lua->set_function("GetItemByName", &GameScript::GetItemByName, this);

/***
Get a MeshInfo by its name.
@function GetMeshByName
@tparam string name the unique name of the mesh as set in, or generated by, Tomb Editor
@return a non-owning MeshInfo referencing the mesh.
*/
	m_lua->set_function("GetMeshByName", &GameScript::GetMeshByName, this);

/***
Get a CameraInfo by its name.
@function GetCameraByName
@tparam string name the unique name of the camera as set in, or generated by, Tomb Editor
@return a non-owning CameraInfo referencing the camera.
*/
	m_lua->set_function("GetCameraByName", &GameScript::GetCameraByName, this);

/***
Get a SinkInfo by its name.
@function GetSinkByName
@tparam string name the unique name of the sink as set in, or generated by, Tomb Editor
@return a non-owning CameraInfo referencing the sink.
*/
	m_lua->set_function("GetSinkByName", &GameScript::GetSinkByName, this);

	auto makeReadOnlyTable = [this](std::string const & tableName, auto const& container)
	{
		auto mt = tableName + "Meta";
		// Put all the data in the metatable	
		m_lua->set(mt, sol::as_table(container));

		// Make the metatable's __index refer to itself so that requests
		// to the main table will go through to the metatable (and thus the
		// container's members)
		m_lua->safe_script(mt + ".__index = " + mt);

		// Don't allow the table to have new elements put into it
		m_lua->safe_script(mt + ".__newindex = function() error('" + tableName + " is read-only') end");

		// Protect the metatable
		m_lua->safe_script(mt + ".__metatable = 'metatable is protected'");

		auto tab = m_lua->create_named_table(tableName);
		m_lua->safe_script("setmetatable(" + tableName + ", " + mt + ")");

		// point the initial metatable variable away from its contents. this is just for cleanliness
		m_lua->safe_script(mt + "= nil");
	};

	makeReadOnlyTable("InvItem", kInventorySlots);
	makeReadOnlyTable("ObjID", kObjIDs);

	// LevelFuncs
	std::string LevelFuncsName{ "LevelFuncs" };
	std::string LevelFuncsNameMeta{ LevelFuncsName + "Meta" };
	auto meta = sol::table{ *m_lua, sol::create };
	m_lua->set(LevelFuncsNameMeta, meta);
	meta.set_function("__newindex", &GameScript::SetLevelFunc, this);
	meta.set("__metatable", "\"metatable is protected\"");
	auto tab = m_lua->create_named_table(LevelFuncsName);
	tab[sol::metatable_key] = meta;
	m_lua->set(LevelFuncsNameMeta, sol::nil);

	// Level
	std::string LevelName{ "Level" };
	std::string LevelNameMeta{ LevelName + "Meta" };
	meta = sol::table{ *m_lua, sol::create };
	m_lua->set(LevelNameMeta, meta);
	meta.set_function("__index", &LuaVariables::GetVariable, &m_locals);
	meta.set_function("__newindex", &LuaVariables::SetVariable, &m_locals);
	meta.set("__metatable", "\"metatable is protected\"");
	tab = m_lua->create_named_table(LevelName);
	tab[sol::metatable_key] = meta;
	m_lua->set(LevelNameMeta, sol::nil);

	// Game
	std::string GameName{ "Game" };
	std::string GameNameMeta{ GameName + "Meta" };
	meta = sol::table{ *m_lua, sol::create };
	m_lua->set(GameName, meta);
	meta.set_function("__index", &LuaVariables::GetVariable, &m_globals);
	meta.set_function("__newindex", &LuaVariables::SetVariable, &m_globals);
	meta.set("__metatable", "\"metatable is protected\"");
	tab = m_lua->create_named_table(GameName);
	tab[sol::metatable_key] = meta;
	m_lua->set(GameNameMeta, sol::nil);

	GameScriptItemInfo::Register(m_lua);
	GameScriptItemInfo::SetNameCallbacks(
		[this](std::string const& str, short num) {	return AddLuaNameItem(str, num); },
		[this](std::string const& str) { return RemoveLuaNameItem(str); }
	);

	GameScriptMeshInfo::Register(m_lua);
	GameScriptMeshInfo::SetNameCallbacks(
		[this](std::string const& str, MESH_INFO & info) {	return AddLuaNameMesh(str, info); },
		[this](std::string const& str) { return RemoveLuaNameMesh(str); }
	);

	GameScriptCameraInfo::Register(m_lua);
	GameScriptCameraInfo::SetNameCallbacks(
		[this](std::string const& str, LEVEL_CAMERA_INFO & info) {	return AddLuaNameCamera(str, info); },
		[this](std::string const& str) { return RemoveLuaNameCamera(str); }
	);

	GameScriptSinkInfo::Register(m_lua);
	GameScriptSinkInfo::SetNameCallbacks(
		[this](std::string const& str, SINK_INFO & info) {	return AddLuaNameSink(str, info); },
		[this](std::string const& str) { return RemoveLuaNameSink(str); }
	);

	GameScriptAIObject::Register(m_lua);
	GameScriptAIObject::SetNameCallbacks(
		[this](std::string const& str, AI_OBJECT & info) {	return AddLuaNameAIObject(str, info); },
		[this](std::string const& str) { return RemoveLuaNameAIObject(str); }
	);

	GameScriptSoundSourceInfo::Register(m_lua);
	GameScriptSoundSourceInfo::SetNameCallbacks(
		[this](std::string const& str, SOUND_SOURCE_INFO & info) {	return AddLuaNameSoundSource(str, info); },
		[this](std::string const& str) { return RemoveLuaNameSoundSource(str); }
	);

	GameScriptPosition::Register(m_lua);
	GameScriptRotation::Register(m_lua);
	GameScriptColor::Register(m_lua);

	m_lua->new_enum<GAME_OBJECT_ID>("Object", {
		{"LARA", ID_LARA}
		});
}

void GameScript::AddTrigger(LuaFunction* function)
{
	m_triggers.push_back(function);
	(*m_lua).script(function->Code);
}

bool GameScript::SetLevelFunc(sol::table tab, std::string const& luaName, sol::object value)
{
	switch (value.get_type())
	{
	case sol::type::lua_nil:
		m_levelFuncs.erase(luaName);
		tab.raw_set(luaName, value);
		break;
	case sol::type::function:
		m_levelFuncs[luaName];
		tab.raw_set(luaName, value);
		break;
	default:
		std::string error{ "Could not assign LevelFuncs." };
		error += luaName + "; it must be a function (or nil).";
		return ScriptAssert(false, error);
	}
	return true;
}

bool GameScript::RemoveLuaNameItem(std::string const & luaName)
{
	return m_itemsMapName.erase(luaName);
}

bool GameScript::AddLuaNameItem(std::string const & luaName, short itemNumber)
{
	return m_itemsMapName.insert(std::pair<std::string, short>(luaName, itemNumber)).second;
}

bool GameScript::RemoveLuaNameMesh(std::string const & luaName)
{
	return m_meshesMapName.erase(luaName);
}

bool GameScript::AddLuaNameMesh(std::string const& luaName, MESH_INFO& infoRef)
{
	return m_meshesMapName.insert(std::pair<std::string, MESH_INFO&>(luaName, infoRef)).second;
}

bool GameScript::RemoveLuaNameCamera(std::string const & luaName)
{
	return m_camerasMapName.erase(luaName);
}

bool GameScript::AddLuaNameCamera(std::string const& luaName, LEVEL_CAMERA_INFO& infoRef)
{
	return m_camerasMapName.insert(std::pair<std::string, LEVEL_CAMERA_INFO&>(luaName, infoRef)).second;
}

bool GameScript::RemoveLuaNameSink(std::string const & luaName)
{
	return m_sinksMapName.erase(luaName);
}

bool GameScript::AddLuaNameSink(std::string const& luaName, SINK_INFO& infoRef)
{
	return m_sinksMapName.insert(std::pair<std::string, SINK_INFO&>(luaName, infoRef)).second;
}
bool GameScript::RemoveLuaNameSoundSource(std::string const & luaName)
{
	return m_soundSourcesMapName.erase(luaName);
}

bool GameScript::AddLuaNameSoundSource(std::string const& luaName, SOUND_SOURCE_INFO& infoRef)
{
	return m_soundSourcesMapName.insert(std::pair<std::string, SOUND_SOURCE_INFO&>(luaName, infoRef)).second;
}

bool GameScript::RemoveLuaNameAIObject(std::string const & luaName)
{
	return m_aiObjectsMapName.erase(luaName);
}

bool GameScript::AddLuaNameAIObject(std::string const& luaName, AI_OBJECT & ref)
{
	return m_aiObjectsMapName.insert(std::pair<std::string, AI_OBJECT&>(luaName, ref)).second;
}

void GameScript::FreeLevelScripts()
{
	/*
	// Delete all triggers
	for (int i = 0; i < m_triggers.size(); i++)
	{
		LuaFunction* trigger = m_triggers[i];
		char* name = (char*)trigger->Name.c_str();
		(*m_lua)[name] = NULL;
		delete m_triggers[i];
	}
	m_triggers.clear();

	// Clear the items mapping
	m_itemsMap.clear();

	(*m_lua)["Lara"] = NULL;
	//delete m_Lara;
	*/
}

bool GameScript::ExecuteTrigger(short index)
{
	return true;
	/*
	// Is this a valid trigger?
	if (index >= m_triggers.size())
		return true;

	LuaFunction* trigger = m_triggers[index];

	// We want to execute a trigger just one time 
	// TODO: implement in the future continoous trigger?
	if (trigger->Executed)
		return true;

	// Get the trigger function name
	char* name = (char*)trigger->Name.c_str();

	// Execute trigger
	bool result = (*m_lua)[name]();

	// Trigger was executed, don't execute it anymore
	trigger->Executed = result;

	m_locals.for_each([&](sol::object const& key, sol::object const& value) {
		if (value.is<bool>())
			std::cout << key.as<std::string>() << " " << value.as<bool>() << std::endl;
		else if (value.is<std::string>())
			std::cout << key.as<std::string>() << " " << value.as<std::string>() << std::endl;
		else
			std::cout << key.as<std::string>() << " " << value.as<int>() << std::endl;		
	});

	return result;
	*/
}

void JumpToLevel(int levelNum)
{
	if (levelNum >= g_GameFlow->GetNumLevels())
		return;
	LevelComplete = levelNum;
}

int GetSecretsCount()
{
	return Savegame.Level.Secrets;
}

void SetSecretsCount(int secretsNum)
{
	if (secretsNum > 255)
		return;
	Savegame.Level.Secrets = secretsNum;
}

void AddOneSecret()
{
	if (Savegame.Level.Secrets >= 255)
		return;
	Savegame.Level.Secrets++;
	S_CDPlay(TRACK_FOUND_SECRET, 0);
}

/*
void GameScript::MakeItemInvisible(short id)
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
void GameScript::GetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals)
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

template void GameScript::GetVariables<bool>(std::map<std::string, bool>& locals, std::map<std::string, bool>& globals);
template void GameScript::GetVariables<float>(std::map<std::string, float>& locals, std::map<std::string, float>& globals);
template void GameScript::GetVariables<std::string>(std::map<std::string, std::string>& locals, std::map<std::string, std::string>& globals);

template <typename T>
void GameScript::SetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals)
{
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

template void GameScript::SetVariables<bool>(std::map<std::string, bool>& locals, std::map<std::string, bool>& globals);
template void GameScript::SetVariables<float>(std::map<std::string, float>& locals, std::map<std::string, float>& globals);
template void GameScript::SetVariables<std::string>(std::map<std::string, std::string>& locals, std::map<std::string, std::string>& globals);

template <typename T, typename Stored>
std::unique_ptr<T> GetTByName(std::string const & type, std::string const& name, std::map<std::string, Stored> const & map)
{
	ScriptAssert(map.find(name) != map.end(), std::string{ type + " name not found: " + name }, ERROR_MODE::TERMINATE);
	return std::make_unique<T>(map.at(name), false);
}

std::unique_ptr<GameScriptItemInfo> GameScript::GetItemByName(std::string const & name)
{
	return GetTByName<GameScriptItemInfo, short>("ItemInfo", name, m_itemsMapName);
}

std::unique_ptr<GameScriptMeshInfo> GameScript::GetMeshByName(std::string const & name)
{
	return GetTByName<GameScriptMeshInfo, MESH_INFO &>("MeshInfo", name, m_meshesMapName);
}

std::unique_ptr<GameScriptCameraInfo> GameScript::GetCameraByName(std::string const & name)
{
	return GetTByName<GameScriptCameraInfo, LEVEL_CAMERA_INFO &>("CameraInfo", name, m_camerasMapName);
}

std::unique_ptr<GameScriptSinkInfo> GameScript::GetSinkByName(std::string const & name)
{
	return GetTByName<GameScriptSinkInfo, SINK_INFO &>("SinkInfo", name, m_sinksMapName);
}

std::unique_ptr<GameScriptAIObject> GameScript::GetAIObjectByName(std::string const & name)
{
	return GetTByName<GameScriptAIObject, AI_OBJECT &>("AIObject", name, m_aiObjectsMapName);
}

std::unique_ptr<GameScriptSoundSourceInfo> GameScript::GetSoundSourceByName(std::string const & name)
{
	return GetTByName<GameScriptSoundSourceInfo, SOUND_SOURCE_INFO &>("SoundSourceInfo", name, m_soundSourcesMapName);
}


void GameScript::AssignItemsAndLara()
{
	m_lua->set("Lara", GameScriptItemInfo(Lara.itemNumber, false)); // do we need GetLara if we have this?
}

void GameScript::ResetVariables()
{
	(*m_lua)["Lara"] = NULL;
}

int CalculateDistance(GameScriptPosition pos1, GameScriptPosition pos2)
{
	return sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.y - pos2.y) + SQUARE(pos1.z - pos2.z));
}

int CalculateHorizontalDistance(GameScriptPosition pos1, GameScriptPosition pos2)
{
	return sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.z - pos2.z));
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

void GameScript::ExecuteFunction(std::string const & name)
{
	sol::protected_function func = (*m_lua)["LevelFuncs"][name.c_str()];
	auto r = func();
	if (!r.valid())
	{
		sol::error err = r;
		ScriptAssert(false, err.what(), ERROR_MODE::TERMINATE);
	}
}

static void doCallback(sol::protected_function const & func) {
	auto r = func();
	if (!r.valid())
	{
		sol::error err = r;
		ScriptAssert(false, err.what(), ERROR_MODE::TERMINATE);
	}
}

void GameScript::OnStart()
{
	if (m_onStart.valid())
		doCallback(m_onStart);
}

void GameScript::OnLoad()
{
	if(m_onLoad.valid())
		doCallback(m_onLoad);
}

void GameScript::OnControlPhase()
{
	if(m_onControlPhase.valid())
		doCallback(m_onControlPhase);
}

void GameScript::OnSave()
{
	if(m_onSave.valid())
		doCallback(m_onSave);
}

void GameScript::OnEnd()
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

	Level.enemiesKilled = Level.enemiesKilled + 1

If the player saves the level after killing three, saves, and then reloads the save
some time later, the values `3` will be put back into `Level.enemiesKilled.`

__This table is emptied when a level is finished.__ If the player needs to be able
to return to the level (e.g. like the Karnak level in *The Last Revelation*,
you will need to use the @{Game} table, below.
@table Level
*/

/*** A table with game data which will be saved and loaded.
This is for information not specific to any level, but which concerns your whole
levelset or game, that you want to store in saved games.

For example, you may wish to have a final boss say a specific voice line based on
a choice the player made in a previous level. In the level with the choice, you could
write:

	Game.playerSnoopedInDraws = true

And in the script file for the level with the boss, you could write:

	if Game.playerSnoopedInDraws then
		PlayAudioTrack("how_dare_you.wav")
	end

Unlike @{Level}, this table will remain intact for the entirety of the game.
@table Game
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
@tfield function OnControlPhase Will be called once per frame
@tfield function OnSave Will be called when the player saves the game
@tfield function OnEnd Will be called when leaving a level. This includes finishing it, exiting to the menu, or loading a save in a different level. 
@table LevelFuncs
*/

void GameScript::InitCallbacks()
{
	auto assignCB = [this](sol::protected_function& func, std::string const & luaFunc) {
		std::string fullName = "LevelFuncs." + luaFunc;
		func = (*m_lua)["LevelFuncs"][luaFunc];
		std::string err{ "Level's script does not define callback " + fullName};
		if (!ScriptAssert(func.valid(), err)) {
			TENLog("Defaulting to no " + fullName + " behaviour.", LogLevel::Warning, LogConfig::All);
		}
	};

	assignCB(m_onStart, "OnStart");
	assignCB(m_onLoad, "OnLoad");
	assignCB(m_onControlPhase, "OnControlPhase");
	assignCB(m_onSave, "OnSave");
	assignCB(m_onEnd, "OnEnd");
}
