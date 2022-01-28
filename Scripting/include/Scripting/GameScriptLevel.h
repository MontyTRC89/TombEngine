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
	std::string LoadScreenFileName;
	std::string AmbientTrack;
	GameScriptSkyLayer Layer1;
	GameScriptSkyLayer Layer2;
	bool ColAddHorizon{ false };
	GameScriptFog Fog;
	bool Storm{ false };
	WeatherType Weather{ WeatherType::None };
	float WeatherStrength{ 1.0f };
	LaraType Type{ LaraType::Normal };
	GameScriptMirror Mirror;
	int LevelFarView{ 0 };
	bool UnlimitedAir{ false };
	std::vector<GameScriptInventoryObject> InventoryObjects;

	float GetWeatherStrength() const override;
	bool GetSkyLayerEnabled(int index) const override;
	bool HasStorm() const override;
	short GetSkyLayerSpeed(int index) const override;
	RGBAColor8Byte GetSkyLayerColor(int index) const override;
	LaraType GetLaraType() const override;
	void SetWeatherStrength(float val);
	void SetLevelFarView(byte val);
	static void Register(sol::state* state);
	WeatherType GetWeatherType() const override;
	short GetMirrorRoom() const override;
};
