#include "GameLogicScript.h"
#include "..\Game\items.h"
#include "..\Game\box.h"
#include "..\Game\lot.h"
#include "..\Game\sound.h"

GameScript::GameScript(sol::state* lua)
{
	m_lua = lua;
	
	// Gameflow type
	m_lua->new_usertype<GameScript>("GameScript",
		"EnableItem", &GameScript::EnableItem,
		"DisableItem", &GameScript::DisableItem,
		"PlayAudioTrack", &GameScript::PlayAudioTrack,
		"ChangeAmbientSoundTrack", &GameScript::ChangeAmbientSoundTrack
		);

	(*m_lua)["TR"] = this;
}

GameScript::~GameScript()
{
	
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

GameScript* g_GameScript;