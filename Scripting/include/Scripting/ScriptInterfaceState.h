#pragma once
#include "ScriptInterfaceGame.h"
#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Objects/ScriptInterfaceObjectsHandler.h"
#include "Strings/ScriptInterfaceStringsHandler.h"

class ScriptInterfaceState
{
public:
	static ScriptInterfaceGame* CreateGame();
	static ScriptInterfaceFlowHandler* CreateFlow();
	static ScriptInterfaceObjectsHandler* CreateObjectsHandler();
	static ScriptInterfaceStringsHandler* CreateStringsHandler();
	static void ScriptInterfaceState::Init();
};

