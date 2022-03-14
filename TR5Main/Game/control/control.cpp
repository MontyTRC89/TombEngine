#include "framework.h"
#include "Game/control/control.h"

#include <process.h>
#include "Game/collision/collide_room.h"
#include "Game/pickup/pickup.h"
#include "Game/camera.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/control/flipeffect.h"
#include "Game/gui.h"
#include "Game/control/volume.h"
#include "Game/control/lot.h"
#include "Game/health.h"
#include "Game/savegame.h"
#include "Game/room.h"
#include "Game/effects/hair.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/debris.h"
#include "Game/effects/footprint.h"
#include "Game/effects/smoke.h"
#include "Game/effects/spark.h"
#include "Game/effects/explosion.h"
#include "Game/effects/drip.h"
#include "Game/effects/weather.h"
#include "Game/effects/lightning.h"
#include "Game/spotcam.h"
#include "Game/control/box.h"
#include "Game/particle/SimpleParticle.h"
#include "Game/collision/sphere.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_cheat.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Effects/tr4_locusts.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Objects/TR4/Entity/tr4_littlebeetle.h"
#include "Objects/TR5/Emitter/tr5_rats_emitter.h"
#include "Objects/TR5/Emitter/tr5_bats_emitter.h"
#include "Objects/TR5/Emitter/tr5_spider_emitter.h"
#include "Sound/sound.h"
#include "Specific/clock.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/prng.h"
#include "Specific/winmain.h"
#include "Scripting/Flow/ScriptInterfaceFlowHandler.h"
#include "Scripting/ScriptInterfaceGame.h"
#include "Scripting/Objects/ScriptInterfaceObjectsHandler.h"
#include "Scripting/Strings/ScriptInterfaceStringsHandler.h"

using std::vector;
using std::unordered_map;
using std::string;

using namespace TEN::Effects::Footprints;
using namespace TEN::Effects::Explosion;
using namespace TEN::Effects::Spark;
using namespace TEN::Effects::Smoke;
using namespace TEN::Effects::Drip;
using namespace TEN::Effects::Lightning;
using namespace TEN::Effects::Environment;
using namespace TEN::Effects;
using namespace TEN::Entities::Switches;
using namespace TEN::Entities::TR4;
using namespace TEN::Renderer;
using namespace TEN::Math::Random;
using namespace TEN::Floordata;

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

bool  InItemControlLoop;
short ItemNewRoomNo;
short ItemNewRooms[MAX_ROOMS];
short NextItemActive;
short NextItemFree;
short NextFxActive;
short NextFxFree;

int WeaponDelay;
int WeaponEnemyTimer;

int DrawPhase()
{
	g_Renderer.Draw();
	Camera.numberFrames = g_Renderer.SyncRenderer();
	return Camera.numberFrames;
}

