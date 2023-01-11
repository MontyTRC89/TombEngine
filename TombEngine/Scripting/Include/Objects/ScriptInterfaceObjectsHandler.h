#pragma once
#include <functional>
#include <string>

#include "Specific/level.h"

typedef DWORD D3DCOLOR;
using VarMapVal = std::variant<
	short,
	std::reference_wrapper<MESH_INFO>,
	std::reference_wrapper<LevelCameraInfo>,
	std::reference_wrapper<SinkInfo>,
	std::reference_wrapper<SoundSourceInfo>,
	std::reference_wrapper<TriggerVolume>,
	std::reference_wrapper<AI_OBJECT>,
	std::reference_wrapper<ROOM_INFO>>;

using CallbackDrawString = std::function<void(const std::string&, D3DCOLOR, int, int, int)>;

class ScriptInterfaceObjectsHandler
{
public:
	virtual ~ScriptInterfaceObjectsHandler() = default;

	[[nodiscard]] virtual short GetIndexByName(const std::string& name) const = 0;
	virtual bool AddName(const std::string& key, VarMapVal val) = 0;
	virtual bool NotifyKilled(ItemInfo*) = 0;
	virtual void FreeEntities() = 0;
	virtual void AssignLara() = 0;

	virtual bool TryAddColliding(short id) = 0;
	virtual bool TryRemoveColliding(short id, bool force = false) = 0;
	virtual void TestCollidingObjects() = 0;
};

extern ScriptInterfaceObjectsHandler* g_GameScriptEntities;
