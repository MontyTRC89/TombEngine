#include "GameLogicScript.h"
#include "..\Game\items.h"
#include "..\Game\box.h"
#include "..\Game\lot.h"
#include "..\Game\sound.h"

extern GameFlow* g_GameFlow;

GameScript::GameScript(sol::state* lua)
{
	m_lua = lua;

	// Add constants
	ExecuteScript("Scripts\\Constants.lua");

	// Add the item type
	m_lua->new_usertype<GameScriptItemPosition>("ItemPosition",
		"x", &GameScriptItemPosition::x,
		"y", &GameScriptItemPosition::y,
		"z", &GameScriptItemPosition::z,
		"xRot", &GameScriptItemPosition::xRot,
		"yRot", &GameScriptItemPosition::yRot,
		"zRot", &GameScriptItemPosition::zRot,
		"room", &GameScriptItemPosition::room
		);

	m_lua->new_usertype<GameScriptItem>("Item",
		"Get", &GameScriptItem::Get,
		"Set", &GameScriptItem::Set,
		"GetPosition", &GameScriptItem::GetItemPosition,
		"SetPosition", &GameScriptItem::SetItemPosition,
		"SetRotation", &GameScriptItem::SetItemRotation
		);
	
	// GameScript type
	m_lua->new_usertype<GameScript>("GameScript",
		"EnableItem", &GameScript::EnableItem,
		"DisableItem", &GameScript::DisableItem,
		"PlayAudioTrack", &GameScript::PlayAudioTrack,
		"ChangeAmbientSoundTrack", &GameScript::ChangeAmbientSoundTrack,
		"MakeItemInvisible", &GameScript::MakeItemInvisible,
		"GetSecretsCount", &GameScript::GetSecretsCount,
		"SetSecretsCount", &GameScript::SetSecretsCount,
		"AddOneSecret", &GameScript::AddOneSecret,
		"JumpToLevel", &GameScript::JumpToLevel,
		"GetItem", &GameScript::GetItem,
		"PlaySoundEffect", &GameScript::PlaySoundEffect,
		"PlaySoundEffectAtPosition", &GameScript::PlaySoundEffectAtPosition
		);

	// Add global variables and namespaces
	(*m_lua)["TR"] = this;

	m_locals = (*m_lua).create_table("Locals");
	m_globals = (*m_lua).create_table("Globals");
}

GameScript::~GameScript()
{
	
}

void GameScript::AddTrigger(LuaFunction* function)
{
	m_triggers.push_back(function);
	(*m_lua).script(function->Code);
}

void GameScript::AddLuaId(int luaId, int itemId)
{
	m_itemsMap.insert(pair<__int32, __int32>(luaId, itemId));
}

void GameScript::FreeLevelScripts()
{
	// Delete all triggers
	for (__int32 i = 0; i < m_triggers.size(); i++)
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
}

string GameScript::loadScriptFromFile(char* luaFilename)
{
	ifstream ifs(luaFilename, ios::in | ios::binary | ios::ate);

	ifstream::pos_type fileSize = ifs.tellg();
	ifs.seekg(0, ios::beg);
	 
	vector<char> bytes(fileSize);
	ifs.read(bytes.data(), fileSize);

	return string(bytes.data(), fileSize);
}

bool GameScript::ExecuteScript(char* luaFilename)
{ 
	string script = loadScriptFromFile(luaFilename);
	m_lua->script(script);
	 
	return true;
}

bool GameScript::ExecuteTrigger(__int16 index)
{
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
}

void GameScript::EnableItem(__int16 id)
{
	if (m_itemsMap.find(id) == m_itemsMap.end())
		return;

	__int16 itemNum = m_itemsMap[id];

	ITEM_INFO* item = &Items[itemNum];

	if (!item->active)
	{
		if (Objects[item->objectNumber].intelligent)
		{
			if (item->status == ITEM_DEACTIVATED)
			{ 
				item->touchBits = 0;
				item->status = ITEM_ACTIVE;
				AddActiveItem(itemNum);
				EnableBaddieAI(itemNum, 1);
			}
			else if (item->status == ITEM_INVISIBLE)
			{
				item->touchBits = 0;
				if (EnableBaddieAI(itemNum, 0))
					item->status = ITEM_ACTIVE;
				else
					item->status = ITEM_INVISIBLE;
				AddActiveItem(itemNum);
			}
		}
		else
		{
			item->touchBits = 0;
			AddActiveItem(itemNum);
			item->status = ITEM_ACTIVE;
		}
	}
}

void GameScript::DisableItem(__int16 id)
{
	if (m_itemsMap.find(id) == m_itemsMap.end())
		return;

	__int16 itemNum = m_itemsMap[id];

	ITEM_INFO* item = &Items[itemNum];

	if (item->active)
	{
		if (Objects[item->objectNumber].intelligent)
		{
			if (item->status == ITEM_ACTIVE)
			{
				item->touchBits = 0;
				item->status = ITEM_DEACTIVATED;
				RemoveActiveItem(itemNum);
				DisableBaddieAI(itemNum);
			}
		}
		else
		{
			item->touchBits = 0;
			RemoveActiveItem(itemNum);
			item->status = ITEM_DEACTIVATED;
		}
	}
}