GAME_STATUS ControlPhase(int numFrames, int demoMode)
{
	short oldLaraFrame;
	ScriptInterfaceLevel* level = g_GameFlow->GetLevel(CurrentLevel);

	RegeneratePickups();

	if (numFrames > 10)
		numFrames = 10;

	if (TrackCameraInit)
		UseSpotCam = false;

	SetDebounce = true;

	g_GameStringsHandler->ProcessDisplayStrings(DELTA_TIME);
	
	static int framesCount = 0;
	for (framesCount += numFrames; framesCount > 0; framesCount -= 2)
	{
		GlobalCounter++;

		// This might not be the exact amount of time that has passed, but giving it a
		// value of 1/30 keeps it in lock-step with the rest of the game logic,
		// which assumes 30 iterations per second.
		g_GameScript->OnControlPhase(DELTA_TIME);

		// Poll the keyboard and update input variables
		if (CurrentLevel != 0)
		{
			if (S_UpdateInput() == -1)
				return GAME_STATUS::GAME_STATUS_NONE;
		}

		// Has Lara control been disabled?
		if (Lara.uncontrollable || CurrentLevel == 0)
		{
			if (CurrentLevel != 0)
				DbInput = 0;
			TrInput &= IN_LOOK;
		}

		// Does the player want to enter inventory?
		SetDebounce = false;

		if (CurrentLevel != 0 && !g_Renderer.isFading())
		{
			if (TrInput & IN_SAVE && LaraItem->hitPoints > 0 && g_Gui.GetInventoryMode() != InventoryMode::Save)
			{
				StopAllSounds();

				g_Gui.SetInventoryMode(InventoryMode::Save);

				if (g_Gui.CallInventory(false))
					return GAME_STATUS::GAME_STATUS_LOAD_GAME;
			}
			else if (TrInput & IN_LOAD && g_Gui.GetInventoryMode() != InventoryMode::Load)
			{
				StopAllSounds();

				g_Gui.SetInventoryMode(InventoryMode::Load);

				if (g_Gui.CallInventory(false))
					return GAME_STATUS::GAME_STATUS_LOAD_GAME;
			}
			else if (TrInput & IN_PAUSE && g_Gui.GetInventoryMode() != InventoryMode::Pause && LaraItem->hitPoints > 0)
			{
				StopAllSounds();
				g_Renderer.DumpGameScene();
				g_Gui.SetInventoryMode(InventoryMode::Pause);
				g_Gui.SetMenuToDisplay(Menu::Pause);
				g_Gui.SetSelectedOption(0);
			}
			else if ((DbInput & IN_DESELECT || g_Gui.GetEnterInventory() != NO_ITEM) && LaraItem->hitPoints > 0)
			{
				// Stop all sounds
				StopAllSounds();

				if (g_Gui.CallInventory(true))
					return GAME_STATUS::GAME_STATUS_LOAD_GAME;
			}
		}

		while (g_Gui.GetInventoryMode() == InventoryMode::Pause)
		{
			g_Gui.DrawInventory();
			g_Renderer.SyncRenderer();

			if (g_Gui.DoPauseMenu() == InventoryResult::ExitToTitle)
				return GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE;
		}

		// Has level been completed?
		if (CurrentLevel != 0 && LevelComplete)
			return GAME_STATUS::GAME_STATUS_LEVEL_COMPLETED;

		int oldInput = TrInput;

		// Is Lara dead?
		if (CurrentLevel != 0 && (Lara.deathCount > 300 || Lara.deathCount > 60 && TrInput))
		{
			return GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE; // Maybe do game over menu like some PSX versions have??
		}

		if (demoMode && TrInput == -1)
		{
			oldInput = 0;
			TrInput = 0;
		}

		if (CurrentLevel == 0)
			TrInput = 0;

		// Handle lasersight and binocular
		if (CurrentLevel != 0)
		{
			if (!(TrInput & IN_LOOK) || UseSpotCam || TrackCameraInit ||
				((LaraItem->currentAnimState != LS_IDLE || LaraItem->animNumber != LA_STAND_IDLE) && (!Lara.isDucked || TrInput & IN_DUCK || LaraItem->animNumber != LA_CROUCH_IDLE || LaraItem->goalAnimState != LS_CROUCH_IDLE)))
			{
				if (BinocularRange == 0)
				{
					if (UseSpotCam || TrackCameraInit)
						TrInput &= ~IN_LOOK;
				}
				else
				{
					// If any input but optic controls (directions + action), immediately exit binoculars mode.
					if (TrInput != IN_NONE && ((TrInput & ~IN_OPTIC_CONTROLS) != IN_NONE))
						BinocularRange = 0;

					if (LaserSight)
					{
						BinocularRange = 0;
						LaserSight = false;
						AlterFOV(ANGLE(80));
						LaraItem->meshBits = 0xFFFFFFFF;
						Lara.busy = false;
						Camera.type = BinocularOldCamera;

						ResetLaraFlex(LaraItem);

						Camera.bounce = 0;
						BinocularOn = -8;

						TrInput &= ~IN_LOOK;
					}
					else
					{
						TrInput |= IN_LOOK;
						DbInput = 0;
					}
				}
			}
			else if (BinocularRange == 0)
			{
				if (Lara.gunStatus == LG_READY && ((Lara.gunType == WEAPON_REVOLVER && Lara.Weapons[WEAPON_REVOLVER].HasLasersight) || 
												   (Lara.gunType == WEAPON_HK) || 
												   (Lara.gunType == WEAPON_CROSSBOW && Lara.Weapons[WEAPON_CROSSBOW].HasLasersight)))
				{
					BinocularRange = 128;
					BinocularOldCamera = Camera.oldType;

					Lara.busy = true;
					LaserSight = true;
				}
			}
		}

		// Update all items
		InItemControlLoop = true;

		short itemNum = NextItemActive;
		while (itemNum != NO_ITEM)
		{
			ITEM_INFO *item = &g_Level.Items[itemNum];
			short nextItem = item->nextActive;

			if (item->afterDeath <= 128)
			{
				if (Objects[item->objectNumber].control)
					Objects[item->objectNumber].control(itemNum);

				TEN::Control::Volumes::TestVolumes(itemNum);

				if (item->afterDeath > 0 && item->afterDeath < 128 && !(Wibble & 3))
					item->afterDeath++;
				if (item->afterDeath == 128)
					KillItem(itemNum);
			}
			else
			{
				KillItem(itemNum);
			}

			itemNum = nextItem;
		}

		InItemControlLoop = false;
		KillMoveItems();

		// Update all effects
		InItemControlLoop = true;

		short fxNum = NextFxActive;
		while (fxNum != NO_ITEM)
		{
			short nextFx = EffectList[fxNum].nextActive;
			FX_INFO *fx = &EffectList[fxNum];
			if (Objects[fx->objectNumber].control)
				Objects[fx->objectNumber].control(fxNum);
			fxNum = nextFx;
		}

		InItemControlLoop = false;
		KillMoveEffects();

		// Update some effect timers
		if (SmokeCountL)
			SmokeCountL--;
		if (SmokeCountR)
			SmokeCountR--;
		if (SplashCount)
			SplashCount--;
		if (WeaponDelay)
			WeaponDelay--;
		if (WeaponEnemyTimer)
			WeaponEnemyTimer--;

		if (CurrentLevel != 0)
		{
			// Control Lara
			InItemControlLoop = true;
			LaraControl(LaraItem, &LaraCollision);
			InItemControlLoop = false;
			KillMoveItems();

			g_Renderer.updateLaraAnimations(true);

			if (g_Gui.GetInventoryItemChosen() != NO_ITEM)
			{
				SayNo();
				g_Gui.SetInventoryItemChosen(NO_ITEM);
			}

			LaraCheatyBits();
			TriggerLaraDrips(LaraItem);

			// Update Lara's ponytails
			HairControl(LaraItem, level->GetLaraType() == LaraType::Young);
		}

		if (UseSpotCam)
		{
			// Draw flyby cameras
			//if (CurrentLevel != 0)
			//	g_Renderer->EnableCinematicBars(true);
			CalculateSpotCameras();
		}
		else
		{
			// Do the standard camera
			//g_Renderer->EnableCinematicBars(false);
			TrackCameraInit = false;
			CalculateCamera();
		}

		// Update oscillator seed
		Wibble = (Wibble + WIBBLE_SPEED) & WIBBLE_MAX;

		// Smash shatters and clear stopper flags under them
		UpdateShatters();

		// Update weather
		Weather.Update();

		// Update special FX
		UpdateSparks();
		UpdateFireSparks();
		UpdateSmoke();
		UpdateBlood();
		UpdateBubbles();
		UpdateDebris();
		UpdateGunShells();
		UpdateFootprints();
		UpdateSplashes();
		UpdateLightning();
		UpdateDrips();
		UpdateRats();
		UpdateBats();
		UpdateSpiders();
		UpdateSparkParticles();
		UpdateSmokeParticles();
		updateSimpleParticles();
		UpdateDripParticles();
		UpdateExplosionParticles();
		UpdateShockwaves();
		UpdateScarabs();
		UpdateLocusts();
		AnimateWaterfalls();

		// Rumble screen (like in submarine level of TRC)
		if (level->Rumble)
			RumbleScreen();

		PlaySoundSources();
		DoFlipEffect(FlipEffect);

		UpdateFadeScreenAndCinematicBars();

		// Clear savegame loaded flag
		JustLoaded = false;

		// Update timers
		HealthBarTimer--;
		GameTimer++;
	}

	return GAME_STATUS::GAME_STATUS_NONE;
}

