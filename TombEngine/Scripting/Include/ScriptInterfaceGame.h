#pragma once
#include <string>
#include <functional>

#include "Game/control/volumetriggerer.h"
#include "Game/room.h"
#include "Specific/level.h"

typedef DWORD D3DCOLOR;
using VarMapVal = std::variant< short,
	std::reference_wrapper<MESH_INFO>,
	std::reference_wrapper<LevelCameraInfo>,
	std::reference_wrapper<SinkInfo>,
	std::reference_wrapper<SoundSourceInfo>,
	std::reference_wrapper<AI_OBJECT>>;

using CallbackDrawString = std::function<void(std::string const&, D3DCOLOR, int, int, int)>;

using VarSaveType = std::variant<bool, double, std::string>;

using IndexTable = std::vector<std::pair<uint32_t, uint32_t>>;

struct FuncName
{
	std::string name;
};

using SavedVar = std::variant<bool, std::string, double, IndexTable, Vector3Int, FuncName>;

class ScriptInterfaceGame
{
public:
	virtual ~ScriptInterfaceGame() = default;
	
	virtual void InitCallbacks() = 0;

	virtual void OnStart() = 0;
	virtual void OnLoad() = 0;
	virtual void OnControlPhase(float dt) = 0;
	virtual void OnSave() = 0;
	virtual void OnEnd() = 0;
	virtual void ShortenTENCalls() = 0;

	virtual void FreeLevelScripts() = 0;
	virtual void ResetScripts(bool clearGameVars) = 0;
	virtual void ExecuteScriptFile(std::string const& luaFileName) = 0;
	virtual void ExecuteString(std::string const& command) = 0;
	virtual void ExecuteFunction(std::string const& luaFuncName, TEN::Control::Volumes::VolumeTriggerer, std::string const& arguments) = 0;
	virtual void ExecuteFunction(std::string const& luaFuncName, short idOne, short idTwo = 0) = 0;

	virtual void GetVariables(std::vector<SavedVar> & vars) = 0;
	virtual void SetVariables(std::vector<SavedVar> const& vars) = 0;

	virtual void GetCallbackStrings(std::vector<std::string> & preControl, std::vector<std::string> & postControl) const = 0;
	virtual void SetCallbackStrings(std::vector<std::string> const & preControl, std::vector<std::string> const & postControl) = 0;
};

extern ScriptInterfaceGame* g_GameScript;
