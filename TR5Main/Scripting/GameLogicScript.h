#pragma once
#include "items.h"
#include "room.h"
#include "LuaHandler.h"
#include "trmath.h"
#include "GameScriptColor.h"
#include "GameScriptPosition.h"
#include "GameScriptRotation.h"
#include "GameScriptItemInfo.h"
#include "GameScriptMeshInfo.h"
#include "GameScriptCameraInfo.h"

struct LuaFunction {
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

struct LuaVariable
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
	LuaVariables								m_globals;
	LuaVariables								m_locals;
	std::map<int, short>						m_itemsMapId;
	std::map<std::string, short>				m_itemsMapName;
	std::map<std::string, MESH_INFO&>			m_meshesMapName;
	std::map<std::string, LEVEL_CAMERA_INFO&>	m_camerasMapName;
	std::vector<LuaFunction*>					m_triggers;
	sol::protected_function						m_onStart;
	sol::protected_function						m_onLoad;
	sol::protected_function						m_onControlPhase;
	sol::protected_function						m_onSave;
	sol::protected_function						m_onEnd;
public:	
	GameScript(sol::state* lua);

	void								FreeLevelScripts();
	void								AddTrigger(LuaFunction* function);
	void								AddLuaId(int luaId, short itemNumber);
	bool								AddLuaName(std::string const & luaName, short itemNumber);
	bool								RemoveLuaName(std::string const& luaName);
	bool								AddLuaNameMesh(std::string const & luaName, MESH_INFO &);
	bool								RemoveLuaNameMesh(std::string const& luaName);
	bool								AddLuaNameCamera(std::string const & luaName, LEVEL_CAMERA_INFO &);
	bool								RemoveLuaNameCamera(std::string const& luaName);
	void								AssignItemsAndLara();


	bool								ExecuteTrigger(short index);
	void								ExecuteFunction(std::string const & name);
	void								MakeItemInvisible(short id);

	std::unique_ptr<GameScriptItemInfo>		GetItemById(int id);
	std::unique_ptr<GameScriptItemInfo>		GetItemByName(std::string const & name);
	std::unique_ptr<GameScriptMeshInfo>		GetMeshByName(std::string const & name);
	std::unique_ptr<GameScriptCameraInfo>	GetCameraByName(std::string const & name);

	// Variables
	template <typename T>
	void								GetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals);
	template <typename T>
	void								SetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals);
	void								ResetVariables();

	// Sound
	static void							PlayAudioTrack(std::string const & trackName, bool looped);
	void								PlaySoundEffect(int id, GameScriptPosition pos, int flags);
	void								PlaySoundEffect(int id, int flags);
	static void							SetAmbientTrack(std::string const & trackName);

	// Special FX
	void								AddLightningArc(GameScriptPosition src, GameScriptPosition dest, GameScriptColor color, int lifetime, int amplitude, int beamWidth, int segments, int flags);
	void								AddShockwave(GameScriptPosition pos, int innerRadius, int outerRadius, GameScriptColor color, int lifetime, int speed, int angle, int flags);
	void								AddSprite(GameScriptPosition pos, VectorInt3 vel, VectorInt2 falloff, GameScriptColor startColor, GameScriptColor endColor, int lifeTime, int fadeIn, int fadeOut, int spriteNum, int startSize, int endSize, float angle, int rotation);
	void								AddDynamicLight(GameScriptPosition pos, GameScriptColor color, int radius, int lifetime);
	void								AddBlood(GameScriptPosition pos, int num);
	void								AddFireFlame(GameScriptPosition pos, int size);
	void								Earthquake(int strength);

	// Inventory
	static void							InventoryAdd(GAME_OBJECT_ID slot, sol::optional<int> count);
	static void							InventoryRemove(GAME_OBJECT_ID slot, sol::optional<int> count);
	static int							InventoryGetCount(GAME_OBJECT_ID slot);
	static void							InventorySetCount(GAME_OBJECT_ID slot, int count);
	void								InventoryCombine(int slot1, int slot2);
	void								InventorySeparate(int slot);

	// Misc
	void								PrintString(std::string key, GameScriptPosition pos, GameScriptColor color, int lifetime, int flags);
	int									FindRoomNumber(GameScriptPosition pos);
	void								JumpToLevel(int levelNum);
	int									GetSecretsCount();
	void								SetSecretsCount(int secretsNum);
	void								AddOneSecret();
	int									CalculateDistance(GameScriptPosition pos1, GameScriptPosition pos2);
	int									CalculateHorizontalDistance(GameScriptPosition pos1, GameScriptPosition pos2);

	void								InitCallbacks();
	void								OnStart();
	void								OnLoad();
	void								OnControlPhase();
	void								OnSave();
	void								OnEnd();
};