#include "framework.h"
#include <process.h>
#include "winmain.h"
#include "collide.h"
#include "control/control.h"
#include "pickup.h"
#include "camera.h"
#include "Lara.h"
#include "effects/hair.h"
#include "items.h"
#include "flipeffect.h"
#ifdef NEW_INV
#include "newinv2.h"
#else
#include "inventory.h"
#endif
#include "control/lot.h"
#include "health.h"
#include "savegame.h"
#include "Sound/sound.h"
#include "spotcam.h"
#include "control/box.h"
#include "objects.h"
#include "sphere.h"
#include "level.h"
#include "input.h"
#include "setup.h"
#include "room.h"
#include "effects/effects.h"
#include "effects/tomb4fx.h"
#include "effects/debris.h"
#include "effects/footprint.h"
#include "effects/smoke.h"
#include "effects/spark.h"
#include "effects/explosion.h"
#include "effects/drip.h"
#include "effects/weather.h"
#include "tr5_rats_emitter.h"
#include "tr5_bats_emitter.h"
#include "tr5_spider_emitter.h"
#include "tr4_locusts.h"
#include "tr4_littlebeetle.h"
#include "particle/SimpleParticle.h"
#include "Specific/prng.h"
#include "Specific/clock.h"
#include "Lara/lara_one_gun.h"
#include "generic_switch.h"
#include "Scripting/GameFlowScript.h"
#include "Game/effects/lightning.h"

using std::vector;
using std::unordered_map;
using std::string;

using namespace TEN::Effects::Footprints;
using namespace TEN::Effects::Explosion;
using namespace TEN::Effects::Spark;
using namespace TEN::Effects::Smoke;
using namespace TEN::Effects::Drip;
using namespace TEN::Effects::Environment;
using namespace TEN::Effects;
using namespace TEN::Entities::Switches;
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
short ItemNewRooms[512];
short NextItemActive;
short NextItemFree;
short NextFxActive;
short NextFxFree;

int WeatherType;
int LaraDrawType;

int WeaponDelay;
int WeaponEnemyTimer;

#ifndef NEW_INV
int LastInventoryItem;
extern Inventory g_Inventory;
#endif

int DrawPhase()
{
	g_Renderer.Draw();
	Camera.numberFrames = g_Renderer.SyncRenderer();
	return Camera.numberFrames;
}

