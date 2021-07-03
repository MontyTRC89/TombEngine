#pragma once
#include "ChunkId.h"
#include "ChunkReader.h"
#include "ChunkWriter.h"
#include "LEB128.h"
#include "Streams.h"
#include "items.h"
#include "LuaHandler.h"
#include "trmath.h"
#include "GameScriptColor.h"
#include "GameScriptPosition.h"
#include "GameScriptRotation.h"
#include "GameScriptItemInfo.h"

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
	std::map<int, short>				m_itemsMapId;
	std::map<std::string, short>		m_itemsMapName;
	std::vector<LuaFunction*>			m_triggers;
	sol::protected_function				m_onStart;
	sol::protected_function				m_onLoad;
	sol::protected_function				m_onControlPhase;
	sol::protected_function				m_onSave;
	sol::protected_function				m_onEnd;
public:	
	GameScript(sol::state* lua);

	void								FreeLevelScripts();
	void								AddTrigger(LuaFunction* function);
	void								AddLuaId(int luaId, short itemNumber);
	void								AddLuaName(std::string luaName, short itemNumber);
	void								AssignItemsAndLara();


	bool								ExecuteTrigger(short index);
	void								MakeItemInvisible(short id);
	std::unique_ptr<GameScriptItemInfo>	GetItemById(int id);
	std::unique_ptr<GameScriptItemInfo>	GetItemByName(std::string name);

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
	void								SetAmbientTrack(std::string const & trackName);

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

	void InitCallbacks();
	void OnStart();
	void OnLoad();
	void OnControlPhase();
	void OnSave();
	void OnEnd();
};