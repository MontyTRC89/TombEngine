#pragma once
#include "ChunkId.h"
#include "ChunkReader.h"
#include "ChunkWriter.h"
#include "LEB128.h"
#include "Streams.h"
#include "items.h"
#include "LuaHandler.h"
#include "trmath.h"

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
	std::string Name;
	std::string Code;
	bool Executed;
};

struct GameScriptVector3 {
	float x;
	float y;
	float z;
};

class GameScriptColor {
public:
	byte r;
	byte g;
	byte b;
	byte a;

	GameScriptColor(byte r, byte g, byte b);
	GameScriptColor(byte r, byte g, byte b, byte a);

	byte								GetR();
	void								SetR(byte v);
	byte								GetG();
	void								SetG(byte v);
	byte								GetB();
	void								SetB(byte v);
	byte								GetA();
	void								SetA(byte v);
};

class GameScriptPosition {
public:
	int x;
	int y;
	int z;

	GameScriptPosition(int x, int y, int z);

	int								GetX();
	void							SetX(int x);
	int								GetY();
	void							SetY(int y);
	int								GetZ();
	void							SetZ(int z);
};

class GameScriptRotation {
private:
	int								x;
	int								y;
	int								z;

public:
	GameScriptRotation(int x, int y, int z);

	int								GetX();
	void							SetX(int x);
	int								GetY();
	void							SetY(int y);
	int								GetZ();
	void							SetZ(int z);
};

class GameScriptItem {
private:
	short								NativeItemNumber;
	ITEM_INFO*							NativeItem;

public:
	GameScriptItem(short itemNumber);

	//GameScriptPosition					GetPosition();
	//GameScriptRotation					GetRotation();
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
	std::map<std::string, sol::object>			variables;

	sol::object							GetVariable(std::string key);
	void								SetVariable(std::string key, sol::object value);
};

typedef struct LuaVariable
{
	bool IsGlobal;
	std::string Name;
	int Type;
	float FloatValue;
	int IntValue;
	std::string StringValue;
	bool BoolValue;
};

class GameScript : public LuaHandler
{
private:
	LuaVariables						m_globals;
	LuaVariables						m_locals;
	std::map<int, short>						m_itemsMapId;
	std::map<std::string, short>					m_itemsMapName;
	std::vector<LuaFunction*>				m_triggers;

public:	
	GameScript(sol::state* lua);

	void								FreeLevelScripts();
	void								AddTrigger(LuaFunction* function);
	void								AddLuaId(int luaId, short itemNumber);
	void								AddLuaName(std::string luaName, short itemNumber);
	void								AssignItemsAndLara();


	bool								ExecuteTrigger(short index);
	void								MakeItemInvisible(short id);
	std::unique_ptr<GameScriptItem>			GetItemById(int id);
	std::unique_ptr<GameScriptItem>			GetItemByName(std::string name);

	// Variables
	template <typename T>
	void								GetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals);
	template <typename T>
	void								SetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals);
	void								ResetVariables();

	// Sound
	void								PlayAudioTrack(std::string trackName, bool looped);
	void								PlaySoundEffect(int id, GameScriptPosition pos, int flags);
	void								PlaySoundEffect(int id, int flags);
	void								SetAmbientTrack(std::string trackName);

	// Special FX
	void								AddLightningArc(GameScriptPosition src, GameScriptPosition dest, GameScriptColor color, int lifetime, int amplitude, int beamWidth, int segments, int flags);
	void								AddShockwave(GameScriptPosition pos, int innerRadius, int outerRadius, GameScriptColor color, int lifetime, int speed, int angle, int flags);
	void								AddSprite(GameScriptPosition pos, VectorInt3 vel, VectorInt2 falloff, GameScriptColor startColor, GameScriptColor endColor, int lifeTime, int fadeIn, int fadeOut, int spriteNum, int startSize, int endSize, float angle, int rotation);
	void								AddDynamicLight(GameScriptPosition pos, GameScriptColor color, int radius, int lifetime);
	void								AddBlood(GameScriptPosition pos, int num);
	void								AddFireFlame(GameScriptPosition pos, int size);
	void								Earthquake(int strength);

	// Inventory
	void								InventoryAdd(int slot, int count);
	void								InventoryRemove(int slot, int count);
	void								InventoryGetCount(int slot);
	void								InventorySetCount(int slot, int count);
	void								InventoryCombine(int slot1, int slot2);
	void								InventorySepare(int slot);

	// Misc
	void								PrintString(std::string key, GameScriptPosition pos, GameScriptColor color, int lifetime, int flags);
	int									FindRoomNumber(GameScriptPosition pos);
	void								JumpToLevel(int levelNum);
	int									GetSecretsCount();
	void								SetSecretsCount(int secretsNum);
	void								AddOneSecret();
	int									CalculateDistance(GameScriptPosition pos1, GameScriptPosition pos2);
	int									CalculateHorizontalDistance(GameScriptPosition pos1, GameScriptPosition pos2);
};