#include "framework.h"
#include <filesystem>

#include "FlowHandler.h"
#include "ReservedScriptNames.h"
#include "Sound/sound.h"
#include "Game/savegame.h"
#include "Flow/InventoryItem/InventoryItem.h"
#include "Game/Gui.h"
#include "Logic/LevelFunc.h"
#include "Vec3/Vec3.h"
#include "Objects/ScriptInterfaceObjectsHandler.h"
#include "Strings/ScriptInterfaceStringsHandler.h"
#include "Specific/trutils.h"

/***
Functions that (mostly) don't directly impact in-game mechanics. Used for setup
in gameflow.lua, settings.lua and strings.lua; some can be used in level
scripts too.
@tentable Flow 
@pragma nostrip
*/

using std::string;
using std::vector;
using std::unordered_map;

ScriptInterfaceGame* g_GameScript;
ScriptInterfaceObjectsHandler* g_GameScriptEntities;
ScriptInterfaceStringsHandler* g_GameStringsHandler;
ScriptInterfaceFlowHandler* g_GameFlow;

FlowHandler::FlowHandler(sol::state* lua, sol::table & parent) : m_handler{ lua }
{

/*** gameflow.lua.
These functions are called in gameflow.lua, a file loosely equivalent to winroomedit's SCRIPT.DAT.
They handle a game's 'metadata'; i.e., things such as level titles, loading screen paths, and default
ambient tracks.
@section Flowlua
*/
	sol::table table_flow{ m_handler.GetState()->lua_state(), sol::create };
	parent.set(ScriptReserved_Flow, table_flow);

/***
Add a level to the Flow.
@function AddLevel
@tparam Flow.Level level a level object
*/
	table_flow.set_function(ScriptReserved_AddLevel, &FlowHandler::AddLevel, this);

/*** Image to show when loading the game.
Must be a .jpg or .png image.
@function SetIntroImagePath
@tparam string path the path to the image, relative to the TombEngine exe
*/
	table_flow.set_function(ScriptReserved_SetIntroImagePath, &FlowHandler::SetIntroImagePath, this);

/*** Image to show in the background of the title screen.
Must be a .jpg or .png image.
__(not yet implemented)__
@function SetTitleScreenImagePath
@tparam string path the path to the image, relative to the TombEngine exe
*/
	table_flow.set_function(ScriptReserved_SetTitleScreenImagePath, &FlowHandler::SetTitleScreenImagePath, this);

/*** Enable or disable Lara drawing in title flyby.
Must be true or false
@function EnableLaraInTitle
@tparam bool enabled true or false
*/
	table_flow.set_function(ScriptReserved_EnableLaraInTitle, &FlowHandler::EnableLaraInTitle, this);

/*** Enable or disable level selection in title flyby.
Must be true or false
@function EnableLevelSelect
@tparam bool enabled true or false
*/
	table_flow.set_function(ScriptReserved_EnableLevelSelect, &FlowHandler::EnableLevelSelect, this);

/*** gameflow.lua or level scripts.
@section FlowluaOrScripts
*/

/*** Enable or disable DOZY mode (fly cheat).
Must be true or false
@function EnableFlyCheat
@tparam bool enabled true or false
*/
	table_flow.set_function(ScriptReserved_EnableFlyCheat, &FlowHandler::EnableFlyCheat, this);

/*** Enable or disable mass pickup.
Must be true or false
@function EnableMassPickup
@tparam bool enabled true or false
*/
	table_flow.set_function(ScriptReserved_EnableMassPickup, &FlowHandler::EnableMassPickup, this);

/*** Returns the level by index.
Indices depend on the order in which AddLevel was called; the first added will
have an ID of 0, the second an ID of 1, and so on.
@function GetLevel
@tparam int index of the level
@treturn Flow.Level the level indicated by the id
*/
	table_flow.set_function(ScriptReserved_GetLevel, &FlowHandler::GetLevel, this);

/*** Returns the level that the game control is running in that moment.
@function GetCurrentLevel
@treturn Flow.Level the current level
*/
	table_flow.set_function(ScriptReserved_GetCurrentLevel, &FlowHandler::GetCurrentLevel, this);

/***
Finishes the current level, with optional level index provided. If level index
is not provided or is zero, jumps to next level. If level index is more than
level count, jumps to title.
@function EndLevel
@int[opt] index level index (default 0)
*/
	table_flow.set_function(ScriptReserved_EndLevel, &FlowHandler::EndLevel, this);

/***
Returns the player's current per-game secret count.
@function GetSecretCount
@treturn int Current game secret count.
*/
	table_flow.set_function(ScriptReserved_GetSecretCount, &FlowHandler::GetSecretCount, this);

/*** 
Sets the player's current per-game secret count.
@function SetSecretCount
@tparam int count new secret count.
*/
	table_flow.set_function(ScriptReserved_SetSecretCount, &FlowHandler::SetSecretCount, this);

/***
Adds one secret to current level secret count and also plays secret music track.
The index argument corresponds to the secret's unique ID, the same that would go in a secret trigger's Param.
@function AddSecret
@tparam int index an index of current level's secret (must be from 0 to 31).
*/
	table_flow.set_function(ScriptReserved_AddSecret, &FlowHandler::AddSecret, this);

/*** Total number of secrets in game.
Must be an integer value (0 means no secrets).
@function SetTotalSecretCount
@tparam int total number of secrets
*/
	table_flow.set_function(ScriptReserved_SetTotalSecretCount, &FlowHandler::SetTotalSecretCount, this);



/*** settings.lua.
These functions are called in settings.lua, a file which holds your local settings.
settings.lua shouldn't be bundled with any finished levels/games.
@section settingslua
*/
/***
@function SetSettings
@tparam Flow.Settings settings a settings object 
*/
	table_flow.set_function(ScriptReserved_SetSettings, &FlowHandler::SetSettings, this);

/***
@function SetAnimations
@tparam Flow.Animations animations an animations object 
*/
	table_flow.set_function(ScriptReserved_SetAnimations, &FlowHandler::SetAnimations, this);

/*** strings.lua. 
These functions used in strings.lua, which is generated by TombIDE.
You will not need to call them manually.
@section stringslua
*/
/*** Set string variable keys and their translations.
@function SetStrings
@tparam tab table array-style table with strings
*/
	table_flow.set_function(ScriptReserved_SetStrings, &FlowHandler::SetStrings, this);

/*** Get translated string
@function GetString
@tparam key string key for translated string 
*/
	table_flow.set_function(ScriptReserved_GetString, &FlowHandler::GetString, this);

/*** Set language names for translations.
Specify which translations in the strings table correspond to which languages.
@function SetLanguageNames
@tparam tab table array-style table with language names
*/
	table_flow.set_function(ScriptReserved_SetLanguageNames, &FlowHandler::SetLanguageNames, this);

	ScriptColor::Register(parent);
	Rotation::Register(parent);
	Vec3::Register(parent);
	Level::Register(table_flow);
	SkyLayer::Register(table_flow);
	Mirror::Register(table_flow);
	InventoryItem::Register(table_flow);
	Animations::Register(table_flow);
	Settings::Register(table_flow);
	Fog::Register(table_flow);
	
	m_handler.MakeReadOnlyTable(table_flow, ScriptReserved_WeatherType, kWeatherTypes);
	m_handler.MakeReadOnlyTable(table_flow, ScriptReserved_LaraType, kLaraTypes);
	m_handler.MakeReadOnlyTable(table_flow, ScriptReserved_RotationAxis, kRotAxes);
	m_handler.MakeReadOnlyTable(table_flow, ScriptReserved_ItemAction, kItemActions);
	m_handler.MakeReadOnlyTable(table_flow, ScriptReserved_ErrorMode, kErrorModes);
}

