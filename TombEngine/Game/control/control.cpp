#include "framework.h"
#include "Game/control/control.h"

#include <chrono>
#include <process.h>

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/control/flipeffect.h"
#include "Game/control/lot.h"
#include "Game/control/volume.h"
#include "Game/effects/debris.h"
#include "Game/effects/drip.h"
#include "Game/effects/effects.h"
#include "Game/effects/Electricity.h"
#include "Game/effects/explosion.h"
#include "Game/effects/footprint.h"
#include "Game/effects/hair.h"
#include "Game/effects/simple_particle.h"
#include "Game/effects/smoke.h"
#include "Game/effects/spark.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/Gui.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_cheat.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/health.h"
#include "Game/items.h"
#include "Game/pickup/pickup.h"
#include "Game/room.h"
#include "Game/savegame.h"
#include "Game/spotcam.h"
#include "Math/Math.h"
#include "Objects/Effects/tr4_locusts.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/Object/rope.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Objects/TR4/Entity/tr4_beetle_swarm.h"
#include "Objects/TR5/Emitter/tr5_bats_emitter.h"
#include "Objects/TR5/Emitter/tr5_rats_emitter.h"
#include "Objects/TR5/Emitter/tr5_spider_emitter.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Scripting/Include/Objects/ScriptInterfaceObjectsHandler.h"
#include "Scripting/Include/ScriptInterfaceGame.h"
#include "Scripting/Include/Strings/ScriptInterfaceStringsHandler.h"
#include "Sound/sound.h"
#include "Specific/clock.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/winmain.h"

using namespace std::chrono;
using namespace TEN::Effects;
using namespace TEN::Effects::Drip;
using namespace TEN::Effects::Electricity;
using namespace TEN::Effects::Environment;
using namespace TEN::Effects::Explosion;
using namespace TEN::Effects::Footprints;
using namespace TEN::Effects::Smoke;
using namespace TEN::Effects::Spark;
using namespace TEN::Entities::Generic;
using namespace TEN::Entities::Switches;
using namespace TEN::Entities::TR4;
using namespace TEN::Floordata;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Renderer;

int GameTimer       = 0;
int GlobalCounter   = 0;
int Wibble          = 0;

bool InitialiseGame;
bool DoTheGame;
bool JustLoaded;
bool ThreadEnded;

int RequiredStartPos;
int CurrentLevel;
int LevelComplete;

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

	Camera.numberFrames = g_Renderer.Synchronize();
	return Camera.numberFrames;
}

