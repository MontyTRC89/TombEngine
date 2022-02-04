#pragma once
#include "ScriptInterfaceGame.h"
#include "ScriptInterfaceFlow.h"
#include "Entities/ScriptInterfaceEntities.h"
#include "Strings/ScriptInterfaceStringsHandler.h"

class ScriptInterfaceState
{
public:
	static ScriptInterfaceGame* CreateGame();
	static ScriptInterfaceFlow* CreateFlow();
	static ScriptInterfaceEntities* CreateEntities();
	static ScriptInterfaceStringsHandler* CreateStringsHandler();
	static void ScriptInterfaceState::Init();
};

