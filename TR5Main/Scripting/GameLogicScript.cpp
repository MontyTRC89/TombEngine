#include "framework.h"
#include "GameLogicScript.h"
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
#include "ObjectIDs.h"
#include "GameScriptDisplayString.h"
#include "ReservedScriptNames.h"
#include "Game/camera.h"
#include <Renderer/Renderer11Enums.h>
#include "Game/effects/lightning.h"

using namespace TEN::Effects::Lightning;

/***
Functions and callbacks for level-specific logic scripts.
@files Level-specific
@pragma nostrip
*/

static void PlayAudioTrack(std::string const & trackName, sol::optional<bool> looped)
{
	auto mode = looped.value_or(false) ? SOUNDTRACK_PLAYTYPE::OneShot : SOUNDTRACK_PLAYTYPE::BGM;
	PlaySoundTrack(trackName, mode);
}

static void PlaySoundEffect(int id, GameScriptPosition p, int flags)
{
	PoseData pos;

	pos.Position.x = p.x;
	pos.Position.y = p.y;
	pos.Position.z = p.z;
	pos.Orientation.x = 0;
	pos.Orientation.y = 0;
	pos.Orientation.z = 0;

	SoundEffect(id, &pos, flags);
}

static void PlaySoundEffect(int id, int flags)
{
	SoundEffect(id, NULL, flags);
}

static void SetAmbientTrack(std::string const & trackName)
{
	PlaySoundTrack(trackName, SOUNDTRACK_PLAYTYPE::BGM);
}

static int FindRoomNumber(GameScriptPosition pos)
{
	return 0;
}

static void AddLightningArc(GameScriptPosition src, GameScriptPosition dest, GameScriptColor color, int lifetime, int amplitude, int beamWidth, int segments, int flags)
{
	Vector3Int p1;
	p1.x = src.x;
	p1.y = src.y;
	p1.z = src.z;

	Vector3Int p2;
	p2.x = dest.x;
	p2.y = dest.y;
	p2.z = dest.z;

	TriggerLightning(&p1, &p2, amplitude, color.GetR(), color.GetG(), color.GetB(), lifetime, flags, beamWidth, segments);
}

