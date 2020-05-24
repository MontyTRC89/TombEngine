#pragma once

#include <sol.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include "global.h"
#include "IO\ChunkId.h"
#include "IO\ChunkReader.h"
#include "IO\LEB128.h"
#include "LanguageScript.h"

#define TITLE_FLYBY			0
#define TITLE_BACKGROUND	1

struct ChunkId;
struct LEB128;

using namespace std;

struct GameScriptSettings {
	int ScreenWidth;
	int ScreenHeight;
	bool EnableLoadSave;
	bool EnableDynamicShadows;
	bool EnableWaterCaustics;
	bool Windowed;
	string WindowTitle;
	int DrawingDistance;
	bool ShowRendererSteps;
	bool ShowDebugInfo;
};

struct GameScriptSkyLayer {
	bool Enabled;
	byte R;
	byte G;
	byte B;
	short CloudSpeed;

	GameScriptSkyLayer()
	{
		Enabled = false;
		R = G = B = CloudSpeed = 0;
	}

	GameScriptSkyLayer(byte r, byte g, byte b, short speed)
	{
		R = r;
		G = g;
		B = b;
		CloudSpeed = speed;
		Enabled = true;
	}
};

struct GameScriptFog {
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

struct GameScriptMirror {
	short Room;
	int StartX;
	int EndX;
	int StartZ;
	int EndZ;

	GameScriptMirror()
	{
		Room = -1;
		StartX = EndX = StartZ = EndZ = 0;
	}

	GameScriptMirror(short room, int startX, int endX, int startZ, int endZ)
	{
		Room = room;
		StartX = startX;
		EndX = endX;
		StartZ = startZ;
		EndZ = endZ;
	}
};

struct GameScriptLevel {
	int NameStringIndex;
	string FileName;
	string ScriptFileName;
	string LoadScreenFileName;
	string Background;
	int Name;
	int Soundtrack;
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

bool __cdecl readGameFlowLevelChunks(ChunkId* chunkId, int maxSize, int arg);
bool __cdecl readGameFlowChunks(ChunkId* chunkId, int maxSize, int arg);
bool __cdecl readGameFlowLevel();
bool __cdecl readGameFlowStrings();
bool __cdecl readGameFlowTracks();
bool __cdecl readGameFlowFlags();

bool __cdecl LoadScript();

class GameFlow
{
private:
	sol::state*							m_lua;
	GameScriptSettings					m_settings;
	
	string								loadScriptFromFile(char* luaFilename);
	map<short, short>				m_itemsMap;

public:
	Vector3								SkyColorLayer1;
	int								SkySpeedLayer1;
	Vector3								SkyColorLayer2;
	int								SkySpeedLayer2;
	Vector3								FogColor;
	int								FogInDistance;
	int								FogOutDistance;
	bool								DrawHorizon;
	bool								ColAddHorizon;
	int								SelectedLevelForNewGame;
	int								SelectedSaveGame;
	bool								EnableLoadSave;
	bool								PlayAnyLevel;
	bool								FlyCheat;
	bool								DebugMode;
	int								LevelFarView;
	int								TitleType;
	char*								Intro;

	// Selected language set
	LanguageScript*						CurrentStrings;
	vector<LanguageScript*>				Strings;
	vector<GameScriptLevel*>			Levels;

	GameFlow(sol::state* lua);
	~GameFlow();

	bool								LoadGameStrings(char* luaFilename);
	bool								LoadGameSettings(char* luaFilename);
	bool								ExecuteScript(char* luaFilename);
	char*								GetString(int id);
	GameScriptSettings*					GetSettings();
	GameScriptLevel*					GetLevel(int id);
	void								SetHorizon(bool horizon, bool colAddHorizon);
	void								SetLayer1(byte r, byte g, byte b, short speed);
	void								SetLayer2(byte r, byte g, byte b, short speed);
	void								SetFog(byte r, byte g, byte b, short startDistance, short endDistance);
	int								GetNumLevels();		
	bool								DoGameflow();
};