#include "GameFlowScript.h"
#include "..\Game\items.h"
#include "..\Game\box.h"
#include "..\Game\lot.h"
#include "..\Game\sound.h"
#include "..\Game\savegame.h"

GameFlow::GameFlow(sol::state* lua)
{
	m_lua = lua;
	
	// Settings type
	m_lua->new_usertype<GameScriptSettings>("GameScriptSettings",
		"screenWidth", &GameScriptSettings::ScreenWidth,
		"screenHeight", &GameScriptSettings::ScreenHeight,
		"windowTitle", &GameScriptSettings::WindowTitle,
		"enableDynamicShadows", &GameScriptSettings::EnableDynamicShadows,
		"windowed", &GameScriptSettings::Windowed,
		"enableWaterCaustics", &GameScriptSettings::EnableWaterCaustics,
		"drawingDistance", &GameScriptSettings::DrawingDistance,
		"showRendererSteps", &GameScriptSettings::ShowRendererSteps,
		"showDebugInfo", &GameScriptSettings::ShowDebugInfo
		);

	// Layer type
	m_lua->new_usertype<GameScriptSkyLayer>("SkyLayer",
		sol::constructors<GameScriptSkyLayer(byte, byte, byte, __int16)>(),
		"r", &GameScriptSkyLayer::R,
		"g", &GameScriptSkyLayer::G,
		"b", &GameScriptSkyLayer::B,
		"speed", &GameScriptSkyLayer::CloudSpeed
		);

	// Mirror type
	m_lua->new_usertype<GameScriptMirror>("Mirror",
		sol::constructors<GameScriptMirror(__int16, __int32, __int32, __int32, __int32)>(),
		"room", &GameScriptMirror::Room,
		"startX", &GameScriptMirror::StartX,
		"endX", &GameScriptMirror::EndX,
		"startZ", &GameScriptMirror::StartZ,
		"endZ", &GameScriptMirror::EndZ
		);

	// Fog type
	m_lua->new_usertype<GameScriptFog>("Fog",
		sol::constructors<GameScriptFog(byte, byte, byte)>(),
		"r", &GameScriptFog::R,
		"g", &GameScriptFog::G,
		"b", &GameScriptFog::B
		);

	// Level type
	m_lua->new_usertype<GameScriptLevel>("Level",
		sol::constructors<GameScriptLevel()>(),
		"name", &GameScriptLevel::Name,
		"script", &GameScriptLevel::ScriptFileName,
		"fileName", &GameScriptLevel::FileName,
		"loadScreen", &GameScriptLevel::LoadScreenFileName,
		"soundTrack", &GameScriptLevel::Soundtrack,
		"layer1", &GameScriptLevel::Layer1,
		"layer2", &GameScriptLevel::Layer2,
		"fog", &GameScriptLevel::Fog,
		"horizon", &GameScriptLevel::Horizon,
		"colAddHorizon", &GameScriptLevel::ColAddHorizon,
		"storm", &GameScriptLevel::Storm,
		"background", &GameScriptLevel::Background,
		"rain", &GameScriptLevel::Rain,
		"snow", &GameScriptLevel::Snow,
		"laraType", &GameScriptLevel::LaraType,
		"rumble", &GameScriptLevel::Rumble,
		"resetHub", &GameScriptLevel::ResetHub,
		"mirror", &GameScriptLevel::Mirror
		);

	// Item type
	m_lua->new_usertype<ITEM_INFO>("ItemInfo",
		"hitPoints", &ITEM_INFO::hitPoints
		);

	// Gameflow type
	m_lua->new_usertype<GameFlow>("GameFlow",
		"levels", &GameFlow::m_levels,
		"settings", &GameFlow::m_settings,
		"strings", &GameFlow::m_strings,
		"AddLevel", &GameFlow::AddLevel
		);

	m_strings.resize(NUM_STRINGS);

	// define some constants
	(*m_lua)["LaraType"] = m_lua->create_table_with(
		"Normal", LARA_DRAW_TYPE::LARA_NORMAL,
		"Young", LARA_DRAW_TYPE::LARA_YOUNG,
		"DiveSuit", LARA_DRAW_TYPE::LARA_DIVESUIT,
		"CatSuit", LARA_DRAW_TYPE::LARA_CATSUIT
	);

	(*m_lua)["Gameflow"] = this;
}

GameFlow::~GameFlow()
{

}

string GameFlow::loadScriptFromFile(char* luaFilename)
{
	ifstream ifs(luaFilename, ios::in | ios::binary | ios::ate);

	ifstream::pos_type fileSize = ifs.tellg();
	ifs.seekg(0, ios::beg);

	vector<char> bytes(fileSize);
	ifs.read(bytes.data(), fileSize);

	return string(bytes.data(), fileSize);
}

bool GameFlow::LoadGameStrings(char* luaFilename)
{
	string script = loadScriptFromFile(luaFilename);
	m_lua->script(script);

	return true;
}

bool GameFlow::LoadGameSettings(char* luaFilename)
{
	string script = loadScriptFromFile(luaFilename);
	m_lua->script(script);

	return true;
}

