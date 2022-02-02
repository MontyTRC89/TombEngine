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
#include "Entity/Entity.h"

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
	bool							PlayAnyLevel{ true };
	bool							FlyCheat{ true };
	bool							DebugMode{ false };
	byte							GameFarView{ 0 };

	// New animation flag table
	GameScriptAnimations			Animations{};

	// Selected language set
	std::vector<GameScriptLevel*>	Levels;

	GameFlow(sol::state* lua);
	~GameFlow();

	void							AddLevel(GameScriptLevel const& level);
	void							LoadGameFlowScript();
	char const *					GetString(const char* id) const;
	void							SetStrings(sol::nested<std::unordered_map<std::string, std::vector<std::string>>> && src);
	void							SetLanguageNames(sol::as_table_t<std::vector<std::string>> && src);
	void							SetAnimations(GameScriptAnimations const & src);
	void							SetSettings(GameScriptSettings const & src);
	GameScriptSettings*				GetSettings();
	GameScriptLevel*				GetLevel(int id);
	int								GetNumLevels() const;
	void							SetIntroImagePath(std::string const& path);
	void							SetTitleScreenImagePath(std::string const& path);
	void							SetGameFarView(byte val);
	bool							IsFlyCheatEnabled() const;
	bool							CanPlayAnyLevel() const;

	bool HasCrawlExtended() const override { return Animations.CrawlExtended; }
	bool HasCrouchRoll() const override { return Animations.CrouchRoll; }
	bool HasCrawlspaceSwandive() const override { return Animations.CrawlspaceSwandive; }
	bool HasMonkeyTurn180() const override { return Animations.MonkeyTurn180; }
	bool HasMonkeyAutoJump() const override { return Animations.MonkeyAutoJump; }
	bool HasOscillateHang() const override { return Animations.OscillateHang; }
	bool HasAFKPose() const override { return Animations.Pose; }
	bool DoGameflow() override;
};

