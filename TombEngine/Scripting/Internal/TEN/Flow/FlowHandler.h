#pragma once
#include <string_view>

#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Scripting/Include/ScriptInterfaceGame.h"
#include "Scripting/Internal/LanguageScript.h"
#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/Logic/LogicHandler.h"
#include "Scripting/Internal/TEN/Flow/Animations/Animations.h"
#include "Scripting/Internal/TEN/Flow/Level/FlowLevel.h"
#include "Scripting/Internal/TEN/Flow/Settings/Settings.h"

class FlowHandler : public ScriptInterfaceFlowHandler
{
private:
	LuaHandler	_handler;
	Settings	_settings = {};
	std::string _gameDir  = {};

	std::map<int, int> _moveableMap = {};

	std::unordered_map<std::string, std::vector<std::string>> _translationMap = {};
	std::vector<std::string>								  _languageNames  = {};

	void PrepareInventoryObjects();

public:
	int FogInDistance  = 0;
	int FogOutDistance = 0;

	bool LevelSelect = true;
	bool HomeLevel	 = false;
	bool LoadSave	 = true;
	bool FlyCheat	 = true;
	bool PointFilter = false;
	bool MassPickup	 = true;
	bool LaraInTitle = false;
	bool DebugMode	 = false;

	// Table for movesets.
	Animations Anims = {};

	std::vector<Level*>	Levels;

	FlowHandler(sol::state* lua, sol::table& parent);
	~FlowHandler() override;

	std::string	GetGameDir() override;
	void		SetGameDir(const std::string& assetDir) override;
	void		AddLevel(Level const& level);
	void		LoadFlowScript();
	char const*	GetString(const char* id) const;
	void		SetStrings(sol::nested<std::unordered_map<std::string, std::vector<std::string>>>&& src);
	void		SetLanguageNames(sol::as_table_t<std::vector<std::string>>&& src);
	void		SetAnimations(const Animations& src);
	void		SetSettings(const Settings& src);
	Settings*	GetSettings();
	Level*		GetLevel(int id);
	Level*		GetCurrentLevel();
	int			GetLevelNumber(const std::string& flieName);
	int			GetNumLevels() const;
	void		EndLevel(std::optional<int> nextLevel, std::optional<int> startPosIndex);
	GameStatus	GetGameStatus();
	void		FlipMap(int group);
	bool		GetFlipMapStatus(std::optional<int> group);
	void		SaveGame(int slot);
	void		LoadGame(int slot);
	void		DeleteSaveGame(int slot);
	bool		DoesSaveGameExist(int slot);
	int			GetSecretCount() const;
	void		SetSecretCount(int secretsNum);
	void		AddSecret(int levelSecretIndex);
	void		SetIntroImagePath(const std::string& path);
	void		SetTitleScreenImagePath(const std::string& path);
	void		SetTotalSecretCount(int secretsNumber);
	bool		IsFlyCheatEnabled() const;
	void		EnableFlyCheat(bool enable);
	bool		IsPointFilterEnabled() const;
	void		EnablePointFilter(bool enable);
	bool		IsMassPickupEnabled() const;
	void		EnableMassPickup(bool enable);
	bool		IsLaraInTitleEnabled() const;
	void		EnableLaraInTitle(bool enable);
	bool		IsLevelSelectEnabled() const;
	void		EnableLevelSelect(bool enable);
	bool		IsHomeLevelEnabled() const;
	void		EnableHomeLevel(bool enable);
	bool		IsLoadSaveEnabled() const;
	void		EnableLoadSave(bool enable);

	bool HasCrawlExtended() const override { return Anims.HasCrawlExtended; }
	bool HasCrouchRoll() const override { return Anims.HasCrouchRoll; }
	bool HasCrawlspaceDive() const override { return Anims.HasCrawlspaceDive; }
	bool HasAFKPose() const override { return Anims.HasPose; }
	bool HasOverhangClimb() const override { return Anims.HasOverhangClimb; }
	bool HasSlideExtended() const override { return Anims.HasSlideExtended; }
	bool HasSprintJump() const override { return Anims.HasSprintJump; }
	bool HasLedgeJumps() const override { return Anims.HasLedgeJumps; }
	bool DoFlow() override;

	// NOTE: Removed. Keep for now to maintain compatibility. -- Sezz 2024.06.06
	bool HasAutoMonkeySwingJump() const override { return Anims.HasAutoMonkeySwingJump; }
};
