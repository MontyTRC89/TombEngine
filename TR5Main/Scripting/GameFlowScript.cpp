#include "framework.h"
#include "GameFlowScript.h"
#include "ExceptionScript.h"
#include "items.h"
#include "box.h"
#include "lot.h"
#include "sound.h"
#include "savegame.h"
#include "draw.h"

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

ChunkReader* g_ScriptChunkIO;

extern std::vector<AudioTrack> g_AudioTracks;

GameFlow::GameFlow()
{
	m_lua.open_libraries(sol::lib::base);
	m_lua.set_exception_handler(lua_exception_handler);

	// Settings type
	m_lua.new_usertype<GameScriptSettings>("GameScriptSettings",
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
	m_lua.new_usertype<GameScriptSkyLayer>("SkyLayer",
		sol::constructors<GameScriptSkyLayer(byte, byte, byte, short)>(),
		"r", &GameScriptSkyLayer::R,
		"g", &GameScriptSkyLayer::G,
		"b", &GameScriptSkyLayer::B,
		"speed", &GameScriptSkyLayer::CloudSpeed
		);

	// Mirror type
	m_lua.new_usertype<GameScriptMirror>("Mirror",
		sol::constructors<GameScriptMirror(short, int, int, int, int)>(),
		"room", &GameScriptMirror::Room,
		"startX", &GameScriptMirror::StartX,
		"endX", &GameScriptMirror::EndX,
		"startZ", &GameScriptMirror::StartZ,
		"endZ", &GameScriptMirror::EndZ
		);

	// Fog type
	m_lua.new_usertype<GameScriptFog>("Fog",
		sol::constructors<GameScriptFog(byte, byte, byte)>(),
		"r", &GameScriptFog::R,
		"g", &GameScriptFog::G,
		"b", &GameScriptFog::B
		);

	// Level type
	/*m_lua.new_usertype<GameScriptLevel>("Level",
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
		);*/

	m_lua["Gameflow"] = this;
}

bool __cdecl readGameFlowFlags()
{
	g_GameFlow->EnableLoadSave = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
	g_GameFlow->PlayAnyLevel = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
	g_GameFlow->FlyCheat = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
	g_GameFlow->DebugMode = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
	g_GameFlow->LevelFarView = LEB128::ReadInt32(g_ScriptChunkIO->GetRawStream());
	//g_GameFlow->TitleType = LEB128::ReadInt32(g_ScriptChunkIO->GetRawStream());

	char* str;
	g_ScriptChunkIO->GetRawStream()->ReadString(&str);
	g_GameFlow->Intro = str;

	return true;
}

bool __cdecl readGameFlowStrings()
{
	char* name;
	g_ScriptChunkIO->GetRawStream()->ReadString(&name);
	LanguageScript* lang = new LanguageScript(name);
	free(name);

	int numStrings = LEB128::ReadUInt32(g_ScriptChunkIO->GetRawStream());
	for (int i = 0; i < numStrings; i++)
	{
		char* str;
		g_ScriptChunkIO->GetRawStream()->ReadString(&str);
		lang->Strings.push_back(std::string(str));
		free(str);
	}

	g_GameFlow->Strings.push_back(lang);

	return true;
}

bool __cdecl readGameFlowTracks()
{
	int numTracks = LEB128::ReadUInt32(g_ScriptChunkIO->GetRawStream());
	for (int i = 0; i < numTracks; i++)
	{
		char* str;
		g_ScriptChunkIO->GetRawStream()->ReadString(&str);

		AudioTrack track;
		track.Name = str;
		track.Mask = 0;
		g_AudioTracks.push_back(track);
	}

	return true;
}

bool __cdecl readGameFlowLevelChunks(ChunkId* chunkId, int maxSize, int arg)
{
	GameScriptLevel* level = g_GameFlow->Levels[arg];

	if (chunkId->EqualsTo(ChunkGameFlowLevelInfo.get()))
	{
		char* str;

		g_ScriptChunkIO->GetRawStream()->ReadString(&str);
		level->FileName = std::string(str);
		free(str);

		g_ScriptChunkIO->GetRawStream()->ReadString(&str);
		level->LoadScreenFileName = std::string(str);
		free(str);

		g_ScriptChunkIO->GetRawStream()->ReadString(&str);
		level->Background = str;
		free(str);

		level->NameStringIndex = LEB128::ReadUInt32(g_ScriptChunkIO->GetRawStream());
		level->Soundtrack = LEB128::ReadUInt32(g_ScriptChunkIO->GetRawStream());

		return true;
	}
	else if (chunkId->EqualsTo(ChunkGameFlowLevelFlags.get()))
	{
		level->Horizon = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		level->Sky = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		level->ColAddHorizon = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		level->Storm = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		level->ResetHub = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		level->LaraType = (LARA_DRAW_TYPE)LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		level->UVRotate = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		level->LevelFarView = LEB128::ReadUInt32(g_ScriptChunkIO->GetRawStream());
		level->Rumble = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		level->Weather = (WEATHER_TYPES)LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		level->UnlimitedAir = (WEATHER_TYPES)LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		
		return true;
	}
	else if (chunkId->EqualsTo(ChunkGameFlowLevelPuzzle.get()))
	{
		int itemIndex = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		int itemStringIndex = LEB128::ReadUInt32(g_ScriptChunkIO->GetRawStream());
		for (int i = 0; i < 6; i++)
			LEB128::ReadInt16(g_ScriptChunkIO->GetRawStream());

		return true;
	}
	else if (chunkId->EqualsTo(ChunkGameFlowLevelKey.get()))
	{
		int itemIndex = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		int itemStringIndex = LEB128::ReadUInt32(g_ScriptChunkIO->GetRawStream());
		for (int i = 0; i < 6; i++)
			LEB128::ReadInt16(g_ScriptChunkIO->GetRawStream());

		return true;
	}
	else if (chunkId->EqualsTo(ChunkGameFlowLevelPickup.get()))
	{
		int itemIndex = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		int itemStringIndex = LEB128::ReadUInt32(g_ScriptChunkIO->GetRawStream());
		for (int i = 0; i < 6; i++)
			LEB128::ReadInt16(g_ScriptChunkIO->GetRawStream());

		return true;
	}
	else if (chunkId->EqualsTo(ChunkGameFlowLevelExamine.get()))
	{
		int itemIndex = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		int itemStringIndex = LEB128::ReadUInt32(g_ScriptChunkIO->GetRawStream());
		for (int i = 0; i < 6; i++)
			LEB128::ReadInt16(g_ScriptChunkIO->GetRawStream());

		return true;
	}
	else if (chunkId->EqualsTo(ChunkGameFlowLevelPuzzleCombo.get()))
	{
		int itemIndex = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		int piece = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		int itemStringIndex = LEB128::ReadUInt32(g_ScriptChunkIO->GetRawStream());
		for (int i = 0; i < 6; i++)
			LEB128::ReadInt16(g_ScriptChunkIO->GetRawStream());

		return true;
	}
	else if (chunkId->EqualsTo(ChunkGameFlowLevelKeyCombo.get()))
	{
		int itemIndex = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		int piece = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		int itemStringIndex = LEB128::ReadUInt32(g_ScriptChunkIO->GetRawStream());
		for (int i = 0; i < 6; i++)
			LEB128::ReadInt16(g_ScriptChunkIO->GetRawStream());

		return true;
	}
	else if (chunkId->EqualsTo(ChunkGameFlowLevelPickupCombo.get()))
	{
		int itemIndex = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		int piece = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
		int itemStringIndex = LEB128::ReadUInt32(g_ScriptChunkIO->GetRawStream());
		for (int i = 0; i < 6; i++)
			LEB128::ReadInt16(g_ScriptChunkIO->GetRawStream());

		return true;
	}
	else if (chunkId->EqualsTo(ChunkGameFlowLevelLayer.get()))
	{
		int layerIndex = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());

		if (layerIndex == 1)
		{
			level->Layer1.Enabled = true;
			level->Layer1.R = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
			level->Layer1.G = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
			level->Layer1.B = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
			level->Layer1.CloudSpeed = LEB128::ReadInt16(g_ScriptChunkIO->GetRawStream());
		}
		else
		{
			level->Layer2.Enabled = true;
			level->Layer2.R = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
			level->Layer2.G = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
			level->Layer2.B = LEB128::ReadByte(g_ScriptChunkIO->GetRawStream());
			level->Layer2.CloudSpeed = LEB128::ReadInt16(g_ScriptChunkIO->GetRawStream());
		}

		return true;
	}

	return false;
}

