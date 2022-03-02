#pragma once
#include <string>
#include <functional>
#include "room.h"
#include "Specific/level.h"

typedef DWORD D3DCOLOR;
using VarMapVal = std::variant< short,
	std::reference_wrapper<MESH_INFO>,
	std::reference_wrapper<LEVEL_CAMERA_INFO>,
	std::reference_wrapper<SINK_INFO>,
	std::reference_wrapper<SOUND_SOURCE_INFO>,
	std::reference_wrapper<AI_OBJECT>>;

using CallbackDrawString = std::function<void(std::string const&, D3DCOLOR, int, int, int)>;

using VarSaveType = std::variant<bool, double, std::string>;

class ScriptInterfaceGame {
public:
	virtual ~ScriptInterfaceGame() = default;
	
	virtual void InitCallbacks() = 0;

	virtual void OnStart() = 0;
	virtual void OnLoad() = 0;
	virtual void OnControlPhase(float dt) = 0;
	virtual void OnSave() = 0;
	virtual void OnEnd() = 0;

	virtual void FreeLevelScripts() = 0;
	virtual void ExecuteScriptFile(std::string const& luaFileName) = 0;
	virtual void ExecuteFunction(std::string const& luaFileName) = 0;

	//todo make this const correct
	virtual void SetVariables(std::map<std::string, VarSaveType>& locals, std::map<std::string, VarSaveType>& globals) = 0;

	//todo make this const correct
	virtual void GetVariables(std::map<std::string, VarSaveType>& locals, std::map<std::string, VarSaveType>& globals) = 0;
};

extern ScriptInterfaceGame* g_GameScript;