static void AddShockwave(GameScriptPosition pos, int innerRadius, int outerRadius, GameScriptColor color, int lifetime, int speed, float angle, int flags)
{
	PoseData p;
	p.Position.x = pos.x;
	p.Position.y = pos.y;
	p.Position.z = pos.z;
	
	TriggerShockwave(&p, innerRadius, outerRadius, speed, color.GetR(), color.GetG(), color.GetB(), lifetime, Angle::DegToRad(angle), flags);
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

static std::tuple<int, int> PercentToScreen(double x, double y)
{
	auto fWidth = static_cast<double>(g_Configuration.Width);
	auto fHeight = static_cast<double>(g_Configuration.Height);
	int resX = std::round(fWidth / 100.0 * x);
	int resY = std::round(fHeight / 100.0 * y);
	//todo this still assumes a resolution of 800/600. account for this somehow
	return std::make_tuple(resX, resY);
}

static std::tuple<double, double> ScreenToPercent(int x, int y)
{

	auto fWidth = static_cast<double>(g_Configuration.Width);
	auto fHeight = static_cast<double>(g_Configuration.Height);
	double resX = x/fWidth * 100.0;
	double resY = y/fHeight * 100.0;
	return std::make_tuple(resX, resY);
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
	m_lua->set_function(ScriptReserved_SetAmbientTrack, &SetAmbientTrack);

/*** Play an audio track
@function PlayAudioTrack
@tparam string name of track (without file extension) to play
@tparam bool loop if true, the track will loop; if false, it won't (default: false)
*/
	m_lua->set_function(ScriptReserved_PlayAudioTrack, &PlayAudioTrack);


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
	m_lua->set_function(ScriptReserved_GiveInvItem, &InventoryAdd);

/***
Remove x of a certain item from the inventory.
As in @{GiveInvItem}, a count of 0 will remove the "default" amount of that item.
@function TakeInvItem
@tparam InvItem item the item to be removed
@tparam int count the number of items to remove (default: 0)
*/
	m_lua->set_function(ScriptReserved_TakeInvItem, &InventoryRemove);

/***
Get the amount the player holds of an item.
@function GetInvItemCount
@tparam InvItem item the item to check
@treturn int the amount of the item the player has in the inventory
*/
	m_lua->set_function(ScriptReserved_GetInvItemCount, &InventoryGetCount);

/***
Set the amount of a certain item the player has in the inventory.
Similar to @{GiveInvItem} but replaces with the new amount instead of adding it.
@function SetInvItemCount
@tparam @{InvItem} item the item to be set
@tparam int count the number of items the player will have
*/
	m_lua->set_function(ScriptReserved_SetInvItemCount, &InventorySetCount);

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
	m_lua->set_function(ScriptReserved_GetItemByName, &GameScript::GetByName<GameScriptItemInfo, ScriptReserved_ItemInfo>, this);

/***
Get a MeshInfo by its name.
@function GetMeshByName
@tparam string name the unique name of the mesh as set in, or generated by, Tomb Editor
@treturn MeshInfo a non-owning MeshInfo referencing the mesh.
*/
	m_lua->set_function(ScriptReserved_GetMeshByName, &GameScript::GetByName<GameScriptMeshInfo, ScriptReserved_MeshInfo>, this);

/***
Get a CameraInfo by its name.
@function GetCameraByName
@tparam string name the unique name of the camera as set in, or generated by, Tomb Editor
@treturn CameraInfo a non-owning CameraInfo referencing the camera.
*/
	m_lua->set_function(ScriptReserved_GetCameraByName, &GameScript::GetByName<GameScriptCameraInfo, ScriptReserved_CameraInfo>, this);

/***
Get a SinkInfo by its name.
@function GetSinkByName
@tparam string name the unique name of the sink as set in, or generated by, Tomb Editor
@treturn SinkInfo a non-owning SinkInfo referencing the sink.
*/
	m_lua->set_function(ScriptReserved_GetSinkByName, &GameScript::GetByName<GameScriptSinkInfo, ScriptReserved_SinkInfo>, this);

/***
Get a SoundSourceInfo by its name.
@function GetSoundSourceByName
@tparam string name the unique name of the sink as set in, or generated by, Tomb Editor
@treturn SoundSourceInfo a non-owning SoundSourceInfo referencing the sink.
*/
	m_lua->set_function(ScriptReserved_GetSoundSourceByName, &GameScript::GetByName<GameScriptSoundSourceInfo, ScriptReserved_SoundSourceInfo>, this);

/***
Calculate the distance between two positions.
@function CalculateDistance
@tparam Position posA first position
@tparam Position posB second position
@treturn int the direct distance from one position to the other
*/
	m_lua->set_function(ScriptReserved_CalculateDistance, &CalculateDistance);

/***
Calculate the horizontal distance between two positions.
@function CalculateHorizontalDistance
@tparam Position posA first position
@tparam Position posB second position
@treturn int the direct distance on the XZ plane from one position to the other
*/
	m_lua->set_function(ScriptReserved_CalculateHorizontalDistance, &CalculateHorizontalDistance);

/***
Show some text on-screen.
@function ShowString
@tparam DisplayString str the string object to draw
@tparam float time the time in seconds for which to show the string.
If not given, the string will have an "infinite" life, and will show
until @{HideString} is called or until the level is finished.
Default: nil (i.e. infinite)
*/

	m_lua->set_function(ScriptReserved_ShowString, &GameScript::ShowString, this);

/***
Hide some on-screen text.
@function HideString
@tparam DisplayString str the string object to hide. Must previously have been shown
with a call to @{ShowString}, or this function will have no effect.
*/
	m_lua->set_function(ScriptReserved_HideString, [this](GameScriptDisplayString const& s) {ShowString(s, 0.0f); });

/***
Translate a pair of percentages to screen-space pixel coordinates.
To be used with @{DisplayString:SetPos} and @{DisplayString.new}.
@function PercentToScreen
@tparam float x percent value to translate to x-coordinate
@tparam float y percent value to translate to y-coordinate
@treturn int x x coordinate in pixels
@treturn int y y coordinate in pixels
*/
	m_lua->set_function(ScriptReserved_PercentToScreen, &PercentToScreen);

/***
Translate a pair of coordinates to percentages of window dimensions.
To be used with @{DisplayString:GetPos}.
@function ScreenToPercent
@tparam int x pixel value to translate to a percentage of the window width
@tparam int y pixel value to translate to a percentage of the window height
@treturn float x coordinate as percentage
@treturn float y coordinate as percentage
*/
	m_lua->set_function(ScriptReserved_ScreenToPercent, &ScreenToPercent);
	MakeReadOnlyTable(ScriptReserved_ObjID, kObjIDs);
	MakeReadOnlyTable(ScriptReserved_DisplayStringOption, kDisplayStringOptionNames);

	ResetLevelTables();

	MakeSpecialTable(m_lua, ScriptReserved_GameVars, &LuaVariables::GetVariable, &LuaVariables::SetVariable, &m_globals);

	GameScriptItemInfo::Register(m_lua);
	GameScriptItemInfo::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptMeshInfo::Register(m_lua);
	GameScriptMeshInfo::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptCameraInfo::Register(m_lua);
	GameScriptCameraInfo::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptSinkInfo::Register(m_lua);
	GameScriptSinkInfo::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptAIObject::Register(m_lua);
	GameScriptAIObject::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptSoundSourceInfo::Register(m_lua);
	GameScriptSoundSourceInfo::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptDisplayString::Register(m_lua);
	GameScriptDisplayString::SetCallbacks(
		[this](auto && ... param) {return SetDisplayString(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) {return ScheduleRemoveDisplayString(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) {return GetDisplayString(std::forward<decltype(param)>(param)...); }
		);
	GameScriptPosition::Register(m_lua);

	m_lua->new_enum<GAME_OBJECT_ID>("Object", {
		{"LARA", ID_LARA}
		});
}

void GameScript::ResetLevelTables()
{
	MakeSpecialTable(m_lua, ScriptReserved_LevelFuncs, &GameScript::GetLevelFunc, &GameScript::SetLevelFunc, this);
	MakeSpecialTable(m_lua, ScriptReserved_LevelVars, &LuaVariables::GetVariable, &LuaVariables::SetVariable, &m_locals);
}

sol::protected_function GameScript::GetLevelFunc(sol::table tab, std::string const& luaName)
{
	if (m_levelFuncs.find(luaName) == m_levelFuncs.end())
		return sol::lua_nil;

	return m_levelFuncs.at(luaName);
}

bool GameScript::SetLevelFunc(sol::table tab, std::string const& luaName, sol::object value)
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

std::optional<std::reference_wrapper<UserDisplayString>> GameScript::GetDisplayString(DisplayStringIDType id)
{
	auto it = m_userDisplayStrings.find(id);
	if (std::cend(m_userDisplayStrings) == it)
		return std::nullopt;
	return std::ref(m_userDisplayStrings.at(id));
}

bool GameScript::ScheduleRemoveDisplayString(DisplayStringIDType id)
{
	auto it = m_userDisplayStrings.find(id);
	if (std::cend(m_userDisplayStrings) == it)
		return false;

	it->second.m_deleteWhenZero = true;
	return true;
}

bool GameScript::SetDisplayString(DisplayStringIDType id, UserDisplayString const & ds)
{
	return m_userDisplayStrings.insert_or_assign(id, ds).second;
}


void GameScript::SetCallbackDrawString(CallbackDrawString cb)
{
	m_callbackDrawSring = cb;
}

void GameScript::FreeLevelScripts()
{
	m_nameMap.clear();
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

template void GameScript::SetVariables<bool>(std::map<std::string, bool>& locals, std::map<std::string, bool>& globals);
template void GameScript::SetVariables<float>(std::map<std::string, float>& locals, std::map<std::string, float>& globals);
template void GameScript::SetVariables<std::string>(std::map<std::string, std::string>& locals, std::map<std::string, std::string>& globals);

template <typename R, char const * S, typename mapType>
std::unique_ptr<R> GetByName(std::string const & type, std::string const & name, mapType const & map)
{
	ScriptAssert(map.find(name) != map.end(), std::string{ type + " name not found: " + name }, ERROR_MODE::TERMINATE);
	return std::make_unique<R>(map.at(name), false);
}

void GameScript::AssignItemsAndLara()
{
	m_lua->set("Lara", GameScriptItemInfo(Lara.ItemNumber, false));
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

void GameScript::ShowString(GameScriptDisplayString const & str, sol::optional<float> nSeconds)
{
	auto it = m_userDisplayStrings.find(str.GetID());
	it->second.m_timeRemaining = nSeconds.value_or(0.0f);
	it->second.m_isInfinite = !nSeconds.has_value();
}

void GameScript::ProcessDisplayStrings(float dt)
{
	auto it = std::begin(m_userDisplayStrings);
	while (it != std::end(m_userDisplayStrings))
	{
		auto& str = it->second;
		bool endOfLife = (0.0f >= str.m_timeRemaining);
		if (str.m_deleteWhenZero && endOfLife)
		{
			ScriptAssertF(!str.m_isInfinite, "The infinite string {} (key \"{}\") went out of scope without being hidden.", it->first, str.m_key);
			it = m_userDisplayStrings.erase(it);
		}
		else
		{
			if (!endOfLife || str.m_isInfinite)
			{
				char const* cstr = str.m_isTranslated ? g_GameFlow->GetString(str.m_key.c_str()) : str.m_key.c_str();
				int flags = 0;

				if (str.m_flags[static_cast<size_t>(DisplayStringOptions::CENTER)])
					flags |= PRINTSTRING_CENTER;

				if (str.m_flags[static_cast<size_t>(DisplayStringOptions::OUTLINE)])
					flags |= PRINTSTRING_OUTLINE;

				m_callbackDrawSring(cstr, str.m_color, str.m_x, str.m_y, flags);

				str.m_timeRemaining -= dt;
			}
			++it;
		}
	}
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
