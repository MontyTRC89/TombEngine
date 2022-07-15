#pragma once
#include <string>
#include "Flow/SkyLayer/SkyLayer.h"
#include "Flow/Mirror/Mirror.h"
#include "Flow/Fog/Fog.h"
#include "ScriptInterfaceLevel.h"
#include "Flow/InventoryItem/InventoryItem.h"

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

struct Level : public ScriptInterfaceLevel
{
	std::string AmbientTrack;
	SkyLayer Layer1;
	SkyLayer Layer2;
	bool ColAddHorizon{ false };
	Fog Fog;
	bool Storm{ false };
	WeatherType Weather{ WeatherType::None };
	float WeatherStrength{ 1.0f };
	LaraType Type{ LaraType::Normal };
	Mirror Mirror;
	int LevelFarView{ 0 };
	bool UnlimitedAir{ false };
	std::vector<InventoryItem> InventoryObjects;

	RGBAColor8Byte GetFogColor() const override;
	bool GetFogEnabled() const override;
	float GetWeatherStrength() const override;
	bool GetSkyLayerEnabled(int index) const override;
	bool HasStorm() const override;
	short GetSkyLayerSpeed(int index) const override;
	RGBAColor8Byte GetSkyLayerColor(int index) const override;
	LaraType GetLaraType() const override;
	void SetWeatherStrength(float val);
	void SetLevelFarView(short val);
	static void Register(sol::table & parent);
	WeatherType GetWeatherType() const override;
	short GetMirrorRoom() const override;
	short GetFogMinDistance() const override;
	short GetFogMaxDistance() const override;
	short GetFarView() const override;
};
