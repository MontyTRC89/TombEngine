#include "framework.h"
#include "GameScriptSkyLayer.h"

void GameScriptSkyLayer::Register(sol::state* lua)
{
	lua->new_usertype<GameScriptSkyLayer>("SkyLayer",
		sol::constructors<GameScriptSkyLayer(byte, byte, byte, short)>(),
		"r", &GameScriptSkyLayer::R,
		"g", &GameScriptSkyLayer::G,
		"b", &GameScriptSkyLayer::B,
		"speed", &GameScriptSkyLayer::CloudSpeed
		);
}

GameScriptSkyLayer::GameScriptSkyLayer(byte r, byte g, byte b, short speed)
	{
		R = r;
		G = g;
		B = b;
		CloudSpeed = speed;
		Enabled = true;
	}