GAME_STATUS ControlPhase(int numFrames, int demoMode)
{
	short oldLaraFrame;
	GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

	RegeneratePickups();

	if (numFrames > 10)
		numFrames = 10;

	if (TrackCameraInit)
		UseSpotCam = false;

	SetDebounce = true;

	g_GameScript->ProcessDisplayStrings(DELTA_TIME);
	
	static int FramesCount = 0;
	for (FramesCount += numFrames; FramesCount > 0; FramesCount -= 2)
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

#ifdef NEW_INV
		if (CurrentLevel != 0 && !g_Renderer.isFading())
		{
			if (TrInput & IN_PAUSE && GLOBAL_invMode != IM_PAUSE && LaraItem->hitPoints > 0)
			{
				Sound_Stop();
				g_Renderer.DumpGameScene();
				GLOBAL_invMode = IM_PAUSE;
				pause_menu_to_display = pause_main_menu;
				pause_selected_option = 1;
			}
			else if ((DbInput & IN_DESELECT || GLOBAL_enterinventory != NO_ITEM) && LaraItem->hitPoints > 0)
			{
				// Stop all sounds
				Sound_Stop();

				if (S_CallInventory2())
					return GAME_STATUS::GAME_STATUS_LOAD_GAME;
			}
		}

		while (GLOBAL_invMode == IM_PAUSE)
		{
			DrawInv();
			g_Renderer.SyncRenderer();

			int z = DoPauseMenu();

			if (z == INV_RESULT_EXIT_TO_TILE)
				return GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE;
		}
#else
		if (CurrentLevel != 0 && !g_Renderer.isFading())
		{
			if ((DbInput & IN_DESELECT || g_Inventory.GetEnterObject() != NO_ITEM) && LaraItem->hitPoints > 0)
			{
				// Stop all sounds
				Sound_Stop();
				int inventoryResult = g_Inventory.DoInventory();
				switch (inventoryResult)
				{
				case INV_RESULT_LOAD_GAME:
					return GAME_STATUS_LOAD_GAME;
				case INV_RESULT_EXIT_TO_TILE:
					return GAME_STATUS_EXIT_TO_TITLE;
				}
			}
		}
#endif

		// Has level been completed?
		if (CurrentLevel != 0 && LevelComplete)
			return GAME_STATUS::GAME_STATUS_LEVEL_COMPLETED;

		int oldInput = TrInput;

		// Is Lara dead?
		if (CurrentLevel != 0 && (Lara.deathCount > 300 || Lara.deathCount > 60 && TrInput))
		{
#ifdef NEW_INV
			return GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE;//maybe do game over menu like some PSX versions have??
#else
			int inventoryResult = g_Inventory.DoInventory();
			switch (inventoryResult)
			{
			case INV_RESULT_NEW_GAME:
				return GAME_STATUS_NEW_GAME;
			case INV_RESULT_LOAD_GAME:
				return GAME_STATUS_LOAD_GAME;
			case INV_RESULT_EXIT_TO_TILE:
				return GAME_STATUS_EXIT_TO_TITLE;
			}
#endif
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
			if (!(TrInput & IN_LOOK) || SniperCameraActive || UseSpotCam || TrackCameraInit ||
				((LaraItem->currentAnimState != LS_STOP || LaraItem->animNumber != LA_STAND_IDLE) && (!Lara.isDucked || TrInput & IN_DUCK || LaraItem->animNumber != LA_CROUCH_IDLE || LaraItem->goalAnimState != LS_CROUCH_IDLE)))
			{
				if (BinocularRange == 0)
				{
					if (SniperCameraActive || UseSpotCam || TrackCameraInit)
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

						Lara.headYrot = 0;
						Lara.headXrot = 0;

						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;

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

		GotLaraSpheres = false;

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

				if (item->afterDeath < 128 && item->afterDeath > 0 && !(Wibble & 3))
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
			LaraControl(Lara.itemNumber);
			InItemControlLoop = false;
			KillMoveItems();

			g_Renderer.updateLaraAnimations(true);

#ifdef NEW_INV
			if (GLOBAL_inventoryitemchosen != -1)
			{
				SayNo();
				GLOBAL_inventoryitemchosen = -1;
			}
#endif
			// Update Lara's ponytails
			HairControl(0, 0, 0);
			if (level->LaraType == LARA_TYPE::YOUNG)
				HairControl(0, 1, 0);
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
		Wibble = (Wibble + 4) & 0xFC;

		// Smash shatters and clear stopper flags under them
		UpdateShatters();

		// Update weather
		Weather.Update();

		// Update special FX
		TriggerLaraDrips();
		UpdateSparks();
		UpdateFireSparks();
		UpdateSmoke();
		UpdateBlood();
		UpdateBubbles();
		UpdateDebris();
		UpdateGunShells();
		UpdateFootprints();
		UpdateSplashes();
		TEN::Effects::Lightning::UpdateLightning();
		UpdateDrips();
		UpdateRats();
		UpdateBats();
		UpdateSpiders();
		UpdateSparkParticles();
		UpdateSmokeParticles();
		updateSimpleParticles();
		TEN::Effects::Drip::UpdateDripParticles();
		UpdateExplosionParticles();
		UpdateShockwaves();
		TEN::Entities::TR4::UpdateScarabs();
		TEN::Entities::TR4::UpdateLocusts();
		AnimateWaterfalls();

		// Rumble screen (like in submarine level of TRC)
		if (level->Rumble)
			RumbleScreen();

		PlaySoundSources();
		DoFlipEffect(FlipEffect);

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
		logD("Starting GameMain...");

		TimeInit();

		if (g_GameFlow->IntroImagePath.empty())
		{
			throw TENScriptException("Intro image path is not set.");
		}

		// Do a fixed time title image
		g_Renderer.renderTitleImage();

		// Execute the LUA gameflow and play the game
		g_GameFlow->DoGameflow();

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

GAME_STATUS DoTitle(int index)
{
	//DB_Log(2, "DoTitle - DLL");
	printf("DoTitle\n");

	// Reset all the globals for the game which needs this
	CleanUp();

	// Load the level
	LoadLevelFile(index);

	int inventoryResult;

	if (g_GameFlow->TitleType == TITLE_TYPE::FLYBY)
	{
		// Initialise items, effects, lots, camera
		InitialiseFXArray(true);
		InitialisePickupDisplay();
		InitialiseCamera();
		Sound_Stop();

		// Run the level script
		GameScriptLevel* level = g_GameFlow->Levels[index];
		std::string err;
		if (!level->ScriptFileName.empty())
		{
			g_GameScript->ExecuteScript(level->ScriptFileName);
			g_GameScript->InitCallbacks();
			g_GameScript->SetCallbackDrawString([](std::string const key, D3DCOLOR col, int x, int y, int flags)
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

		Savegame.Level.Timer = 0;
		if (CurrentLevel == 1)
			Savegame.TLCount = 0;
#ifndef NEW_INV
		LastInventoryItem = -1;
#endif

		// Initialise flyby cameras
		InitSpotCamSequences();
		InitialiseSpotCam(0);
		UseSpotCam = true;

		// Play background music
		PlaySoundTrack("083_horus", SOUND_TRACK_BGM);

		// Initialise ponytails
		InitialiseHair();

		InitialiseItemBoxData();

		g_GameScript->OnStart();

		ControlPhase(2, 0);
#ifdef NEW_INV
		int status = 0, frames;
		while (!status)
		{
			g_Renderer.renderTitle();

			SetDebounce = true;
			S_UpdateInput();
			SetDebounce = false;

			status = TitleOptions();

			if (status)
				break;

			Camera.numberFrames = g_Renderer.SyncRenderer();
			frames = Camera.numberFrames;
			ControlPhase(frames, 0);
		}

		inventoryResult = status;
#else
		inventoryResult = g_Inventory.DoTitleInventory();
#endif
	}
	else
	{
#ifdef NEW_INV
		inventoryResult = TitleOptions();
#else
		inventoryResult = g_Inventory.DoTitleInventory();
#endif
	}

	StopSoundTracks();

	g_GameScript->OnEnd();
	g_GameScript->FreeLevelScripts();

	switch (inventoryResult)
	{
	case INV_RESULT_NEW_GAME:
		return GAME_STATUS::GAME_STATUS_NEW_GAME;
	case INV_RESULT_LOAD_GAME:
		return GAME_STATUS::GAME_STATUS_LOAD_GAME;
	case INV_RESULT_EXIT_GAME:
		return GAME_STATUS::GAME_STATUS_EXIT_GAME;
	}

	return GAME_STATUS::GAME_STATUS_NEW_GAME;
}

GAME_STATUS DoLevel(int index, std::string ambient, bool loadFromSavegame)
{
	// If not loading a savegame, then clear all the infos
	if (!loadFromSavegame)
	{
		Savegame.Level.Timer = 0;
		Savegame.Level.Distance = 0;
		Savegame.Level.AmmoUsed = 0;
		Savegame.Level.AmmoHits = 0;
		Savegame.Level.Kills = 0;
	}

	// Reset all the globals for the game which needs this
	CleanUp();

	// Load the level
	LoadLevelFile(index);

	// Initialise items, effects, lots, camera
	InitialiseFXArray(true);
	InitialisePickupDisplay();
	InitialiseCamera();
	Sound_Stop();

	// Run the level script
	GameScriptLevel* level = g_GameFlow->Levels[index];
  
	if (!level->ScriptFileName.empty())
	{
		g_GameScript->ExecuteScript(level->ScriptFileName);
		g_GameScript->InitCallbacks();
		g_GameScript->SetCallbackDrawString([](std::string const key, D3DCOLOR col, int x, int y, int flags)
		{
			g_Renderer.drawString(float(x)/float(g_Configuration.Width) * ASSUMED_WIDTH_FOR_TEXT_DRAWING, float(y)/float(g_Configuration.Height) * ASSUMED_HEIGHT_FOR_TEXT_DRAWING, key.c_str(), col, flags);
		});
	}

	// Restore the game?
	if (loadFromSavegame)
	{
		char fileName[255];
		ZeroMemory(fileName, 255);
		sprintf(fileName, "savegame.%d", g_GameFlow->SelectedSaveGame);
		SaveGame::Load(fileName);

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

		Savegame.Level.Timer = 0;
		if (CurrentLevel == 1)
			Savegame.TLCount = 0;
	}
#ifndef NEW_INV
	LastInventoryItem = -1;
#endif

#ifdef NEW_INV
	GLOBAL_inventoryitemchosen = NO_ITEM;
	GLOBAL_enterinventory = NO_ITEM;
#else
	g_Inventory.SetEnterObject(NO_ITEM);
	g_Inventory.SetSelectedObject(NO_ITEM);
#endif

	// Initialise flyby cameras
	InitSpotCamSequences();

	// Play background music
	PlaySoundTrack(ambient, SOUND_TRACK_BGM);

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
	g_Renderer.fadeIn();

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
			// Here is the only way for exiting from the loop
			Sound_Stop();
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
		while (floor->RoomAbove(x, z, y).value_or(NO_ROOM) != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->RoomAbove(x, z, y).value_or(floor->Room)];
			if (!(room->flags & ENV_FLAG_WATER))
				return (floor->CeilingHeight(x, z));
			floor = GetSector(room, x - room->x, z - room->z);
		}

		return NO_HEIGHT;
	}
	else
	{
		while (floor->RoomBelow(x, z, y).value_or(NO_ROOM) != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->RoomBelow(x, z, y).value_or(floor->Room)];
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
		ShatterItem.meshp = &g_Level.Meshes[Objects[item->objectNumber].meshIndex + Node];
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
			else if (xBlock > r->ySize - 2)
				xBlock = r->ySize - 2;
		}
		else if (zBlock >= r->xSize - 1)
		{
			zBlock = r->xSize - 1;
			if (xBlock < 1)
				xBlock = 1;
			else if (xBlock > r->ySize - 2)
				xBlock = r->ySize - 2;
		}
		else if (xBlock < 0)
			xBlock = 0;
		else if (xBlock >= r->ySize)
			xBlock = r->ySize - 1;

		floor = &r->floor[zBlock + xBlock * r->xSize];
		adjoiningRoom = floor->WallPortal;

		if (adjoiningRoom != NO_ROOM)
		{
			roomNumber = adjoiningRoom;
			r = &g_Level.Rooms[adjoiningRoom];
		}
	} while (adjoiningRoom != NO_ROOM);

	if (r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
	{
		while (floor->RoomAbove(x, z, y).value_or(NO_ROOM) != NO_ROOM)
		{
			auto r = &g_Level.Rooms[floor->RoomAbove(x, z, y).value_or(floor->Room)];

			if (!(r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP)))
				return r->minfloor;

			floor = GetSector(r, x - r->x, z - r->z);

			if (floor->RoomAbove(x, z, y).value_or(NO_ROOM) == NO_ROOM)
				break;
		}

		return r->maxceiling;
	}
	else
	{
		while (floor->RoomBelow(x, z, y).value_or(NO_ROOM) != NO_ROOM)
		{
			auto r = &g_Level.Rooms[floor->RoomBelow(x, z, y).value_or(floor->Room)];

			if (r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
				return r->maxceiling;

			floor = GetSector(r, x - r->x, z - r->z);

			if (floor->RoomBelow(x, z, y).value_or(NO_ROOM) == NO_ROOM)
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
}