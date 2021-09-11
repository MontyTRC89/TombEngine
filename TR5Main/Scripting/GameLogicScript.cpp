#include "framework.h"
#include "GameLogicScript.h"
#include "ScriptAssert.h"
#include "items.h"
#include "box.h"
#include "lara.h"
#include "savegame.h"
#include "lot.h"
#include "Sound\sound.h"
#include "setup.h"
#include "level.h"
#include "effects\tomb4fx.h"
#include "effects\effects.h"
#include "pickup.h"
#include "newinv2.h"
#include "ObjectIDs.h"
#include "camera.h"
/***
Functions and callbacks for level-specific logic scripts.
@files Level-specific
@pragma nostrip
*/

extern GameFlow* g_GameFlow;

static void PlayAudioTrack(std::string const & trackName, sol::optional<bool> looped)
{
	S_CDPlay(trackName, looped.value_or(false));
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

static void InventoryAdd(ItemEnumPair slot, sol::optional<int> count)
{
	// If 0 is passed in, then the amount added will be the default amount
	// for that pickup - i.e. the amount you would get from picking up the
	// item in-game (e.g. 1 for medipacks, 12 for flares).
	PickedUpObject(slot.m_pair.first, count.value_or(0));
}

static void InventoryRemove(ItemEnumPair slot, sol::optional<int> count)
{
	// 0 is default for the same reason as in InventoryAdd.
	RemoveObjectFromInventory(slot.m_pair.first, count.value_or(0));
}

static int InventoryGetCount(ItemEnumPair slot)
{
	return GetInventoryCount(slot.m_pair.first);
}

static void InventorySetCount(ItemEnumPair slot, int count)
{
	// add the amount we'd need to add to get to count
	int currAmt = GetInventoryCount(slot.m_pair.first);
	InventoryAdd(slot, count - currAmt);
}

static void InventoryCombine(int slot1, int slot2)
{
	
}

static void InventorySeparate(int slot)
{

}

static int CalculateDistance(GameScriptPosition const & pos1, GameScriptPosition const & pos2)
{
	auto result = sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.y - pos2.y) + SQUARE(pos1.z - pos2.z));
	return static_cast<int>(round(result));
}

static int CalculateHorizontalDistance(GameScriptPosition const & pos1, GameScriptPosition const & pos2)
{
	auto result = sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.z - pos2.z));
	return static_cast<int>(round(result));
}

// Misc
static void PrintString(std::string key, GameScriptPosition pos, GameScriptColor color, int lifetime, int flags)
{

}

// A "special" table is essentially one that TEN reserves and does things with.
template <typename funcIndex, typename funcNewindex, typename obj>
static void MakeSpecialTable(sol::state * state, std::string const & name, funcIndex const & fi, funcNewindex const & fni, obj objPtr)
{
	std::string metaName{ name + "Meta" };
	auto meta = sol::table{ *state, sol::create };
	state->set(metaName, meta);
	meta.set("__metatable", "\"metatable is protected\"");
	auto tab = state->create_named_table(name);
	tab[sol::metatable_key] = meta;
	state->set(metaName, sol::nil);
	meta.set_function("__index", fi, objPtr);
	meta.set_function("__newindex", fni, objPtr);
}

