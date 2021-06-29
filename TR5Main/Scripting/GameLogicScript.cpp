#include "framework.h"
#include "GameLogicScript.h"
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

extern GameFlow* g_GameFlow;
GameScript* g_GameScript;
bool WarningsAsErrors = false;

GameScript::GameScript(sol::state* lua) : LuaHandler{ lua }
{
	GameScriptItemInfo::Register(m_lua);
	m_lua->new_enum<GAME_OBJECT_ID>("Object", {
		{"LARA", ID_LARA}
		});

	// Add the item type
	m_lua->new_usertype<GameScriptPosition>("Position",
		sol::constructors<GameScriptPosition(int, int, int)>(),
		"X", sol::property(&GameScriptPosition::GetX, &GameScriptPosition::SetX),
		"Y", sol::property(&GameScriptPosition::GetY, &GameScriptPosition::SetY),
		"Z", sol::property(&GameScriptPosition::GetZ, &GameScriptPosition::SetZ)
		);

	m_lua->new_usertype<GameScriptRotation>("Rotation",
		sol::constructors<GameScriptRotation(int, int, int)>(),
		"X", sol::property(&GameScriptRotation::GetX, &GameScriptRotation::SetX),
		"Y", sol::property(&GameScriptRotation::GetY, &GameScriptRotation::SetY),
		"Z", sol::property(&GameScriptRotation::GetZ, &GameScriptRotation::SetZ)
		);

	m_lua->new_usertype<GameScriptItem>("Item",
		//"Position", sol::property(&GameScriptItem::GetPosition),
		//"Rotation", sol::property(&GameScriptItem::GetRotation),
		"HP", sol::property(&GameScriptItem::GetHP, &GameScriptItem::SetHP),
		"Room", sol::property(&GameScriptItem::GetRoom, &GameScriptItem::SetRoom),
		"CurrentState", sol::property(&GameScriptItem::GetCurrentState, &GameScriptItem::SetCurrentState),
		"GoalState", sol::property(&GameScriptItem::GetGoalState, &GameScriptItem::SetGoalState),
		"RequiredState", sol::property(&GameScriptItem::GetRequiredState, &GameScriptItem::SetRequiredState),
		"new", sol::no_constructor
		);

	m_lua->set_function("EnableItem", &GameScriptItem::EnableItem);
	m_lua->set_function("DisableItem", &GameScriptItem::DisableItem);

	m_lua->new_usertype<LuaVariables>("Variable",
		sol::meta_function::index, &LuaVariables::GetVariable,
		sol::meta_function::new_index, &LuaVariables::SetVariable,
		"new", sol::no_constructor
		);
}

void GameScript::AddTrigger(LuaFunction* function)
{
	m_triggers.push_back(function);
	(*m_lua).script(function->Code);
}

void GameScript::AddLuaId(int luaId, short itemNumber)
{
	m_itemsMapId.insert(std::pair<int, short>(luaId, itemNumber));
}

