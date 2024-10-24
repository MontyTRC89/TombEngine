#include "Game/control/control.h"
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
#include "Game/Lara/lara_initialise.h"

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
using namespace TEN::Renderer;

int GameTimer       = 0;
int GlobalCounter   = 0;

bool InitializeGame;
bool DoTheGame;
bool JustLoaded;
bool ThreadEnded;

int RequiredStartPos;
int CurrentLevel;
int NextLevel;

int SystemNameHash = 0;

bool  InItemControlLoop;
short ItemNewRoomNo;
short ItemNewRooms[MAX_ROOMS];
short NextItemActive;
short NextItemFree;
short NextFxActive;
short NextFxFree;

int ControlPhaseTime;

int DrawPhase(bool isTitle)
{
	if (isTitle)
	{
		g_Renderer.RenderTitle();
	}
	else
	{
		g_Renderer.Render();
	}

	// Clear display sprites.
	ClearDisplaySprites();

	Camera.numberFrames = g_Renderer.Synchronize();
	return Camera.numberFrames;
}

GameStatus ControlPhase(int numFrames)
{
	auto time1 = std::chrono::high_resolution_clock::now();

	bool isTitle = (CurrentLevel == 0);

	RegeneratePickups();

	numFrames = std::clamp(numFrames, 0, 10);

	if (TrackCameraInit)
	{
		UseSpotCam = false;
		AlterFOV(LastFOV);
	}

	g_GameStringsHandler->ProcessDisplayStrings(DELTA_TIME);
	
	bool isFirstTime = true;
	static int framesCount = 0;

	for (framesCount += numFrames; framesCount > 0; framesCount -= LOOP_FRAME_COUNT)
	{
		// Controls are polled before OnLoop, so input data could be
		// overwritten by script API methods.
		HandleControls(isTitle);

		// Pre-loop script and event handling.
		g_GameScript->OnLoop(DELTA_TIME, false); // TODO: Don't use DELTA_TIME constant with variable framerate
		HandleAllGlobalEvents(EventType::Loop, (Activator)LaraItem->Index);

		// Clear last selected item in inventory (need to be after on loop event handling, so they can detect that).
		g_Gui.CancelInventorySelection();

		// Control lock is processed after handling scripts, because builder may want to
		// process input externally, while still locking Lara from input.
		if (!isTitle && Lara.Control.IsLocked)
			ClearAllActions();

		// Handle inventory / pause / load / save screens.
		auto result = HandleMenuCalls(isTitle);
		if (result != GameStatus::Normal)
			return result;

		// Handle global input events.
		result = HandleGlobalInputEvents(isTitle);
		if (result != GameStatus::Normal)
			return result;

		// Queued input actions are read again and cleared after UI 
		// interrupts are processed, so first frame after exiting UI
		// will still register it.
		ApplyActionQueue();
		ClearActionQueue();

		UpdateCamera();
		UpdateAllItems();
		UpdateAllEffects();
		UpdateLara(LaraItem, isTitle);

		g_GameScriptEntities->TestCollidingObjects();

		// Smash shatters and clear stopper flags under them.
		UpdateShatters();

		// Update weather.
		Weather.Update();

		// Update effects.
		UpdateWibble();
		StreamerEffect.Update();
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
		UpdateFishSwarm();
		UpdateLocusts();
		UpdateUnderwaterBloodParticles();

		// Update HUD.
		g_Hud.Update(*LaraItem);
		UpdateFadeScreenAndCinematicBars();

		// Rumble screen (like in submarine level of TRC).
		if (g_GameFlow->GetLevel(CurrentLevel)->Rumble)
			RumbleScreen();

		PlaySoundSources();
		DoFlipEffect(FlipEffect, LaraItem);

		// Post-loop script and event handling.
		g_GameScript->OnLoop(DELTA_TIME, true);

		// Clear savegame loaded flag.
		JustLoaded = false;

		// Update timers.
		GameTimer++;
		GlobalCounter++;

		// Add renderer objects on the first processed frame.
		if (isFirstTime)
		{
			g_Renderer.Lock();
			isFirstTime = false;
		}
	}

	using ns = std::chrono::nanoseconds;
	using get_time = std::chrono::steady_clock;

	auto time2 = std::chrono::high_resolution_clock::now();
	ControlPhaseTime = (std::chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;

	return GameStatus::Normal;
}

unsigned CALLBACK GameMain(void *)
{
	TENLog("Starting GameMain...", LogLevel::Info);

	TimeInit();

	// Do a fixed time title image.
	if (g_GameFlow->IntroImagePath.empty())
		TENLog("Intro image path is not set.", LogLevel::Warning);
	else
		g_Renderer.RenderTitleImage();

	// Execute the Lua gameflow and play the game.
	g_GameFlow->DoFlow();

	DoTheGame = false;

	// Finish the thread.
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
		return isTitle ? GameStatus::ExitGame : GameStatus::ExitToTitle;

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
			short itemNumber = ItemNewRooms[2 * i];
			if (itemNumber >= 0)
				ItemNewRoom(itemNumber, ItemNewRooms[2 * i + 1]);
			else
				KillItem(itemNumber & 0x7FFF);
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
			short itemNumber = ItemNewRooms[2 * i];
			if (itemNumber >= 0)
				EffectNewRoom(itemNumber, ItemNewRooms[2 * i + 1]);
			else
				KillEffect(itemNumber & 0x7FFF);
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

	// Needs to be cleared, otherwise controls will lock if user exits to title while playing flyby with locked controls.
	Lara.Control.IsLocked = false;

	// Resets lightning and wind parameters to avoid holding over previous weather to new level.
	Weather.Clear();

	// Needs to be cleared, otherwise a list of active creatures from previous level will spill into new level.
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
	ClearDisplaySprites();
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

	auto* level = g_GameFlow->GetLevel(levelIndex);

	// Run level script if it exists.
	if (!level->ScriptFileName.empty())
	{
		g_GameScript->ExecuteScriptFile(g_GameFlow->GetGameDir() + level->ScriptFileName);
		g_GameScript->InitCallbacks();
		g_GameStringsHandler->SetCallbackDrawString([](const std::string& key, D3DCOLOR color, const Vec2& pos, float scale, int flags)
		{
			g_Renderer.AddString(
				key,
				Vector2(((float)pos.x / (float)g_Configuration.ScreenWidth * DISPLAY_SPACE_RES.x),
				((float)pos.y / (float)g_Configuration.ScreenHeight * DISPLAY_SPACE_RES.y)),
				Color(color), scale, flags);
		});
	}

	// Play default background music.
	if (type != LevelLoadType::Load)
		PlaySoundTrack(level->GetAmbientTrack(), SoundTrackType::BGM);
}

void DeInitializeScripting(int levelIndex, GameStatus reason)
{
	g_GameScript->OnEnd(reason);
	HandleAllGlobalEvents(EventType::End, (Activator)LaraItem->Index);

	g_GameScript->FreeLevelScripts();
	g_GameScriptEntities->FreeEntities();

	if (!levelIndex)
		g_GameScript->ResetScripts(true);
}

void InitializeOrLoadGame(bool loadGame)
{
	g_Gui.SetInventoryItemChosen(NO_VALUE);
	g_Gui.SetEnterInventory(NO_VALUE);

	// Restore the game?
	if (loadGame)
	{
		SaveGame::Load(g_GameFlow->SelectedSaveGame);

		Camera.pos.x = LaraItem->Pose.Position.x + 256;
		Camera.pos.y = LaraItem->Pose.Position.y + 256;
		Camera.pos.z = LaraItem->Pose.Position.z + 256;

		Camera.target.x = LaraItem->Pose.Position.x;
		Camera.target.y = LaraItem->Pose.Position.y;
		Camera.target.z = LaraItem->Pose.Position.z;

		InitializeGame = false;

		g_GameFlow->SelectedSaveGame = 0;
		g_GameScript->OnLoad();
		HandleAllGlobalEvents(EventType::Load, (Activator)LaraItem->Index);
	}
	else
	{
		// If not loading a savegame, clear all info.
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

			// Restore vehicle.
			auto* item = FindItem(ID_LARA);
			InitializePlayerVehicle(*item);
		}

		g_GameScript->OnStart();
		HandleAllGlobalEvents(EventType::Start, (Activator)LaraItem->Index);
	}
}

