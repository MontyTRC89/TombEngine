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
using namespace std;
extern GameFlow* g_GameFlow;
GameScript* g_GameScript;
bool WarningsAsErrors = false;

GameScript::GameScript(sol::state* lua)
{
	m_lua = lua;

	// Add constants
	//ExecuteScript("Scripts\\Constants.lua");

	m_lua->new_enum<GAME_OBJECT_ID>("Object", {
		{"LARA", ID_LARA}
		});

	// Add the item type
	m_lua->new_usertype<GameScriptPosition>("Position",
		"X", sol::property(&GameScriptPosition::GetXPos, &GameScriptPosition::SetXPos),
		"Y", sol::property(&GameScriptPosition::GetYPos, &GameScriptPosition::SetYPos),
		"Z", sol::property(&GameScriptPosition::GetZPos, &GameScriptPosition::SetZPos),
		"new", sol::no_constructor
		);

	m_lua->new_usertype<GameScriptRotation>("Rotation",
		"X", sol::property(&GameScriptRotation::GetXRot, &GameScriptRotation::SetXRot),
		"Y", sol::property(&GameScriptRotation::GetYRot, &GameScriptRotation::SetYRot),
		"Z", sol::property(&GameScriptRotation::GetZRot, &GameScriptRotation::SetZRot),
		"new", sol::no_constructor
		);

	m_lua->new_usertype<GameScriptItem>("Item",
		"Position", sol::property(&GameScriptItem::GetPosition),
		"Rotation", sol::property(&GameScriptItem::GetRotation),
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

	// GameScript type
	/*m_lua->new_usertype<GameScript>("GameScript",
		"PlayAudioTrack", &GameScript::PlayAudioTrack,
		"ChangeAmbientSoundTrack", &GameScript::ChangeAmbientSoundTrack,
		"MakeItemInvisible", &GameScript::MakeItemInvisible,
		"GetSecretsCount", &GameScript::GetSecretsCount,
		"SetSecretsCount", &GameScript::SetSecretsCount,
		"AddOneSecret", &GameScript::AddOneSecret,
		"JumpToLevel", &GameScript::JumpToLevel,
		"PlaySoundEffect", &GameScript::PlaySoundEffect,
		"PlaySoundEffectAtPosition", &GameScript::PlaySoundEffectAtPosition
		);*/

	// Add global variables and namespaces
	//(*m_lua)["TR"] = this;
}

void GameScript::AddTrigger(LuaFunction* function)
{
	m_triggers.push_back(function);
	(*m_lua).script(function->Code);
}

void GameScript::AddLuaId(int luaId, short itemNumber)
{
	m_itemsMapId.insert(pair<int, short>(luaId, itemNumber));
}

void GameScript::AddLuaName(string luaName, short itemNumber)
{
	m_itemsMapName.insert(pair<string, short>(luaName, itemNumber));
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

bool GameScript::ExecuteScript(const string& luaFilename, string& message)
{ 
	auto result = m_lua->safe_script_file(luaFilename, sol::environment(m_lua->lua_state(), sol::create, m_lua->globals()), sol::script_pass_on_error);
	if (!result.valid())
	{
		sol::error error = result;
		message = error.what();
		return false;
	}
	return true;
}

bool GameScript::ExecuteString(const string& command, string& message)
{
	auto result = m_lua->safe_script(command, sol::environment(m_lua->lua_state(), sol::create, m_lua->globals()), sol::script_pass_on_error);
	if (!result.valid())
	{
		sol::error error = result;
		message = error.what();
		return false;
	}
	return true;
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
			std::cout << key.as<string>() << " " << value.as<bool>() << std::endl;
		else if (value.is<string>())
			std::cout << key.as<string>() << " " << value.as<string>() << std::endl;
		else
			std::cout << key.as<string>() << " " << value.as<int>() << std::endl;		
	});

	return result;
	*/
}

void GameScript::PlayAudioTrack(short track)
{
	S_CDPlay(track, SOUND_TRACK_ONESHOT);
}