unsigned CALLBACK GameMain(void *)
{
	try 
	{
		TENLog("Starting GameMain...", LogLevel::Info);

		TimeInit();

		if (g_GameFlow->IntroImagePath.empty())
		{
			throw TENScriptException("Intro image path is not set.");
		}

		// Do a fixed time title image
		g_Renderer.renderTitleImage();

		// Execute the LUA gameflow and play the game
		g_GameFlow->DoFlow();

		DoTheGame = false;

		// Finish the thread
		PostMessage(WindowsHandle, WM_CLOSE, NULL, NULL);
		EndThread();
	}
	catch (TENScriptException const& e) 
	{
		std::string msg = std::string{ "An unrecoverable error occurred in " } + __func__ + ": " + e.what();
		TENLog(msg, LogLevel::Error, LogConfig::All);
		throw;
	}

	return true;
}

GAME_STATUS DoTitle(int index, std::string const & ambient)
{
	TENLog("DoTitle", LogLevel::Info);

	// Reset all the globals for the game which needs this
	CleanUp();

	// Load the level
	LoadLevelFile(index);

	InventoryResult inventoryResult;

	if (g_GameFlow->TitleType == TITLE_TYPE::FLYBY)
	{
		// Initialise items, effects, lots, camera
		InitialiseFXArray(true);
		InitialisePickupDisplay();
		InitialiseCamera();
		StopAllSounds();

		// Run the level script
		ScriptInterfaceLevel* level = g_GameFlow->GetLevel(index);
		std::string err;
		if (!level->ScriptFileName.empty())
		{
			g_GameScript->ExecuteScriptFile(level->ScriptFileName);
			g_GameScript->InitCallbacks();
			g_GameStringsHandler->SetCallbackDrawString([](std::string const key, D3DCOLOR col, int x, int y, int flags)
			{
				g_Renderer.drawString(float(x)/float(g_Configuration.Width) * ASSUMED_WIDTH_FOR_TEXT_DRAWING, float(y)/float(g_Configuration.Height) * ASSUMED_HEIGHT_FOR_TEXT_DRAWING, key.c_str(), col, flags);
			});
		}

		RequiredStartPos = false;
		if (InitialiseGame)
		{
			GameTimer = 0;
			RequiredStartPos = false;
			InitialiseGame = false;
		}

		Statistics.Level.Timer = 0;

		// Initialise flyby cameras
		InitSpotCamSequences();
		InitialiseSpotCam(0);
		UseSpotCam = true;

		// Play background music
		PlaySoundTrack(ambient);

		// Initialize menu
		g_Gui.SetMenuToDisplay(Menu::Title);
		g_Gui.SetSelectedOption(0);

		// Initialise ponytails
		InitialiseHair();

		InitialiseItemBoxData();

		g_GameScript->OnStart();

		SetScreenFadeIn(FADE_SCREEN_SPEED);

		ControlPhase(2, 0);

		int frames = 0;
		auto status = InventoryResult::None;

		while (status == InventoryResult::None)
		{
			g_Renderer.renderTitle();

			SetDebounce = true;
			S_UpdateInput();
			SetDebounce = false;

			status = g_Gui.TitleOptions();

			if (status != InventoryResult::None)
				break;

			Camera.numberFrames = g_Renderer.SyncRenderer();
			frames = Camera.numberFrames;
			ControlPhase(frames, 0);
		}

		inventoryResult = status;
	}
	else
		inventoryResult = g_Gui.TitleOptions();

	StopSoundTracks();

	g_GameScript->OnEnd();
	g_GameScript->FreeLevelScripts();
	g_GameScriptEntities->FreeEntities();

	switch (inventoryResult)
	{
	case InventoryResult::NewGame:
		return GAME_STATUS::GAME_STATUS_NEW_GAME;
	case InventoryResult::LoadGame:
		return GAME_STATUS::GAME_STATUS_LOAD_GAME;
	case InventoryResult::ExitGame:
		return GAME_STATUS::GAME_STATUS_EXIT_GAME;
	}

	return GAME_STATUS::GAME_STATUS_NEW_GAME;
}

