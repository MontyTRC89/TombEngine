#include "framework.h"
#include "Game/control/control.h"

#include <process.h>
#include <filesystem>

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/flipeffect.h"
#include "Game/control/lot.h"
#include "Game/control/volume.h"
#include "Game/effects/debris.h"
#include "Game/effects/Blood.h"
#include "Game/effects/Bubble.h"
#include "Game/effects/DisplaySprite.h"
#include "Game/effects/Drip.h"
#include "Game/effects/effects.h"
#include "Game/effects/Electricity.h"
#include "Game/effects/explosion.h"
#include "Game/effects/Footprint.h"
#include "Game/effects/Hair.h"
#include "Game/effects/Ripple.h"
#include "Game/effects/simple_particle.h"
#include "Game/effects/smoke.h"
#include "Game/effects/spark.h"
#include "Game/effects/Streamer.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/Gui.h"
#include "Game/Hud/Hud.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_cheat.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/items.h"
#include "Game/pickup/pickup.h"
#include "Game/room.h"
#include "Game/savegame.h"
#include "Game/Setup.h"
#include "Game/spotcam.h"
#include "Math/Math.h"
#include "Objects/Effects/LensFlare.h"
#include "Objects/Effects/tr4_locusts.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/Object/rope.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Objects/TR3/Entity/FishSwarm.h"
#include "Objects/TR4/Entity/tr4_beetle_swarm.h"
#include "Objects/TR5/Emitter/tr5_bats_emitter.h"
#include "Objects/TR5/Emitter/tr5_rats_emitter.h"
#include "Objects/TR5/Emitter/tr5_spider_emitter.h"
#include "Objects/TR5/Trap/LaserBarrier.h"
#include "Objects/TR5/Trap/LaserBeam.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Scripting/Include/Objects/ScriptInterfaceObjectsHandler.h"
#include "Scripting/Include/ScriptInterfaceGame.h"
#include "Scripting/Include/Strings/ScriptInterfaceStringsHandler.h"
#include "Sound/sound.h"
#include "Specific/clock.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/winmain.h"

using namespace std::chrono;
using namespace TEN::Effects;
using namespace TEN::Effects::Blood;
using namespace TEN::Effects::Bubble;
using namespace TEN::Effects::DisplaySprite;
using namespace TEN::Effects::Drip;
using namespace TEN::Effects::Electricity;
using namespace TEN::Effects::Environment;
using namespace TEN::Effects::Explosion;
using namespace TEN::Effects::Footprint;
using namespace TEN::Effects::Hair;
using namespace TEN::Effects::Ripple;
using namespace TEN::Effects::Smoke;
using namespace TEN::Effects::Spark;
using namespace TEN::Effects::Streamer;
using namespace TEN::Entities::Creatures::TR3;
using namespace TEN::Entities::Generic;
using namespace TEN::Entities::Switches;
using namespace TEN::Entities::Traps;
using namespace TEN::Entities::TR4;
using namespace TEN::Collision::Floordata;
using namespace TEN::Control::Volumes;
using namespace TEN::Hud;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Renderer;
using namespace TEN::Entities::Creatures::TR3;
using namespace TEN::Entities::Effects;

constexpr auto DEATH_NO_INPUT_TIMEOUT = 10 * FPS;
constexpr auto DEATH_INPUT_TIMEOUT	  = 3 * FPS;

int GameTimer       = 0;
int GlobalCounter   = 0;

bool InitializeGame;
bool DoTheGame;
bool JustLoaded;
bool ThreadEnded;

int RequiredStartPos;
int CurrentLevel;
int NextLevel;

bool  InItemControlLoop;
short ItemNewRoomNo;
short ItemNewRooms[MAX_ROOMS];
short NextItemActive;
short NextItemFree;
short NextFxActive;
short NextFxFree;

int ControlPhaseTime;

void DrawPhase(bool isTitle, float interpolationFactor)
{
	if (isTitle)
	{
		g_Renderer.RenderTitle(interpolationFactor);
	}
	else
	{
		g_Renderer.Render(interpolationFactor);
	}

	g_Renderer.Lock();
}

