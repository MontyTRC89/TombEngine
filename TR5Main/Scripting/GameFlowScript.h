#pragma once
#include "LanguageScript.h"
#include "LuaHandler.h"

#define TITLE_FLYBY			0
#define TITLE_BACKGROUND	1

typedef enum WEATHER_TYPES
{
	WEATHER_NORMAL,
	WEATHER_RAIN,
	WEATHER_SNOW
};

typedef enum LARA_DRAW_TYPE
{
	LARA_NORMAL = 1,
	LARA_YOUNG = 2,
	LARA_BUNHEAD = 3,
	LARA_CATSUIT = 4,
	LARA_DIVESUIT = 5,
	LARA_INVISIBLE = 7
};

struct GameScriptSettings
{
	int ScreenWidth;
	int ScreenHeight;
	bool EnableLoadSave;
	bool EnableDynamicShadows;
	bool EnableWaterCaustics;
	bool Windowed;
	std::string WindowTitle;
	int DrawingDistance;
	bool ShowRendererSteps;
	bool ShowDebugInfo;
};

struct GameScriptInventoryObject
{
	std::string name;
	short slot;
	float yOffset;
	float scale;
	float xRot;
	float yRot;
	float zRot;
	short rotationFlags;
	int meshBits;
	__int64 operation;

	GameScriptInventoryObject(std::string name, short slot, float yOffset, float scale, float xRot, float yRot, float zRot, short rotationFlags, int meshBits, __int64 operation)
	{
		this->name = name;
		this->slot = slot;
		this->yOffset = yOffset;
		this->scale = scale;
		this->xRot = xRot;
		this->yRot = yRot;
		this->zRot = zRot;
		this->rotationFlags = rotationFlags;
		this->meshBits = meshBits;
		this->operation = operation;
	}
};

struct GameScriptSkyLayer
{
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

struct GameScriptFog
{
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

struct GameScriptMirror
{
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

struct GameScriptLevel
{
	std::string NameStringKey;
	std::string FileName;
	std::string ScriptFileName;
	std::string LoadScreenFileName;
	std::string Background;
	int Name;
	std::string AmbientTrack;
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
	std::vector<GameScriptInventoryObject> InventoryObjects;

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

struct GameScriptAudioTrack
{
	std::string trackName;
	bool looped;

	GameScriptAudioTrack(std::string trackName, bool looped)
	{
		this->trackName = trackName;
		this->looped = looped;
	}
};

bool __cdecl LoadScript();

class GameFlow : public LuaHandler
{
private:
	GameScriptSettings					m_settings;

	std::unordered_map < std::string, std::vector<std::string > > m_translationsMap;
	std::vector<std::string> m_languageNames;

	std::map<short, short>				m_itemsMap;

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
	std::vector<GameScriptLevel*>			Levels;

	GameFlow(sol::state* lua);
	~GameFlow();

	void								WriteDefaults();
	void								AddLevel(GameScriptLevel const& level);
	void								SetAudioTracks(sol::as_table_t<std::vector<GameScriptAudioTrack>>&& src);
	bool								LoadGameFlowScript();
	char*								GetString(const char* id);
	void								SetStrings(sol::nested<std::unordered_map<std::string, std::vector<std::string>>> && src);
	void								SetLanguageNames(sol::as_table_t<std::vector<std::string>> && src);
	GameScriptSettings*					GetSettings();
	GameScriptLevel*					GetLevel(int id);
	void								SetHorizon(bool horizon, bool colAddHorizon);
	void								SetLayer1(byte r, byte g, byte b, short speed);
	void								SetLayer2(byte r, byte g, byte b, short speed);
	void								SetFog(byte r, byte g, byte b, short startDistance, short endDistance);
	int									GetNumLevels();		
	bool								DoGameflow();
};