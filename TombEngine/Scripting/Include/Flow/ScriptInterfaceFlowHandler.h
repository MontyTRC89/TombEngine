#pragma once

enum class TITLE_TYPE
{
	FLYBY,
	BACKGROUND
};

class ScriptInterfaceLevel;

class ScriptInterfaceFlowHandler {
public:

	std::string	IntroImagePath{};
	int	SelectedLevelForNewGame{ 0 };
	int SelectedSaveGame{ 0 };
	bool EnableLoadSave{ true };
	std::string	TitleScreenImagePath{};
	TITLE_TYPE TitleType{ TITLE_TYPE::FLYBY };

	virtual ~ScriptInterfaceFlowHandler() = default;
	virtual void LoadFlowScript() = 0;
	virtual int	GetNumLevels() const = 0;
	virtual char const* GetString(const char* id) const = 0;
	virtual bool IsFlyCheatEnabled() const = 0;
	virtual bool HasCrawlExtended() const = 0;
	virtual bool HasCrouchRoll() const = 0;
	virtual bool HasCrawlspaceSwandive() const = 0;
	virtual bool HasMonkeyAutoJump() const = 0;
	virtual bool HasSprintJump() const = 0;
	virtual bool HasAFKPose() const = 0;
	virtual bool HasOverhangClimb() const = 0;
	virtual bool HasSlideExtended() const = 0;
	virtual ScriptInterfaceLevel * GetLevel(int level) = 0;
	virtual int	GetLevelNumber(std::string const& flieName) = 0;
	virtual short GetGameFarView() const = 0;
	virtual bool CanPlayAnyLevel() const = 0;
	virtual bool DoFlow() = 0;
};

extern ScriptInterfaceFlowHandler* g_GameFlow;