bool __cdecl readGameFlowLevel()
{
	g_GameFlow->Levels.push_back(new GameScriptLevel());
	return g_ScriptChunkIO->ReadChunks(readGameFlowLevelChunks, g_GameFlow->Levels.size() - 1);
}

bool __cdecl readGameFlowChunks(ChunkId* chunkId, int maxSize, int arg)
{
	if (chunkId->EqualsTo(ChunkGameFlowFlags.get()))
		return readGameFlowFlags();
	else if (chunkId->EqualsTo(ChunkGameFlowStrings.get()))
		return readGameFlowStrings();
	else if (chunkId->EqualsTo(ChunkGameFlowAudioTracks.get()))
		return readGameFlowTracks();
	else if (chunkId->EqualsTo(ChunkGameFlowLevel.get()))
		return readGameFlowLevel();
	return false;
}

bool __cdecl LoadScript()
{
	// Load the new script file
	FileStream stream("Script.dat", true, false);
	g_ScriptChunkIO = new ChunkReader(0x4D355254, &stream);
	if (!g_ScriptChunkIO->IsValid())
		return false;

	g_ScriptChunkIO->ReadChunks(readGameFlowChunks, 0);

	g_GameFlow->CurrentStrings = g_GameFlow->Strings[0];

	return true;
}

std::string GameFlow::loadScriptFromFile(char* luaFilename)
{
	using std::ifstream;
	using std::ios;
	ifstream ifs(luaFilename, ios::in | ios::binary | ios::ate);

	ifstream::pos_type fileSize = ifs.tellg();
	ifs.seekg(0, ios::beg);

	std::vector<char> bytes(fileSize);
	ifs.read(bytes.data(), fileSize);

	return std::string(bytes.data(), fileSize);
}

bool GameFlow::LoadGameStrings(char* luaFilename)
{
	std::string script = loadScriptFromFile(luaFilename);
	m_lua.script(script);

	return true;
}

bool GameFlow::LoadGameSettings(char* luaFilename)
{
	std::string script = loadScriptFromFile(luaFilename);
	m_lua.script(script);

	return true;
}

bool GameFlow::ExecuteScript(char* luaFilename)
{
	std::string script = loadScriptFromFile(luaFilename);
	m_lua.script(script);

	return true;
}

char* GameFlow::GetString(int id)
{
	return (char*)(CurrentStrings->Strings[id].c_str()); 
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