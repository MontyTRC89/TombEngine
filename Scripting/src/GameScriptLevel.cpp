#include "frameworkandsol.h"
#include "GameScriptLevel.h"
#include "ScriptAssert.h"

/***
Stores level metadata.
These are things things which aren't present in the compiled level file itself.

@pregameclass Level
@pragma nostrip
*/

/*** Make a new Level object.
	@function Level.new
	@return a Level object
	*/
void GameScriptLevel::Register(sol::state* state)
{
	state->new_usertype<GameScriptLevel>("Level",
		sol::constructors<GameScriptLevel()>(),

/// (string) string key for the level's (localised) name.
// Corresponds to an entry in strings.lua.
//@mem nameKey
		"nameKey", &GameScriptLevel::NameStringKey,

/// (string) Level-specific Lua script file.
// Path of the Lua file holding the level's logic script, relative to the location of the tombengine executable
//@mem scriptFile
		"scriptFile", &GameScriptLevel::ScriptFileName,

/// (string) Compiled file path.
// This path is relative to the location of the TombEngine executable.
//@mem levelFile
		"levelFile", &GameScriptLevel::FileName,

/// (string) Load screen image.
// Path of the level's load screen file (.png or .jpg), relative to the location of the tombengine executable
//@mem loadScreenFile
		"loadScreenFile", &GameScriptLevel::LoadScreenFileName,
		
/// (string) initial ambient sound track to play.
// This is the filename of the track __without__ the .wav extension.
//@mem ambientTrack
		"ambientTrack", &GameScriptLevel::AmbientTrack,

/// (@{SkyLayer}) Primary sky layer  
//@mem layer1
		"layer1", &GameScriptLevel::Layer1,

/// (@{SkyLayer}) Secondary sky layer
// __(not yet implemented)__
//@mem layer2
		"layer2", &GameScriptLevel::Layer2,

/// (@{Fog}) omni fog RGB color and distance.
// As seen in TR4's Desert Railroad.
// If not provided, distance fog will be black.
//
// __(not yet implemented)__
//@mem fog
		"fog", &GameScriptLevel::Fog,

/// (bool) Draw sky layer? (default: false)
//@mem horizon
		"horizon", &GameScriptLevel::Horizon,

/// (bool) Enable smooth transition from horizon graphic to sky layer.
// If set to false, there will be a black band between the two.
//
// __(not yet implemented)__
//@mem colAddHorizon
		"colAddHorizon", &GameScriptLevel::ColAddHorizon,

/// (bool) Enable flickering lightning in the sky.
// Equivalent to classic TRLE's LIGHTNING setting. As in the TRC Ireland levels.
//
//@mem storm
		"storm", &GameScriptLevel::Storm,

/// (WeatherType) Choose weather effect.
// Must be one of the values `WeatherType.None`, `WeatherType.Rain`, or `WeatherType.Snow`.
//
//@mem weather
		"weather", &GameScriptLevel::Weather,

/// (float) Choose weather strength.
// Must be value between `0.1` and `1.0`.
//
//@mem weatherStrength
		"weatherStrength", sol::property(&GameScriptLevel::SetWeatherStrength),

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
		"laraType", &GameScriptLevel::Type,

/// (bool) Enable occasional screen shake effect.
// As seen in TRC's Sinking Submarine.
//@mem rumble
		"rumble", &GameScriptLevel::Rumble,

/// (@{Mirror}) Location and size of the level's mirror, if present.
//
// __(not yet implemented)__
//@mem mirror
		"mirror", &GameScriptLevel::Mirror,

/*** (byte) The maximum draw distance for level.
Given in sectors (blocks).
Must be in the range [1, 127], and equal to or less than the value passed to SetGameFarView.

This is equivalent to TRNG's LevelFarView variable.

__(not yet implemented)__
@mem farView
*/
		"farView", sol::property(&GameScriptLevel::SetLevelFarView),

/*** (bool) Enable unlimited oxygen supply when in water.

 __(not yet implemented)__
@mem unlimitedAir
*/
		"unlimitedAir", &GameScriptLevel::UnlimitedAir,

/// (table of @{InventoryObject}s) table of inventory object overrides
//@mem objects
		"objects", &GameScriptLevel::InventoryObjects
		);
}

//todo add bindings and documents for this
bool GameScriptLevel::GetSkyLayerEnabled(int index) const
{
	bool cond = index == 0 || index == 1;
	std::string msg{ "Index must be 0 or 1." };

	if (index == 0)
	{
		return Layer1.Enabled;
	}
	else if (index == 1)
	{
		return Layer2.Enabled;
	}
	else
	{
		ScriptAssert(false, msg);
		ScriptWarn("Returning false.");
		return false;
	}
}

//todo add bindings and documents for this
short GameScriptLevel::GetSkyLayerSpeed(int index) const
{
	bool cond = index == 0 || index == 1;
	std::string msg{ "Index must be 0 or 1." };

	if (index == 0)
	{
		return Layer1.CloudSpeed;
	}
	else if (index == 1)
	{
		return Layer2.CloudSpeed;
	}
	else
	{
		ScriptAssert(false, msg);
		ScriptWarn("Returning 0.");
		return 0;
	}
}

void GameScriptLevel::SetWeatherStrength(float val)
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

void GameScriptLevel::SetLevelFarView(byte val)
{
	bool cond = val <= 127 && val >= 1;
	std::string msg{ "levelFarView value must be in the range [1, 127]." };
	if (!ScriptAssert(cond, msg))
	{
		ScriptWarn("Setting levelFarView view to 32.");
		LevelFarView = 32;
	}
	else
	{
		LevelFarView = val;
	}
}
// todo test this
RGBAColor8Byte GameScriptLevel::GetSkyLayerColor(int index) const
{
	bool cond = index == 0 || index == 1;
	std::string msg{ "Index must be 0 or 1." };

	if (index == 0)
	{
		return Layer1.GetColor();
	}
	else if (index == 1)
	{
		return Layer2.GetColor();
	}
	else
	{
		ScriptAssert(false, msg);
		ScriptWarn("Returning 0, 0, 0, 0.");
		return RGBAColor8Byte{ 0 };
	}
}

LaraType GameScriptLevel::GetLaraType() const
{
	return Type;
}

bool GameScriptLevel::HasStorm() const
{
	return Storm;
}

float GameScriptLevel::GetWeatherStrength() const
{
	return WeatherStrength;
}

WeatherType GameScriptLevel::GetWeatherType() const
{
	return Weather;
}

