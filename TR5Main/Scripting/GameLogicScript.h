#pragma once

#include <sol.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include "..\Global\global.h"

using namespace std;

class GameScript
{
private:
	sol::state*							m_lua;

	string								loadScriptFromFile(char* luaFilename);
	map<__int16, __int16>				m_itemsMap;

public:
	GameScript(sol::state* lua);
	~GameScript();

	bool								ExecuteScript(char* luaFilename);
	void								EnableItem(__int16 id);
	void								DisableItem(__int16 id);
	void								PlayAudioTrack(__int16 track);
	void								ChangeAmbientSoundTrack(__int16 track);
};