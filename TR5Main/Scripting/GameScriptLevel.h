#pragma once
#include <string>
#include "GameScriptSkyLayer.h"
#include "GameScriptMirror.h"
#include "GameScriptColor.h"
#include "GameScriptInventoryObject.h"

enum class WEATHER_TYPE
{
	NORMAL,
	RAIN,
	SNOW
};

static const std::unordered_map<std::string, WEATHER_TYPE> kWeatherTypes{
	{"NORMAL", WEATHER_TYPE::NORMAL},
	{"RAIN", WEATHER_TYPE::RAIN},
	{"SNOW", WEATHER_TYPE::SNOW}
};

enum LARA_TYPE
{
	NORMAL = 1,
	YOUNG = 2,
	BUNHEAD = 3,
	CATSUIT = 4,
	DIVESUIT = 5,
	INVISIBLE = 7
};

static const std::unordered_map<std::string, LARA_TYPE> kLaraTypes{
	{"NORMAL", LARA_TYPE::NORMAL},
	{"YOUNG", LARA_TYPE::YOUNG},
	{"BUNHEAD", LARA_TYPE::BUNHEAD},
	{"CATSUIT", LARA_TYPE::CATSUIT},
	{"DIVESUIT", LARA_TYPE::DIVESUIT},
	{"INVISIBLE", LARA_TYPE::INVISIBLE}
};

struct GameScriptLevel
{
	std::string NameStringKey;
	std::string FileName;
	std::string ScriptFileName;
	std::string LoadScreenFileName;
	int Name;
	std::string AmbientTrack;
	GameScriptSkyLayer Layer1;
	GameScriptSkyLayer Layer2;
	bool Horizon{ false };
	bool Sky;
	bool ColAddHorizon{ false };
	GameScriptColor Fog{ 0, 0, 0 };
	bool Storm{ false };
	WEATHER_TYPE Weather{ WEATHER_TYPE::NORMAL };
	bool Rumble{ false };
	LARA_TYPE LaraType{ LARA_TYPE::NORMAL };
	GameScriptMirror Mirror;
	byte UVRotate;
	int LevelFarView;
	bool UnlimitedAir{ false };
	std::vector<GameScriptInventoryObject> InventoryObjects;
	
	static void Register(sol::state* state);
};
