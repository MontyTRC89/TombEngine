#include "framework.h"
#include <process.h>
#include "collide.h"
#include "control.h"
#include "pickup.h"
#include "camera.h"
#include "Lara.h"
#include "effects/hair.h"
#include "items.h"
#include "flipeffect.h"
#include "draw.h"
#ifdef NEW_INV
#include "newinv2.h"
#else
#include "inventory.h"
#endif
#include "lot.h"
#include "health.h"
#include "savegame.h"
#include "Sound/sound.h"
#include "spotcam.h"
#include "box.h"
#include "sphere.h"
#include "level.h"
#include "input.h"
#include "winmain.h"
#include "setup.h"
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
#include "control/flipmap.h"
#include "Lara/lara_one_gun.h"
#include "generic_switch.h"

using std::vector;
using std::unordered_map;
using std::string;

using namespace TEN::Effects::Footprints;
using namespace TEN::Effects::Explosion;
using namespace TEN::Effects::Spark;
using namespace TEN::Effects::Smoke;
using namespace TEN::Effects::Environment;
using namespace TEN::Effects;
using namespace TEN::Entities::Switches;
using namespace TEN::Renderer;
using namespace TEN::Math::Random;
using namespace TEN::Floordata;

int RumbleTimer = 0;
int InGameCnt = 0;
bool InItemControlLoop;
short ItemNewRoomNo;
short ItemNewRooms[512];
short NextFxActive;
short NextFxFree;
short NextItemActive;
short NextItemFree;

int DisableLaraControl = 0;
int WeatherType;
int LaraDrawType;
int NumAnimatedTextures;
short *AnimTextureRanges;
int nAnimUVRanges;
int Wibble = 0;
int SetDebounce = 0;

std::string CurrentAtmosphere;
short CurrentRoom;
int GameTimer;
short GlobalCounter;
byte LevelComplete;
#ifndef NEW_INV
int LastInventoryItem;
#endif
int TrackCameraInit;
short TorchRoom;
int InitialiseGame;
int RequiredStartPos;
int WeaponDelay;
int WeaponEnemyTimer;
int CutSeqNum;
int CurrentLevel;
bool DoTheGame;
bool ThreadEnded;
int OnFloor;
int TriggerTimer;
int JustLoaded;
int OldLaraBusy;
int Infrared;

std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];
short IsRoomOutsideNo;

extern GameFlow *g_GameFlow;
extern GameScript *g_GameScript;
#ifndef NEW_INV
extern Inventory g_Inventory;
#endif
extern int SplashCount;

// This might not be the exact amount of time that has passed, but giving it a
// value of 1/30 keeps it in lock-step with the rest of the game logic,
// which assumes 30 iterations per second.
static constexpr float deltaTime = 1.0f / 30.0f;

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

	g_GameScript->ProcessDisplayStrings(deltaTime);
	
	static int FramesCount = 0;
	for (FramesCount += numFrames; FramesCount > 0; FramesCount -= 2)
	{
		GlobalCounter++;

		// This might not be the exact amount of time that has passed, but giving it a
		// value of 1/30 keeps it in lock-step with the rest of the game logic,
		// which assumes 30 iterations per second.
		g_GameScript->OnControlPhase(deltaTime);

		// Poll the keyboard and update input variables
		if (CurrentLevel != 0)
		{
			if (S_UpdateInput() == -1)
				return GAME_STATUS_NONE;
		}

		// Has Lara control been disabled?
		if (DisableLaraControl || CurrentLevel == 0)
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
				SOUND_Stop();
				g_Renderer.DumpGameScene();
				GLOBAL_invMode = IM_PAUSE;
				pause_menu_to_display = pause_main_menu;
				pause_selected_option = 1;
			}
			else if ((DbInput & IN_DESELECT || GLOBAL_enterinventory != NO_ITEM) && LaraItem->hitPoints > 0)
			{
				// Stop all sounds
				SOUND_Stop();

				if (S_CallInventory2())
					return GAME_STATUS_LOAD_GAME;
			}
		}

		while (GLOBAL_invMode == IM_PAUSE)
		{
			DrawInv();
			g_Renderer.SyncRenderer();

			int z = DoPauseMenu();

			if (z == INV_RESULT_EXIT_TO_TILE)
				return GAME_STATUS_EXIT_TO_TITLE;
		}