bool GameFlow::ExecuteScript(char* luaFilename)
{
	string script = loadScriptFromFile(luaFilename);
	m_lua->script(script);

	return true;
}

char* GameFlow::GetString(__int32 id)
{
	return ((char*)m_strings[id - 1].c_str());
}

GameScriptSettings* GameFlow::GetSettings()
{
	return &m_settings;
}

GameScriptLevel* GameFlow::GetLevel(__int32 id)
{
	return m_levels[id];
}

void GameFlow::SetHorizon(bool horizon, bool colAddHorizon)
{
	DrawHorizon = horizon;
	ColAddHorizon = colAddHorizon;
}

void GameFlow::SetLayer1(byte r, byte g, byte b, __int16 speed)
{
	SkyColor1.r = r;
	SkyColor1.g = g;
	SkyColor1.b = b;
	SkyVelocity1 = speed;

	SkyColorLayer1.x = r / 255.0f;
	SkyColorLayer1.y = g / 255.0f;
	SkyColorLayer1.z = b / 255.0f;
	SkySpeedLayer1 = speed;
}

void GameFlow::SetLayer2(byte r, byte g, byte b, __int16 speed)
{
	SkyColor2.r = r;
	SkyColor2.g = g;
	SkyColor2.b = b;
	SkyVelocity2 = speed;

	SkyColorLayer2.x = r / 255.0f;
	SkyColorLayer2.y = g / 255.0f;
	SkyColorLayer2.z = b / 255.0f;
	SkySpeedLayer2 = speed;
}

void GameFlow::SetFog(byte r, byte g, byte b, __int16 startDistance, __int16 endDistance)
{
	FogColor.x = r / 255.0f;
	FogColor.y = g / 255.0f;
	FogColor.z = b / 255.0f;
	FogInDistance = startDistance;
	FogOutDistance = endDistance;
}

__int32	GameFlow::GetNumLevels()
{
	return m_levels.size();
}

void GameFlow::AddLevel(GameScriptLevel* level)
{
	m_levels.push_back(level);
}

bool GameFlow::DoGameflow()
{
	// We start with the title level
	CurrentLevel = 0;
	SelectedLevelForNewGame = 0;
	SelectedSaveGame = 0;
	SaveGameHeader header;

	// DEBUG: test   
	CurrentLevel = 3; 
	SelectedLevelForNewGame = 0;
	gfInitialiseGame = true;
	DoLevel(CurrentLevel, 126, false);
	 
	return true;

	// We loop indefinitely, looking for return values of DoTitle or DoLevel
	bool loadFromSavegame = false;
	while (true)
	{
		// First we need to fill some legacy variables in PCTomb5.exe
		GameScriptLevel* level = m_levels[CurrentLevel];

		CurrentAtmosphere = level->Soundtrack;

		if (level->Horizon)
		{
			SkyColor1.r = level->Layer1.R;
			SkyColor1.g = level->Layer1.G;
			SkyColor1.b = level->Layer1.B;
			SkyVelocity1 = level->Layer1.CloudSpeed;

			SkyColor2.r = level->Layer2.R;
			SkyColor2.g = level->Layer2.G;
			SkyColor2.b = level->Layer2.B;
			SkyVelocity2 = level->Layer2.CloudSpeed;
		}

		if (level->Storm)
		{
			SkyStormColor[0] = level->Layer1.R;
			SkyStormColor[1] = level->Layer1.G;
			SkyStormColor[2] = level->Layer1.B;

			SkyStormColor2[0] = level->Layer1.R;
			SkyStormColor2[1] = level->Layer1.G;
			SkyStormColor2[2] = level->Layer1.B;
		}

		GAME_STATUS status;

		if (CurrentLevel == 0)
		{
			status = DoTitle(0);
		}
		else
		{
			status = DoLevel(CurrentLevel, CurrentAtmosphere, loadFromSavegame);
			loadFromSavegame = false;
		}

		switch (status)
		{
		case GAME_STATUS::GAME_STATUS_EXIT_GAME:
			return true;
		case GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE:
			CurrentLevel = 0;
			break;
		case GAME_STATUS::GAME_STATUS_NEW_GAME:
			CurrentLevel = (SelectedLevelForNewGame != 0 ? SelectedLevelForNewGame : 1);
			SelectedLevelForNewGame = 0;
			gfInitialiseGame = true;
			break;
		case GAME_STATUS::GAME_STATUS_LOAD_GAME:
			// Load the header of the savegame for getting the level to load
			char fileName[255];
			ZeroMemory(fileName, 255);
			sprintf(fileName, "savegame.%d", SelectedSaveGame);
			SaveGame::LoadHeader(fileName, &header);

			// Load level
			CurrentLevel = header.Level;
			loadFromSavegame = true;

			break;
		case GAME_STATUS::GAME_STATUS_LEVEL_COMPLETED:
			if (LevelComplete == m_levels.size())
			{
				// TODO: final credits
			}
			else
				CurrentLevel++;
			break;
		}
	}

	return true;
}

GameFlow* g_GameFlow;