GameStatus ControlPhase(bool insideMenu)
{
	auto time1 = std::chrono::high_resolution_clock::now();
	bool isTitle = (CurrentLevel == 0);

	g_Renderer.PrepareScene();
	g_Renderer.SaveOldState();

	ClearFires();
	ClearLensFlares();
	ClearAllDisplaySprites();

	SetupInterpolation();
	PrepareCamera();

	RegeneratePickups();

	g_GameStringsHandler->ProcessDisplayStrings(DELTA_TIME);

	// Controls are polled before OnLoop to allow input data to be overwritten by script API methods.
	HandleControls(isTitle);

	// Pre-loop script and event handling.
	g_GameScript->OnLoop(DELTA_TIME, false); // TODO: Don't use DELTA_TIME constant with high framerate.
	HandleAllGlobalEvents(EventType::Loop, (Activator)LaraItem->Index);

	// Queued input actions are read again after OnLoop, so that remaining control loop can immediately register
	// emulated keypresses from the script.
	ApplyActionQueue();

	// Control lock is processed after handling scripts because builder may want to process input externally
	// while locking player from input.
	if (!isTitle && Lara.Control.IsLocked)
		ClearAllActions();

	// Item update should happen before camera update, so potential flyby/track camera triggers
	// are processed correctly.
	UpdateAllItems();
	UpdateAllEffects();
	UpdateLara(LaraItem, isTitle);
	g_GameScriptEntities->TestCollidingObjects();

	// Smash shatters and clear stopper flags under them.
	UpdateShatters();

	// Clear last selected item in inventory (must be after on loop event handling, so they can detect that).
	g_Gui.CancelInventorySelection();

	// Update weather.
	Weather.Update();

	// Update effects.
	StreamerEffect.Update();
	UpdateWibble();
	UpdateSparks();
	UpdateFireSparks();
	UpdateSmoke();
	UpdateBlood();
	UpdateBubbles();
	UpdateDebris();
	UpdateGunShells();
	UpdateFootprints();
	UpdateSplashes();
	UpdateElectricityArcs();
	UpdateHelicalLasers();
	UpdateDrips();
	UpdateRats();
	UpdateRipples();
	UpdateBats();
	UpdateSpiders();
	UpdateSparkParticles();
	UpdateSmokeParticles();
	UpdateSimpleParticles();
	UpdateExplosionParticles();
	UpdateShockwaves();
	UpdateBeetleSwarm();
	UpdateLocusts();
	UpdateUnderwaterBloodParticles();
	UpdateFishSwarm();
	UpdateGlobalLensFlare();

	// Update HUD.
	g_Hud.Update(*LaraItem);
	UpdateFadeScreenAndCinematicBars();

	// Rumble screen (like in submarine level of TRC).
	if (g_GameFlow->GetLevel(CurrentLevel)->Rumble)
		RumbleScreen();

	DoFlipEffect(FlipEffect, LaraItem);

	PlaySoundSources();
	Sound_UpdateScene();

	// Handle inventory, pause, load, save screens.
	if (!insideMenu)
	{
		auto result = HandleMenuCalls(isTitle);
		if (result != GameStatus::Normal)
			return result;

		// Handle global input events.
		result = HandleGlobalInputEvents(isTitle);
		if (result != GameStatus::Normal)
			return result;
	}

	UpdateCamera();

	// Post-loop script and event handling.
	g_GameScript->OnLoop(DELTA_TIME, true);

	// Clear savegame loaded flag.
	JustLoaded = false;

	// Update timers.
	GameTimer++;
	GlobalCounter++;

	auto time2 = std::chrono::high_resolution_clock::now();
	ControlPhaseTime = (std::chrono::duration_cast<std::chrono::nanoseconds>(time2 - time1)).count() / 1000000;

	return GameStatus::Normal;
}

unsigned CALLBACK GameMain(void *)
{
	TENLog("Starting GameMain()...", LogLevel::Info);

	TimeInit();

	// Do fixed-time title image.
	if (g_GameFlow->IntroImagePath.empty())
	{
		TENLog("Intro image path not set.", LogLevel::Warning);
	}
	else
	{
		g_Renderer.RenderTitleImage();
	}

	// Execute Lua gameflow and play game.
	g_GameFlow->DoFlow();

	DoTheGame = false;

	// Finish thread.
	PostMessage(WindowsHandle, WM_CLOSE, NULL, NULL);
	EndThread();

	return true;
}

