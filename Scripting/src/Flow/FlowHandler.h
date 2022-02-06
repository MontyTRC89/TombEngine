#pragma once
#include "LanguageScript.h"
#include "LuaHandler.h"
#include "Logic/LogicHandler.h"
#include "GameScriptColor.h"
#include "Flow/Level/Level.h"
#include "GameScriptSettings.h"
#include "Flow/Animations/Animations.h"
#include "ScriptInterfaceGame.h"
#include "Flow/ScriptInterfaceFlowHandler.h"

class FlowHandler : public LuaHandler, public ScriptInterfaceFlowHandler
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
	Animations			Anims{};

	// Selected language set
	std::vector<Level*>	Levels;

	FlowHandler(sol::state* lua, sol::table & parent);
	~FlowHandler();

	void				AddLevel(Level const& level);
	void				LoadFlowScript();
	char const *		GetString(const char* id) const;
	void				SetStrings(sol::nested<std::unordered_map<std::string, std::vector<std::string>>> && src);
	void				SetLanguageNames(sol::as_table_t<std::vector<std::string>> && src);
	void				SetAnimations(Animations const & src);
	void				SetSettings(GameScriptSettings const & src);
	GameScriptSettings*	GetSettings();
	Level*				GetLevel(int id);
	int					GetNumLevels() const;
	void				SetIntroImagePath(std::string const& path);
	void				SetTitleScreenImagePath(std::string const& path);
	void				SetGameFarView(byte val);
	bool				IsFlyCheatEnabled() const;
	bool				CanPlayAnyLevel() const;

	bool HasCrawlExtended() const override { return Anims.CrawlExtended; }
	bool HasCrouchRoll() const override { return Anims.CrouchRoll; }
	bool HasCrawlspaceSwandive() const override { return Anims.CrawlspaceSwandive; }
	bool HasMonkeyTurn180() const override { return Anims.MonkeyTurn180; }
	bool HasMonkeyAutoJump() const override { return Anims.MonkeyAutoJump; }
	bool HasOscillateHang() const override { return Anims.OscillateHang; }
	bool HasAFKPose() const override { return Anims.Pose; }
	bool DoFlow() override;
};

