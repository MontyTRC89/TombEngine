#include "framework.h"
#include "GameScriptLevel.h"

void GameScriptLevel::Register(sol::state* state)
{
	state->new_usertype<GameScriptLevel>("Level",
		sol::constructors<GameScriptLevel()>(),
		"name", &GameScriptLevel::NameStringKey,
		"script", &GameScriptLevel::ScriptFileName,
		"fileName", &GameScriptLevel::FileName,
		"loadScreen", &GameScriptLevel::LoadScreenFileName,
		"ambientTrack", &GameScriptLevel::AmbientTrack,
		"layer1", &GameScriptLevel::Layer1,
		"layer2", &GameScriptLevel::Layer2,
		"fog", &GameScriptLevel::Fog,
		"horizon", &GameScriptLevel::Horizon,
		"colAddHorizon", &GameScriptLevel::ColAddHorizon,
		"storm", &GameScriptLevel::Storm,
		"background", &GameScriptLevel::Background,
		"weather", &GameScriptLevel::Weather,
		"laraType", &GameScriptLevel::LaraType,
		"rumble", &GameScriptLevel::Rumble,
		"resetHub", &GameScriptLevel::ResetHub,
		"mirror", &GameScriptLevel::Mirror,
		"objects", &GameScriptLevel::InventoryObjects
		);
}
