#pragma once
#include "ScriptInterfaceGame.h"
#include "ScriptInterfaceFlow.h"

class ScriptInterfaceState
{
public:
	static ScriptInterfaceGame* CreateGame();
	static ScriptInterfaceFlow* CreateFlow();
	static void ScriptInterfaceState::Init();
};

