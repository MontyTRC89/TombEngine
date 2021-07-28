#pragma once
#include <string>
#include "GameScriptSkyLayer.h"
#include "GameScriptMirror.h"
#include "GameScriptColor.h"
#include "GameScriptInventoryObject.h"

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

	static void Register(sol::state* state);
};
