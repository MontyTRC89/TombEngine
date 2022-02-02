#pragma once
#include "ScriptInterfaceGame.h"
#include "ScriptInterfaceFlow.h"
#include "Entities/ScriptInterfaceEntities.h"

class ScriptInterfaceState
{
public:
	static ScriptInterfaceGame* CreateGame();
	static ScriptInterfaceFlow* CreateFlow();
	static ScriptInterfaceEntities* CreateEntities();
	static void ScriptInterfaceState::Init();
};