GameStatus DoLevel(int levelIndex, bool loadGame)
{
	bool isTitle = !levelIndex;
	auto loadType = loadGame ? LevelLoadType::Load : (SaveGame::IsOnHub(levelIndex) ? LevelLoadType::Hub : LevelLoadType::New);

	TENLog(isTitle ? "DoTitle" : "DoLevel", LogLevel::Info);

	// Load level. Fall back to title if unsuccessful.
	if (!LoadLevelFile(levelIndex))
		return (isTitle ? GameStatus::ExitGame : GameStatus::ExitToTitle);

	// Initialize items, effects, lots, and cameras.
	HairEffect.Initialize();
	InitializeFXArray();
	InitializeCamera();
	InitializeSpotCamSequences(isTitle);
	InitializeItemBoxData();

	// Initialize scripting.
	InitializeScripting(levelIndex, loadType);
	InitializeNodeScripts();

	// Initialize menu and inventory state.
	g_Gui.Initialize();

	// Initialize game variables and optionally load game.
	InitializeOrLoadGame(loadGame);

	// DoGameLoop() returns only when level has ended.
	return DoGameLoop(levelIndex);
}

void UpdateShatters()
{
	if (!SmashedMeshCount)
		return;

	do
	{
		SmashedMeshCount--;

		auto* floor = GetFloor(
			SmashedMesh[SmashedMeshCount]->pos.Position.x,
			SmashedMesh[SmashedMeshCount]->pos.Position.y,
			SmashedMesh[SmashedMeshCount]->pos.Position.z,
			&SmashedMeshRoom[SmashedMeshCount]);

		TestTriggers(SmashedMesh[SmashedMeshCount]->pos.Position.x,
			SmashedMesh[SmashedMeshCount]->pos.Position.y,
			SmashedMesh[SmashedMeshCount]->pos.Position.z,
			SmashedMeshRoom[SmashedMeshCount], true);

		TestVolumes(SmashedMeshRoom[SmashedMeshCount], SmashedMesh[SmashedMeshCount]);

		floor->Stopper = false;
		SmashedMesh[SmashedMeshCount] = 0;
	}
	while (SmashedMeshCount != 0);
}

void KillMoveItems()
{
	if (ItemNewRoomNo > 0)
	{
		for (int i = 0; i < ItemNewRoomNo; i++)
		{
			int itemNumber = ItemNewRooms[i * 2];
			if (itemNumber >= 0)
			{
				ItemNewRoom(itemNumber, ItemNewRooms[(i * 2) + 1]);
			}
			else
			{
				KillItem(itemNumber & 0x7FFF);
			}
		}
	}

	ItemNewRoomNo = 0;
}

void KillMoveEffects()
{
	if (ItemNewRoomNo > 0)
	{
		for (int i = 0; i < ItemNewRoomNo; i++)
		{
			int itemNumber = ItemNewRooms[i * 2];
			if (itemNumber >= 0)
			{
				EffectNewRoom(itemNumber, ItemNewRooms[(i * 2) + 1]);
			}
			else
			{
				KillEffect(itemNumber & 0x7FFF);
			}
		}
	}

	ItemNewRoomNo = 0;
}

// NOTE: No one should use this ever again.
int GetRandomControl()
{
	return Random::GenerateInt();
}

int GetRandomDraw()
{
	return Random::GenerateInt();
}

void CleanUp()
{
	// Reset oscillator seed.
	Wibble = 0;

	// Clear player lock, otherwise controls will lock if user exits to title while playing flyby with locked controls.
	Lara.Control.IsLocked = false;

	// Resets lightning and wind parameters to avoid holding over previous weather to new level.
	Weather.Clear();

	// Clear creatures, otherwise list of active creatures from previous level will spill into new level.
	ActiveCreatures.clear();

	// Clear ropes.
	Ropes.clear();

	// Clear camera data.
	ClearSpotCamSequences();
	ClearCinematicBars();

	// Clear effects.
	StreamerEffect.Clear();
	ClearUnderwaterBloodParticles();
	ClearBubbles();
	ClearAllDisplaySprites();
	ClearFootprints();
	ClearDrips();
	ClearRipples();
	ClearLaserBarrierEffects();
	ClearLaserBeamEffects();
	DisableSmokeParticles();
	DisableSparkParticles();
	DisableDebris();

	// Clear swarm enemies.
	ClearSwarmEnemies(nullptr);

	// Clear HUD.
	g_Hud.Clear();

	// Clear soundtrack masks.
	ClearSoundTrackMasks();

	// Clear all remaining renderer data.
	g_Renderer.ClearScene();
	g_Renderer.SetPostProcessMode(PostProcessMode::None);
	g_Renderer.SetPostProcessStrength(1.0f);
	g_Renderer.SetPostProcessTint(Vector3::One);

	// Reset Itemcamera
	ClearObjCamera();
}

