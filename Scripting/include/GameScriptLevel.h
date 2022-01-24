#pragma once
#include <string>
#include "GameScriptSkyLayer.h"
#include "GameScriptMirror.h"
#include "GameScriptColor.h"
#include "GameScriptInventoryObject.h"
#include <GameScriptFog.h>
#include "ScriptInterfaceLevel.h"

static const std::unordered_map<std::string, WeatherType> kWeatherTypes
{
	{"None", WeatherType::None},
	{"Rain", WeatherType::Rain},
	{"Snow", WeatherType::Snow}
};

enum LaraType
{
	Normal = 1,
	Young = 2,
	Bunhead = 3,
	Catsuit = 4,
	Divesuit = 5,
	Invisible = 7
};

static const std::unordered_map<std::string, LaraType> kLaraTypes
{
	{"Normal", LaraType::Normal},
	{"Young", LaraType::Young},
	{"Bunhead", LaraType::Bunhead},
	{"Catsuit", LaraType::Catsuit},
	{"Divesuit", LaraType::Divesuit},
	{"Invisible", LaraType::Invisible}
};

struct GameScriptLevel : public ScriptInterfaceLevel
{
	std::string NameStringKey;
	std::string FileName;
	std::string ScriptFileName;
	std::string LoadScreenFileName;
	std::string AmbientTrack;
	GameScriptSkyLayer Layer1;
	GameScriptSkyLayer Layer2;
	bool Horizon{ false };
	bool ColAddHorizon{ false };
	GameScriptFog Fog;
	bool Storm{ false };
	WeatherType Weather{ WeatherType::None };
	float WeatherStrength{ 1.0f };
	bool Rumble{ false };
	LaraType LaraType{ LaraType::Normal };
	GameScriptMirror Mirror;
	int LevelFarView{ 0 };
	bool UnlimitedAir{ false };
	std::vector<GameScriptInventoryObject> InventoryObjects;

	bool GetSkyLayerEnabled(int index) override;
	short GetSkyLayerSpeed(int index) override;
	void SetWeatherStrength(float val);
	void SetLevelFarView(byte val);
	static void Register(sol::state* state);
};
