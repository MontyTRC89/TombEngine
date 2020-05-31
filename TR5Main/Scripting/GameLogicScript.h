#pragma once
#include "ChunkId.h"
#include "ChunkReader.h"
#include "ChunkWriter.h"
#include "LEB128.h"
#include "Streams.h"
#include "items.h"

#define ITEM_PARAM_currentAnimState		0
#define ITEM_PARAM_goalAnimState			1
#define ITEM_PARAM_REQUIRED_ANIM_STATE		2
#define ITEM_PARAM_frameNumber				3
#define ITEM_PARAM_animNumber				4
#define ITEM_PARAM_hitPoints				5
#define ITEM_PARAM_HIT_STATUS				6
#define ITEM_PARAM_GRAVITY_STATUS			7
#define ITEM_PARAM_COLLIDABLE				8
#define ITEM_PARAM_POISONED					9
#define ITEM_PARAM_roomNumber				10

#define LUA_VARIABLE_TYPE_INT				0
#define LUA_VARIABLE_TYPE_BOOL				1
#define LUA_VARIABLE_TYPE_FLOAT				2
#define LUA_VARIABLE_TYPE_STRING			3

typedef struct LuaFunction {
	string Name;
	string Code;
	bool Executed;
};

class GameScriptPosition {
private:
	float								xPos;
	float								yPos;
	float								zPos;
	function<float()>					readXPos;
	function<void(float)>				writeXPos;
	function<float()>					readYPos;
	function<void(float)>				writeYPos;
	function<float()>					readZPos;
	function<void(float)>				writeZPos;

public:
	GameScriptPosition(float x, float y, float z);
	GameScriptPosition(function<float()> readX, function<void(float)> writeX, function<float()> readY, function<void(float)> writeY, function<float()> readZ, function<void(float)> writeZ);

	float								GetXPos();
	void								SetXPos(float x);
	float								GetYPos();
	void								SetYPos(float y);
	float								GetZPos();
	void								SetZPos(float z);
};

class GameScriptRotation {
private:
	float								xRot;
	float								yRot;
	float								zRot;
	function<float()>					readXRot;
	function<void(float)>				writeXRot;
	function<float()>					readYRot;
	function<void(float)>				writeYRot;
	function<float()>					readZRot;
	function<void(float)>				writeZRot;

public:
	GameScriptRotation(float x, float y, float z);
	GameScriptRotation(function<float()> readX, function<void(float)> writeX, function<float()> readY, function<void(float)> writeY, function<float()> readZ, function<void(float)> writeZ);

	float								GetXRot();
	void								SetXRot(float x);
	float								GetYRot();
	void								SetYRot(float y);
	float								GetZRot();
	void								SetZRot(float z);
};

class GameScriptItem {
private:
	short								NativeItemNumber;
	ITEM_INFO*							NativeItem;

public:
	GameScriptItem(short itemNumber);

	GameScriptPosition					GetPosition();
	GameScriptRotation					GetRotation();
	short								GetHP();
	void								SetHP(short hp);
	short								GetRoom();
	void								SetRoom(short room);
	short								GetCurrentState();
	void								SetCurrentState(short state);
	short								GetGoalState();
	void								SetGoalState(short state);
	short								GetRequiredState();
	void								SetRequiredState(short state);
	void								EnableItem();
	void								DisableItem();
};

class LuaVariables
{
public:
	map<string, sol::object>			variables;

	sol::object							GetVariable(string key);
	void								SetVariable(string key, sol::object value);
};

typedef struct LuaVariable
{
	bool IsGlobal;
	string Name;
	int Type;
	float FloatValue;
	int IntValue;
	string StringValue;
	bool BoolValue;
};

class GameScript
{
private:
	sol::state*							m_lua;
	LuaVariables						m_globals;
	LuaVariables						m_locals;
	map<int, short>						m_itemsMapId;
	map<string, short>					m_itemsMapName;
	vector<LuaFunction*>				m_triggers;

public:	
	GameScript(sol::state* lua);

	bool								ExecuteScript(const string& luaFilename, string& message);
	bool								ExecuteString(const string& command, string& message);
	void								FreeLevelScripts();
	void								AddTrigger(LuaFunction* function);
	void								AddLuaId(int luaId, short itemNumber);
	void								AddLuaName(string luaName, short itemNumber);
	void								AssignItemsAndLara();
	void								ResetVariables();

	template <typename T>
	void								GetVariables(map<string, T>& locals, map<string, T>& globals);
	template <typename T>
	void								SetVariables(map<string, T>& locals, map<string, T>& globals);
	void								PlayAudioTrack(short track);
	void								ChangeAmbientSoundTrack(short track);
	bool								ExecuteTrigger(short index);
	void								JumpToLevel(int levelNum);
	int									GetSecretsCount();
	void								SetSecretsCount(int secretsNum);
	void								AddOneSecret();
	void								MakeItemInvisible(short id);
	unique_ptr<GameScriptItem>			GetItemById(int id);
	unique_ptr<GameScriptItem>			GetItemByName(string name);
	void								PlaySoundEffectAtPosition(short id, int x, int y, int z, int flags);
	void								PlaySoundEffect(short id, int flags);
	GameScriptPosition					CreatePosition(float x, float y, float z);
	GameScriptPosition					CreateSectorPosition(float x, float y, float z);
	GameScriptRotation					CreateRotation(float x, float y, float z);
	float								CalculateDistance(GameScriptPosition pos1, GameScriptPosition pos2);
	float								CalculateHorizontalDistance(GameScriptPosition pos1, GameScriptPosition pos2);
};