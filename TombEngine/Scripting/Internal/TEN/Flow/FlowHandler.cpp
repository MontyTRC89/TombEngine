#include "framework.h"
#include "Scripting/Internal/TEN/Flow/FlowHandler.h"

#include <filesystem>

#include "Game/Gui.h"
#include "Game/savegame.h"
#include "Scripting/Include/Objects/ScriptInterfaceObjectsHandler.h"
#include "Scripting/Include/Strings/ScriptInterfaceStringsHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/TEN/DisplaySprite/AlignModes.h"
#include "Scripting/Internal/TEN/DisplaySprite/ScaleModes.h"
#include "Scripting/Internal/TEN/DisplaySprite/ScriptDisplaySprite.h"
#include "Scripting/Internal/TEN/Flow/InventoryItem/InventoryItem.h"
#include "Scripting/Internal/TEN/Logic/LevelFunc.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"
#include "Scripting/Internal/TEN/Vec3/Vec3.h"
#include "Sound/sound.h"
#include "Specific/trutils.h"

using namespace TEN::Scripting::DisplaySprite;

/***
Functions that (mostly) don't directly impact in-game mechanics. Used for setup
in gameflow.lua, settings.lua and strings.lua; some can be used in level
scripts too.
@tentable Flow 
@pragma nostrip
*/

ScriptInterfaceGame* g_GameScript;
ScriptInterfaceObjectsHandler* g_GameScriptEntities;
ScriptInterfaceStringsHandler* g_GameStringsHandler;
ScriptInterfaceFlowHandler* g_GameFlow;

