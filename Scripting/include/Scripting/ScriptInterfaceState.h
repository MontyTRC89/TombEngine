#pragma once
#include "ScriptInterfaceGame.h"
#include "ScriptInterfaceFlow.h"
#include "Objects/ScriptInterfaceObjectsHandler.h"
#include "Strings/ScriptInterfaceStringsHandler.h"

class ScriptInterfaceState
{
public:
	static ScriptInterfaceGame* CreateGame();
	static ScriptInterfaceFlow* CreateFlow();
	static ScriptInterfaceObjectsHandler* CreateEntities();
	static ScriptInterfaceStringsHandler* CreateStringsHandler();
	static void ScriptInterfaceState::Init();
};