void GameScript::ChangeAmbientSoundTrack(short track)
{
	CurrentAtmosphere = track;
	S_CDStop();
	S_CDPlay(track, SOUND_TRACK_BGM);
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
	S_CDPlay(6, 0);
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
void GameScript::GetVariables(map<string, T>& locals, map<string, T>& globals)
{
	for (const auto& it : m_locals.variables)
	{
		if (it.second.is<T>())
			locals.insert(pair<string, T>(it.first, it.second.as<T>()));
	}
	for (const auto& it : m_globals.variables)
	{
		if (it.second.is<T>())
			globals.insert(pair<string, T>(it.first, it.second.as<T>()));
	}
}

template void GameScript::GetVariables<bool>(map<string, bool>& locals, map<string, bool>& globals);
template void GameScript::GetVariables<float>(map<string, float>& locals, map<string, float>& globals);
template void GameScript::GetVariables<string>(map<string, string>& locals, map<string, string>& globals);

template <typename T>
void GameScript::SetVariables(map<string, T>& locals, map<string, T>& globals)
{
	m_locals.variables.clear();
	for (const auto& it : locals)
	{
		m_locals.variables.insert(pair<string, sol::object>(it.first, sol::object(m_lua->lua_state(), sol::in_place, it.second)));
	}
	for (const auto& it : globals)
	{
		m_globals.variables.insert(pair<string, sol::object>(it.first, sol::object(m_lua->lua_state(), sol::in_place, it.second)));
	}
}

template void GameScript::SetVariables<bool>(map<string, bool>& locals, map<string, bool>& globals);
template void GameScript::SetVariables<float>(map<string, float>& locals, map<string, float>& globals);
template void GameScript::SetVariables<string>(map<string, string>& locals, map<string, string>& globals);

unique_ptr<GameScriptItem> GameScript::GetItemById(int id)
{
	if (m_itemsMapId.find(id) == m_itemsMapId.end())
	{
		if (WarningsAsErrors)
			throw "item id not found";
		return unique_ptr<GameScriptItem>(nullptr);
	}

	return unique_ptr<GameScriptItem>(new GameScriptItem(m_itemsMapId[id]));
}

unique_ptr<GameScriptItem> GameScript::GetItemByName(string name)
{
	if (m_itemsMapName.find(name) == m_itemsMapName.end())
	{
		if (WarningsAsErrors)
			throw "item name not found";
		return unique_ptr<GameScriptItem>(nullptr);
	}

	return unique_ptr<GameScriptItem>(new GameScriptItem(m_itemsMapName[name]));
}

void GameScript::PlaySoundEffectAtPosition(short id, int x, int y, int z, int flags)
{
	PHD_3DPOS pos;

	pos.xPos = x;
	pos.yPos = y;
	pos.zPos = z;
	pos.xRot = 0;
	pos.yRot = 0;
	pos.zRot = 0;

	SoundEffect(id, &pos, flags);
}

void GameScript::PlaySoundEffect(short id, int flags)
{
	SoundEffect(id, NULL, flags);
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

GameScriptPosition GameScript::CreatePosition(float x, float y, float z)
{
	return GameScriptPosition(x, y, z);
}

GameScriptPosition GameScript::CreateSectorPosition(float x, float y, float z)
{
	return GameScriptPosition(1024 * x + 512, 1024 * y + 512, 1024 * z + 512);
}

GameScriptRotation GameScript::CreateRotation(float x, float y, float z)
{
	return GameScriptRotation(x, y, z);
}

float GameScript::CalculateDistance(GameScriptPosition pos1, GameScriptPosition pos2)
{
	return sqrt(SQUARE(pos1.GetXPos() - pos2.GetXPos()) + SQUARE(pos1.GetYPos() - pos2.GetYPos()) + SQUARE(pos1.GetZPos() - pos2.GetZPos()));
}

float GameScript::CalculateHorizontalDistance(GameScriptPosition pos1, GameScriptPosition pos2)
{
	return sqrt(SQUARE(pos1.GetXPos() - pos2.GetXPos()) + SQUARE(pos1.GetZPos() - pos2.GetZPos()));
}

GameScriptPosition::GameScriptPosition(float x, float y, float z)
	:
	xPos(x),
	yPos(y),
	zPos(z)
{

}

GameScriptPosition::GameScriptPosition(function<float()> readX, function<void(float)> writeX, function<float()> readY, function<void(float)> writeY, function<float()> readZ, function<void(float)> writeZ)
	:
	readXPos(readX),
	writeXPos(writeX),
	readYPos(readY),
	writeYPos(writeY),
	readZPos(readZ),
	writeZPos(writeZ)
{

}

float GameScriptPosition::GetXPos()
{
	if (readXPos)
		xPos = readXPos();
	return xPos;
}

void GameScriptPosition::SetXPos(float x)
{
	xPos = x;
	if (writeXPos)
		writeXPos(xPos);
}

float GameScriptPosition::GetYPos()
{
	if (readYPos)
		yPos = readYPos();
	return yPos;
}

void GameScriptPosition::SetYPos(float y)
{
	yPos = y;
	if (writeYPos)
		writeYPos(yPos);
}

float GameScriptPosition::GetZPos()
{
	if (readZPos)
		zPos = readZPos();
	return zPos;
}

void GameScriptPosition::SetZPos(float z)
{
	zPos = z;
	if (writeZPos)
		writeZPos(zPos);
}

GameScriptRotation::GameScriptRotation(float x, float y, float z)
	:
	xRot(x),
	yRot(y),
	zRot(z)
{

}

GameScriptRotation::GameScriptRotation(function<float()> readX, function<void(float)> writeX, function<float()> readY, function<void(float)> writeY, function<float()> readZ, function<void(float)> writeZ)
	:
	readXRot(readX),
	writeXRot(writeX),
	readYRot(readY),
	writeYRot(writeY),
	readZRot(readZ),
	writeZRot(writeZ)
{

}

float GameScriptRotation::GetXRot()
{
	if (readXRot)
		xRot = readXRot();
	return xRot;
}

void GameScriptRotation::SetXRot(float x)
{
	x = remainder(x, 360);
	if (x < 0)
		x += 360;
	xRot = x;
	if (writeXRot)
		writeXRot(xRot);
}

float GameScriptRotation::GetYRot()
{
	if (readYRot)
		yRot = readYRot();
	return yRot;
}

void GameScriptRotation::SetYRot(float y)
{
	y = remainder(y, 360);
	if (y < 0)
		y += 360;
	yRot = y;
	if (writeYRot)
		writeYRot(yRot);
}

float GameScriptRotation::GetZRot()
{
	if (readZRot)
		zRot = readZRot();
	return zRot;
}

void GameScriptRotation::SetZRot(float z)
{
	z = remainder(z, 360);
	if (z < 0)
		z += 360;
	zRot = z;
	if (writeZRot)
		writeZRot(zRot);
}

GameScriptItem::GameScriptItem(short itemNumber)
	:
	NativeItemNumber(itemNumber),
	NativeItem(&g_Level.Items[itemNumber])
{

}

GameScriptPosition GameScriptItem::GetPosition()
{
	return GameScriptPosition(
		[this]() -> float { return NativeItem->pos.xPos; },
		[this](float x) -> void { NativeItem->pos.xPos = x; },
		[this]() -> float { return NativeItem->pos.yPos; },
		[this](float y) -> void { NativeItem->pos.yPos = y; },
		[this]() -> float { return NativeItem->pos.zPos; },
		[this](float z) -> void { NativeItem->pos.zPos = z; }
	);
}

GameScriptRotation GameScriptItem::GetRotation()
{
	return GameScriptRotation(
		[this]() -> float { return TO_DEGREES(NativeItem->pos.xRot); },
		[this](float x) -> void { NativeItem->pos.xRot = ANGLE(x); },
		[this]() -> float { return TO_DEGREES(NativeItem->pos.yRot); },
		[this](float y) -> void { NativeItem->pos.yRot = ANGLE(y); },
		[this]() -> float { return TO_DEGREES(NativeItem->pos.zRot); },
		[this](float z) -> void { NativeItem->pos.zRot = ANGLE(z); }
	);
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
			throw runtime_error("invalid HP");
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
			throw runtime_error("invalid room number");
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

sol::object LuaVariables::GetVariable(string key)
{
	if (variables.find(key) == variables.end())
		return sol::lua_nil;
	return variables[key];
}

void LuaVariables::SetVariable(string key, sol::object value)
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
			throw runtime_error("unsupported variable type");
		break;
	}
}