GameStatus ControlPhase(int numFrames)
{
	auto time1 = std::chrono::high_resolution_clock::now();

	auto* level = g_GameFlow->GetLevel(CurrentLevel);
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

	for (framesCount += numFrames; framesCount > 0; framesCount -= 2)
	{
		// Controls are polled before OnControlPhase, so input data could be
		// overwritten by script API methods.
		HandleControls(isTitle);

		// This might not be the exact amount of time that has passed, but giving it a
		// value of 1/30 keeps it in lock-step with the rest of the game logic,
		// which assumes 30 iterations per second.
		g_GameScript->OnControlPhase(DELTA_TIME);

		// Handle inventory / pause / load / save screens.
		auto result = HandleMenuCalls(isTitle);
		if (result != GameStatus::None)
			return result;

		// Handle global input events.
		result = HandleGlobalInputEvents(isTitle);
		if (result != GameStatus::None)
			return result;

		// Queued input actions are read again and cleared after UI 
		// interrupts are processed, so first frame after exiting UI
		// will still register it.
		ApplyActionQueue();
		ClearActionQueue();

		UpdateAllItems();
		UpdateAllEffects();
		UpdateLara(LaraItem, isTitle);

		g_GameScriptEntities->TestCollidingObjects();

		if (UseSpotCam)
		{
			// Draw flyby cameras.
			CalculateSpotCameras();
		}
		else
		{
			// Do the standard camera.
			TrackCameraInit = false;
			CalculateCamera();
		}

		// Update oscillator seed.
		Wibble = (Wibble + WIBBLE_SPEED) & WIBBLE_MAX;

		// Smash shatters and clear stopper flags under them.
		UpdateShatters();

		// Update weather.
		Weather.Update();

		// Update special FX.
		UpdateSparks();
		UpdateFireSparks();
		UpdateSmoke();
		UpdateBlood();
		UpdateBubbles();
		UpdateDebris();
		UpdateGunShells();
		UpdateFootprints();
		UpdateSplashes();
		UpdateElectricitys();
		UpdateHelicalLasers();
		UpdateDrips();
		UpdateRats();
		UpdateBats();
		UpdateSpiders();
		UpdateSparkParticles();
		UpdateSmokeParticles();
		UpdateSimpleParticles();
		UpdateDripParticles();
		UpdateExplosionParticles();
		UpdateShockwaves();
		UpdateBeetleSwarm();
		UpdateLocusts();

		// Update screen UI and overlays.
		UpdateBars(LaraItem);
		UpdateFadeScreenAndCinematicBars();

		// Rumble screen (like in submarine level of TRC).
		if (g_GameFlow->GetLevel(CurrentLevel)->Rumble)
			RumbleScreen();

		PlaySoundSources();
		DoFlipEffect(FlipEffect, LaraItem);

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

	return GameStatus::None;
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

	TENLog(isTitle ? "DoTitle" : "DoLevel", LogLevel::Info);

	// Load the level. Fall back to title if unsuccessful.
	if (!LoadLevelFile(levelIndex))
		return isTitle ? GameStatus::ExitGame : GameStatus::ExitToTitle;

	// Initialize items, effects, lots, and cameras.
	InitialiseFXArray(true);
	InitialisePickupDisplay();
	InitialiseCamera();
	InitialiseSpotCamSequences(isTitle);
	InitialiseHair();
	InitialiseItemBoxData();

	// Initialize scripting.
	InitialiseScripting(levelIndex, loadGame);
	InitialiseNodeScripts();

	// Initialize game variables and optionally load game.
	InitialiseOrLoadGame(loadGame);

	// Prepare title menu, if necessary.
	if (isTitle)
	{
		g_Gui.SetMenuToDisplay(Menu::Title);
		g_Gui.SetSelectedOption(0);
	}

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
	Lara.Control.Locked = false;

	// Resets lightning and wind parameters to avoid holding over previous weather to new level.
	Weather.Clear();

	// Needs to be cleared, otherwise a list of active creatures from previous level will spill into new level.
	ActiveCreatures.clear();

	// Clear ropes.
	Ropes.clear();

	// Clear camera data.
	ClearSpotCamSequences();
	ClearCinematicBars();

	// Clear all kinds of particles.
	DisableSmokeParticles();
	DisableDripParticles();
	DisableBubbles();
	DisableDebris();

	// Clear swarm enemies.
	ClearSwarmEnemies(nullptr);

	// Clear soundtrack masks.
	ClearSoundTrackMasks();

	// Clear all remaining renderer data.
	g_Renderer.ClearScene();
}

void InitialiseScripting(int levelIndex, bool loadGame)
{
	g_GameStringsHandler->ClearDisplayStrings();
	g_GameScript->ResetScripts(!levelIndex || loadGame);

	auto* level = g_GameFlow->GetLevel(levelIndex);

	// Run level script if it exists.
	if (!level->ScriptFileName.empty())
	{
		g_GameScript->ExecuteScriptFile(level->ScriptFileName);
		g_GameScript->InitCallbacks();
		g_GameStringsHandler->SetCallbackDrawString([](std::string const key, D3DCOLOR col, int x, int y, int flags)
		{
			g_Renderer.AddString(float(x) / float(g_Configuration.Width) * REFERENCE_RES_WIDTH,
								 float(y) / float(g_Configuration.Height) * REFERENCE_RES_HEIGHT,
								 key.c_str(), col, flags);
		});
	}

	// Play default background music.
	PlaySoundTrack(level->GetAmbientTrack(), SoundTrackType::BGM);
}

void DeInitialiseScripting(int levelIndex)
{
	g_GameScript->OnEnd();
	g_GameScript->FreeLevelScripts();
	g_GameScriptEntities->FreeEntities();

	if (!levelIndex)
		g_GameScript->ResetScripts(true);
}

void InitialiseOrLoadGame(bool loadGame)
{
	RequiredStartPos = false;

	g_Gui.SetInventoryItemChosen(NO_ITEM);
	g_Gui.SetEnterInventory(NO_ITEM);

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

		InitialiseGame = false;

		g_GameFlow->SelectedSaveGame = 0;
		g_GameScript->OnLoad();
	}
	else
	{
		// If not loading a savegame, clear all info.
		Statistics.Level = {};

		if (InitialiseGame)
		{
			// Clear all game info as well.
			Statistics.Game = {};
			GameTimer = 0;
			InitialiseGame = false;

			TENLog("Starting new game.", LogLevel::Info);
		}
		else
		{
			TENLog("Starting new level.", LogLevel::Info);
		}

		g_GameScript->OnStart();
	}
}