GAME_STATUS DoLevel(int index, std::string const & ambient, bool loadFromSavegame)
{
	// If not loading a savegame, then clear all the infos
	if (!loadFromSavegame)
	{
		Statistics.Level.Timer = 0;
		Statistics.Level.Distance = 0;
		Statistics.Level.AmmoUsed = 0;
		Statistics.Level.AmmoHits = 0;
		Statistics.Level.Kills = 0;
	}

	// Reset all the globals for the game which needs this
	CleanUp();

	// Load the level
	LoadLevelFile(index);

	// Initialise items, effects, lots, camera
	InitialiseFXArray(true);
	InitialisePickupDisplay();
	InitialiseCamera();
	StopAllSounds();

	// Run the level script
	ScriptInterfaceLevel* level = g_GameFlow->GetLevel(index);
  
	if (!level->ScriptFileName.empty())
	{
		g_GameScript->ExecuteScriptFile(level->ScriptFileName);
		g_GameScript->InitCallbacks();
		g_GameStringsHandler->SetCallbackDrawString([](std::string const key, D3DCOLOR col, int x, int y, int flags)
		{
			g_Renderer.drawString(float(x)/float(g_Configuration.Width) * ASSUMED_WIDTH_FOR_TEXT_DRAWING, float(y)/float(g_Configuration.Height) * ASSUMED_HEIGHT_FOR_TEXT_DRAWING, key.c_str(), col, flags);
		});
	}

	// Play default background music
	PlaySoundTrack(ambient, SOUNDTRACK_PLAYTYPE::BGM);

	// Restore the game?
	if (loadFromSavegame)
	{
		SaveGame::Load(g_GameFlow->SelectedSaveGame);

		Camera.pos.x = LaraItem->pos.xPos + 256;
		Camera.pos.y = LaraItem->pos.yPos + 256;
		Camera.pos.z = LaraItem->pos.zPos + 256;

		Camera.target.x = LaraItem->pos.xPos;
		Camera.target.y = LaraItem->pos.yPos;
		Camera.target.z = LaraItem->pos.zPos;

		int x = Lara.weaponItem;

		RequiredStartPos = false;
		InitialiseGame = false;
		g_GameFlow->SelectedSaveGame = 0;
	}
	else
	{
		RequiredStartPos = false;
		if (InitialiseGame)
		{
			GameTimer = 0;
			RequiredStartPos = false;
			InitialiseGame = false;
		}

		Statistics.Level.Timer = 0;
	}

	g_Gui.SetInventoryItemChosen(NO_ITEM);
	g_Gui.SetEnterInventory(NO_ITEM);

	// Initialise flyby cameras
	InitSpotCamSequences();

	// Initialise ponytails
	InitialiseHair();

	InitialiseItemBoxData();

	g_GameScript->OnStart();
	if (loadFromSavegame)
	{
		g_GameScript->OnLoad();
	}

	int nframes = 2;

	// First control phase
	g_Renderer.resetAnimations();
	GAME_STATUS result = ControlPhase(nframes, 0);

	// Fade in screen
	SetScreenFadeIn(FADE_SCREEN_SPEED);

	// The game loop, finally!
	while (true)
	{
		result = ControlPhase(nframes, 0);
		nframes = DrawPhase();
		Sound_UpdateScene();

		if (result == GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE ||
			result == GAME_STATUS::GAME_STATUS_LOAD_GAME ||
			result == GAME_STATUS::GAME_STATUS_LEVEL_COMPLETED)
		{
			g_GameScript->OnEnd();
			g_GameScript->FreeLevelScripts();
			g_GameScriptEntities->FreeEntities();

			// Here is the only way for exiting from the loop
			StopAllSounds();
			StopSoundTracks();

			return result;
		}
	}
}