GameScript::GameScript(sol::state* lua) : LuaHandler{ lua }
{
/*** Ambience and music
@section Music
*/

/*** Set and play an ambient track
@function SetAmbientTrack
@tparam string name of track (without file extension) to play
*/
	m_lua->set_function("SetAmbientTrack", &SetAmbientTrack);

/*** Play an audio track
@function PlayAudioTrack
@tparam string name of track (without file extension) to play
@tparam bool loop if true, the track will loop; if false, it won't (default: false)
*/
	m_lua->set_function("PlayAudioTrack", &PlayAudioTrack);

/*** Player inventory management
@section Inventory
*/

/*** Add x of an item to the inventory.
A count of 0 will add the "default" amount of that item
(i.e. the amount the player would get from a pickup of that type).
For example, giving "zero" crossbow ammo would give the player
10 instead, whereas giving "zero" medkits would give the player 1 medkit.
@function GiveInvItem
@tparam InvItem item the item to be added
@tparam int count the number of items to add (default: 0)
*/
	m_lua->set_function("GiveInvItem", &InventoryAdd);

/***
Remove x of a certain item from the inventory.
As in @{GiveInvItem}, a count of 0 will remove the "default" amount of that item.
@function TakeInvItem
@tparam InvItem item the item to be removed
@tparam int count the number of items to remove (default: 0)
*/
	m_lua->set_function("TakeInvItem", &InventoryRemove);

/***
Get the amount the player holds of an item.
@function GetInvItemCount
@tparam InvItem item the item to check
@treturn int the amount of the item the player has in the inventory
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
@treturn ItemInfo a non-owning ItemInfo referencing the item.
*/
	m_lua->set_function("GetItemByName", &GameScript::GetItemByName, this);

/***
Get a MeshInfo by its name.
@function GetMeshByName
@tparam string name the unique name of the mesh as set in, or generated by, Tomb Editor
@treturn MeshInfo a non-owning MeshInfo referencing the mesh.
*/
	m_lua->set_function("GetMeshByName", &GameScript::GetMeshByName, this);

/***
Get a CameraInfo by its name.
@function GetCameraByName
@tparam string name the unique name of the camera as set in, or generated by, Tomb Editor
@treturn CameraInfo a non-owning CameraInfo referencing the camera.
*/
	m_lua->set_function("GetCameraByName", &GameScript::GetCameraByName, this);

/***
Get a SinkInfo by its name.
@function GetSinkByName
@tparam string name the unique name of the sink as set in, or generated by, Tomb Editor
@treturn SinkInfo a non-owning SinkInfo referencing the sink.
*/
	m_lua->set_function("GetSinkByName", &GameScript::GetSinkByName, this);

/***
Get a SoundSourceInfo by its name.
@function GetSoundSourceByName
@tparam string name the unique name of the sink as set in, or generated by, Tomb Editor
@treturn SoundSourceInfo a non-owning SoundSourceInfo referencing the sink.
*/
	m_lua->set_function("GetSoundSourceByName", &GameScript::GetSoundSourceByName, this);

/***
Calculate the distance between two positions.
@function CalculateDistance
@tparam Position posA first position
@tparam Position posB second position
@treturn int the direct distance from one position to the other
*/
	m_lua->set_function("CalculateDistance", &CalculateDistance);

/***
Calculate the horizontal distance between two positions.
@function CalculateHorizontalDistance
@tparam Position posA first position
@tparam Position posB second position
@treturn int the direct distance on the XZ plane from one position to the other
*/
	m_lua->set_function("CalculateHorizontalDistance", &CalculateHorizontalDistance);

	MakeReadOnlyTable("ObjID", kObjIDs);

	ResetLevelTables();

	MakeSpecialTable(m_lua, "GameVars", &LuaVariables::GetVariable, &LuaVariables::SetVariable, &m_globals);

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

	m_lua->new_enum<GAME_OBJECT_ID>("Object", {
		{"LARA", ID_LARA}
		});
}

void GameScript::ResetLevelTables()
{
	MakeSpecialTable(m_lua, "LevelFuncs", &GameScript::GetLevelFunc, &GameScript::SetLevelFunc, this);
	MakeSpecialTable(m_lua, "LevelVars", &LuaVariables::GetVariable, &LuaVariables::SetVariable, &m_locals);
}

sol::protected_function GameScript::GetLevelFunc(sol::table tab, std::string const& luaName)
{
	return tab.raw_get<sol::protected_function>(luaName);
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
		m_levelFuncs.insert(luaName);
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
	m_levelFuncs.clear();
	m_locals = LuaVariables{};
	ResetLevelTables();
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
std::unique_ptr<T> GetTByName(std::string const & type, std::string const& name, std::unordered_map<std::string, Stored> const & map)
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
	m_lua->set("Lara", GameScriptItemInfo(Lara.itemNumber, false));
}

/*** Special objects
@section specialobjects
*/

/*** An @{ItemInfo} representing Lara herself.
@table Lara
*/
void GameScript::ResetVariables()
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

void GameScript::ExecuteFunction(std::string const & name)
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

void GameScript::OnControlPhase(float dt)
{
	if(m_onControlPhase.valid())
		doCallback(m_onControlPhase, dt);
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

void GameScript::InitCallbacks()
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