GameStatus DoGameLoop(int levelIndex)
{
	// Before entering actual game loop, ControlPhase must be
	// called once to sort out various runtime shenanigangs (e.g. hair).

	int numFrames = 2;
	auto result = ControlPhase(numFrames);

	while (DoTheGame)
	{
		result = ControlPhase(numFrames);

		if (!levelIndex)
		{
			UpdateInputActions(LaraItem);

			auto invStatus = g_Gui.TitleOptions(LaraItem);

			switch (invStatus)
			{
			case InventoryResult::NewGame:
			case InventoryResult::NewGameSelectedLevel:
				result = GameStatus::NewGame;
				break;

			case InventoryResult::LoadGame:
				result = GameStatus::LoadGame;
				break;

			case InventoryResult::ExitGame:
				result = GameStatus::ExitGame;
				break;
			}

			if (invStatus != InventoryResult::None)
				break;
		}
		else
		{
			if (result == GameStatus::ExitToTitle ||
				result == GameStatus::LoadGame ||
				result == GameStatus::LevelComplete)
			{
				break;
			}
		}

		numFrames = DrawPhase(!levelIndex);
		Sound_UpdateScene();
	}

	EndGameLoop(levelIndex);
	return result;
}

void EndGameLoop(int levelIndex)
{
	DeInitialiseScripting(levelIndex);

	StopAllSounds();
	StopSoundTracks();
	StopRumble();
}

void HandleControls(bool isTitle)
{
	// Poll keyboard and update input variables.
	if (!isTitle)
	{
		if (Lara.Control.Locked)
			ClearAllActions();
		else
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
	auto result = GameStatus::None;

	if (isTitle || ScreenFading)
		return result;

	// Does the player want to enter inventory?
	if (IsClicked(In::Save) && LaraItem->HitPoints > 0 &&
		g_Gui.GetInventoryMode() != InventoryMode::Save)
	{
		g_Gui.SetInventoryMode(InventoryMode::Save);

		if (g_Gui.CallInventory(LaraItem, false))
			result = GameStatus::SaveGame;
	}
	else if (IsClicked(In::Load) &&
			 g_Gui.GetInventoryMode() != InventoryMode::Load)
	{
		g_Gui.SetInventoryMode(InventoryMode::Load);

		if (g_Gui.CallInventory(LaraItem, false))
			result = GameStatus::LoadGame;
	}
	else if (IsClicked(In::Pause) && LaraItem->HitPoints > 0 &&
			 g_Gui.GetInventoryMode() != InventoryMode::Pause)
	{
		g_Renderer.DumpGameScene();
		g_Gui.SetInventoryMode(InventoryMode::Pause);
		g_Gui.SetMenuToDisplay(Menu::Pause);
		g_Gui.SetSelectedOption(0);

		while (g_Gui.GetInventoryMode() == InventoryMode::Pause)
		{
			g_Gui.DrawInventory();
			g_Renderer.Synchronize();

			if (g_Gui.DoPauseMenu(LaraItem) == InventoryResult::ExitToTitle)
			{
				result = GameStatus::ExitToTitle;
				break;
			}
		}
	}
	else if ((IsClicked(In::Option) || g_Gui.GetEnterInventory() != NO_ITEM) &&
			 LaraItem->HitPoints > 0 && !BinocularOn)
	{
		if (g_Gui.CallInventory(LaraItem, true))
			result = GameStatus::LoadGame;
	}

	if (result != GameStatus::None)
	{
		StopAllSounds();
		StopRumble();
	}

	return result;
}

GameStatus HandleGlobalInputEvents(bool isTitle)
{
	if (isTitle)
		return GameStatus::None;

	HandleOptics(LaraItem);

	// Is Lara dead?
	static constexpr int DEATH_NO_INPUT_TIMEOUT = 5 * FPS;
	static constexpr int DEATH_INPUT_TIMEOUT = 10 * FPS;

	if (Lara.Control.Count.Death > DEATH_NO_INPUT_TIMEOUT ||
		Lara.Control.Count.Death > DEATH_INPUT_TIMEOUT && !NoAction())
	{
		return GameStatus::ExitToTitle; // Maybe do game over menu like some PSX versions have??
	}

	// Has level been completed?
	if (LevelComplete)
	{
		return GameStatus::LevelComplete;
	}

	return GameStatus::None;
}