void GameScript::PlayAudioTrack(__int16 track)
{
	S_CDPlay(track, SOUND_TRACK_ONESHOT);
}

void GameScript::ChangeAmbientSoundTrack(__int16 track)
{
	CurrentAtmosphere = track;
	S_CDStop();
	S_CDPlay(track, SOUND_TRACK_BGM);
}

void GameScript::JumpToLevel(__int32 levelNum)
{
	if (levelNum >= g_GameFlow->GetNumLevels())
		return;
	LevelComplete = levelNum;
}

__int32 GameScript::GetSecretsCount()
{
	return Savegame.Level.Secrets;
}

void GameScript::SetSecretsCount(__int32 secretsNum)
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

void GameScript::MakeItemInvisible(__int16 id)
{
	if (m_itemsMap.find(id) == m_itemsMap.end())
		return;

	__int16 itemNum = m_itemsMap[id];

	ITEM_INFO* item = &Items[itemNum];

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

GameScriptItem GameScript::GetItem(__int16 id)
{
	if (m_itemsMap.find(id) == m_itemsMap.end())
		throw "Item not found";

	__int16 itemNum = m_itemsMap[id];
	return m_items[itemNum];
}

void GameScript::PlaySoundEffectAtPosition(__int16 id, __int32 x, __int32 y, __int32 z, __int32 flags)
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

void GameScript::PlaySoundEffect(__int16 id, __int32 flags)
{
	SoundEffect(id, NULL, flags);
}

void GameScript::AssignItemsAndLara()
{
	for (__int32 i = 0; i < NUM_ITEMS; i++)
		m_items[i].NativeItem = NULL;

	for (__int32 i = 0; i < NumItems; i++)
		m_items[i].NativeItem = &Items[i];

	(*m_lua)["Lara"] = m_items[Lara.itemNumber];
}

void GameScript::ResetVariables()
{
	(*m_lua)["Lara"] = NULL;
}

void GameScript::SetItem(__int32 index, ITEM_INFO* item)
{
	if (index >= NUM_ITEMS)
		return;
	m_items[index].NativeItem = item;
}

void GameScript::GetVariables(vector<LuaVariable>* list)
{
	LuaVariable variable;

	m_globals.for_each([&](sol::object const& key, sol::object const& value) {
		variable.IsGlobal = true;
		variable.Name = (char*)key.as<string>().c_str();
		if (value.is<bool>())
		{
			variable.Type = LUA_VARIABLE_TYPE_BOOL;
			variable.BoolValue = value.as<bool>();
		}
		else if (value.is<__int32>())
		{
			variable.Type = LUA_VARIABLE_TYPE_INT;
			variable.IntValue = value.as<__int32>();
		}
		else if (value.is<float>())
		{
			variable.Type = LUA_VARIABLE_TYPE_FLOAT;
			variable.FloatValue = value.as<float>();
		}
		else if (value.is<string>())
		{
			variable.Type = LUA_VARIABLE_TYPE_STRING;
			variable.StringValue = (char*)value.as<string>().c_str();
		}

		list->push_back(variable);
	});

	m_locals.for_each([&](sol::object const& key, sol::object const& value) {
		variable.IsGlobal = true;
		variable.Name = (char*)key.as<string>().c_str();
		if (value.is<bool>())
		{
			variable.Type = LUA_VARIABLE_TYPE_BOOL;
			variable.BoolValue = value.as<bool>();
		}
		else if (value.is<__int32>())
		{
			variable.Type = LUA_VARIABLE_TYPE_INT;
			variable.IntValue = value.as<__int32>();
		}
		else if (value.is<float>())
		{
			variable.Type = LUA_VARIABLE_TYPE_FLOAT;
			variable.FloatValue = value.as<float>();
		}
		else if (value.is<string>())
		{
			variable.Type = LUA_VARIABLE_TYPE_STRING;
			variable.StringValue = (char*)value.as<string>().c_str();
		}

		list->push_back(variable);
	});
}

void GameScript::SetVariables(vector<LuaVariable>* list)
{
	for (__int32 i = 0; i < list->size(); i++)
	{
		LuaVariable variable = (*list)[i];
		if (variable.IsGlobal)
		{
			if (variable.Type == LUA_VARIABLE_TYPE_BOOL)
				m_globals[variable.Name] = variable.BoolValue;
			else if (variable.Type == LUA_VARIABLE_TYPE_INT)
				m_globals[variable.Name] = variable.IntValue;
			else if (variable.Type == LUA_VARIABLE_TYPE_FLOAT)
				m_globals[variable.Name] = variable.FloatValue;
			else if (variable.Type == LUA_VARIABLE_TYPE_STRING)
				m_globals[variable.Name] = variable.StringValue;
		}
		else
		{
			if (variable.Type == LUA_VARIABLE_TYPE_BOOL)
				m_locals[variable.Name] = variable.BoolValue;
			else if (variable.Type == LUA_VARIABLE_TYPE_INT)
				m_locals[variable.Name] = variable.IntValue;
			else if (variable.Type == LUA_VARIABLE_TYPE_FLOAT)
				m_locals[variable.Name] = variable.FloatValue;
			else if (variable.Type == LUA_VARIABLE_TYPE_STRING)
				m_locals[variable.Name] = variable.StringValue;
		}
	}
}

GameScript* g_GameScript;