void InitializeScripting(int levelIndex, LevelLoadType type)
{
	TENLog("Loading level script...", LogLevel::Info);

	g_GameStringsHandler->ClearDisplayStrings();
	g_GameScript->ResetScripts(!levelIndex || type != LevelLoadType::New);

	const auto& level = *g_GameFlow->GetLevel(levelIndex);

	// Run level script if it exists.
	if (!level.ScriptFileName.empty())
	{
		auto levelScriptName = g_GameFlow->GetGameDir() + level.ScriptFileName;
		if (std::filesystem::is_regular_file(levelScriptName))
		{
			g_GameScript->ExecuteScriptFile(levelScriptName);
		}
		else
		{
			TENLog("Level script not found: " + levelScriptName, LogLevel::Warning);
		}

		g_GameScript->InitCallbacks();
		g_GameStringsHandler->SetCallbackDrawString([](const std::string& key, D3DCOLOR color, const Vec2& pos, float scale, int flags)
		{
			g_Renderer.AddString(
				key,
				Vector2(
					(pos.x / g_Configuration.ScreenWidth) * DISPLAY_SPACE_RES.x,
					(pos.y / g_Configuration.ScreenHeight) * DISPLAY_SPACE_RES.y),
				Color(color), scale, flags);
		});
	}

	// Play default background music.
	if (type != LevelLoadType::Load)
		PlaySoundTrack(level.GetAmbientTrack(), SoundTrackType::BGM);
}

void DeInitializeScripting(int levelIndex, GameStatus reason)
{
	g_GameScript->OnEnd(reason);
	HandleAllGlobalEvents(EventType::End, (Activator)LaraItem->Index);

	g_GameScript->FreeLevelScripts();
	g_GameScriptEntities->FreeEntities();

	if (levelIndex == 0)
		g_GameScript->ResetScripts(true);
}

void InitializeOrLoadGame(bool loadGame)
{
	g_Gui.SetInventoryItemChosen(NO_VALUE);
	g_Gui.SetEnterInventory(NO_VALUE);

	// Restore game?
	if (loadGame && SaveGame::Load(g_GameFlow->SelectedSaveGame))
	{
		InitializeGame = false;

		g_GameFlow->SelectedSaveGame = 0;
		g_GameScript->OnLoad();
		HandleAllGlobalEvents(EventType::Load, (Activator)LaraItem->Index);
	}
	else
	{
		// If not loading savegame, clear all info.
		SaveGame::Statistics.Level = {};

		if (InitializeGame)
		{
			// Clear all game info as well.
			SaveGame::Statistics.Game = {};
			GameTimer = 0;
			InitializeGame = false;

			SaveGame::ResetHub();
			TENLog("Starting new game.", LogLevel::Info);
		}
		else
		{
			SaveGame::LoadHub(CurrentLevel);
			TENLog("Starting new level.", LogLevel::Info);
		}

		g_GameScript->OnStart();
		HandleAllGlobalEvents(EventType::Start, (Activator)LaraItem->Index);
	}
}

GameStatus DoGameLoop(int levelIndex)
{
	int frameCount = LOOP_FRAME_COUNT;
	auto& status = g_GameFlow->LastGameStatus;

	// Before entering actual game loop, ControlPhase() must be
	// called once to sort out various runtime shenanigangs (e.g. hair).
	status = ControlPhase(false);

	g_Synchronizer.Init();
	bool legacy30FpsDoneDraw = false;

	while (DoTheGame)
	{
		g_Synchronizer.Sync();

		while (g_Synchronizer.Synced())
		{
			status = ControlPhase(false);
			g_Synchronizer.Step();

			legacy30FpsDoneDraw = false;
		}

		if (status != GameStatus::Normal)
			break;

		if (!g_Configuration.EnableHighFramerate)
		{
			if (!legacy30FpsDoneDraw)
			{
				DrawPhase(!levelIndex, 0.0f);
				legacy30FpsDoneDraw = true;
			}
		}
		else
		{
			DrawPhase(!levelIndex, g_Synchronizer.GetInterpolationFactor());
		}
	}

	EndGameLoop(levelIndex, status);

	return status;
}