FlowHandler::FlowHandler(sol::state* lua, sol::table& parent) :
	m_handler(lua)
{
/*** gameflow.lua.
These functions are called in gameflow.lua, a file loosely equivalent to winroomedit's SCRIPT.DAT.
They handle a game's 'metadata'; i.e., things such as level titles, loading screen paths, and default
ambient tracks.
@section Flowlua
*/
	sol::table tableFlow{ m_handler.GetState()->lua_state(), sol::create };
	parent.set(ScriptReserved_Flow, tableFlow);

/***
Add a level to the Flow.
@function AddLevel
@tparam Flow.Level level a level object
*/
	tableFlow.set_function(ScriptReserved_AddLevel, &FlowHandler::AddLevel, this);

/*** Image to show when loading the game.
Must be a .jpg or .png image.
@function SetIntroImagePath
@tparam string path the path to the image, relative to the TombEngine exe
*/
	tableFlow.set_function(ScriptReserved_SetIntroImagePath, &FlowHandler::SetIntroImagePath, this);

/*** Image to show in the background of the title screen.
Must be a .jpg or .png image.
__(not yet implemented)__
@function SetTitleScreenImagePath
@tparam string path the path to the image, relative to the TombEngine exe
*/
	tableFlow.set_function(ScriptReserved_SetTitleScreenImagePath, &FlowHandler::SetTitleScreenImagePath, this);

/*** Enable or disable Lara drawing in title flyby.
Must be true or false
@function EnableLaraInTitle
@tparam bool enabled true or false
*/
	tableFlow.set_function(ScriptReserved_EnableLaraInTitle, &FlowHandler::EnableLaraInTitle, this);

/*** Enable or disable level selection in title flyby.
Must be true or false
@function EnableLevelSelect
@tparam bool enabled true or false
*/
	tableFlow.set_function(ScriptReserved_EnableLevelSelect, &FlowHandler::EnableLevelSelect, this);

	/*** Enable or disable saving and loading of savegames.
	@function EnableLoadSave
	@tparam bool enabled true or false.
	*/
	tableFlow.set_function(ScriptReserved_EnableLoadSave, &FlowHandler::EnableLoadSave, this);

/*** gameflow.lua or level scripts.
@section FlowluaOrScripts
*/

/*** Enable or disable DOZY mode (fly cheat).
Must be true or false
@function EnableFlyCheat
@tparam bool enabled true or false
*/
	tableFlow.set_function(ScriptReserved_EnableFlyCheat, &FlowHandler::EnableFlyCheat, this);

/*** Enable or disable point texture filter.
Must be true or false
@function EnablePointFilter
@tparam bool enabled true or false
*/
	tableFlow.set_function(ScriptReserved_EnablePointFilter, &FlowHandler::EnablePointFilter, this);

/*** Enable or disable mass pickup.
Must be true or false
@function EnableMassPickup
@tparam bool enabled true or false
*/
	tableFlow.set_function(ScriptReserved_EnableMassPickup, &FlowHandler::EnableMassPickup, this);

/*** Returns the level by index.
Indices depend on the order in which AddLevel was called; the first added will
have an ID of 0, the second an ID of 1, and so on.
@function GetLevel
@tparam int index of the level
@treturn Flow.Level the level indicated by the id
*/
	tableFlow.set_function(ScriptReserved_GetLevel, &FlowHandler::GetLevel, this);

/*** Returns the level that the game control is running in that moment.
@function GetCurrentLevel
@treturn Flow.Level the current level
*/
	tableFlow.set_function(ScriptReserved_GetCurrentLevel, &FlowHandler::GetCurrentLevel, this);

/***
Finishes the current level, with optional level index provided. If level index
is not provided or is zero, jumps to next level. If level index is more than
level count, jumps to title.
@function EndLevel
@int[opt] index level index (default 0)
*/
	tableFlow.set_function(ScriptReserved_EndLevel, &FlowHandler::EndLevel, this);

	/***
	Save the game to a savegame slot.
	@function SaveGame
	@tparam int slotID ID of the savegame slot to save to.
	*/
	tableFlow.set_function(ScriptReserved_SaveGame, &FlowHandler::SaveGame, this);

	/***
	Load the game from a savegame slot.
	@function LoadGame
	@tparam int slotID ID of the savegame slot to load from.
	*/
	tableFlow.set_function(ScriptReserved_LoadGame, &FlowHandler::LoadGame, this);

	/***
	Delete a savegame.
	@function DeleteSaveGame
	@tparam int slotID ID of the savegame slot to clear.
	*/
	tableFlow.set_function(ScriptReserved_DeleteSaveGame, &FlowHandler::DeleteSaveGame, this);

	/***
	Check if a savegame exists.
	@function DoesSaveGameExist
	@tparam int slotID ID of the savegame slot to check.
	@treturn bool true if the savegame exists, false if not.
	*/
	tableFlow.set_function(ScriptReserved_DoesSaveGameExist, &FlowHandler::DoesSaveGameExist, this);

/***
Returns the player's current per-game secret count.
@function GetSecretCount
@treturn int Current game secret count.
*/
	tableFlow.set_function(ScriptReserved_GetSecretCount, &FlowHandler::GetSecretCount, this);

/*** 
Sets the player's current per-game secret count.
@function SetSecretCount
@tparam int count new secret count.
*/
	tableFlow.set_function(ScriptReserved_SetSecretCount, &FlowHandler::SetSecretCount, this);

/***
Adds one secret to current level secret count and also plays secret music track.
The index argument corresponds to the secret's unique ID, the same that would go in a secret trigger's Param.
@function AddSecret
@tparam int index an index of current level's secret (must be from 0 to 31).
*/
	tableFlow.set_function(ScriptReserved_AddSecret, &FlowHandler::AddSecret, this);

/*** Total number of secrets in game.
Must be an integer value (0 means no secrets).
@function SetTotalSecretCount
@tparam int total number of secrets
*/
	tableFlow.set_function(ScriptReserved_SetTotalSecretCount, &FlowHandler::SetTotalSecretCount, this);

/*** settings.lua.
These functions are called in settings.lua, a file which holds your local settings.
settings.lua shouldn't be bundled with any finished levels/games.
@section settingslua
*/
/***
@function SetSettings
@tparam Flow.Settings settings a settings object 
*/
	tableFlow.set_function(ScriptReserved_SetSettings, &FlowHandler::SetSettings, this);

/***
@function SetAnimations
@tparam Flow.Animations animations an animations object 
*/
	tableFlow.set_function(ScriptReserved_SetAnimations, &FlowHandler::SetAnimations, this);

/*** strings.lua. 
These functions used in strings.lua, which is generated by TombIDE.
You will not need to call them manually.
@section stringslua
*/
/*** Set string variable keys and their translations.
@function SetStrings
@tparam tab table array-style table with strings
*/
	tableFlow.set_function(ScriptReserved_SetStrings, &FlowHandler::SetStrings, this);

/*** Get translated string.
@function GetString
@tparam key string key for translated string 
*/
	tableFlow.set_function(ScriptReserved_GetString, &FlowHandler::GetString, this);

/*** Set language names for translations.
Specify which translations in the strings table correspond to which languages.
@function SetLanguageNames
@tparam tab table array-style table with language names
*/
	tableFlow.set_function(ScriptReserved_SetLanguageNames, &FlowHandler::SetLanguageNames, this);

/*** Do FlipMap with specific ID.
//@function FlipMap
//@tparam int flipmap (ID of flipmap)
*/
	tableFlow.set_function(ScriptReserved_FlipMap, &FlowHandler::FlipMap);

	ScriptColor::Register(parent);
	ScriptDisplaySprite::Register(*lua, parent);
	Rotation::Register(parent);
	Vec2::Register(parent);
	Vec3::Register(parent);
	Level::Register(tableFlow);
	SkyLayer::Register(tableFlow);
	Mirror::Register(tableFlow);
	InventoryItem::Register(tableFlow);
	Animations::Register(tableFlow);
	Settings::Register(tableFlow);
	Fog::Register(tableFlow);
	
	m_handler.MakeReadOnlyTable(tableFlow, ScriptReserved_WeatherType, WEATHER_TYPES);
	m_handler.MakeReadOnlyTable(tableFlow, ScriptReserved_LaraType, PLAYER_TYPES);
	m_handler.MakeReadOnlyTable(tableFlow, ScriptReserved_RotationAxis, ROTATION_AXES);
	m_handler.MakeReadOnlyTable(tableFlow, ScriptReserved_ItemAction, ITEM_MENU_ACTIONS);
	m_handler.MakeReadOnlyTable(tableFlow, ScriptReserved_ErrorMode, ERROR_MODES);
}

