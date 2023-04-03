#pragma once
#include <functional>
#include <string>

#include "Game/room.h"
#include "Specific/level.h"

using CallbackDrawString = std::function<void(const std::string&, D3DCOLOR, int, int, int)>;

class ScriptInterfaceStringsHandler
{
public:
	virtual ~ScriptInterfaceStringsHandler() = default;
	virtual void ProcessDisplayStrings(float deltaTime) = 0;
	virtual void ClearDisplayStrings() = 0;
	virtual void SetCallbackDrawString(CallbackDrawString) = 0;
};

extern ScriptInterfaceStringsHandler* g_GameStringsHandler;
