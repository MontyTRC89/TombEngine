#pragma once
#include "LanguageScript.h"
#include "LuaHandler.h"
#include "GameLogicScript.h"
#include "GameScriptColor.h"
#include "GameScriptLevel.h"
#include "GameScriptSettings.h"
#include "GameScriptAudioTrack.h"
#include "GameScriptAnimations.h"
#include "ScriptInterfaceGame.h"
#include "ScriptInterfaceFlow.h"

enum class TITLE_TYPE
{
	FLYBY,
	BACKGROUND
};

class GameFlow : public LuaHandler, public ScriptInterfaceFlow
{
private:
	GameScriptSettings				m_settings;

	std::unordered_map < std::string, std::vector<std::string > > m_translationsMap;
	std::vector<std::string> m_languageNames;

	std::map<short, short>			m_itemsMap;

public:
	int								FogInDistance{ 0 };
	int								FogOutDistance{ 0 };
	int								SelectedLevelForNewGame{ 0 };
	int								SelectedSaveGame{ 0 };
	bool							EnableLoadSave{ true };
	bool							PlayAnyLevel{ true };
	bool							FlyCheat{ true };
	bool							DebugMode{ false };
	byte							GameFarView{ 0 };
	TITLE_TYPE						TitleType{ TITLE_TYPE::FLYBY };
	std::string						IntroImagePath{};
	std::string						TitleScreenImagePath{};

	// New animation flag table
	GameScriptAnimations			Animations{};

	// Selected language set
	std::vector<GameScriptLevel*>	Levels;

	GameFlow(sol::state* lua);
	~GameFlow();

	void							AddLevel(GameScriptLevel const& level);
	void							SetAudioTracks(sol::as_table_t<std::vector<GameScriptAudioTrack>>&& src);
	void							LoadGameFlowScript();
	char const *					GetString(const char* id) const;
	void							SetStrings(sol::nested<std::unordered_map<std::string, std::vector<std::string>>> && src);
	void							SetLanguageNames(sol::as_table_t<std::vector<std::string>> && src);
	void							SetAnimations(GameScriptAnimations const & src);
	void							SetSettings(GameScriptSettings const & src);
	GameScriptSettings*				GetSettings();
	GameScriptLevel*				GetLevel(int id);
	int								GetNumLevels() const;
	bool							DoGameflow();
	void							SetIntroImagePath(std::string const& path);
	void							SetTitleScreenImagePath(std::string const& path);
	void							SetGameFarView(byte val);
	bool							IsFlyCheatEnabled() const;

	bool HasCrawlExtended() const { return Animations.CrawlExtended; }
	bool HasCrouchRoll() const { return Animations.CrouchRoll; }
	bool HasCrawlspaceSwandive() const { return Animations.CrawlspaceSwandive; }
	bool HasMonkeyTurn180() const { return Animations.MonkeyTurn180; }
	bool HasMonkeyAutoJump() const { return Animations.MonkeyAutoJump; }
	bool HasOscillateHang() const { return Animations.OscillateHang; }
	bool HasAFKPose() const { return Animations.Pose; }
};

extern ScriptInterfaceFlow* g_GameFlow;
extern ScriptInterfaceGame* g_GameScript;