FlowHandler::~FlowHandler()
{
	for (auto& level : Levels)
		delete level;
}

std::string FlowHandler::GetGameDir()
{
	return m_gameDir;
}

void FlowHandler::SetGameDir(const std::string& assetDir)
{
	m_gameDir = assetDir;
}

void FlowHandler::SetLanguageNames(sol::as_table_t<std::vector<std::string>>&& src)
{
	m_languageNames = std::move(src);
}

void FlowHandler::SetStrings(sol::nested<std::unordered_map<std::string, std::vector<std::string>>>&& src)
{
	if (m_translationsMap.empty())
	{
		m_translationsMap = std::move(src);
	}
	else
	{
		for (auto& stringPair : src.value())
			m_translationsMap.insert_or_assign(stringPair.first, stringPair.second);
	}
}

void FlowHandler::SetSettings(Settings const & src)
{
	m_settings = src;
}

void FlowHandler::SetAnimations(Animations const& src)
{
	Anims = src;
}

void FlowHandler::AddLevel(Level const& level)
{
	Levels.push_back(new Level{ level });
}

void FlowHandler::SetIntroImagePath(const std::string& path)
{
	IntroImagePath = path;
}

void FlowHandler::SetTitleScreenImagePath(const std::string& path)
{
	TitleScreenImagePath = path;
}

void FlowHandler::SetTotalSecretCount(int secretsNumber)
{
	TotalNumberOfSecrets = secretsNumber;
}

void FlowHandler::LoadFlowScript()
{
	m_handler.ExecuteScript(m_gameDir + "Scripts/Gameflow.lua");
	m_handler.ExecuteScript(m_gameDir + "Scripts/SystemStrings.lua", true);
	m_handler.ExecuteScript(m_gameDir + "Scripts/Strings.lua", true);
	m_handler.ExecuteScript(m_gameDir + "Scripts/Settings.lua", true);

	SetScriptErrorMode(GetSettings()->ErrorMode);
	
	// Check if levels exist in Gameflow.lua.
	if (Levels.empty())
	{
		throw TENScriptException("No levels found. Check Gameflow.lua file integrity.");
	}
	else
	{
		TENLog("Level count: " + std::to_string(Levels.size()), LogLevel::Info);
	}
}

char const * FlowHandler::GetString(const char* id) const
{
	if (!ScriptAssert(m_translationsMap.find(id) != m_translationsMap.end(), std::string{ "Couldn't find string " } + id))
	{
		return "String not found.";
	}
	else
	{
		return m_translationsMap.at(std::string(id)).at(0).c_str();
	}
}

Settings* FlowHandler::GetSettings()
{
	return &m_settings;
}

Level* FlowHandler::GetLevel(int id)
{
	return Levels[id];
}