int GetWaterSurface(int x, int y, int z, short roomNumber)
{
	ROOM_INFO *room = &g_Level.Rooms[roomNumber];
	FLOOR_INFO *floor = GetSector(room, x - room->x, z - room->z);

	if (room->flags & ENV_FLAG_WATER)
	{
		while (floor->RoomAbove(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->RoomAbove(x, y, z).value_or(floor->Room)];
			if (!(room->flags & ENV_FLAG_WATER))
				return (floor->CeilingHeight(x, z));
			floor = GetSector(room, x - room->x, z - room->z);
		}

		return NO_HEIGHT;
	}
	else
	{
		while (floor->RoomBelow(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->RoomBelow(x, y, z).value_or(floor->Room)];
			if (room->flags & ENV_FLAG_WATER)
				return (floor->FloorHeight(x, z));
			floor = GetSector(room, x - room->x, z - room->z);
		}
	}

	return NO_HEIGHT;
}

void UpdateShatters()
{
	if (!SmashedMeshCount)
		return;

	do
	{
		SmashedMeshCount--;

		FLOOR_INFO* floor = GetFloor(
			SmashedMesh[SmashedMeshCount]->pos.xPos,
			SmashedMesh[SmashedMeshCount]->pos.yPos,
			SmashedMesh[SmashedMeshCount]->pos.zPos,
			&SmashedMeshRoom[SmashedMeshCount]);

		TestTriggers(SmashedMesh[SmashedMeshCount]->pos.xPos,
			SmashedMesh[SmashedMeshCount]->pos.yPos,
			SmashedMesh[SmashedMeshCount]->pos.zPos,
			SmashedMeshRoom[SmashedMeshCount], true);

		floor->Stopper = false;
		SmashedMesh[SmashedMeshCount] = 0;
	} while (SmashedMeshCount != 0);
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

void AlterFloorHeight(ITEM_INFO *item, int height)
{
	FLOOR_INFO *floor;
	FLOOR_INFO *ceiling;
	BOX_INFO *box;
	short roomNumber;
	int flag = 0;

	if (abs(height))
	{
		flag = 1;
		if (height >= 0)
			height++;
		else
			height--;
	}

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	ceiling = GetFloor(item->pos.xPos, height + item->pos.yPos - WALL_SIZE, item->pos.zPos, &roomNumber);

	floor->FloorCollision.Planes[0].z += height;
	floor->FloorCollision.Planes[1].z += height;

	box = &g_Level.Boxes[floor->Box];
	if (box->flags & BLOCKABLE)
	{
		if (height >= 0)
			box->flags &= ~BLOCKED;
		else
			box->flags |= BLOCKED;
	}
}

FLOOR_INFO *GetFloor(int x, int y, int z, short *roomNumber)
{
	const auto location = GetRoom(ROOM_VECTOR{*roomNumber, y}, x, y, z);
	*roomNumber = location.roomNumber;
	return &GetFloor(*roomNumber, x, z);
}

int GetFloorHeight(FLOOR_INFO *floor, int x, int y, int z)
{
	return GetFloorHeight(ROOM_VECTOR{floor->Room, y}, x, z).value_or(NO_HEIGHT);
}

int GetRandomControl()
{
	return GenerateInt();
}

int GetRandomDraw()
{
	return GenerateInt();
}

int GetCeiling(FLOOR_INFO *floor, int x, int y, int z)
{
	return GetCeilingHeight(ROOM_VECTOR{floor->Room, y}, x, z).value_or(NO_HEIGHT);
}

int ExplodeItemNode(ITEM_INFO *item, int Node, int NoXZVel, int bits)
{
	if (1 << Node & item->meshBits)
	{
		auto num = bits;
		if (item->objectNumber == ID_SHOOT_SWITCH1 && (CurrentLevel == 4 || CurrentLevel == 7)) // TODO: remove hardcoded think !
		{
			SoundEffect(SFX_TR5_SMASH_METAL, &item->pos, 0);
		}
		else if (num == 256)
		{
			num = -64;
		}
		GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD | SPHERES_SPACE_BONE_ORIGIN, Matrix::Identity);
		ShatterItem.yRot = item->pos.yRot;
		ShatterItem.bit = 1 << Node;
		ShatterItem.meshIndex = Objects[item->objectNumber].meshIndex + Node;
		ShatterItem.sphere.x = CreatureSpheres[Node].x;
		ShatterItem.sphere.y = CreatureSpheres[Node].y;
		ShatterItem.sphere.z = CreatureSpheres[Node].z;
		ShatterItem.flags = item->objectNumber == ID_CROSSBOW_BOLT ? 0x400 : 0;
		ShatterImpactData.impactDirection = Vector3(0, -1, 0);
		ShatterImpactData.impactLocation = {(float)ShatterItem.sphere.x, (float)ShatterItem.sphere.y, (float)ShatterItem.sphere.z};
		ShatterObject(&ShatterItem, NULL, num, item->roomNumber, NoXZVel);
		item->meshBits &= ~ShatterItem.bit;
		return 1;
	}
	return 0;
}

