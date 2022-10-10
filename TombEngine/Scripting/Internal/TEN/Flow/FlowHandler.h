#pragma once
#include "LanguageScript.h"
#include "LuaHandler.h"
#include "Logic/LogicHandler.h"
#include "Color/Color.h"
#include "Flow/Level/FlowLevel.h"
#include "Settings/Settings.h"
#include "Flow/Animations/Animations.h"
#include "ScriptInterfaceGame.h"
#include "Flow/ScriptInterfaceFlowHandler.h"

class FlowHandler : public ScriptInterfaceFlowHandler
{
private:
	Settings				m_settings;

	std::unordered_map < std::string, std::vector<std::string > > m_translationsMap;
	std::vector<std::string> m_languageNames;

	std::map<short, short>			m_itemsMap;

	LuaHandler m_handler;

public:
	int								FogInDistance{ 0 };
	int								FogOutDistance{ 0 };
	bool							PlayAnyLevel{ true };
	bool							FlyCheat{ true };
	bool							DebugMode{ false };

	// New animation flag table
	Animations			Anims{};

	// Selected language set
	std::vector<Level*>	Levels;

	FlowHandler(sol::state* lua, sol::table & parent);
	~FlowHandler() override;

	void				AddLevel(Level const& level);
	void				LoadFlowScript();
	char const *		GetString(const char* id) const;
	void				SetStrings(sol::nested<std::unordered_map<std::string, std::vector<std::string>>> && src);
	void				SetLanguageNames(sol::as_table_t<std::vector<std::string>> && src);
	void				SetAnimations(Animations const & src);
	void				SetSettings(Settings const & src);
	Settings*			GetSettings();
	Level*				GetLevel(int id);
	Level*				GetCurrentLevel();
	int					GetLevelNumber(std::string const& flieName);
	int					GetNumLevels() const;
	void				EndLevel(std::optional<int> nextLevel);
	int					GetSecretCount() const;
	void				SetSecretCount(int secretsNum);
	void				AddSecret(int levelSecretIndex);
	void				SetIntroImagePath(std::string const& path);
	void				SetTitleScreenImagePath(std::string const& path);
	bool				IsFlyCheatEnabled() const;
	bool				CanPlayAnyLevel() const;

	bool HasCrawlExtended() const override { return Anims.HasCrawlExtended; }
	bool HasCrouchRoll() const override { return Anims.HasCrouchRoll; }
	bool HasCrawlspaceSwandive() const override { return Anims.HasCrawlspaceDive; }
	bool HasMonkeyAutoJump() const override { return Anims.HasMonkeyAutoJump; }
	bool HasAFKPose() const override { return Anims.HasPose; }
	bool HasOverhangClimb() const override { return Anims.HasOverhangClimb; }
	bool HasSlideExtended() const override { return Anims.HasSlideExtended; }
	bool HasSprintJump() const override { return Anims.HasSprintJump; }
	bool HasLedgeJumpUp() const override { return Anims.HasLedgeJumpUp; }
	bool HasLedgeJumpBack() const override { return Anims.HasLedgeJumpBack; }
	bool DoFlow() override;
};