Level* FlowHandler::GetCurrentLevel()
{
	return Levels[CurrentLevel];
}

int	FlowHandler::GetNumLevels() const
{
	return (int)Levels.size();
}

int FlowHandler::GetLevelNumber(const std::string& fileName)
{
	if (fileName.empty())
		return -1;

	auto fileNameWithForwardSlashes = fileName;
	std::replace(fileNameWithForwardSlashes.begin(), fileNameWithForwardSlashes.end(), '\\', '/');

	auto requestedPath = std::filesystem::path{ fileName };
	bool isAbsolute = requestedPath.is_absolute();
	if (!isAbsolute)
		requestedPath = std::filesystem::path{ GetGameDir() + fileName };

	if (std::filesystem::is_regular_file(requestedPath))
	{
		auto lcFileName = TEN::Utils::ToLower(fileNameWithForwardSlashes);

		if (isAbsolute)
		{
			for (int i = 0; i < Levels.size(); i++)
			{
				auto lcFullLevelPathFromFlow = TEN::Utils::ToLower(GetGameDir() + GetLevel(i)->FileName);
				std::replace(lcFullLevelPathFromFlow.begin(), lcFullLevelPathFromFlow.end(), '\\', '/');

				if (lcFullLevelPathFromFlow == lcFileName)
					return i;
			}
		}
		else
		{
			for (int i = 0; i < Levels.size(); i++)
			{
				auto lcLevelNameFromFlow = TEN::Utils::ToLower(GetLevel(i)->FileName);
				std::replace(lcLevelNameFromFlow.begin(), lcLevelNameFromFlow.end(), '\\', '/');

				if (lcLevelNameFromFlow == lcFileName)
					return i;
			}
		}
	}
	else
	{
		TENLog("Provided -level arg \"" + fileName + "\" does not exist.");
		return -1;
	}

	TENLog("Provided -level arg \"" + fileName + "\" was not found in gameflow.lua.");
	return -1;
}

void FlowHandler::EndLevel(std::optional<int> nextLevel)
{
	int index = (nextLevel.has_value() && nextLevel.value() != 0) ? nextLevel.value() : CurrentLevel + 1;
	NextLevel = index;
}

void FlowHandler::FlipMap(int flipmap)
{
	DoFlipMap(flipmap);
}

void FlowHandler::SaveGame(int slot)
{
	SaveGame::Save(slot);
}

void FlowHandler::LoadGame(int slot)
{
	if (!SaveGame::DoesSaveGameExist(slot))
		return;

	NextLevel = -(slot + 1);
}

void FlowHandler::DeleteSaveGame(int slot)
{
	SaveGame::Delete(slot);
}

bool FlowHandler::DoesSaveGameExist(int slot)
{
	return SaveGame::DoesSaveGameExist(slot, true);
}

int FlowHandler::GetSecretCount() const
{
	return Statistics.Game.Secrets;
}

void FlowHandler::SetSecretCount(int secretsNum)
{
	if (secretsNum > UCHAR_MAX)
		return;

	Statistics.Game.Secrets = secretsNum;
}

void FlowHandler::AddSecret(int levelSecretIndex)
{
	static constexpr unsigned int maxSecretIndex = CHAR_BIT * sizeof(unsigned int);

	if (levelSecretIndex >= maxSecretIndex)
	{
		TENLog("Current maximum amount of secrets per level is " + std::to_string(maxSecretIndex) + ".", LogLevel::Warning);
		return;
	}

	if (Statistics.Level.Secrets & (1 << levelSecretIndex))
		return;

	if (Statistics.Game.Secrets >= UINT_MAX)
	{
		TENLog("Maximum amount of level secrets is already reached!", LogLevel::Warning);
		return;
	}

	PlaySecretTrack();
	Statistics.Level.Secrets |= (1 << levelSecretIndex);
	Statistics.Game.Secrets++;
}

bool FlowHandler::IsFlyCheatEnabled() const
{
	return FlyCheat;
}

void FlowHandler::EnableFlyCheat(bool flyCheat)
{
	FlyCheat = flyCheat;
}

bool FlowHandler::IsPointFilterEnabled() const
{
	return PointFilter;
}

void FlowHandler::EnablePointFilter(bool pointFilter)
{
	PointFilter = pointFilter;
}

bool FlowHandler::IsMassPickupEnabled() const
{
	return MassPickup;
}

