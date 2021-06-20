#include "framework.h"
#include "GameFlowScript.h"
#include "items.h"
#include "box.h"
#include "lot.h"
#include "sound.h"
#include "savegame.h"
#include "draw.h"
#include "AudioTracks.h"

using std::string;
using std::vector;
std::unique_ptr<ChunkId> ChunkGameFlowFlags = ChunkId::FromString("Tr5MainFlags");
std::unique_ptr<ChunkId> ChunkGameFlowLevel = ChunkId::FromString("Tr5MainLevel");
std::unique_ptr<ChunkId> ChunkGameFlowLevelFlags = ChunkId::FromString("Tr5MainLevelFlags");
std::unique_ptr<ChunkId> ChunkGameFlowLevelInfo = ChunkId::FromString("Tr5MainLevelInfo");
std::unique_ptr<ChunkId> ChunkGameFlowLevelPuzzle = ChunkId::FromString("Tr5MainLevelPuzzle");
std::unique_ptr<ChunkId> ChunkGameFlowLevelKey = ChunkId::FromString("Tr5MainLevelKey");
std::unique_ptr<ChunkId> ChunkGameFlowLevelPuzzleCombo = ChunkId::FromString("Tr5MainLevelPuzzleCombo");
std::unique_ptr<ChunkId> ChunkGameFlowLevelKeyCombo = ChunkId::FromString("Tr5MainLevelKeyCombo");
std::unique_ptr<ChunkId> ChunkGameFlowLevelPickup = ChunkId::FromString("Tr5MainLevelPickup");
std::unique_ptr<ChunkId> ChunkGameFlowLevelPickupCombo = ChunkId::FromString("Tr5MainLevelPickupCombo");
std::unique_ptr<ChunkId> ChunkGameFlowLevelExamine = ChunkId::FromString("Tr5MainLevelExamine");
std::unique_ptr<ChunkId> ChunkGameFlowLevelLayer = ChunkId::FromString("Tr5MainLevelLayer");
std::unique_ptr<ChunkId> ChunkGameFlowLevelLuaEvent = ChunkId::FromString("Tr5MainLevelLuaEvent");
std::unique_ptr<ChunkId> ChunkGameFlowLevelLegend = ChunkId::FromString("Tr5MainLevelLegend");
std::unique_ptr<ChunkId> ChunkGameFlowStrings = ChunkId::FromString("Tr5MainStrings");
std::unique_ptr<ChunkId> ChunkGameFlowAudioTracks = ChunkId::FromString("Tr5MainAudioTracks");
std::unique_ptr<ChunkId> ChunkGameFlowTitleBackground = ChunkId::FromString("Tr5MainTitleBackground");

extern vector<AudioTrack> g_AudioTracks;

GameFlow::GameFlow(sol::state* lua) : LuaHandler{ lua }
{
	//Hardcode in English - old strings for now; will be removed shortly
	LanguageScript* lang = new LanguageScript("English");
	Strings.push_back(lang);

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
		sol::constructors<GameScriptSkyLayer(byte, byte, byte, short)>(),
		"r", &GameScriptSkyLayer::R,
		"g", &GameScriptSkyLayer::G,
		"b", &GameScriptSkyLayer::B,
		"speed", &GameScriptSkyLayer::CloudSpeed
		);

	// Mirror type
	m_lua->new_usertype<GameScriptMirror>("Mirror",
		sol::constructors<GameScriptMirror(short, int, int, int, int)>(),
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

	// Weather type
	m_lua->new_enum("WeatherType",
		"Normal", WEATHER_TYPES::WEATHER_NORMAL,
		"Rain", WEATHER_TYPES::WEATHER_RAIN,
		"Snow", WEATHER_TYPES::WEATHER_SNOW);

	m_lua->new_enum("LaraType",
		"Normal", LARA_DRAW_TYPE::LARA_NORMAL,
		"Young", LARA_DRAW_TYPE::LARA_YOUNG,
		"Bunhead", LARA_DRAW_TYPE::LARA_BUNHEAD,
		"Catsuit", LARA_DRAW_TYPE::LARA_CATSUIT,
		"Divesuit", LARA_DRAW_TYPE::LARA_DIVESUIT,
		"Invisible", LARA_DRAW_TYPE::LARA_INVISIBLE);

	// Level type
	m_lua->new_usertype<GameScriptLevel>("Level",
		sol::constructors<GameScriptLevel()>(),
		"name", &GameScriptLevel::NameStringKey,
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
		"weather", &GameScriptLevel::Weather,
		"laraType", &GameScriptLevel::LaraType,
		"rumble", &GameScriptLevel::Rumble,
		"resetHub", &GameScriptLevel::ResetHub,
		"mirror", &GameScriptLevel::Mirror
	);

	(*m_lua)["GameFlow"] = std::ref(*this);


	m_lua->new_usertype<GameFlow>("_GameFlow",
		sol::no_constructor,
		"AddLevel", &GameFlow::AddLevel,
		"WriteDefaults", &GameFlow::WriteDefaults,
		"AddTracks", &GameFlow::AddTracks,
		"strings", sol::property(&GameFlow::GetLang), // for compatibility with old strings
		"SetStrings", &GameFlow::SetStrings,
		"SetLanguageNames", &GameFlow::SetLanguageNames
		);
}

