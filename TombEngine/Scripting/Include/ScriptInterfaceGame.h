#pragma once
#include <functional>
#include <string>

#include "Game/control/volumeactivator.h"
#include "Game/room.h"
#include "Scripting/Include/VarMapVal.h"
#include "Specific/level.h"

typedef DWORD D3DCOLOR;

using CallbackDrawString = std::function<void(const std::string&, D3DCOLOR, int, int, int)>;
using VarSaveType = std::variant<bool, double, std::string>;
using IndexTable = std::vector<std::pair<uint32_t, uint32_t>>;

struct FuncName
{
	std::string name;
};

enum class SavedVarType
{
	Bool,
	String,
	Number,
	IndexTable,
	Vec3,
	Rotation,
	Color,
	FuncName,

	NumTypes
};

using SavedVar = std::variant<
	bool,
	std::string,
	double,
	IndexTable,
	Vector3i, // Vec3
	Vector3,  // Rotation
	D3DCOLOR, // Color
	FuncName>;

// Make sure SavedVarType and SavedVar have same number of types.
static_assert(static_cast<int>(SavedVarType::NumTypes) == std::variant_size_v<SavedVar>);

class ScriptInterfaceGame
{
public:
	virtual ~ScriptInterfaceGame() = default;
	
	virtual void InitCallbacks() = 0;

	virtual void OnStart() = 0;
	virtual void OnLoad() = 0;
	virtual void OnControlPhase(float deltaTime) = 0;
	virtual void OnSave() = 0;
	virtual void OnEnd() = 0;
	virtual void ShortenTENCalls() = 0;

	virtual void FreeLevelScripts() = 0;
	virtual void ResetScripts(bool clearGameVars) = 0;
	virtual void ExecuteScriptFile(const std::string& luaFileName) = 0;
	virtual void ExecuteString(const std::string& command) = 0;
	virtual void ExecuteFunction(const std::string& luaFuncName, TEN::Control::Volumes::VolumeActivator, const std::string& arguments) = 0;
	virtual void ExecuteFunction(const std::string& luaFuncName, short idOne, short idTwo = 0) = 0;

	virtual void GetVariables(std::vector<SavedVar>& vars) = 0;
	virtual void SetVariables(const std::vector<SavedVar>& vars) = 0;

	virtual void GetCallbackStrings(std::vector<std::string>& preControl, std::vector<std::string>& postControl) const = 0;
	virtual void SetCallbackStrings(const std::vector<std::string>& preControl, const std::vector<std::string>& postControl) = 0;
};

extern ScriptInterfaceGame* g_GameScript;
