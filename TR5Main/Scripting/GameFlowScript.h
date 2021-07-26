#pragma once
#include "LanguageScript.h"
#include "LuaHandler.h"

enum TITLE_TYPE
{
	TITLE_FLYBY,
	TITLE_BACKGROUND
};

enum WEATHER_TYPE
{
	WEATHER_NORMAL,
	WEATHER_RAIN,
	WEATHER_SNOW
};

enum LARA_DRAW_TYPE
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
	bool Horizon{ false };
	bool Sky;
	bool ColAddHorizon{ false };
	GameScriptColor Fog{ 0,0,0 };
	bool Storm{ false };
	WEATHER_TYPE Weather{ WEATHER_NORMAL };
	bool ResetHub{ false };
	bool Rumble{ false };
	LARA_DRAW_TYPE LaraType{ LARA_NORMAL };
	GameScriptMirror Mirror;
	byte UVRotate;
	int LevelFarView;
	bool UnlimitedAir{ false };
	std::vector<GameScriptInventoryObject> InventoryObjects;
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
	Vector3							SkyColorLayer1{};
	Vector3							SkyColorLayer2{};
	Vector3							FogColor{};
	int								SkySpeedLayer1{ 0 };
	int								SkySpeedLayer2{ 0 };
	int								FogInDistance{ 0 };
	int								FogOutDistance{ 0 };
	bool							DrawHorizon{ false };
	bool							ColAddHorizon{ false };
	int								SelectedLevelForNewGame{ 0 };
	int								SelectedSaveGame{ 0 };
	bool							EnableLoadSave{ true };
	bool							PlayAnyLevel{ true };
	bool							FlyCheat{ true };
	bool							DebugMode{ false };
	int								LevelFarView{ 0 };
	TITLE_TYPE						TitleType{ TITLE_BACKGROUND };
	char const*						Intro{ nullptr };

	// Selected language set
	std::vector<GameScriptLevel*>			Levels;

	GameFlow(sol::state* lua);
	~GameFlow();

	void								WriteDefaults();
	void								AddLevel(GameScriptLevel const& level);
	void								SetAudioTracks(sol::as_table_t<std::vector<GameScriptAudioTrack>>&& src);
	bool								LoadGameFlowScript();
	char const *						GetString(const char* id);
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