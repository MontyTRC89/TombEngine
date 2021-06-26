#include "framework.h"
#include "GameFlowScript.h"
#include "items.h"
#include "box.h"
#include "lot.h"
#include "sound.h"
#include "savegame.h"
#include "draw.h"
#include "AudioTracks.h"
#include <Objects/objectslist.h>
#include <Game/newinv2.h>

using std::string;
using std::vector;
using std::unordered_map;

extern unordered_map<string, AudioTrack> g_AudioTracks;

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

	// Inventory object type
	m_lua->new_usertype<GameScriptInventoryObject>("InventoryObject",
		sol::constructors<GameScriptInventoryObject(std::string, short, float, float, float, float, float, short, int, __int64)>(),
		"name", &GameScriptInventoryObject::name,
		"yOffset", &GameScriptInventoryObject::yOffset,
		"scale", &GameScriptInventoryObject::scale,
		"xRot", &GameScriptInventoryObject::xRot,
		"yRot", &GameScriptInventoryObject::yRot,
		"zRot", &GameScriptInventoryObject::zRot,
		"rotationFlags", &GameScriptInventoryObject::rotationFlags,
		"meshBits", &GameScriptInventoryObject::meshBits,
		"operation", &GameScriptInventoryObject::operation
		);

	// Audio track type
	m_lua->new_usertype<GameScriptAudioTrack>("AudioTrack",
		sol::constructors<GameScriptAudioTrack(std::string, bool)>(),
		"trackName", &GameScriptAudioTrack::trackName,
		"looped", &GameScriptAudioTrack::looped
		);

	// Level type
	m_lua->new_usertype<GameScriptLevel>("Level",
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

	(*m_lua)["GameFlow"] = std::ref(*this);

	m_lua->new_usertype<GameFlow>("_GameFlow",
		sol::no_constructor,
		"AddLevel", &GameFlow::AddLevel,
		"WriteDefaults", &GameFlow::WriteDefaults,
		"SetAudioTracks", &GameFlow::SetAudioTracks,
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

void GameFlow::SetAudioTracks(sol::as_table_t<std::vector<GameScriptAudioTrack>>&& src)
{
	std::vector<GameScriptAudioTrack> tracks = std::move(src);
	for (auto t : tracks) {
		AudioTrack track;
		track.Name = t.trackName;
		track.Mask = 0;
		track.looped = t.looped;
		g_AudioTracks.insert_or_assign(track.Name, track);
	}
}

bool __cdecl LoadScript()
{
	g_GameFlow->CurrentStrings = g_GameFlow->Strings[0];

	return true;
}

bool GameFlow::LoadGameFlowScript()
{
	// Load the enums file
	std::string err;
	if (!ExecuteScript("Scripts/Enums.lua", err)) {
		std::cout << err << "\n";
	}

	// Load the new audio tracks file
	if (!ExecuteScript("Scripts/Tracks.lua", err)) {
		std::cout << err << "\n";
	}

	// Load the new script file
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

	while (true)
	{
		// First we need to fill some legacy variables in PCTomb5.exe
		GameScriptLevel* level = Levels[CurrentLevel];

		CurrentAtmosphere = level->AmbientTrack;

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
			// Prepare inventory objects table
			for (int i = 0; i < level->InventoryObjects.size(); i++)
			{
				GameScriptInventoryObject* obj = &level->InventoryObjects[i];
				if (obj->slot >= 0 && obj->slot < INVENTORY_TABLE_SIZE)
				{
					INVOBJ* invObj = &inventry_objects_list[obj->slot];

					invObj->objname = obj->name.c_str();
					invObj->scale1 = obj->scale;
					invObj->yoff = obj->yOffset;
					invObj->xrot = obj->xRot;
					invObj->zrot = obj->zRot;
					invObj->meshbits = obj->meshBits;
					invObj->opts = obj->operation;
					invObj->rot_flags = obj->rotationFlags;
				}
			}

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