int GetWaterDepth(int x, int y, int z, short roomNumber)
{
	FLOOR_INFO* floor;
	ROOM_INFO* r = &g_Level.Rooms[roomNumber];

	short roomIndex = NO_ROOM;
	do
	{
		int zFloor = (z - r->z) / SECTOR(1);
		int xFloor = (x - r->x) / SECTOR(1);

		if (zFloor <= 0)
		{
			zFloor = 0;
			if (xFloor < 1)
				xFloor = 1;
			else if (xFloor > r->xSize - 2)
				xFloor = r->xSize - 2;
		}
		else if (zFloor >= r->zSize - 1)
		{
			zFloor = r->zSize - 1;
			if (xFloor < 1)
				xFloor = 1;
			else if (xFloor > r->xSize - 2)
				xFloor = r->xSize - 2;
		}
		else if (xFloor < 0)
			xFloor = 0;
		else if (xFloor >= r->xSize)
			xFloor = r->xSize - 1;

		floor = &r->floor[zFloor + xFloor * r->zSize];
		roomIndex = floor->WallPortal;
		if (roomIndex != NO_ROOM)
		{
			roomNumber = roomIndex;
			r = &g_Level.Rooms[roomIndex];
		}
	} while (roomIndex != NO_ROOM);

	if (r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
	{
		while (floor->RoomAbove(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			r = &g_Level.Rooms[floor->RoomAbove(x, y, z).value_or(floor->Room)];
			if (!(r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP)))
			{
				int wh = floor->CeilingHeight(x, z);
				floor = GetFloor(x, y, z, &roomNumber);
				return (GetFloorHeight(floor, x, y, z) - wh);
			}
			floor = GetSector(r, x - r->x, z - r->z);
		}

		return DEEP_WATER;
	}
	else
	{
		while (floor->RoomBelow(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			r = &g_Level.Rooms[floor->RoomBelow(x, y, z).value_or(floor->Room)];
			if (r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
			{
				int wh = floor->FloorHeight(x, z);
				floor = GetFloor(x, y, z, &roomNumber);
				return (GetFloorHeight(floor, x, y, z) - wh);
			}
			floor = GetSector(r, x - r->x, z - r->z);
		}

		return NO_HEIGHT;
	}
}

int GetWaterHeight(int x, int y, int z, short roomNumber)
{
	ROOM_INFO *r = &g_Level.Rooms[roomNumber];
	FLOOR_INFO *floor;
	short adjoiningRoom = NO_ROOM;

	do
	{
		int xBlock = (x - r->x) / SECTOR(1);
		int zBlock = (z - r->z) / SECTOR(1);

		if (zBlock <= 0)
		{
			zBlock = 0;
			if (xBlock < 1)
				xBlock = 1;
			else if (xBlock > r->xSize - 2)
				xBlock = r->xSize - 2;
		}
		else if (zBlock >= r->zSize - 1)
		{
			zBlock = r->zSize - 1;
			if (xBlock < 1)
				xBlock = 1;
			else if (xBlock > r->xSize - 2)
				xBlock = r->xSize - 2;
		}
		else if (xBlock < 0)
			xBlock = 0;
		else if (xBlock >= r->xSize)
			xBlock = r->xSize - 1;

		floor = &r->floor[zBlock + xBlock * r->zSize];
		adjoiningRoom = floor->WallPortal;

		if (adjoiningRoom != NO_ROOM)
		{
			roomNumber = adjoiningRoom;
			r = &g_Level.Rooms[adjoiningRoom];
		}
	} while (adjoiningRoom != NO_ROOM);

	if (floor->IsWall(x, z))
		return NO_HEIGHT;

	if (r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
	{
		while (floor->RoomAbove(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			auto r = &g_Level.Rooms[floor->RoomAbove(x, y, z).value_or(floor->Room)];

			if (!(r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP)))
				return GetCollisionResult(x, r->maxceiling, z, floor->RoomAbove(x, r->maxceiling, z).value_or(NO_ROOM)).Block->FloorHeight(x, r->maxceiling, z);
				//return r->minfloor; // TODO: check if individual block floor height checks provoke any game-breaking bugs!

			floor = GetSector(r, x - r->x, z - r->z);

			if (floor->RoomAbove(x, y, z).value_or(NO_ROOM) == NO_ROOM)
				break;
		}

		return r->maxceiling;
	}
	else
	{
		while (floor->RoomBelow(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			auto r = &g_Level.Rooms[floor->RoomBelow(x, y, z).value_or(floor->Room)];

			if (r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
				return GetCollisionResult(x, r->minfloor, z, floor->RoomBelow(x, r->minfloor, z).value_or(NO_ROOM)).Block->CeilingHeight(x, r->minfloor, z);
				//return r->maxceiling; // TODO: check if individual block ceiling height checks provoke any game-breaking bugs!

			floor = GetSector(r, x - r->x, z - r->z);

			if (floor->RoomBelow(x, y, z).value_or(NO_ROOM) == NO_ROOM)
				break;
		}
	}

	return NO_HEIGHT;
}

int GetDistanceToFloor(int itemNumber, bool precise)
{
	auto item = &g_Level.Items[itemNumber];
	auto result = GetCollisionResult(item);

	// HACK: Remove item from bridge objects temporarily.
	result.Block->RemoveItem(itemNumber);
	auto height = GetFloorHeight(result.Block, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	result.Block->AddItem(itemNumber);

	auto bounds = GetBoundsAccurate(item);
	int minHeight = precise ? bounds->Y2 : 0;

	return minHeight + item->pos.yPos - height;
}

void CleanUp()
{
	// Reset oscillator seed
	Wibble = 0;

	// Needs to be cleared or otherwise controls will lockup if user will exit to title
	// while playing flyby with locked controls
	Lara.uncontrollable = false;

	// Weather.Clear resets lightning and wind parameters so user won't see prev weather in new level
	Weather.Clear();

	// Needs to be cleared because otherwise a list of active creatures from previous level
	// will spill into new level
	ActiveCreatures.clear();

	// Clear spotcam array
	ClearSpotCamSequences();

	// Clear all kinds of particles
	DisableSmokeParticles();
	DisableDripParticles();
	DisableBubbles();
	DisableDebris();

	// Clear soundtrack masks
	ClearSoundTrackMasks();
}