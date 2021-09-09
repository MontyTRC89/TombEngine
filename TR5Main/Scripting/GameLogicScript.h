#pragma once
#include "items.h"
#include "room.h"
#include "LuaHandler.h"
#include "Specific\trmath.h"
#include <unordered_set>
#include "GameScriptColor.h"
#include "GameScriptPosition.h"
#include "GameScriptRotation.h"
#include "GameScriptItemInfo.h"
#include "GameScriptMeshInfo.h"
#include "GameScriptSinkInfo.h"
#include "GameScriptAIObject.h"
#include "GameScriptSoundSourceInfo.h"
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

	sol::object							GetVariable(sol::table tab, std::string key);
	void								SetVariable(sol::table tab, std::string key, sol::object value);
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
	LuaVariables										m_globals{};
	LuaVariables										m_locals{};
	std::unordered_map<std::string, short>				m_itemsMapName{};
	std::unordered_map<std::string, MESH_INFO&>			m_meshesMapName{};
	std::unordered_map<std::string, LEVEL_CAMERA_INFO&>	m_camerasMapName{};
	std::unordered_map<std::string, SINK_INFO&>			m_sinksMapName{};
	std::unordered_map<std::string, SOUND_SOURCE_INFO&>	m_soundSourcesMapName{};
	std::unordered_map<std::string, AI_OBJECT&>			m_aiObjectsMapName{};
	std::unordered_set<std::string>						m_levelFuncs{};
	sol::protected_function								m_onStart{};
	sol::protected_function								m_onLoad{};
	sol::protected_function								m_onControlPhase{};
	sol::protected_function								m_onSave{};
	sol::protected_function								m_onEnd{};

	void ResetLevelTables();

public:	
	GameScript(sol::state* lua);

	void								FreeLevelScripts();

	bool								AddLuaNameItem(std::string const & luaName, short itemNumber);
	bool								RemoveLuaNameItem(std::string const& luaName);

	bool								AddLuaNameMesh(std::string const & luaName, MESH_INFO &);
	bool								RemoveLuaNameMesh(std::string const& luaName);

	bool								AddLuaNameCamera(std::string const & luaName, LEVEL_CAMERA_INFO &);
	bool								RemoveLuaNameCamera(std::string const& luaName);

	bool								AddLuaNameSink(std::string const & luaName, SINK_INFO &);
	bool								RemoveLuaNameSink(std::string const& luaName);

	bool								AddLuaNameSoundSource(std::string const& luaName, SOUND_SOURCE_INFO&);
	bool								RemoveLuaNameSoundSource(std::string const& luaName);

	bool								AddLuaNameAIObject(std::string const & luaName, AI_OBJECT &);
	bool								RemoveLuaNameAIObject(std::string const& luaName);

	bool								SetLevelFunc(sol::table tab, std::string const& luaName, sol::object obj);
	sol::protected_function				GetLevelFunc(sol::table tab, std::string const& luaName);

	void								AssignItemsAndLara();


	void								ExecuteFunction(std::string const & name);
	void								MakeItemInvisible(short id);

	std::unique_ptr<GameScriptItemInfo>			GetItemByName(std::string const & name);
	std::unique_ptr<GameScriptMeshInfo>			GetMeshByName(std::string const & name);
	std::unique_ptr<GameScriptCameraInfo>		GetCameraByName(std::string const & name);
	std::unique_ptr<GameScriptSinkInfo>			GetSinkByName(std::string const & name);
	std::unique_ptr<GameScriptSoundSourceInfo>	GetSoundSourceByName(std::string const & name);
	std::unique_ptr<GameScriptAIObject>			GetAIObjectByName(std::string const & name);

	// Variables
	template <typename T>
	void								GetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals);
	template <typename T>
	void								SetVariables(std::map<std::string, T>& locals, std::map<std::string, T>& globals);
	void								ResetVariables();


	void								InitCallbacks();
	void								OnStart();
	void								OnLoad();
	void								OnControlPhase(float dt);
	void								OnSave();
	void								OnEnd();
};