GameStatus DoGameLoop(int levelIndex)
{
	int numFrames = LOOP_FRAME_COUNT;
	auto& status = g_GameFlow->LastGameStatus;

	// Before entering actual game loop, ControlPhase must be
	// called once to sort out various runtime shenanigangs (e.g. hair).
	status = ControlPhase(numFrames);

	while (DoTheGame)
	{
		status = ControlPhase(numFrames);

		if (!levelIndex)
		{
			UpdateInputActions(LaraItem);

			auto invStatus = g_Gui.TitleOptions(LaraItem);

			switch (invStatus)
			{
			case InventoryResult::NewGame:
			case InventoryResult::NewGameSelectedLevel:
				status = GameStatus::NewGame;
				break;

			case InventoryResult::HomeLevel:
				status = GameStatus::HomeLevel;
				break;

			case InventoryResult::LoadGame:
				status = GameStatus::LoadGame;
				break;

			case InventoryResult::ExitGame:
				status = GameStatus::ExitGame;
				break;
			}

			if (invStatus != InventoryResult::None)
				break;
		}
		else
		{
			if (status == GameStatus::ExitToTitle ||
				status == GameStatus::LaraDead ||
				status == GameStatus::LoadGame ||
				status == GameStatus::LevelComplete)
			{
				break;
			}
		}

		numFrames = DrawPhase(!levelIndex);
		Sound_UpdateScene();
	}

	EndGameLoop(levelIndex, status);
	return status;
}

