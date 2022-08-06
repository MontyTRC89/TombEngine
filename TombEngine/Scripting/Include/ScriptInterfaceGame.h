#pragma once
#include <string>
#include <functional>
#include "room.h"
#include "Specific/level.h"
#include "Game/control/volumetriggerer.h"

typedef DWORD D3DCOLOR;
using VarMapVal = std::variant< short,
	std::reference_wrapper<MESH_INFO>,
	std::reference_wrapper<LEVEL_CAMERA_INFO>,
	std::reference_wrapper<SINK_INFO>,
	std::reference_wrapper<SOUND_SOURCE_INFO>,
	std::reference_wrapper<AI_OBJECT>>;

using CallbackDrawString = std::function<void(std::string const&, D3DCOLOR, int, int, int)>;

using VarSaveType = std::variant<bool, double, std::string>;

using IndexTable = std::vector<std::pair<uint32_t, uint32_t>>;

using SavedVar = std::variant<bool, std::string, double, IndexTable>;

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
	virtual void ResetScripts(bool clearGameVars) = 0;
	virtual void ExecuteScriptFile(std::string const& luaFileName) = 0;
	virtual void ExecuteFunction(std::string const& luaFuncName, TEN::Control::Volumes::VolumeTriggerer, std::string const& arguments) = 0;
	virtual void ExecuteFunction(std::string const& luaFuncName, short idOne, short idTwo = 0) = 0;

	virtual void GetVariables(std::vector<SavedVar> & vars) = 0;
	virtual void SetVariables(std::vector<SavedVar> const& vars) = 0;
};

extern ScriptInterfaceGame* g_GameScript;