GameFlow::~GameFlow()
{
	for (auto& lev : Levels)
	{
		delete lev;
	}

	for (auto& lang : Strings)
	{
		delete lang;
	}
}

// This is for compatibility with the current English.strings and will be removed
// once the new Lua strings system is in place
auto GameFlow::GetLang() -> decltype(std::ref(Strings[0]->Strings))
{
	auto * eng = Strings[0];
	return std::ref(eng->Strings);
};

void GameFlow::SetLanguageNames(sol::as_table_t<std::vector<std::string>> && src)
{
	m_languageNames = std::move(src);
}

void GameFlow::SetStrings(sol::nested<std::unordered_map<std::string, std::vector<std::string>>> && src)
{
	m_translationsMap = std::move(src);
}

//hardcoded for now
void GameFlow::WriteDefaults()
{
	Intro = "SCREENS\\MAIN.PNG";
	EnableLoadSave = true;
	PlayAnyLevel = true;
	FlyCheat = true;
	DebugMode = false;
	LevelFarView = 0;
}

void GameFlow::AddLevel(GameScriptLevel const& level)
{
	Levels.push_back(new GameScriptLevel{ level });
}

void GameFlow::AddTracks() {
	for (auto str : kAudioTracks) {
		AudioTrack track;
		track.Name = str;
		track.Mask = 0;
		g_AudioTracks.push_back(track);
	}
}

bool __cdecl LoadScript()
{
	g_GameFlow->CurrentStrings = g_GameFlow->Strings[0];

	return true;
}

bool GameFlow::LoadGameFlowScript()
{
	// Load the new script file
	std::string err;
	if (!ExecuteScript("Scripts/Gameflow.lua", err)) {
		std::cout << err << "\n";
	}

	// Hardcode English for now - this will be changed
	// to something like "Strings.lua" once that system
	//  through
	if (!ExecuteScript("Scripts/Strings.lua", err)) {
		std::cout << err << "\n";
	}

	return true;
}

char* GameFlow::GetString(const char* id)
{
	if (m_translationsMap.find(id) == m_translationsMap.end())
		return "String not found";
	else
		return (char*)(m_translationsMap.at(string(id)).at(0).c_str());
}

GameScriptSettings* GameFlow::GetSettings()
{
	return &m_settings;
}

GameScriptLevel* GameFlow::GetLevel(int id)
{
	return Levels[id];
}

void GameFlow::SetHorizon(bool horizon, bool colAddHorizon)
{
	DrawHorizon = horizon;
	ColAddHorizon = colAddHorizon;
}

void GameFlow::SetLayer1(byte r, byte g, byte b, short speed)
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

void GameFlow::SetLayer2(byte r, byte g, byte b, short speed)
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

void GameFlow::SetFog(byte r, byte g, byte b, short startDistance, short endDistance)
{
	FogColor.x = r / 255.0f;
	FogColor.y = g / 255.0f;
	FogColor.z = b / 255.0f;
	FogInDistance = startDistance;
	FogOutDistance = endDistance;
}

int	GameFlow::GetNumLevels()
{
	return Levels.size();
}

bool GameFlow::DoGameflow()
{
	// We start with the title level
	CurrentLevel = 0;
	SelectedLevelForNewGame = 0;
	SelectedSaveGame = 0;
	SaveGameHeader header;

	// We loop indefinitely, looking for return values of DoTitle or DoLevel
	bool loadFromSavegame = false;

	//DoLevel(0, 120, false);

	while (true)
	{
		// First we need to fill some legacy variables in PCTomb5.exe
		GameScriptLevel* level = Levels[CurrentLevel];
		//level->LaraType = LARA_YOUNG;
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
		case GAME_STATUS_EXIT_GAME:
			return true;
		case GAME_STATUS_EXIT_TO_TITLE:
			CurrentLevel = 0;
			break;
		case GAME_STATUS_NEW_GAME:
			CurrentLevel = (SelectedLevelForNewGame != 0 ? SelectedLevelForNewGame : 1);
			SelectedLevelForNewGame = 0;
			InitialiseGame = true;
			break;
		case GAME_STATUS_LOAD_GAME:
			// Load the header of the savegame for getting the level to load
			char fileName[255];
			ZeroMemory(fileName, 255);
			sprintf(fileName, "savegame.%d", SelectedSaveGame);
			SaveGame::LoadHeader(fileName, &header);

			// Load level
			CurrentLevel = header.Level;
			loadFromSavegame = true;

			break;
		case GAME_STATUS_LEVEL_COMPLETED:
			if (LevelComplete == Levels.size())
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