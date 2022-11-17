#include "framework.h"
#include "FlowLevel.h"
#include "ScriptAssert.h"

/***
Stores level metadata.
These are things things which aren't present in the compiled level file itself.

@tenclass Flow.Level
@pragma nostrip
*/

/*** Make a new Level object.
	@function Level
	@return a Level object
	*/
void Level::Register(sol::table & parent)
{	
	parent.new_usertype<Level>("Level",
		sol::constructors<Level()>(),
		sol::call_constructor, sol::constructors<Level()>(),
/// (string) string key for the level's (localised) name.
// Corresponds to an entry in strings.lua.
//@mem nameKey
		"nameKey", &Level::NameStringKey,

/// (string) Level-specific Lua script file.
// Path of the Lua file holding the level's logic script, relative to the location of the tombengine executable
//@mem scriptFile
		"scriptFile", &Level::ScriptFileName,

/// (string) Compiled file path.
// This path is relative to the location of the TombEngine executable.
//@mem levelFile
		"levelFile", &Level::FileName,

/// (string) Load screen image.
// Path of the level's load screen file (.png or .jpg), relative to the location of the tombengine executable
//@mem loadScreenFile
		"loadScreenFile", &Level::LoadScreenFileName,
		
/// (string) initial ambient sound track to play.
// This is the filename of the track __without__ the .wav extension.
//@mem ambientTrack
		"ambientTrack", &Level::AmbientTrack,

/// (@{Flow.SkyLayer}) Primary sky layer  
//@mem layer1
		"layer1", &Level::Layer1,

/// (@{Flow.SkyLayer}) Secondary sky layer
//@mem layer2
		"layer2", &Level::Layer2,

/// (@{Flow.Fog}) omni fog RGB color and distance.
// As seen in TR4's Desert Railroad.
// If not provided, distance fog will be black.
//@mem fog
		"fog", &Level::Fog,

/// (bool) Draw sky layer? (default: false)
//@mem horizon
		"horizon", &Level::Horizon,

/// (bool) Enable flickering lightning in the sky.
// Equivalent to classic TRLE's LIGHTNING setting. As in the TRC Ireland levels.
//
//@mem storm
		"storm", &Level::Storm,

/// (WeatherType) Choose weather effect.
// Must be one of the values `WeatherType.None`, `WeatherType.Rain`, or `WeatherType.Snow`.
//
//@mem weather
		"weather", &Level::Weather,

/// (float) Choose weather strength.
// Must be value between `0.1` and `1.0`.
//
//@mem weatherStrength
		"weatherStrength", sol::property(&Level::SetWeatherStrength),

/*** (LaraType) Must be one of the LaraType values.
These are:

	Normal  
	Young
	Bunhead
	Catsuit
	Divesuit
	Invisible

e.g. `myLevel.laraType = LaraType.Divesuit`

 __(not yet fully implemented)__
 @mem laraType*/
		"laraType", &Level::Type,

/// (bool) Enable occasional screen shake effect.
// As seen in TRC's Sinking Submarine.
//@mem rumble
		"rumble", &Level::Rumble,

/// (@{Flow.Mirror}) Location and size of the level's mirror, if present.
//
// __(not yet implemented)__
//@mem mirror
		"mirror", &Level::Mirror,

		"farView", sol::property(&Level::SetLevelFarView),

/*** (bool) Enable unlimited oxygen supply when in water.

 __(not yet implemented)__
@mem unlimitedAir
*/
		"unlimitedAir", &Level::UnlimitedAir,

/// (table of @{Flow.InventoryItem}s) table of inventory object overrides
//@mem objects
		"objects", &Level::InventoryObjects,

/// (int) Set Secrets for Level
//@mem secrets
		"secrets", &Level::Secrets
		);
}

void Level::SetWeatherStrength(float val)
{
	bool cond = val <= 1.0f && val >= 0.0f;
	std::string msg{ "weatherStrength value must be in the range [0.1, 1.0]." };
	if (!ScriptAssert(cond, msg))
	{
		ScriptWarn("Setting weatherStrength view to 1.");
		WeatherStrength = 1.0f;
	}
	else
	{
		WeatherStrength = val;
	}
}

/*** (int) The maximum draw distance for level.
Given in sectors (blocks).
Must be at least 4.

This is equivalent to TRNG's LevelFarView variable.

@mem farView
*/
void Level::SetLevelFarView(short val)
{
	static_assert(MIN_FAR_VIEW == 3200.0f, "Please update the comment, docs, and warning message if this number changes.");
	const short min = std::ceil(MIN_FAR_VIEW / SECTOR(1));
	bool cond = val >= min;

	std::string msg{ "farView value must be 4 or greater." };
	if (!ScriptAssert(cond, msg))
	{
		// Will be set to default by the renderer
		LevelFarView = 0;
	}
	else
	{
		LevelFarView = val;
	}
}

RGBAColor8Byte Level::GetSkyLayerColor(int index) const
{
	assertion(index == 0 || index == 1, "Sky layer index must be 0 or 1.");

	if (index == 0)
	{
		return Layer1.GetColor();
	}
	else
	{
		return Layer2.GetColor();
	}
}

bool Level::GetSkyLayerEnabled(int index) const
{
	assertion(index == 0 || index == 1, "Sky layer index must be 0 or 1.");

	if (index == 0)
	{
		return Layer1.Enabled;
	}
	else
	{
		return Layer2.Enabled;
	}
}

short Level::GetSkyLayerSpeed(int index) const
{
	assertion(index == 0 || index == 1, "Sky layer index must be 0 or 1.");

	if (index == 0)
	{
		return Layer1.CloudSpeed;
	}
	else
	{
		return Layer2.CloudSpeed;
	}
}

LaraType Level::GetLaraType() const
{
	return Type;
}

bool Level::HasStorm() const
{
	return Storm;
}

float Level::GetWeatherStrength() const
{
	return WeatherStrength;
}

WeatherType Level::GetWeatherType() const
{
	return Weather;
}

short Level::GetMirrorRoom() const
{
	return Mirror.Room;
}

bool Level::GetFogEnabled() const
{
	return Fog.Enabled;
}


RGBAColor8Byte Level::GetFogColor() const
{
	return Fog.GetColor();
}

short Level::GetFogMinDistance() const
{
	return Fog.MinDistance;
}

short Level::GetFogMaxDistance() const
{
	return Fog.MaxDistance;
}

short Level::GetFarView() const
{
	return float(LevelFarView);
}