void FlowHandler::EnableMassPickup(bool massPickup)
{
	MassPickup = massPickup;
}

bool FlowHandler::IsLaraInTitleEnabled() const
{
	return LaraInTitle;
}

void FlowHandler::EnableLaraInTitle(bool laraInTitle)
{
	LaraInTitle = laraInTitle;
}

void FlowHandler::EnableLevelSelect(bool levelSelect)
{
	LevelSelect = levelSelect;
}

bool FlowHandler::IsLoadSaveEnabled() const
{
	return LoadSave;
}

void FlowHandler::EnableLoadSave(bool loadSave)
{
	LoadSave = loadSave;
}

bool FlowHandler::DoFlow()
{
	// We start with the title level, if no other index is specified
	if (CurrentLevel == -1)
		CurrentLevel = SystemNameHash = 0;

	SelectedLevelForNewGame = 0;
	SelectedSaveGame = 0;
	SaveGameHeader header;

	// We loop indefinitely, looking for return values of DoLevel
	bool loadFromSavegame = false;

	while (DoTheGame)
	{
		// Check if called level exists in script.
		if (CurrentLevel >= Levels.size())
		{
			TENLog("Level not found. Check Gameflow.lua file integrity.", LogLevel::Error, LogConfig::All);
			CurrentLevel = 0;
		}
		
		// First we need to fill some legacy variables in PCTomb5.exe
		Level* level = Levels[CurrentLevel];

		GameStatus status;

		if (CurrentLevel == 0)
		{
			try
			{
				status = DoLevel(CurrentLevel);
			}
			catch (TENScriptException const& e)
			{
				std::string msg = std::string{ "A Lua error occurred while running the title level; " } + __func__ + ": " + e.what();
				TENLog(msg, LogLevel::Error, LogConfig::All);
				ShutdownTENLog();
				throw;
			}
		}
		else
		{
			// Prepare inventory objects table
			for (size_t i = 0; i < level->InventoryObjects.size(); i++)
			{
				InventoryItem* obj = &level->InventoryObjects[i];
				if (obj->ObjectID >= 0 && obj->ObjectID < INVENTORY_TABLE_SIZE)
				{
					InventoryObject* invObj = &InventoryObjectTable[obj->ObjectID];

					invObj->ObjectName = obj->Name.c_str();
					invObj->Scale1 = obj->Scale;
					invObj->YOffset = obj->YOffset;
					invObj->Orientation = EulerAngles(ANGLE(obj->Rot.x), ANGLE(obj->Rot.y), ANGLE(obj->Rot.z));
					invObj->MeshBits = obj->MeshBits;
					invObj->Options = obj->MenuAction;
					invObj->RotFlags = obj->RotFlags;
				}
			}

			try
			{
				status = DoLevel(CurrentLevel, loadFromSavegame);
			}
			catch (TENScriptException const& e) 
			{
				std::string msg = std::string{ "A Lua error occurred while running a level; " } + __func__ + ": " + e.what();
				TENLog(msg, LogLevel::Error, LogConfig::All);
				status = GameStatus::ExitToTitle;
			}

			loadFromSavegame = false;
		}

		switch (status)
		{
		case GameStatus::ExitGame:
			DoTheGame = false;
			break;

		case GameStatus::ExitToTitle:
		case GameStatus::LaraDead:
			CurrentLevel = 0;
			break;

		case GameStatus::NewGame:
			CurrentLevel = (SelectedLevelForNewGame != 0 ? SelectedLevelForNewGame : 1);
			SelectedLevelForNewGame = 0;
			InitializeGame = true;
			break;

		case GameStatus::LoadGame:
			// Load the header of the savegame for getting the level to load
			SaveGame::LoadHeader(SelectedSaveGame, &header);

			// Load level
			CurrentLevel = header.Level;
			NextLevel = 0;
			GameTimer = header.Timer;
			loadFromSavegame = true;
			break;

		case GameStatus::LevelComplete:
			if (NextLevel >= Levels.size())
			{
				CurrentLevel = 0; // TODO: final credits
			}
			else
			{
				CurrentLevel = NextLevel;
			}

			NextLevel = 0;
			break;
		}
	}

	g_GameScript->ResetScripts(true);
	return true;
}

bool FlowHandler::IsLevelSelectEnabled() const
{
	return LevelSelect;
}
