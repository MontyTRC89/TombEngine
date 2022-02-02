#pragma once
#include "ScriptInterfaceGame.h"
#include "ScriptInterfaceFlow.h"
#include "Entity/ScriptInterfaceEntity.h"

class ScriptInterfaceState
{
public:
	static ScriptInterfaceGame* CreateGame();
	static ScriptInterfaceFlow* CreateFlow();
	static ScriptInterfaceEntity* CreateEntities();
	static void ScriptInterfaceState::Init();
};

