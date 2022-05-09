#pragma once
#include <string>
#include <functional>
#include "room.h"
#include "Specific/level.h"

using CallbackDrawString = std::function<void(std::string const&, D3DCOLOR, int, int, int)>;

class ScriptInterfaceStringsHandler {
public:
	virtual ~ScriptInterfaceStringsHandler() = default;
	virtual void ProcessDisplayStrings(float dt) = 0;
	virtual void SetCallbackDrawString(CallbackDrawString) = 0;
};

extern ScriptInterfaceStringsHandler* g_GameStringsHandler;