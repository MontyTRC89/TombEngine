#pragma once
#include "ChunkId.h"
#include "ChunkReader.h"
#include "LEB128.h"
#include "LanguageScript.h"
#include "savegame.h"

#define TITLE_FLYBY			0
#define TITLE_BACKGROUND	1
#define MAX_SAVEGAMES 16

struct ChunkId;
struct LEB128;

namespace T5M::Script
{
	typedef enum WEATHER_TYPES
	{
		WEATHER_NORMAL,
		WEATHER_RAIN,
		WEATHER_SNOW
	};

	typedef enum LARA_DRAW_TYPE
	{
		LARA_NORMAL = 1,
		LARA_YOUNG = 2,
		LARA_BUNHEAD = 3,
		LARA_CATSUIT = 4,
		LARA_DIVESUIT = 5,
		LARA_INVISIBLE = 7
	};

	struct GameScriptSettings
	{
		int ScreenWidth;
		int ScreenHeight;
		bool EnableLoadSave;
		bool EnableDynamicShadows;
		bool EnableWaterCaustics;
		bool Windowed;
		std::string WindowTitle;
		int DrawingDistance;
		bool ShowRendererSteps;
		bool ShowDebugInfo;
	};

	struct GameScriptSkyLayer
	{
		bool Enabled;
		byte R;
		byte G;
		byte B;
		short CloudSpeed;

		GameScriptSkyLayer()
		{
			Enabled = false;
			R = G = B = CloudSpeed = 0;
		}

		GameScriptSkyLayer(byte r, byte g, byte b, short speed)
		{
			R = r;
			G = g;
			B = b;
			CloudSpeed = speed;
			Enabled = true;
		}
	};

	struct GameScriptFog
	{
		byte R;
		byte G;
		byte B;

		GameScriptFog()
		{

		}

		GameScriptFog(byte r, byte g, byte b)
		{
			R = r;
			G = g;
			B = b;
		}
	};

	struct GameScriptMirror
	{
		short Room;
		int StartX;
		int EndX;
		int StartZ;
		int EndZ;

		GameScriptMirror()
		{
			Room = -1;
			StartX = EndX = StartZ = EndZ = 0;
		}

		GameScriptMirror(short room, int startX, int endX, int startZ, int endZ)
		{
			Room = room;
			StartX = startX;
			EndX = endX;
			StartZ = startZ;
			EndZ = endZ;
		}
	};

	struct GameScriptLevel
	{
		int NameStringIndex;
		std::string FileName;
		std::string ScriptFileName;
		std::string LoadScreenFileName;
		std::string Background;
		int Name;
		int Soundtrack;
		GameScriptSkyLayer Layer1;
		GameScriptSkyLayer Layer2;
		bool Horizon;
		bool Sky;
		bool ColAddHorizon;
		GameScriptFog Fog;
		bool Storm;
		WEATHER_TYPES Weather;
		bool ResetHub;
		bool Rumble;
		LARA_DRAW_TYPE LaraType;
		GameScriptMirror Mirror;
		byte UVRotate;
		int LevelFarView;
		bool UnlimitedAir;

		GameScriptLevel()
		{
			Storm = false;
			Horizon = false;
			ColAddHorizon = false;
			ResetHub = false;
			Rumble = false;
			Weather = WEATHER_NORMAL;
			LaraType = LARA_NORMAL;
			UnlimitedAir = false;
		}
	};

	bool readGameFlowLevelChunks(ChunkId* chunkId, int maxSize, int arg);
	bool readGameFlowChunks(ChunkId* chunkId, int maxSize, int arg);
	bool readGameFlowLevel();
	bool readGameFlowStrings();
	bool readGameFlowTracks();
	bool readGameFlowFlags();

	bool LoadScript();

	typedef struct SavegameInfo
	{
		bool present;
		char levelName[75];
		int saveNumber;
		short days;
		short hours;
		short minutes;
		short seconds;
		char fileName[255];
	};

	struct GAMEFLOW
	{
		unsigned int CheatEnabled : 1;
		unsigned int LoadSaveEnabled : 1;
		unsigned int TitleEnabled : 1;
		unsigned int PlayAnyLevel : 1;
		unsigned int Language : 3;
		unsigned int DemoDisc : 1;
		unsigned int Unused : 24;
		unsigned int InputTimeout;
		unsigned char SecurityTag;
		unsigned char nLevels;
		unsigned char nFileNames;
		unsigned char Pad;
		unsigned short FileNameLen;
		unsigned short ScriptLen;
	};

	extern SavegameInfo g_SavegameInfos[MAX_SAVEGAMES];
	extern std::vector<std::string> g_NewStrings;
	extern SaveGameHeader g_NewSavegameInfos[MAX_SAVEGAMES];

	int LoadSavegameInfos();

	class GameFlow
	{
	private:
		sol::state							m_lua;
		GameScriptSettings					m_settings;
	
		std::string								loadScriptFromFile(char* luaFilename);
		std::map<short, short>				m_itemsMap;

	public:
		Vector3								SkyColorLayer1;
		int								SkySpeedLayer1;
		Vector3								SkyColorLayer2;
		int								SkySpeedLayer2;
		Vector3								FogColor;
		int								FogInDistance;
		int								FogOutDistance;
		bool								DrawHorizon;
		bool								ColAddHorizon;
		int								SelectedLevelForNewGame;
		int								SelectedSaveGame;
		bool								EnableLoadSave;
		bool								PlayAnyLevel;
		bool								FlyCheat;
		bool								DebugMode;
		int								LevelFarView;
		int								TitleType;
		char*								Intro;

		// Selected language set
		LanguageScript*						CurrentStrings;
		std::vector<LanguageScript*>				Strings;
		std::vector<GameScriptLevel*>			Levels;

		GameFlow();

		bool								LoadGameStrings(char* luaFilename);
		bool								LoadGameSettings(char* luaFilename);
		bool								ExecuteScript(char* luaFilename);
		char*								GetString(int id);
		GameScriptSettings*					GetSettings();
		GameScriptLevel*					GetLevel(int id);
		void								SetHorizon(bool horizon, bool colAddHorizon);
		void								SetLayer1(byte r, byte g, byte b, short speed);
		void								SetLayer2(byte r, byte g, byte b, short speed);
		void								SetFog(byte r, byte g, byte b, short startDistance, short endDistance);
		int								GetNumLevels();		
		bool								DoGameflow();
	};

	extern GameFlow *g_GameFlow;
}
