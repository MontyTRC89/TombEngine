#include "framework.h"
#include "GameFlowScript.h"
#include "items.h"
#include "box.h"
#include "lot.h"
#include "sound.h"
#include "savegame.h"
#include "draw.h"
#include "AudioTracks.h"
#include "GameScriptColor.h"
#include "ScriptAssert.h"
#include <Objects/objectslist.h>
#include <Game/newinv2.h>

#ifndef _DEBUG
#include <iostream>
#endif

/***
functions for gameflow
@module gameflow
@pragma nostrip
*/

using std::string;
using std::vector;
using std::unordered_map;

extern unordered_map<string, AudioTrack> g_AudioTracks;

GameFlow::GameFlow(sol::state* lua) : LuaHandler{ lua }
{
	GameScriptLevel::Register(m_lua);
	GameScriptSkyLayer::Register(m_lua);
	GameScriptMirror::Register(m_lua);
	GameScriptInventoryObject::Register(m_lua);
	GameScriptSettings::Register(m_lua);
	GameScriptAudioTrack::Register(m_lua);
	GameScriptColor::Register(m_lua);

/***
Add a level to the gameflow.
@function AddLevel
@tparam @{Level} level a level object
*/
	m_lua->set_function("AddLevel", &GameFlow::AddLevel, this);

/*** The path of the .jpg or .png image to show when loading the game.
@function SetIntroImagePath
@tparam string path the path to the image, relative to the TombEngine exe
*/
	m_lua->set_function("SetIntroImagePath", &GameFlow::SetIntroImagePath, this);

/*** The path of the .jpg or .png image to show in the background of the title screen.
__(not yet implemented)__
@function SetTitleScreenImagePath
@tparam string path the path to the image, relative to the TombEngine exe
*/
	m_lua->set_function("SetTitleScreenImagePath", &GameFlow::SetTitleScreenImagePath, this);

/***
@function SetAudioTracks
@tparam table table array-style table with @{AudioTrack} objects 
*/
	m_lua->set_function("SetAudioTracks", &GameFlow::SetAudioTracks, this);

/***
@function SetStrings
@tparam table table array-style table with strings
*/
	m_lua->set_function("SetStrings", &GameFlow::SetStrings, this);

/***
@function SetLanguageNames
@tparam table table array-style table with TODO EXTRA INFO HERE
*/
	m_lua->set_function("SetLanguageNames", &GameFlow::SetLanguageNames, this);

/***
@function SetSettings
@tparam table table array-style table with TODO EXTRA INFO HERE
*/
	m_lua->set_function("SetSettings", &GameFlow::SetSettings, this);
}

GameFlow::~GameFlow()
{
	for (auto& lev : Levels)
	{
		delete lev;
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

void GameFlow::SetSettings(GameScriptSettings const & src)
{
	m_settings = src;
}

void GameFlow::AddLevel(GameScriptLevel const& level)
{
	Levels.push_back(new GameScriptLevel{ level });
}

void GameFlow::SetIntroImagePath(std::string const& path)
{
	IntroImagePath = path;
}

void GameFlow::SetTitleScreenImagePath(std::string const& path)
{
	TitleScreenImagePath = path;
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

void GameFlow::LoadGameFlowScript()
{
	ExecuteScript("Scripts/Enums.lua");
	ExecuteScript("Scripts/Tracks.lua");
	ExecuteScript("Scripts/Gameflow.lua");
	ExecuteScript("Scripts/Strings.lua");
	ExecuteScript("Scripts/Settings.lua");

	SetErrorMode(GetSettings()->ErrorMode);
}

char const * GameFlow::GetString(const char* id)
{
	if (m_translationsMap.find(id) == m_translationsMap.end())
		return "String not found";
	else
		return m_translationsMap.at(string(id)).at(0).c_str();
}

GameScriptSettings* GameFlow::GetSettings()
{
	return &m_settings;
}

GameScriptLevel* GameFlow::GetLevel(int id)
{
	return Levels[id];
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