FlowHandler::~FlowHandler()
{
	for (auto& lev : Levels)
		delete lev;
}

void FlowHandler::SetLanguageNames(sol::as_table_t<std::vector<std::string>> && src)
{
	m_languageNames = std::move(src);
}

void FlowHandler::SetStrings(sol::nested<std::unordered_map<std::string, std::vector<std::string>>> && src)
{
	m_translationsMap = std::move(src);
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

void FlowHandler::SetIntroImagePath(std::string const& path)
{
	IntroImagePath = path;
}

void FlowHandler::SetTitleScreenImagePath(std::string const& path)
{
	TitleScreenImagePath = path;
}

void FlowHandler::SetTotalSecretCount(int secretsNumber)
{
	TotalNumberOfSecrets = secretsNumber;
}

void FlowHandler::LoadFlowScript()
{
	m_handler.ExecuteScript("Scripts/Gameflow.lua");
	m_handler.ExecuteScript("Scripts/Strings.lua");
	m_handler.ExecuteScript("Scripts/Settings.lua");

	SetScriptErrorMode(GetSettings()->ErrorMode);
}

char const * FlowHandler::GetString(const char* id) const
{
	if (!ScriptAssert(m_translationsMap.find(id) != m_translationsMap.end(), std::string{ "Couldn't find string " } + id))
		return "String not found.";
	else
		return m_translationsMap.at(string(id)).at(0).c_str();
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

int FlowHandler::GetLevelNumber(std::string const& fileName)
{
	if (fileName.empty())
		return -1;

	auto lcFilename = TEN::Utils::ToLower(fileName);

	for (int i = 0; i < Levels.size(); i++)
	{
		auto level = TEN::Utils::ToLower(this->GetLevel(i)->FileName);
		if (level == lcFilename && std::filesystem::exists(fileName))
			return i;
	}

	TENLog("Specified level filename was not found in script. Level won't be loaded. Please edit level filename in gameflow.lua.");
	return -1;
}

void FlowHandler::EndLevel(std::optional<int> nextLevel)
{
	int index = (nextLevel.has_value() && nextLevel.value() != 0) ? nextLevel.value() : CurrentLevel + 1;
	LevelComplete = index;
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
				if (obj->slot >= 0 && obj->slot < INVENTORY_TABLE_SIZE)
				{
					InventoryObject* invObj = &InventoryObjectTable[obj->slot];

					invObj->ObjectName = obj->name.c_str();
					invObj->Scale1 = obj->scale;
					invObj->YOffset = obj->yOffset;
					invObj->Orientation.x = ANGLE(obj->rot.x);
					invObj->Orientation.y = ANGLE(obj->rot.y);
					invObj->Orientation.z = ANGLE(obj->rot.z);
					invObj->MeshBits = obj->meshBits;
					invObj->Options = obj->action;
					invObj->RotFlags = obj->rotationFlags;
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
			CurrentLevel = 0;
			break;
		case GameStatus::NewGame:
			CurrentLevel = (SelectedLevelForNewGame != 0 ? SelectedLevelForNewGame : 1);
			SelectedLevelForNewGame = 0;
			InitialiseGame = true;
			break;
		case GameStatus::LoadGame:
			// Load the header of the savegame for getting the level to load
			SaveGame::LoadHeader(SelectedSaveGame, &header);

			// Load level
			CurrentLevel = header.Level;
			GameTimer = header.Timer;
			loadFromSavegame = true;

			break;
		case GameStatus::LevelComplete:
			if (LevelComplete >= Levels.size())
				CurrentLevel = 0; // TODO: final credits
			else
				CurrentLevel = LevelComplete;
			LevelComplete = 0;
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

