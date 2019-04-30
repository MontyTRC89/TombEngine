#pragma once

#include <sol.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include "..\Global\global.h"
#include "..\Specific\IO\ChunkId.h"
#include "..\Specific\IO\ChunkReader.h"
#include "..\Specific\IO\LEB128.h"
#include "LanguageScript.h"

struct ChunkId;
struct LEB128;

using namespace std;

typedef struct GameScriptSettings {
	__int32 ScreenWidth;
	__int32 ScreenHeight;
	bool EnableLoadSave;
	bool EnableDynamicShadows;
	bool EnableWaterCaustics;
	bool Windowed;
	string WindowTitle;
	__int32 DrawingDistance;
	bool ShowRendererSteps;
	bool ShowDebugInfo;
};

typedef struct GameScriptSkyLayer {
	bool Enabled;
	byte R;
	byte G;
	byte B;
	__int16 CloudSpeed;

	GameScriptSkyLayer()
	{
		Enabled = false;
		R = G = B = CloudSpeed = 0;
	}

	GameScriptSkyLayer(byte r, byte g, byte b, __int16 speed)
	{
		R = r;
		G = g;
		B = b;
		CloudSpeed = speed;
		Enabled = true;
	}
};

typedef struct GameScriptFog {
	byte R;
	byte G;
	byte B;

	GameScriptFog()
	{

	}

	GameScriptFog(byte r, byte g, byte b)
	{
		R = r;
		G = g;
		B = b;
	}
};

typedef struct GameScriptMirror {
	__int16 Room;
	__int32 StartX;
	__int32 EndX;
	__int32 StartZ;
	__int32 EndZ;

	GameScriptMirror()
	{
		Room = -1;
		StartX = EndX = StartZ = EndZ = 0;
	}

	GameScriptMirror(__int16 room, __int32 startX, __int32 endX, __int32 startZ, __int32 endZ)
	{
		Room = room;
		StartX = startX;
		EndX = endX;
		StartZ = startZ;
		EndZ = endZ;
	}
};

typedef struct GameScriptLevel {
	__int32 NameStringIndex;
	string FileName;
	string ScriptFileName;
	string LoadScreenFileName;
	string Background;
	__int32 Name;
	__int32 Soundtrack;
	GameScriptSkyLayer Layer1;
	GameScriptSkyLayer Layer2;
	bool Horizon;
	bool Sky;
	bool ColAddHorizon;
	GameScriptFog Fog;
	bool Storm;
	WEATHER_TYPES Weather;
	bool ResetHub;
	bool Rumble;
	LARA_DRAW_TYPE LaraType;
	GameScriptMirror Mirror;
	byte UVRotate;
	int LevelFarView;
	bool UnlimitedAir;

	GameScriptLevel()
	{
		Storm = false;
		Horizon = false;
		ColAddHorizon = false;
		ResetHub = false;
		Rumble = false;
		Weather = WEATHER_NORMAL;
		LaraType = LARA_NORMAL;
		UnlimitedAir = false;
	}
};

extern ChunkReader* g_ScriptChunkIO;

bool __cdecl readGameFlowLevelChunks(ChunkId* chunkId, __int32 maxSize, __int32 arg);
bool __cdecl readGameFlowChunks(ChunkId* chunkId, __int32 maxSize, __int32 arg);
bool __cdecl readGameFlowLevel();
bool __cdecl readGameFlowStrings();
bool __cdecl readGameFlowFlags();

bool __cdecl LoadScript();

class GameFlow
{
private:
	sol::state*							m_lua;
	GameScriptSettings					m_settings;
	
	string								loadScriptFromFile(char* luaFilename);
	map<__int16, __int16>				m_itemsMap;

public:
	Vector3								SkyColorLayer1;
	__int32								SkySpeedLayer1;
	Vector3								SkyColorLayer2;
	__int32								SkySpeedLayer2;
	Vector3								FogColor;
	__int32								FogInDistance;
	__int32								FogOutDistance;
	bool								DrawHorizon;
	bool								ColAddHorizon;
	__int32								SelectedLevelForNewGame;
	__int32								SelectedSaveGame;
	bool								EnableLoadSave;
	bool								PlayAnyLevel;
	bool								FlyCheat;
	bool								DebugMode;
	__int32								LevelFarView;

	// Selected language set
	LanguageScript*						CurrentStrings;
	vector<LanguageScript*>				Strings;
	vector<GameScriptLevel*>			Levels;

	GameFlow(sol::state* lua);
	~GameFlow();

	bool								LoadGameStrings(char* luaFilename);
	bool								LoadGameSettings(char* luaFilename);
	bool								ExecuteScript(char* luaFilename);
	char*								GetString(__int32 id);
	GameScriptSettings*					GetSettings();
	GameScriptLevel*					GetLevel(__int32 id);
	void								SetHorizon(bool horizon, bool colAddHorizon);
	void								SetLayer1(byte r, byte g, byte b, __int16 speed);
	void								SetLayer2(byte r, byte g, byte b, __int16 speed);
	void								SetFog(byte r, byte g, byte b, __int16 startDistance, __int16 endDistance);
	__int32								GetNumLevels();		
	bool								DoGameflow();
};