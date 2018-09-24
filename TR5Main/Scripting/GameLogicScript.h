#pragma once

#include <sol.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include "..\Global\global.h"

using namespace std;

#define ITEM_PARAM_CURRENT_ANIM_STATE		0
#define ITEM_PARAM_GOAL_ANIM_STATE			1
#define ITEM_PARAM_REQUIRED_ANIM_STATE		2
#define ITEM_PARAM_FRAME_NUMBER				3
#define ITEM_PARAM_ANIM_NUMBER				4
#define ITEM_PARAM_HIT_POINTS				5
#define ITEM_PARAM_HIT_STATUS				6
#define ITEM_PARAM_GRAVITY_STATUS			7
#define ITEM_PARAM_COLLIDABLE				8
#define ITEM_PARAM_POISONED					9

typedef struct LuaFunction {
	string Name;
	string Code;
	bool Executed;
};

typedef struct GameScriptItem {
private:
	ITEM_INFO*		m_item;

public:
	GameScriptItem(ITEM_INFO* item)
	{
		m_item = item;
	}

	__int16 Get(__int32 param)
	{
		switch (param)
		{
		case ITEM_PARAM_CURRENT_ANIM_STATE:
			return m_item->currentAnimState;
		case ITEM_PARAM_REQUIRED_ANIM_STATE:
			return m_item->requiredAnimState;
		case ITEM_PARAM_GOAL_ANIM_STATE:
			return m_item->goalAnimState;
		case ITEM_PARAM_ANIM_NUMBER:
			return m_item->animNumber;
		case ITEM_PARAM_FRAME_NUMBER:
			return m_item->frameNumber;
		case ITEM_PARAM_HIT_POINTS:
			return m_item->hitPoints;
		case ITEM_PARAM_HIT_STATUS:
			return m_item->hitStatus;
		case ITEM_PARAM_GRAVITY_STATUS:
			return m_item->gravityStatus;
		case ITEM_PARAM_COLLIDABLE:
			return m_item->collidable;
		case ITEM_PARAM_POISONED:
			return m_item->poisoned;
		default:
			return 0;
		}
	}

	void Set(__int32 param, __int16 value)
	{
		switch (param)
		{
		case ITEM_PARAM_CURRENT_ANIM_STATE:
			m_item->currentAnimState = value; break;
		case ITEM_PARAM_REQUIRED_ANIM_STATE:
			m_item->requiredAnimState = value; break;
		case ITEM_PARAM_GOAL_ANIM_STATE:
			m_item->goalAnimState = value; break;
		case ITEM_PARAM_ANIM_NUMBER:
			m_item->animNumber = value; 
			m_item->frameNumber = Anims[m_item->animNumber].frameBase;
			break;
		case ITEM_PARAM_FRAME_NUMBER:
			m_item->frameNumber = value; break;
		case ITEM_PARAM_HIT_POINTS:
			m_item->hitPoints = value; break;
		case ITEM_PARAM_HIT_STATUS:
			m_item->hitStatus = value; break;
		case ITEM_PARAM_GRAVITY_STATUS:
			m_item->gravityStatus = value; break;
		case ITEM_PARAM_COLLIDABLE:
			m_item->collidable = value; break;
		case ITEM_PARAM_POISONED:
			m_item->poisoned = value; break;
		default:
			break;
		}
	}
};

class GameScript
{
private:
	sol::state*							m_lua;

	string								loadScriptFromFile(char* luaFilename);
	map<__int16, __int16>				m_itemsMap;

public:
	vector<LuaFunction*>		Triggers;

	GameScript(sol::state* lua);
	~GameScript();
	
	bool								ExecuteScript(char* luaFilename);
	void								FreeLevelScripts();
	void								AddTrigger(LuaFunction* function);

	void								EnableItem(__int16 id);
	void								DisableItem(__int16 id);
	void								PlayAudioTrack(__int16 track);
	void								ChangeAmbientSoundTrack(__int16 track);
	bool								ExecuteTrigger(__int16 index);
	void								JumpToLevel(__int32 levelNum);
	__int32								GetSecretsCount();
	void								SetSecretsCount(__int32 secretsNum);
	void								AddOneSecret();
	void								MakeItemInvisible(__int16 id);
	GameScriptItem						GetItem(__int16 id);
};