#else
		if (CurrentLevel != 0 && !g_Renderer.isFading())
		{
			if ((DbInput & IN_DESELECT || g_Inventory.GetEnterObject() != NO_ITEM) && LaraItem->hitPoints > 0)
			{
				// Stop all sounds
				SOUND_Stop();
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
			return GAME_STATUS_LEVEL_COMPLETED;

		int oldInput = TrInput;

		// Is Lara dead?
		if (CurrentLevel != 0 && (Lara.deathCount > 300 || Lara.deathCount > 60 && TrInput))
		{
#ifdef NEW_INV
			return GAME_STATUS_EXIT_TO_TITLE;//maybe do game over menu like some PSX versions have??
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

				Infrared = false;
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
					Infrared = true;
				}
				else
					Infrared = false;
			}
			else
			{
				if (LaserSight)
				{
					Infrared = true;
				}
				else
				{
					Infrared = false;
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
			if (Lara.hasFired)
			{
				AlertNearbyGuards(LaraItem);
				Lara.hasFired = false;
			}

			// Is Lara poisoned?
			if (Lara.poisoned)
			{
				if (Lara.poisoned > 4096)
					Lara.poisoned = 4096;

				if (Lara.poisoned >= 256 && !(Wibble & 0xFF))
				{
					LaraItem->hitPoints -= Lara.poisoned >> 8;
				}
			}

			// Control Lara
			InItemControlLoop = true;
		//	Lara.skelebob = NULL;
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

		//WTF: what is this? It's used everywhere so it has to stay
		Wibble = (Wibble + 4) & 0xFC;

		// Update special effects
		TriggerLaraDrips();

		if (SmashedMeshCount)
		{
			do
			{
				SmashedMeshCount--;

				FLOOR_INFO *floor = GetFloor(
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
		UpdateEnergyArcs();
		UpdateLightning();
		UpdateDrips();
		UpdateRats();
		UpdateBats();
		UpdateSpiders();
		UpdateSparkParticles();
		UpdateSmokeParticles();
		updateSimpleParticles();
		TEN::Effects::Drip::UpdateDrips();
		UpdateExplosionParticles();
		UpdateShockwaves();
		TEN::Entities::TR4::UpdateScarabs();
		TEN::Entities::TR4::UpdateLocusts();
		//Legacy_UpdateLightning();
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

	return GAME_STATUS_NONE;
}

unsigned CALLBACK GameMain(void *)
{
	try {
		printf("GameMain\n");
		TIME_Init();
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
	catch (TENScriptException const& e) {
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
	ResetGlobals();

	// Load the level
	S_LoadLevelFile(index);

	int inventoryResult;

	if (g_GameFlow->TitleType == TITLE_TYPE::FLYBY)
	{
		// Initialise items, effects, lots, camera
		InitialiseFXArray(true);
		InitialisePickupDisplay();
		InitialiseCamera();
		SOUND_Stop();

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

		InitialiseSpotCam(2);
		CurrentAtmosphere = "083_horus";
		UseSpotCam = true;

		// Play background music
		//CurrentAtmosphere = ambient;
		S_CDPlay(CurrentAtmosphere, 1);

		// Initialise ponytails
		InitialiseHair();

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

	UseSpotCam = false;
	S_CDStop();

	g_GameScript->OnEnd();
	g_GameScript->FreeLevelScripts();
	switch (inventoryResult)
	{
	case INV_RESULT_NEW_GAME:
		return GAME_STATUS_NEW_GAME;
	case INV_RESULT_LOAD_GAME:
		return GAME_STATUS_LOAD_GAME;
	case INV_RESULT_EXIT_GAME:
		return GAME_STATUS_EXIT_GAME;
	}

	return GAME_STATUS_NEW_GAME;
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
	ResetGlobals();

	// Load the level
	S_LoadLevelFile(index);

	// Initialise items, effects, lots, camera
	InitialiseFXArray(true);
	InitialisePickupDisplay();
	InitialiseCamera();
	SOUND_Stop();

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
	CurrentAtmosphere = ambient;
	S_CDPlay(CurrentAtmosphere, 1);

	// Initialise ponytails
	InitialiseHair();

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
		nframes = DrawPhaseGame();
		Sound_UpdateScene();

		if (result == GAME_STATUS_EXIT_TO_TITLE ||
			result == GAME_STATUS_LOAD_GAME ||
			result == GAME_STATUS_LEVEL_COMPLETED)
		{
			g_GameScript->OnEnd();
			g_GameScript->FreeLevelScripts();
			// Here is the only way for exiting from the loop
			SOUND_Stop();
			S_CDStop();
			DisableBubbles();
			DisableDebris();

			return result;
		}
	}
}

void TranslateItem(ITEM_INFO *item, int x, int y, int z)
{
	float c = phd_cos(item->pos.yRot);
	float s = phd_sin(item->pos.yRot);

	item->pos.xPos += c * x + s * z;
	item->pos.yPos += y;
	item->pos.zPos += -s * x + c * z;
}

int GetWaterSurface(int x, int y, int z, short roomNumber)
{
	ROOM_INFO *room = &g_Level.Rooms[roomNumber];
	FLOOR_INFO *floor = XZ_GET_SECTOR(room, x - room->x, z - room->z);

	if (room->flags & ENV_FLAG_WATER)
	{
		while (floor->RoomAbove() != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->RoomAbove()];
			if (!(room->flags & ENV_FLAG_WATER))
				return (floor->CeilingHeight(x, z));
			floor = XZ_GET_SECTOR(room, x - room->x, z - room->z);
		}
		return NO_HEIGHT;
	}
	else
	{
		while (floor->RoomBelow() != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->RoomBelow()];
			if (room->flags & ENV_FLAG_WATER)
				return (floor->FloorHeight(x, z));
			floor = XZ_GET_SECTOR(room, x - room->x, z - room->z);
		}
	}

	return NO_HEIGHT;
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

int GetChange(ITEM_INFO *item, ANIM_STRUCT *anim)
{
	if (item->currentAnimState == item->goalAnimState)
		return 0;

	if (anim->numberChanges <= 0)
		return 0;

	for (int i = 0; i < anim->numberChanges; i++)
	{
		CHANGE_STRUCT *change = &g_Level.Changes[anim->changeIndex + i];
		if (change->goalAnimState == item->goalAnimState)
		{
			for (int j = 0; j < change->numberRanges; j++)
			{
				RANGE_STRUCT *range = &g_Level.Ranges[change->rangeIndex + j];
				if (item->frameNumber >= range->startFrame && item->frameNumber <= range->endFrame)
				{
					item->animNumber = range->linkAnimNum;
					item->frameNumber = range->linkFrameNum;

					return 1;
				}
			}
		}
	}

	return 0;
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

	/*if (floor->AverageFloor == NO_HEIGHT / STEP_SIZE)
	{
		floor->AverageFloor = ceiling->ceiling + height / STEP_SIZE;
	}
	else
	{
		floor->AverageFloor += height / STEP_SIZE;
		if (floor->AverageFloor == ceiling->ceiling && !flag)
			floor->AverageFloor = NO_HEIGHT / STEP_SIZE;
	}*/

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
	return generateInt();
}

int GetRandomDraw()
{
	return generateInt();
}

int GetCeiling(FLOOR_INFO *floor, int x, int y, int z)
{
	return GetCeilingHeight(ROOM_VECTOR{floor->Room, y}, x, z).value_or(NO_HEIGHT);
}

void AnimateItem(ITEM_INFO *item)
{
	item->touchBits = 0;
	item->hitStatus = false;

	item->frameNumber++;

	ANIM_STRUCT *anim = &g_Level.Anims[item->animNumber];
	if (anim->numberChanges > 0 && GetChange(item, anim))
	{
		anim = &g_Level.Anims[item->animNumber];

		item->currentAnimState = anim->currentAnimState;

		if (item->requiredAnimState == item->currentAnimState)
			item->requiredAnimState = 0;
	}

	if (item->frameNumber > anim->frameEnd)
	{
		if (anim->numberCommands > 0)
		{
			short *cmd = &g_Level.Commands[anim->commandIndex];
			for (int i = anim->numberCommands; i > 0; i--)
			{
				switch (*(cmd++))
				{
				case COMMAND_MOVE_ORIGIN:
					TranslateItem(item, cmd[0], cmd[1], cmd[2]);
					cmd += 3;
					break;

				case COMMAND_JUMP_VELOCITY:
					item->fallspeed = *(cmd++);
					item->speed = *(cmd++);
					item->gravityStatus = true;
					break;

				case COMMAND_DEACTIVATE:
					if (Objects[item->objectNumber].intelligent && !item->afterDeath)
						item->afterDeath = 1;
					item->status = ITEM_DEACTIVATED;
					break;
				case COMMAND_SOUND_FX:
				case COMMAND_EFFECT:
					cmd += 2;
					break;

				default:
					break;
				}
			}
		}

		item->animNumber = anim->jumpAnimNum;
		item->frameNumber = anim->jumpFrameNum;

		anim = &g_Level.Anims[item->animNumber];

		if (item->currentAnimState != anim->currentAnimState)
		{
			item->currentAnimState = anim->currentAnimState;
			item->goalAnimState = anim->currentAnimState;
		}

		if (item->requiredAnimState == item->currentAnimState)
			item->requiredAnimState = 0;
	}

	if (anim->numberCommands > 0)
	{
		short *cmd = &g_Level.Commands[anim->commandIndex];
		int flags;
		int effectID = 0;

		for (int i = anim->numberCommands; i > 0; i--)
		{
			switch (*(cmd++))
			{
			case COMMAND_MOVE_ORIGIN:
				cmd += 3;
				break;

			case COMMAND_JUMP_VELOCITY:
				cmd += 2;
				break;

			case COMMAND_SOUND_FX:
				if (item->frameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				flags = cmd[1] & 0xC000;

				if (!Objects[item->objectNumber].waterCreature)
				{
					if (item->roomNumber == NO_ROOM)
					{
						item->pos.xPos = LaraItem->pos.xPos;
						item->pos.yPos = LaraItem->pos.yPos - 762;
						item->pos.zPos = LaraItem->pos.zPos;

						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
					}
					else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
					{
						if (!flags || flags == SFX_WATERONLY && (g_Level.Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER || Objects[item->objectNumber].intelligent))
						{
							SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
						}
					}
					else if (!flags || flags == SFX_LANDONLY && !(g_Level.Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER))
					{
						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
					}
				}
				else
				{
					if (g_Level.Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER)
						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 1);
					else
						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 0);
				}
				break;

			case COMMAND_EFFECT:
				if (item->frameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				FXType = cmd[1] & 0xC000;
				effectID = cmd[1] & 0x3FFF;
				DoFlipEffect(effectID, item);

				cmd += 2;
				break;

			default:
				break;
			}
		}
	}

	int lateral = 0;

	if (item->gravityStatus)
	{
		item->fallspeed += (item->fallspeed >= 128 ? 1 : 6);
		item->pos.yPos += item->fallspeed;
	}
	else
	{
		int velocity = anim->velocity;
		if (anim->acceleration)
			velocity += anim->acceleration * (item->frameNumber - anim->frameBase);
		item->speed = velocity >> 16;

		lateral = anim->Xvelocity;
		if (anim->Xacceleration)
			lateral += anim->Xacceleration * (item->frameNumber - anim->frameBase);

		lateral >>= 16;
	}

	item->pos.xPos += item->speed * phd_sin(item->pos.yRot);
	item->pos.zPos += item->speed * phd_cos(item->pos.yRot);

	item->pos.xPos += lateral * phd_sin(item->pos.yRot + ANGLE(90));
	item->pos.zPos += lateral * phd_cos(item->pos.yRot + ANGLE(90));

	// Update matrices
	short itemNumber = item - g_Level.Items.data();
	g_Renderer.updateItemAnimations(itemNumber, true);
}

void RumbleScreen()
{
	if (!(GlobalCounter & 0x1FF))
		SoundEffect(SFX_TR5_KLAXON, 0, 4104);

	if (RumbleTimer >= 0)
		RumbleTimer++;

	if (RumbleTimer > 450)
	{
		if (!(GetRandomControl() & 0x1FF))
		{
			InGameCnt = 0;
			RumbleTimer = -32 - (GetRandomControl() & 0x1F);
			return;
		}
	}

	if (RumbleTimer < 0)
	{
		if (InGameCnt >= abs(RumbleTimer))
		{
			Camera.bounce = -(GetRandomControl() % abs(RumbleTimer));
			RumbleTimer++;
		}
		else
		{
			InGameCnt++;
			Camera.bounce = -(GetRandomControl() % InGameCnt);
		}
	}
}

void RefreshCamera(short type, short *data)
{
	short trigger, value, targetOk;

	targetOk = 2;

	do
	{
		trigger = *(data++);
		value = trigger & VALUE_BITS;

		switch (TRIG_BITS(trigger))
		{
		case TO_CAMERA:
			data++;

			if (value == Camera.last)
			{
				Camera.number = value;

				if ((Camera.timer < 0) || (Camera.type == LOOK_CAMERA) || (Camera.type == COMBAT_CAMERA))
				{
					Camera.timer = -1;
					targetOk = 0;
					break;
				}
				Camera.type = FIXED_CAMERA;
				targetOk = 1;
			}
			else
				targetOk = 0;
			break;

		case TO_TARGET:
			if (Camera.type == LOOK_CAMERA || Camera.type == COMBAT_CAMERA)
				break;

			Camera.item = &g_Level.Items[value];
			break;
		}
	} while (!(trigger & END_BIT));

	if (Camera.item)
		if (!targetOk || (targetOk == 2 && Camera.item->lookedAt && Camera.item != Camera.lastItem))
			Camera.item = NULL;

	if (Camera.number == -1 && Camera.timer > 0)
		Camera.timer = -1;
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
		while (floor->RoomAbove() != NO_ROOM)
		{
			if (CheckNoColCeilingTriangle(floor, x, z) == SPLIT_SOLID)
				break;
			r = &g_Level.Rooms[floor->RoomAbove()];
			if (!(r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP)))
				return r->minfloor;
			floor = XZ_GET_SECTOR(r, x - r->x, z - r->z);
			if (floor->RoomAbove() == NO_ROOM)
				break;
		}

		return r->maxceiling;
	}
	else
	{
		while (floor->RoomBelow() != NO_ROOM)
		{
			if (CheckNoColFloorTriangle(floor, x, z) == SPLIT_SOLID)
				break;
			r = &g_Level.Rooms[floor->RoomBelow()];
			if (r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
				return r->maxceiling;
			floor = XZ_GET_SECTOR(r, x - r->x, z - r->z);
			if (floor->RoomBelow() == NO_ROOM)
				break;
		}
	}

	return NO_HEIGHT;
}

int IsObjectInRoom(short roomNumber, short objectNumber)
{
	short itemNumber = g_Level.Rooms[roomNumber].itemNumber;

	if (itemNumber == NO_ITEM)
		return 0;

	while (true)
	{
		ITEM_INFO *item = &g_Level.Items[itemNumber];

		if (item->objectNumber == objectNumber)
			break;

		itemNumber = item->nextItem;

		if (itemNumber == NO_ITEM)
			return 0;
	}

	return 1;
}

int IsRoomOutside(int x, int y, int z)
{
	if (x < 0 || z < 0)
		return -2;

	int xTable = x / 1024;
	int zTable = z / 1024;

	if (OutsideRoomTable[xTable][zTable].size() == 0)
		return -2;

	for (size_t i = 0; i < OutsideRoomTable[xTable][zTable].size(); i++)
	{
		short roomNumber = OutsideRoomTable[xTable][zTable][i];
		ROOM_INFO* r = &g_Level.Rooms[roomNumber];

		if ((y > r->maxceiling) && (y < r->minfloor)
			&& ((z > (r->z + 1024)) && (z < (r->z + ((r->xSize - 1) * 1024))))
			&& ((x > (r->x + 1024)) && (x < (r->x + ((r->ySize - 1) * 1024)))))
		{
			IsRoomOutsideNo = roomNumber;

			FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
			int height = GetFloorHeight(floor, x, y, z);
			if (height == NO_HEIGHT || y > height)
				return -2;
			height = GetCeiling(floor, x, y, z);
			if (y < height)
				return -2;

			return ((r->flags & (ENV_FLAG_WIND | ENV_FLAG_WATER)) != 0 ? 1 : -3);
		}
	}

	return -2;
}

void ResetGlobals()
{
	// Needs to be cleared or otherwise controls will lockup if user will exit to title
	// while playing flyby with locked controls
	DisableLaraControl = false;

	// Weather.Clear resets lightning and wind parameters so user won't see prev weather in new level
	Weather.Clear();

	// Needs to be cleared because otherwise a list of active creatures from previous level
	// will spill into new level
	ActiveCreatures.clear();

	// Clear spotcam array
	ClearSpotCamSequences();
}