void GameScript::AddLuaName(std::string luaName, short itemNumber)
{
	m_itemsMapName.insert(std::pair<std::string, short>(luaName, itemNumber));
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

void GameScript::JumpToLevel(int levelNum)
{
	if (levelNum >= g_GameFlow->GetNumLevels())
		return;
	LevelComplete = levelNum;
}

int GameScript::GetSecretsCount()
{
	return Savegame.Level.Secrets;
}

void GameScript::SetSecretsCount(int secretsNum)
{
	if (secretsNum > 255)
		return;
	Savegame.Level.Secrets = secretsNum;
}

void GameScript::AddOneSecret()
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

std::unique_ptr<GameScriptItem> GameScript::GetItemById(int id)
{
	if (m_itemsMapId.find(id) == m_itemsMapId.end())
	{
		if (WarningsAsErrors)
			throw "item id not found";
		return std::unique_ptr<GameScriptItem>(nullptr);
	}

	return std::unique_ptr<GameScriptItem>(new GameScriptItem(m_itemsMapId[id]));
}

std::unique_ptr<GameScriptItem> GameScript::GetItemByName(std::string name)
{
	if (m_itemsMapName.find(name) == m_itemsMapName.end())
	{
		if (WarningsAsErrors)
			throw "item name not found";
		return std::unique_ptr<GameScriptItem>(nullptr);
	}

	return std::unique_ptr<GameScriptItem>(new GameScriptItem(m_itemsMapName[name]));
}

void GameScript::PlayAudioTrack(std::string trackName, bool looped)
{
	S_CDPlay(trackName, g_AudioTracks[trackName].looped);
}

void GameScript::PlaySoundEffect(int id, GameScriptPosition p, int flags)
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

void GameScript::PlaySoundEffect(int id, int flags)
{
	SoundEffect(id, NULL, flags);
}

void GameScript::SetAmbientTrack(std::string trackName)
{
	CurrentAtmosphere = trackName;
	S_CDPlay(CurrentAtmosphere, 1);
}

void GameScript::AddLightningArc(GameScriptPosition src, GameScriptPosition dest, GameScriptColor color, int lifetime, int amplitude, int beamWidth, int segments, int flags)
{
	PHD_VECTOR p1;
	p1.x = src.GetX();
	p1.y = src.GetY();
	p1.z = src.GetZ();

	PHD_VECTOR p2;
	p2.x = dest.GetX();
	p2.y = dest.GetY();
	p2.z = dest.GetZ();

	TriggerLightning(&p1, &p2, amplitude, color.GetR(), color.GetG(), color.GetB(), lifetime, flags, beamWidth, segments);
}

void GameScript::AddShockwave(GameScriptPosition pos, int innerRadius, int outerRadius, GameScriptColor color, int lifetime, int speed, int angle, int flags)
{
	PHD_3DPOS p;
	p.xPos = pos.GetX();
	p.yPos = pos.GetY();
	p.zPos = pos.GetZ();
	
	TriggerShockwave(&p, innerRadius, outerRadius, speed, color.GetR(), color.GetG(), color.GetB(), lifetime, FROM_DEGREES(angle), flags);
}

void GameScript::AddDynamicLight(GameScriptPosition pos, GameScriptColor color, int radius, int lifetime)
{
	TriggerDynamicLight(pos.GetX(), pos.GetY(), pos.GetZ(), radius, color.GetR(), color.GetG(), color.GetB());
}

void GameScript::AddBlood(GameScriptPosition pos, int num)
{
	TriggerBlood(pos.GetX(), pos.GetY(), pos.GetZ(), -1, num);
}

void GameScript::AddFireFlame(GameScriptPosition pos, int size)
{
	AddFire(pos.GetX(), pos.GetY(), pos.GetZ(), size, FindRoomNumber(pos), true);
}

void GameScript::Earthquake(int strength)
{
	Camera.bounce = -strength;
}

// Inventory
void GameScript::InventoryAdd(int slot, int count)
{
	PickedUpObject(slot, count);
}

void GameScript::InventoryRemove(int slot, int count)
{
	RemoveObjectFromInventory(slot, count);
}

void GameScript::InventoryGetCount(int slot)
{

}

void GameScript::InventorySetCount(int slot, int count)
{

}

void GameScript::InventoryCombine(int slot1, int slot2)
{
	
}

void GameScript::InventorySepare(int slot)
{

}

// Misc
void GameScript::PrintString(std::string key, GameScriptPosition pos, GameScriptColor color, int lifetime, int flags)
{

}

int GameScript::FindRoomNumber(GameScriptPosition pos)
{
	return 0;
}

void GameScript::AssignItemsAndLara()
{
	m_lua->set("Level", m_locals);
	m_lua->set("Game", m_globals);
	m_lua->set("Lara", GameScriptItem(Lara.itemNumber));
}

void GameScript::ResetVariables()
{
	(*m_lua)["Lara"] = NULL;
}

int GameScript::CalculateDistance(GameScriptPosition pos1, GameScriptPosition pos2)
{
	return sqrt(SQUARE(pos1.GetX() - pos2.GetX()) + SQUARE(pos1.GetY() - pos2.GetY()) + SQUARE(pos1.GetZ() - pos2.GetZ()));
}

int GameScript::CalculateHorizontalDistance(GameScriptPosition pos1, GameScriptPosition pos2)
{
	return sqrt(SQUARE(pos1.GetX() - pos2.GetX()) + SQUARE(pos1.GetZ() - pos2.GetZ()));
}

GameScriptItem::GameScriptItem(short itemNumber)
	:
	NativeItemNumber(itemNumber),
	NativeItem(&g_Level.Items[itemNumber])
{

}

short GameScriptItem::GetHP()
{
	return NativeItem->hitPoints;
}

void GameScriptItem::SetHP(short hp)
{
	if (hp < 0 || hp > Objects[NativeItem->objectNumber].hitPoints)
	{
		if (WarningsAsErrors)
			throw std::runtime_error("invalid HP");
		if (hp < 0)
		{
			hp = 0;
		}
		else if (hp > Objects[NativeItem->objectNumber].hitPoints)
		{
			hp = Objects[NativeItem->objectNumber].hitPoints;
		}
	}
	NativeItem->hitPoints = hp;
}

short GameScriptItem::GetRoom()
{
	return NativeItem->roomNumber;
}

void GameScriptItem::SetRoom(short room)
{
	if (room < 0 || room >= g_Level.Rooms.size())
	{
		if (WarningsAsErrors)
			throw std::runtime_error("invalid room number");
		return;
	}
	NativeItem->roomNumber = room;
}

void GameScriptItem::EnableItem()
{
	if (!NativeItem->active)
	{
		if (Objects[NativeItem->objectNumber].intelligent)
		{
			if (NativeItem->status == ITEM_DEACTIVATED)
			{
				NativeItem->touchBits = 0;
				NativeItem->status = ITEM_ACTIVE;
				AddActiveItem(NativeItemNumber);
				EnableBaddieAI(NativeItemNumber, 1);
			}
			else if (NativeItem->status == ITEM_INVISIBLE)
			{
				NativeItem->touchBits = 0;
				if (EnableBaddieAI(NativeItemNumber, 0))
					NativeItem->status = ITEM_ACTIVE;
				else
					NativeItem->status = ITEM_INVISIBLE;
				AddActiveItem(NativeItemNumber);
			}
		}
		else
		{
			NativeItem->touchBits = 0;
			AddActiveItem(NativeItemNumber);
			NativeItem->status = ITEM_ACTIVE;
		}
	}
}

void GameScriptItem::DisableItem()
{
	if (NativeItem->active)
	{
		if (Objects[NativeItem->objectNumber].intelligent)
		{
			if (NativeItem->status == ITEM_ACTIVE)
			{
				NativeItem->touchBits = 0;
				NativeItem->status = ITEM_DEACTIVATED;
				RemoveActiveItem(NativeItemNumber);
				DisableBaddieAI(NativeItemNumber);
			}
		}
		else
		{
			NativeItem->touchBits = 0;
			RemoveActiveItem(NativeItemNumber);
			NativeItem->status = ITEM_DEACTIVATED;
		}
	}
}

short GameScriptItem::GetCurrentState()
{
	return NativeItem->currentAnimState;
}

void GameScriptItem::SetCurrentState(short state)
{
	NativeItem->currentAnimState = state;
}

short GameScriptItem::GetGoalState()
{
	return NativeItem->goalAnimState;
}

void GameScriptItem::SetGoalState(short state)
{
	NativeItem->goalAnimState = state;
}

short GameScriptItem::GetRequiredState()
{
	return NativeItem->requiredAnimState;
}

void GameScriptItem::SetRequiredState(short state)
{
	NativeItem->requiredAnimState = state;
}

sol::object LuaVariables::GetVariable(std::string key)
{
	if (variables.find(key) == variables.end())
		return sol::lua_nil;
	return variables[key];
}

void LuaVariables::SetVariable(std::string key, sol::object value)
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
		if (WarningsAsErrors)
			throw std::runtime_error("unsupported variable type");
		break;
	}
}


