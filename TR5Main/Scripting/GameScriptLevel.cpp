#include "framework.h"
#include "GameScriptLevel.h"
#include <unordered_map>

/***
A container for level metadata - things which aren't present in the compiled
level file itself.

@classmod Level
@pragma nostrip
*/

void GameScriptLevel::Register(sol::state* state)
{
	state->new_usertype<GameScriptLevel>("Level",
		sol::constructors<GameScriptLevel()>(),

/// (string) string key for the level's (localised) name. Corresponds to an entry in strings.lua.
//@mem nameKey
		"nameKey", &GameScriptLevel::NameStringKey,

/// (string) path of the Lua file holding the level's logic script, relative to the location of the tombengine executable
//@mem scriptFile
		"scriptFile", &GameScriptLevel::ScriptFileName,

/// (string) path of the compiled level file, relative to the location of the tombengine executable
//@mem levelFile
		"levelFile", &GameScriptLevel::FileName,

/// (string) path of the level's load screen file (.png or .jpg), relative to the location of the tombengine executable
//@mem loadScreenFile
		"loadScreenFile", &GameScriptLevel::LoadScreenFileName,
		
/// (string) initial ambient sound track to play - this is the filename of the track __without__ the .wav extension
//@mem ambientTrack
		"ambientTrack", &GameScriptLevel::AmbientTrack,

/// (@{SkyLayer}) Primary sky layer  
//@mem layer1
		"layer1", &GameScriptLevel::Layer1,

/// (@{SkyLayer}) Secondary sky layer __(not yet implemented)__
//@mem layer2
		"layer2", &GameScriptLevel::Layer2,

/// (@{Color}) distance fog RGB color (as seen in TR4's Desert Railroad).
// if not provided, distance fog will be black.
// __(not yet implemented)__
//@mem fog
		"fog", &GameScriptLevel::Fog,

/// (bool) if set to true, the horizon and sky layer will be drawn; if set to false; they won't.
//@mem horizon
		"horizon", &GameScriptLevel::Horizon,

/// (bool) if true, the horizon graphic will transition smoothly to the sky layer.
// If set to false, there will be a black band between the two.
// __(not yet implemented)__
//@mem colAddHorizon
		"colAddHorizon", &GameScriptLevel::ColAddHorizon,

/// (bool) equivalent to classic TRLE's LIGHTNING setting.
// If true, there will be a flickering lightning in the skylayer, as in the TRC Ireland levels.
// __(thunder sounds not yet implemented)__
//@mem storm
		"storm", &GameScriptLevel::Storm,

/// (WeatherType) Must be one of the values WeatherType.NORMAL, WeatherType.RAIN, or WeatherType.SNOW.
// __(not yet implemented)__
//@mem weather
		"weather", &GameScriptLevel::Weather,

/*** (LaraType) Must be one of the LaraType values.
These are:

	NORMAL  
	YOUNG
	BUNHEAD
	CATSUIT
	DIVESUIT
	INVISIBLE

e.g. `myLevel.laraType = LaraType.DIVESUIT`

 __(not yet fully implemented)__
 @mem laraType*/
		"laraType", &GameScriptLevel::LaraType,

/// (bool) If true, an occasional screen shake effect (as seen in TRC's Sinking Submarine) will
// happen throughout the level.
//@mem rumble
		"rumble", &GameScriptLevel::Rumble,

/// (Mirror) object holding the location and size of the room's mirror, if present.
// __(not yet implemented)__
//@mem mirror
		"mirror", &GameScriptLevel::Mirror,

/// (table of InventoryObjects) table of inventory object overrides
//@mem objects
		"objects", &GameScriptLevel::InventoryObjects
		);
}
