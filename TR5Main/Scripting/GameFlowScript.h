#pragma once

#include <sol.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include "..\Global\global.h"

using namespace std;

#define NUM_STRINGS 10000
#define NUM_LEVELS 100

// Define string ids
#define STRING_INV_PASSPORT				1
#define STRING_INV_LARA_HOME			2
#define STRING_INV_CONTROLS				3
#define STRING_INV_DISPLAY				4
#define STRING_INV_SOUND				5
#define STRING_INV_NEW_GAME				6
#define STRING_INV_LOAD_GAME			7
#define STRING_INV_SAVE_GAME			8
#define STRING_INV_EXIT_GAME			9
#define STRING_INV_EXIT_TO_TITLE		10	
#define STRING_INV_UZI					11
#define STRING_INV_PISTOLS				12
#define STRING_INV_SHOTGUN				13
#define STRING_INV_REVOLVER				14
#define STRING_INV_REVOLVER_LASER		15
#define STRING_INV_DESERT_EAGLE			16
#define STRING_INV_DESERT_EAGLE_LASER	17
#define STRING_INV_DESERT_EAGLE_AMMO	18
#define STRING_INV_HK					19
#define STRING_INV_HK_SILENCED			20
#define STRING_INV_SHOTGUN_AMMO1		21
#define STRING_INV_SHOTGUN_AMMO2		22
#define STRING_INV_HK_SNIPER_MODE		23
#define STRING_INV_HK_BURST_MODE		24
#define STRING_INV_HK_RAPID_MODE		25
#define STRING_INV_HK_AMMO				26
#define STRING_INV_REVOLVER_AMMO		27
#define STRING_INV_UZI_AMMO				28
#define STRING_INV_PISTOLS_AMMO			29
#define STRING_INV_LASERSIGHT			30
#define STRING_INV_SILENCER				31
#define STRING_INV_LARGE_MEDIPACK		32
#define STRING_INV_SMALL_MEDIPACK		33
#define STRING_INV_BINOCULARS			34
#define STRING_INV_HEADSET				35
#define STRING_INV_FLARES				36
#define STRING_INV_TIMEX				37
#define STRING_INV_CROWBAR				38
#define STRING_INV_USE					39
#define STRING_INV_COMBINE				40
#define STRING_INV_SEPARE				41
#define STRING_INV_CHOOSE_AMMO			42
#define STRING_INV_SELECT_LEVEL			43
#define STRING_INV_GRENADE_ITEM			46
#define STRING_INV_GRENADE_AMMO			47
#define STRING_INV_HARPOON_ITEM			48
#define STRING_INV_HARPOON_AMMO			49
#define STRING_INV_ROCKET_ITEM			50
#define STRING_INV_ROCKET_AMMO			51

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
	byte R;
	byte G;
	byte B;
	__int16 CloudSpeed;

	GameScriptSkyLayer()
	{

	}

	GameScriptSkyLayer(byte r, byte g, byte b, __int16 speed)
	{
		R = r;
		G = g;
		B = b;
		CloudSpeed = speed;
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

typedef struct GameScriptLevel {
	string FileName;
	string ScriptFileName;
	string LoadScreenFileName;
	string Background;
	__int32 Name;
	__int32 Soundtrack;
	GameScriptSkyLayer Layer1;
	GameScriptSkyLayer Layer2;
	bool Horizon;
	bool ColAddHorizon;
	GameScriptFog Fog;
	bool Storm;
	bool Rain;
	bool Snow;
	bool ResetHub;
	bool Rumble;
	LARA_DRAW_TYPE LaraType;

	GameScriptLevel()
	{
		Storm = false;
		Rain = false;
		Snow = false;
		Horizon = false;
		ColAddHorizon = false;
		ResetHub = false;
		Rumble = false;
		LaraType = LARA_DRAW_TYPE::LARA_NORMAL;
	}
};

class GameFlow
{
private:
	sol::state*							m_lua;
	GameScriptSettings					m_settings;
	vector<string>						m_strings;
	vector<GameScriptLevel*>			m_levels;

	string								loadScriptFromFile(char* luaFilename);
	map<__int16, __int16>				m_itemsMap;

public:
	D3DXVECTOR3							SkyColorLayer1;
	__int32								SkySpeedLayer1;
	D3DXVECTOR3							SkyColorLayer2;
	__int32								SkySpeedLayer2;
	D3DXVECTOR3							FogColor;
	__int32								FogInDistance;
	__int32								FogOutDistance;
	bool								DrawHorizon;
	bool								ColAddHorizon;
	__int32								SelectedLevelForNewGame;
	__int32								SelectedSaveGame;

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
	void								AddLevel(GameScriptLevel* level);
	bool								DoGameflow();
};