void EndGameLoop(int levelIndex, GameStatus reason)
{
	SaveGame::SaveHub(levelIndex);
	DeInitializeScripting(levelIndex, reason);

	StopAllSounds();
	StopSoundTracks(SOUND_XFADETIME_LEVELJUMP, true);
	StopRumble();
}

void SetupInterpolation()
{
	for (auto& item : g_Level.Items)
		item.DisableInterpolation = false;
}

void HandleControls(bool isTitle)
{
	// Poll input devices and update input variables.
	// TODO: To allow cutscene skipping later, don't clear Deselect action.
	UpdateInputActions(LaraItem, true);

	if (isTitle)
		ClearAction(In::Look);
}

GameStatus HandleMenuCalls(bool isTitle)
{
	auto gameStatus = GameStatus::Normal;

	if (ScreenFading)
		return gameStatus;

	if (isTitle)
	{
		auto invStatus = g_Gui.TitleOptions(LaraItem);

		switch (invStatus)
		{
		case InventoryResult::NewGame:
		case InventoryResult::NewGameSelectedLevel:
			return GameStatus::NewGame;

		case InventoryResult::HomeLevel:
			return GameStatus::HomeLevel;
			break;

		case InventoryResult::LoadGame:
			return GameStatus::LoadGame;

		case InventoryResult::ExitGame:
			return GameStatus::ExitGame;
		}

		return gameStatus;
	}

	bool playerAlive = LaraItem->HitPoints > 0;
	
	bool doLoad      = IsClicked(In::Load) || 
					   (!IsClicked(In::Inventory) && !NoAction() && SaveGame::IsLoadGamePossible() && Lara.Control.Count.Death > DEATH_INPUT_TIMEOUT);
	bool doSave      = IsClicked(In::Save) && playerAlive;
	bool doPause     = IsClicked(In::Pause) && playerAlive;
	bool doInventory = (IsClicked(In::Inventory) || g_Gui.GetEnterInventory() != NO_VALUE) && playerAlive;

	// Handle inventory.
	if (doSave && g_GameFlow->IsLoadSaveEnabled() && g_Gui.GetInventoryMode() != InventoryMode::Save)
	{
		SaveGame::LoadHeaders();
		g_Gui.SetInventoryMode(InventoryMode::Save);
		g_Gui.CallInventory(LaraItem, false);
	}
	else if (doLoad && g_GameFlow->IsLoadSaveEnabled() && g_Gui.GetInventoryMode() != InventoryMode::Load)
	{
		SaveGame::LoadHeaders();

		g_Gui.SetInventoryMode(InventoryMode::Load);

		if (g_Gui.CallInventory(LaraItem, false))
			gameStatus = GameStatus::LoadGame;
	}
	else if (doPause && g_Gui.GetInventoryMode() != InventoryMode::Pause)
	{
		if (g_Gui.CallPause())
			gameStatus = GameStatus::ExitToTitle;
	}
	else if (doInventory && LaraItem->HitPoints > 0 && !Lara.Control.Look.IsUsingBinoculars)
	{
		if (g_Gui.CallInventory(LaraItem, true))
			gameStatus = GameStatus::LoadGame;
	}

	if (gameStatus != GameStatus::Normal)
	{
		StopAllSounds();
		StopRumble();
	}

	return gameStatus;
}

GameStatus HandleGlobalInputEvents(bool isTitle)
{
	if (isTitle)
		return GameStatus::Normal;

	// Check if player dead.
	if (Lara.Control.Count.Death > DEATH_NO_INPUT_TIMEOUT ||
		Lara.Control.Count.Death > DEATH_INPUT_TIMEOUT && !NoAction())
	{
		// TODO: Maybe do game over menu like some PSX versions have?
		return GameStatus::LaraDead;
	}

	// Check if level has been completed.
	// Negative NextLevel indicates that a savegame must be loaded from corresponding slot.
	if (NextLevel > 0)
	{
		return GameStatus::LevelComplete;
	}
	else if (NextLevel < 0)
	{
		g_GameFlow->SelectedSaveGame = -(NextLevel + 1);
		return GameStatus::LoadGame;
	}

	return GameStatus::Normal;
}
