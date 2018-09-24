#include "GameLogicScript.h"
#include "..\Game\items.h"
#include "..\Game\box.h"
#include "..\Game\lot.h"
#include "..\Game\sound.h"

extern GameFlow* g_GameFlow;

GameScript::GameScript(sol::state* lua)
{
	m_lua = lua;

	(*m_lua)["HIT_POINTS"] = ITEM_PARAM_HIT_POINTS;
	(*m_lua)["CURRENT_ANIM_STATE"] = ITEM_PARAM_CURRENT_ANIM_STATE;
	(*m_lua)["ANIM_NUMBER"] = ITEM_PARAM_ANIM_NUMBER;
	
	m_lua->new_usertype<GameScriptItem>("Item",
		sol::constructors<GameScriptItem(ITEM_INFO*)>(),
		"Get", &GameScriptItem::Get,
		"Set", &GameScriptItem::Set
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
		"GetItem", &GameScript::GetItem
		);

	(*m_lua)["TR"] = this;

	// DEBUG: just for testing
	LuaFunction* function = new LuaFunction();
	function->Name = "Trigger_0";
	function->Code = "function Trigger_0() \n TR:EnableItem(2); \n TR:PlayAudioTrack(15); \n item = TR:GetItem(2); \n item:Set(HIT_POINTS, -16384); \n return true; \n end";
	m_lua->script(function->Code);
	Triggers.push_back(function);
	
	m_itemsMap.insert(pair<__int16, __int16>(0, 0));
	m_itemsMap.insert(pair<__int16, __int16>(9, 9));
	m_itemsMap.insert(pair<__int16, __int16>(10, 10));
	m_itemsMap.insert(pair<__int16, __int16>(2, 8));
}

GameScript::~GameScript()
{
	
}

void GameScript::AddTrigger(LuaFunction* function)
{
	Triggers.push_back(function);
	(*m_lua).script(function->Code);
}

void GameScript::FreeLevelScripts()
{
	// Delete all triggers
	for (__int32 i = 0; i < Triggers.size(); i++)
	{
		LuaFunction* trigger = Triggers[i];
		char* name = (char*)trigger->Name.c_str();
		(*m_lua)[name] = NULL;
		delete Triggers[i];
	}

	Triggers.clear();
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
	if (index >= Triggers.size())
		return true;

	LuaFunction* trigger = Triggers[index];

	// We want to execute a trigger just one time 
	// TODO: implement in the future continoous trigger?
	if (trigger->Executed)
		return true;

	// Get the trigger function name
	char* name = (char*)trigger->Name.c_str();

	// Execute trigger
	bool result = (*m_lua)[name]();

	// Trigger was executed, don't execute it anymore
	Triggers[index]->Executed = result;

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
		return NULL;

	__int16 itemNum = m_itemsMap[id];
	ITEM_INFO* item = &Items[itemNum];
	GameScriptItem scriptItem = GameScriptItem(item);

	return scriptItem;
}

GameScript* g_GameScript;