void EndGameLoop(int levelIndex, GameStatus reason)
{
	SaveGame::SaveHub(levelIndex);
	DeInitializeScripting(levelIndex, reason);

	StopAllSounds();
	StopSoundTracks();
	StopRumble();
}

void HandleControls(bool isTitle)
{
	// Poll input devices and update input variables.
	if (!isTitle)
	{
		// TODO: To allow cutscene skipping later, don't clear Deselect action.
		UpdateInputActions(LaraItem, true);
	}
	else
	{
		ClearAction(In::Look);
	}
}

GameStatus HandleMenuCalls(bool isTitle)
{
	auto result = GameStatus::Normal;

	if (isTitle || ScreenFading)
		return result;

	// Does the player want to enter inventory?
	if (IsClicked(In::Save) && LaraItem->HitPoints > 0 &&
		g_Gui.GetInventoryMode() != InventoryMode::Save &&
		g_GameFlow->IsLoadSaveEnabled())
	{
		SaveGame::LoadHeaders();
		g_Gui.SetInventoryMode(InventoryMode::Save);
		g_Gui.CallInventory(LaraItem, false);
	}
	else if (IsClicked(In::Load) &&
		g_Gui.GetInventoryMode() != InventoryMode::Load &&
		g_GameFlow->IsLoadSaveEnabled())
	{
		SaveGame::LoadHeaders();
		g_Gui.SetInventoryMode(InventoryMode::Load);

		if (g_Gui.CallInventory(LaraItem, false))
			result = GameStatus::LoadGame;
	}
	else if (IsClicked(In::Pause) && LaraItem->HitPoints > 0 &&
			 g_Gui.GetInventoryMode() != InventoryMode::Pause)
	{
		if (g_Gui.CallPause())
			result = GameStatus::ExitToTitle;
	}
	else if ((IsClicked(In::Inventory) || g_Gui.GetEnterInventory() != NO_VALUE) &&
			 LaraItem->HitPoints > 0 && !Lara.Control.Look.IsUsingBinoculars)
	{
		if (g_Gui.CallInventory(LaraItem, true))
			result = GameStatus::LoadGame;
	}

	if (result != GameStatus::Normal)
	{
		StopAllSounds();
		StopRumble();
	}

	return result;
}

GameStatus HandleGlobalInputEvents(bool isTitle)
{
	constexpr auto DEATH_NO_INPUT_TIMEOUT = 10 * FPS;
	constexpr auto DEATH_INPUT_TIMEOUT	  = 3 * FPS;

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
