#pragma once
#include "LanguageScript.h"
#include "LuaHandler.h"
#include "GameScriptColor.h"
#include "GameScriptLevel.h"
#include "GameScriptSettings.h"
#include "GameScriptAudioTrack.h"

enum TITLE_TYPE
{
	TITLE_FLYBY,
	TITLE_BACKGROUND
};

class GameFlow : public LuaHandler
{
private:
	GameScriptSettings				m_settings;

	std::unordered_map < std::string, std::vector<std::string > > m_translationsMap;
	std::vector<std::string> m_languageNames;

	std::map<short, short>			m_itemsMap;

public:
	Vector3							SkyColorLayer1{};
	Vector3							SkyColorLayer2{};
	Vector3							FogColor{};
	int								SkySpeedLayer1{ 0 };
	int								SkySpeedLayer2{ 0 };
	int								FogInDistance{ 0 };
	int								FogOutDistance{ 0 };
	bool							DrawHorizon{ false };
	bool							ColAddHorizon{ false };
	int								SelectedLevelForNewGame{ 0 };
	int								SelectedSaveGame{ 0 };
	bool							EnableLoadSave{ true };
	bool							PlayAnyLevel{ true };
	bool							FlyCheat{ true };
	bool							DebugMode{ false };
	int								LevelFarView{ 0 };
	TITLE_TYPE						TitleType{ TITLE_BACKGROUND };
	std::string						IntroImagePath{};

	// Selected language set
	std::vector<GameScriptLevel*>			Levels;

	GameFlow(sol::state* lua);
	~GameFlow();

	void								AddLevel(GameScriptLevel const& level);
	void								SetAudioTracks(sol::as_table_t<std::vector<GameScriptAudioTrack>>&& src);
	bool								LoadGameFlowScript();
	char const *						GetString(const char* id);
	void								SetStrings(sol::nested<std::unordered_map<std::string, std::vector<std::string>>> && src);
	void								SetLanguageNames(sol::as_table_t<std::vector<std::string>> && src);
	GameScriptSettings*					GetSettings();
	GameScriptLevel*					GetLevel(int id);
	void								SetHorizon(bool horizon, bool colAddHorizon);
	void								SetLayer1(byte r, byte g, byte b, short speed);
	void								SetLayer2(byte r, byte g, byte b, short speed);
	void								SetFog(byte r, byte g, byte b, short startDistance, short endDistance);
	int									GetNumLevels();		
	bool								DoGameflow();
	void								SetIntroImagePath(std::string const& path);
};