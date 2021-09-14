#include "framework.h"
#include "collide.h"
#include "control.h"
#include "pickup.h"
#include "puzzles_keys.h"
#include "camera.h"
#include "Lara.h"
#include "effects\hair.h"
#include "items.h"
#include "flipeffect.h"
#include "draw.h"
#ifdef NEW_INV
#include "newinv2.h"
#else
#include "inventory.h"
#endif
#include "gameflow.h"
#include "lot.h"
#include "draw.h"
#include "health.h"
#include "savegame.h"
#include "Sound\sound.h"
#include "spotcam.h"
#include "box.h"
#include "objects.h"
#include "switch.h"
#include "rope.h"
#include "traps.h"
#include "sphere.h"
#include "lara_fire.h"
#include "level.h"
#include "input.h"
#include "winmain.h"
#include "Renderer11.h"
#include "setup.h"
#include "effects\effects.h"
#include "effects\tomb4fx.h"
#include "effects\debris.h"
#include "effects\footprint.h"
#include "effects\smoke.h"
#include "effects\spark.h"
#include "effects\explosion.h"
#include "effects\drip.h"
#include "effects\sky.h"
#include "tr5_rats_emitter.h"
#include "tr5_bats_emitter.h"
#include "tr5_spider_emitter.h"
#include "tr4_locusts.h"
#include "tr4_littlebeetle.h"
#include "particle/SimpleParticle.h"
#include <process.h>
#include "Specific\prng.h"
#include <Game/Lara/lara_one_gun.h>
#include <Game/Lara/lara_climb.h>
#include "generic_switch.h"
#include "creature_info.h"

using std::vector;
using std::unordered_map;
using std::string;

using namespace TEN::Effects::Footprints;
using namespace TEN::Effects::Explosion;
using namespace TEN::Effects::Spark;
using namespace TEN::Effects::Smoke;
using namespace TEN::Effects;
using namespace TEN::Entities::Switches;
using namespace TEN::Renderer;
using namespace TEN::Math::Random;
using namespace TEN::Floordata;

int KeyTriggerActive;
int number_los_rooms;
short los_rooms[20];
int ClosestItem;
int ClosestDist;
PHD_VECTOR ClosestCoord;
int RumbleTimer = 0;
int InGameCnt = 0;
byte FlipStatus = 0;
int FlipStats[MAX_FLIPMAP];
int FlipMap[MAX_FLIPMAP];
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
int SmokeWindX;
int SmokeWindZ;
int FlipTimer;
int FlipEffect;
int TriggerTimer;
int JustLoaded;
int OldLaraBusy;
int Infrared;
short FlashFadeR;
short FlashFadeG;
short FlashFadeB;
short FlashFader;

std::vector<short> OutsideRoomTable[OUTSIDE_SIZE][OUTSIDE_SIZE];
short IsRoomOutsideNo;

extern GameFlow *g_GameFlow;
extern GameScript *g_GameScript;
#ifndef NEW_INV
extern Inventory g_Inventory;
#endif
extern int SplashCount;
extern short FXType;

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
					         SmashedMeshRoom[SmashedMeshCount], true, 0);

				floor->Stopper = false;
				SmashedMesh[SmashedMeshCount] = 0;
			} while (SmashedMeshCount != 0);
		}

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
		TEN::Effects::Sky::Sky.UpdateSky();
		UpdateExplosionParticles();
		UpdateShockwaves();
		TEN::Entities::TR4::UpdateScarabs();
		TEN::Entities::TR4::UpdateLocusts();
		//Legacy_UpdateLightning();
		AnimateWaterfalls();

		// Rumble screen (like in submarine level of TRC)
		if (level->Rumble)
			RumbleScreen();

		// Play sound sources
		for (size_t i = 0; i < g_Level.SoundSources.size(); i++)
		{
			SOUND_SOURCE_INFO* sound = &g_Level.SoundSources[i];

			short t = sound->flags & 31;
			short group = t & 1;
			group += t & 2;
			group += ((t >> 2) & 1) * 3;
			group += ((t >> 3) & 1) * 4;
			group += ((t >> 4) & 1) * 5;

			if (!FlipStats[group] && (sound->flags & 128) == 0)
				continue;
			else if (FlipStats[group] && (sound->flags & 128) == 0)
				continue;

			SoundEffect(sound->soundId, (PHD_3DPOS*)&sound->x, 0);
		}

		// Do flipeffects
		if (FlipEffect != -1)
			effect_routines[FlipEffect](NULL);

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

int LOS(GAME_VECTOR *start, GAME_VECTOR *end)
{
	int result1, result2;

	end->roomNumber = start->roomNumber;
	if (abs(end->z - start->z) > abs(end->x - start->x))
	{
		result1 = xLOS(start, end);
		result2 = zLOS(start, end);
	}
	else
	{
		result1 = zLOS(start, end);
		result2 = xLOS(start, end);
	}
	if (result2)
	{
		GetFloor(end->x, end->y, end->z, &end->roomNumber);
		if (ClipTarget(start, end) && result1 == 1 && result2 == 1)
		{
			return 1;
		}
	}
	return 0;
}

int xLOS(GAME_VECTOR *start, GAME_VECTOR *end)
{
	int dx, dy, dz, x, y, z, flag;
	short room, room2;
	FLOOR_INFO *floor;

	dx = end->x - start->x;
	if (!dx)
		return 1;
	dy = (end->y - start->y << 10) / dx;
	dz = (end->z - start->z << 10) / dx;
	number_los_rooms = 1;
	los_rooms[0] = start->roomNumber;
	room = start->roomNumber;
	room2 = start->roomNumber;
	flag = 1;
	if (dx < 0)
	{
		x = start->x & 0xFFFFFC00;
		y = ((x - start->x) * dy >> 10) + start->y;
		z = ((x - start->x) * dz >> 10) + start->z;
		while (x > end->x)
		{
			floor = GetFloor(x, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
			{
				flag = -1;
				break;
			}
			floor = GetFloor(x - 1, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x - 1, y, z) || y < GetCeiling(floor, x - 1, y, z))
			{
				flag = 0;
				break;
			}
			x -= 1024;
			y -= dy;
			z -= dz;
		}
		if (flag != 1)
		{
			end->x = x;
			end->y = y;
			end->z = z;
		}
		end->roomNumber = flag ? room : room2;
	}
	else
	{
		x = start->x | 0x3FF;
		y = ((x - start->x) * dy >> 10) + start->y;
		z = ((x - start->x) * dz >> 10) + start->z;
		while (x < end->x)
		{
			floor = GetFloor(x, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
			{
				flag = -1;
				break;
			}
			floor = GetFloor(x + 1, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x + 1, y, z) || y < GetCeiling(floor, x + 1, y, z))
			{
				flag = 0;
				break;
			}
			x += 1024;
			y += dy;
			z += dz;
		}
		if (flag != 1)
		{
			end->x = x;
			end->y = y;
			end->z = z;
		}
		end->roomNumber = flag ? room : room2;
	}
	return flag;
}

int zLOS(GAME_VECTOR *start, GAME_VECTOR *end)
{
	int dx, dy, dz, x, y, z, flag;
	short room, room2;
	FLOOR_INFO *floor;

	dz = end->z - start->z;
	if (!dz)
		return 1;
	dx = (end->x - start->x << 10) / dz;
	dy = (end->y - start->y << 10) / dz;
	number_los_rooms = 1;
	los_rooms[0] = start->roomNumber;
	room = start->roomNumber;
	room2 = start->roomNumber;
	flag = 1;
	if (dz < 0)
	{
		z = start->z & 0xFFFFFC00;
		x = ((z - start->z) * dx >> 10) + start->x;
		y = ((z - start->z) * dy >> 10) + start->y;
		while (z > end->z)
		{
			floor = GetFloor(x, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
			{
				flag = -1;
				break;
			}
			floor = GetFloor(x, y, z - 1, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x, y, z - 1) || y < GetCeiling(floor, x, y, z - 1))
			{
				flag = 0;
				break;
			}
			z -= 1024;
			x -= dx;
			y -= dy;
		}
		if (flag != 1)
		{
			end->x = x;
			end->y = y;
			end->z = z;
		}
		end->roomNumber = flag ? room : room2;
	}
	else
	{
		z = start->z | 0x3FF;
		x = ((z - start->z) * dx >> 10) + start->x;
		y = ((z - start->z) * dy >> 10) + start->y;
		while (z < end->z)
		{
			floor = GetFloor(x, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
			{
				flag = -1;
				break;
			}
			floor = GetFloor(x, y, z + 1, &room);
			if (room != room2)
			{
				room2 = room;
				los_rooms[number_los_rooms] = room;
				++number_los_rooms;
			}
			if (y > GetFloorHeight(floor, x, y, z + 1) || y < GetCeiling(floor, x, y, z + 1))
			{
				flag = 0;
				break;
			}
			z += 1024;
			x += dx;
			y += dy;
		}
		if (flag != 1)
		{
			end->x = x;
			end->y = y;
			end->z = z;
		}
		end->roomNumber = flag ? room : room2;
	}
	return flag;
}

int ClipTarget(GAME_VECTOR *start, GAME_VECTOR *target)
{
	short room;
	int x, y, z, wx, wy, wz;

	room = target->roomNumber;
	if (target->y > GetFloorHeight(GetFloor(target->x, target->y, target->z, &room), target->x, target->y, target->z))
	{
		x = (7 * (target->x - start->x) >> 3) + start->x;
		y = (7 * (target->y - start->y) >> 3) + start->y;
		z = (7 * (target->z - start->z) >> 3) + start->z;
		for (int i = 3; i > 0; --i)
		{
			wx = ((target->x - x) * i >> 2) + x;
			wy = ((target->y - y) * i >> 2) + y;
			wz = ((target->z - z) * i >> 2) + z;
			if (wy < GetFloorHeight(GetFloor(wx, wy, wz, &room), wx, wy, wz))
				break;
		}
		target->x = wx;
		target->y = wy;
		target->z = wz;
		target->roomNumber = room;
		return 0;
	}
	room = target->roomNumber;
	if (target->y < GetCeiling(GetFloor(target->x, target->y, target->z, &room), target->x, target->y, target->z))
	{
		x = (7 * (target->x - start->x) >> 3) + start->x;
		y = (7 * (target->y - start->y) >> 3) + start->y;
		z = (7 * (target->z - start->z) >> 3) + start->z;
		for (int i = 3; i > 0; --i)
		{
			wx = ((target->x - x) * i >> 2) + x;
			wy = ((target->y - y) * i >> 2) + y;
			wz = ((target->z - z) * i >> 2) + z;
			if (wy > GetCeiling(GetFloor(wx, wy, wz, &room), wx, wy, wz))
				break;
		}
		target->x = wx;
		target->y = wy;
		target->z = wz;
		target->roomNumber = room;
		return 0;
	}
	return 1;
}

int GetTargetOnLOS(GAME_VECTOR *src, GAME_VECTOR *dest, int DrawTarget, int firing)
{
	GAME_VECTOR target;
	int result, hit, itemNumber, count;
	MESH_INFO *mesh;
	PHD_VECTOR vector;
	ITEM_INFO *item;
	short angle, triggerItems[8];
	VECTOR dir;
	Vector3 direction = Vector3(dest->x, dest->y, dest->z) - Vector3(src->x, src->y, src->z);
	direction.Normalize();
	target.x = dest->x;
	target.y = dest->y;
	target.z = dest->z;

	result = LOS(src, &target);

	GetFloor(target.x, target.y, target.z, &target.roomNumber);

	if (firing && LaserSight)
	{
		Lara.hasFired = true;
		Lara.fired = true;

		if (Lara.gunType == WEAPON_REVOLVER)
		{
			SoundEffect(SFX_TR4_DESSERT_EAGLE_FIRE, NULL, 0);
		}
	}

	hit = 0;
	itemNumber = ObjectOnLOS2(src, dest, &vector, &mesh);
	if (itemNumber != NO_LOS_ITEM)
	{
		target.x = vector.x - (vector.x - src->x >> 5);
		target.y = vector.y - (vector.y - src->y >> 5);
		target.z = vector.z - (vector.z - src->z >> 5);

		GetFloor(target.x, target.y, target.z, &target.roomNumber);

		// TODO: for covering scientist
//		if ((itemNumber >= 0) && (BaddieSlots[itemNumber].itemNum != NO_ITEM))  // BUGFIX: ensure target has AI. No more pistol desync and camera wobble when shooting non-AI movable objects.
//			Lara.target = &g_Level.Items[itemNumber];
		// this is crashing and it's not really doing anything..

		if (firing)
		{
			if (Lara.gunType != WEAPON_CROSSBOW)
			{
				if (itemNumber < 0)
				{
					if (StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
					{
						ShatterImpactData.impactDirection = direction;
						ShatterImpactData.impactLocation = Vector3(mesh->pos.xPos, mesh->pos.yPos, mesh->pos.zPos);
						ShatterObject(NULL, mesh, 128, target.roomNumber, 0);
						SmashedMeshRoom[SmashedMeshCount] = target.roomNumber;
						SmashedMesh[SmashedMeshCount] = mesh;
						++SmashedMeshCount;
						mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
						SoundEffect(GetShatterSound(mesh->staticNumber), (PHD_3DPOS *)mesh, 0);
					}
					TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, 0);
					TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, 0);
				}
				else
				{
					item = &g_Level.Items[itemNumber];
					if (item->objectNumber < ID_SHOOT_SWITCH1 || item->objectNumber > ID_SHOOT_SWITCH4)
					{
						if ((Objects[item->objectNumber].explodableMeshbits & ShatterItem.bit) 
							&& LaserSight)
						{
							//if (!Objects[item->objectNumber].intelligent)
							//{
								item->meshBits &= ~ShatterItem.bit;
								ShatterImpactData.impactDirection = direction;
								ShatterImpactData.impactLocation = Vector3(ShatterItem.sphere.x, ShatterItem.sphere.y, ShatterItem.sphere.z);
								ShatterObject(&ShatterItem, 0, 128, target.roomNumber, 0);
								TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, 0);
							/*}
							else
							{
								if (item->objectNumber != ID_GUARD_LASER)
								{
									item->hitPoints -= 30;
									if (item->hitPoints < 0)
										item->hitPoints = 0;
									HitTarget(item, &target, Weapons[Lara.gunType].damage, 0);
								}
								else
								{
									angle = phd_atan(LaraItem->pos.zPos - item->pos.zPos, LaraItem->pos.xPos - item->pos.xPos) - item->pos.yRot;
									if (angle > -ANGLE(90) && angle < ANGLE(90))
									{
										item->hitPoints = 0;
										HitTarget(item, &target, Weapons[Lara.gunType].damage, 0);
									}
								}
							}*/
						}
						else
						{
							if (DrawTarget && (Lara.gunType == WEAPON_REVOLVER || Lara.gunType == WEAPON_HK))
							{
								if (Objects[item->objectNumber].intelligent)
								{
									HitTarget(item, &target, Weapons[Lara.gunType].damage, 0);
								}
								else
								{
									// TR5
									if (Objects[item->objectNumber].hitEffect == HIT_RICOCHET)
										TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, 0);
								}
							}
							else
							{
								if (item->objectNumber >= ID_SMASH_OBJECT1 && item->objectNumber <= ID_SMASH_OBJECT8)
								{
									SmashObject(itemNumber);
								}
								else
								{
									if (Objects[item->objectNumber].hitEffect == HIT_BLOOD)
									{
										DoBloodSplat(target.x, target.y, target.z, (GetRandomControl() & 3) + 3, item->pos.yRot, item->roomNumber);
									}
									else if (Objects[item->objectNumber].hitEffect == HIT_SMOKE)
									{
										TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, -5);
									}
									else if (Objects[item->objectNumber].hitEffect == HIT_RICOCHET)
									{
										TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, 0);
									}
									item->hitStatus = true;
									if (!Objects[item->objectNumber].undead)
									{
										item->hitPoints -= Weapons[Lara.gunType].damage;
									}
								}
							}
						}
					}
					else
					{
						if (ShatterItem.bit == 1 << Objects[item->objectNumber].nmeshes - 1)
						{
							if (!(item->flags & 0x40))
							{
								if (item->objectNumber == ID_SHOOT_SWITCH1)
									ExplodeItemNode(item, Objects[item->objectNumber].nmeshes - 1, 0, 64);
								if (item->triggerFlags == 444 && item->objectNumber == ID_SHOOT_SWITCH2)
								{
									// TR5 ID_SWITCH_TYPE_8/ID_SHOOT_SWITCH2
									ProcessExplodingSwitchType8(item);
								}
								else 
								{
									/*if (item->objectNumber == ID_SHOOT_SWITCH3)
									{
										// TR4 ID_SWITCH_TYPE7
										ExplodeItemNode(item, Objects[item->objectNumber].nmeshes - 1, 0, 64);
									}*/

									if (item->flags & IFLAG_ACTIVATION_MASK && (item->flags & IFLAG_ACTIVATION_MASK) != IFLAG_ACTIVATION_MASK)
									{
										TestTriggers(item->pos.xPos, item->pos.yPos - 256, item->pos.zPos, item->roomNumber, true, item->flags & IFLAG_ACTIVATION_MASK);
									}
									else
									{
										for (count = GetSwitchTrigger(item, triggerItems, 1); count > 0; --count)
										{
											AddActiveItem(triggerItems[count - 1]);
											g_Level.Items[triggerItems[count - 1]].status = ITEM_ACTIVE;
											g_Level.Items[triggerItems[count - 1]].flags |= IFLAG_ACTIVATION_MASK;
										}
									}
								}
							}
							if (item->status != ITEM_DEACTIVATED)
							{
								AddActiveItem(itemNumber);
								item->status = ITEM_ACTIVE;
								item->flags |= IFLAG_ACTIVATION_MASK | 0x40;
							}
						}
						TriggerRicochetSpark(&target, LaraItem->pos.yRot, 3, 0);
					}
				}
			}
			else
			{
				if (LaserSight && firing)
				{
					FireCrossBowFromLaserSight(src, &target);
				}
			}
		}

		hit = 1;
	}
	else
	{
		if (Lara.gunType == WEAPON_CROSSBOW)
		{
			if (firing && LaserSight)
				FireCrossBowFromLaserSight(src, &target);
		}
		else
		{
			target.x -= target.x - src->x >> 5;
			target.y -= target.y - src->y >> 5;
			target.z -= target.z - src->z >> 5;
			if (firing && !result)
				TriggerRicochetSpark(&target, LaraItem->pos.yRot, 8, 0);
		}
	}

	if (DrawTarget && (hit || !result))
	{
		TriggerDynamicLight(target.x, target.y, target.z, 64, 255, 0, 0);
		LaserSightActive = 1;
		LaserSightX = target.x;
		LaserSightY = target.y;
		LaserSightZ = target.z;
	}

	return hit;
}

int ObjectOnLOS2(GAME_VECTOR *start, GAME_VECTOR *end, PHD_VECTOR *vec, MESH_INFO **mesh)
{
	int r, m;
	ROOM_INFO *room;
	short linknum;
	ITEM_INFO *item;
	PHD_3DPOS pos;
	MESH_INFO *meshp;
	BOUNDING_BOX* box;

	ClosestItem = NO_LOS_ITEM;
	ClosestDist = SQUARE(end->x - start->x) + SQUARE(end->y - start->y) + SQUARE(end->z - start->z);

	for (r = 0; r < number_los_rooms; ++r)
	{
		room = &g_Level.Rooms[los_rooms[r]];

		for (m = 0; m < room->mesh.size(); m++)
		{
			meshp = &room->mesh[m];

			if (meshp->flags & StaticMeshFlags::SM_VISIBLE)
			{
				pos.xPos = meshp->pos.xPos;
				pos.yPos = meshp->pos.yPos;
				pos.zPos = meshp->pos.zPos;
				pos.yRot = meshp->pos.yRot;

				if (DoRayBox(start, end, &StaticObjects[meshp->staticNumber].collisionBox, &pos, vec, -1 - meshp->staticNumber))
				{
					*mesh = meshp;
					end->roomNumber = los_rooms[r];
				}
			}
		}

		for (linknum = room->itemNumber; linknum != NO_ITEM; linknum = g_Level.Items[linknum].nextItem)
		{
			item = &g_Level.Items[linknum];

			if (item->status != ITEM_DEACTIVATED && item->status != ITEM_INVISIBLE
				&& (item->objectNumber != ID_LARA 
					&& Objects[item->objectNumber].collision != NULL 
					|| item->objectNumber == ID_LARA
					&& GetLaraOnLOS))
			{
				box = GetBoundsAccurate(item);

				pos.xPos = item->pos.xPos;
				pos.yPos = item->pos.yPos;
				pos.zPos = item->pos.zPos;
				pos.yRot = item->pos.yRot;

				if (DoRayBox(start, end, box, &pos, vec, linknum))
				{
					end->roomNumber = los_rooms[r];
				}
			}
		}
	}

	vec->x = ClosestCoord.x;
	vec->y = ClosestCoord.y;
	vec->z = ClosestCoord.z;

	return ClosestItem;
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

int DoRayBox(GAME_VECTOR *start, GAME_VECTOR *end, BOUNDING_BOX *box, PHD_3DPOS *itemOrStaticPos, PHD_VECTOR *hitPos, short closesItemNumber)
{
	// Ray
	FXMVECTOR rayStart = {start->x, start->y, start->z};
	FXMVECTOR rayEnd = {end->x, end->y, end->z};
	FXMVECTOR rayDir = {end->x - start->x, end->y - start->y, end->z - start->z};
	XMVECTOR rayDirNormalized = XMVector3Normalize(rayDir);

	// Create the bounding box for raw collision detection
	auto obox = TO_DX_BBOX(*itemOrStaticPos, box);

	// Get the collision with the bounding box
	float distance;
	bool collided = obox.Intersects(rayStart, rayDirNormalized, distance);

	// If no collision happened, then don't test spheres
	if (!collided)
		return 0;

	// Get the raw collision point
	Vector3 collidedPoint = rayStart + distance * rayDirNormalized;
	hitPos->x = collidedPoint.x - itemOrStaticPos->xPos;
	hitPos->y = collidedPoint.y - itemOrStaticPos->yPos;
	hitPos->z = collidedPoint.z - itemOrStaticPos->zPos;

	// Now in the case of items we need to test single spheres
	MESH* meshPtr = NULL;
	int bit = 0;
	int sp = -2;
	float minDistance = std::numeric_limits<float>::max();

	int action = TrInput & IN_ACTION;

	if (closesItemNumber < 0)
	{
		// Static meshes don't require further tests
		sp = -1;
		minDistance = distance;
	}
	else
	{
		// For items instead we need to test spheres
		ITEM_INFO *item = &g_Level.Items[closesItemNumber];
		OBJECT_INFO *obj = &Objects[item->objectNumber];

		// Get the ransformed sphere of meshes
		GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
		SPHERE spheres[34];
		memcpy(spheres, CreatureSpheres, sizeof(SPHERE) * 34);
		if (obj->nmeshes <= 0)
			return 0;

		meshPtr = &g_Level.Meshes[obj->meshIndex];

		for (int i = 0; i < obj->nmeshes; i++)
		{
			// If mesh is visibile...
			if (item->meshBits & (1 << i))
			{
				SPHERE *sphere = &CreatureSpheres[i];

				// TODO: this approach is the correct one but, again, Core's math is a mistery and this test was meant
				// to fail delberately in some way. I've so added again Core's legacy test for allowing the current game logic
				// but after more testing we should trash it in the future and restore the new way.
					
#if 0
				// Create the bounding sphere and test it against the ray
				BoundingSphere sph = BoundingSphere(Vector3(sphere->x, sphere->y, sphere->z), sphere->r);
				float newDist;
				if (sph.Intersects(rayStart, rayDirNormalized, newDist))
				{
					// HACK: Core seems to take in account for distance not the real hit point but the centre of the sphere.
					// This can work well for example for GUARDIAN because the head sphere is so big that would always be hit
					// and eyes would not be destroyed.
					newDist = sqrt(SQUARE(sphere->x - start->x) + SQUARE(sphere->y - start->y) + SQUARE(sphere->z - start->z));

					// Test for min distance
					if (newDist < minDistance)
					{
						minDistance = newDist;
						meshPtr = &g_Level.Meshes[obj->meshIndex + i];
						bit = 1 << i;
						sp = i;
					}
				}
#endif

				PHD_VECTOR p[4];

				p[1].x = start->x;
				p[1].y = start->y;
				p[1].z = start->z;
				p[2].x = end->x;
				p[2].y = end->y;
				p[2].z = end->z;
				p[3].x = sphere->x;
				p[3].y = sphere->y;
				p[3].z = sphere->z;

				int r0 = (p[3].x - p[1].x) * (p[2].x - p[1].x) + 
					(p[3].y - p[1].y) * (p[2].y - p[1].y) +
					(p[3].z - p[1].z) * (p[2].z - p[1].z);

				int r1 = SQUARE(p[2].x - p[1].x) + 
					SQUARE(p[2].y - p[1].y) + 
					SQUARE(p[2].z - p[1].z);

				if (((r0 < 0 && r1 < 0) 
					|| (r1 > 0 && r0 > 0))
					&& (abs(r0) <= abs(r1))) 
				{
					r1 >>= 16;
					if (r1)
						r0 /= r1;
					else
						r0 = 0;

					p[0].x = p[1].x + ((r0 * (p[2].x - p[1].x)) >> 16);
					p[0].y = p[1].y + ((r0 * (p[2].y - p[1].y)) >> 16);
					p[0].z = p[1].z + ((r0 * (p[2].z - p[1].z)) >> 16);
					
					int dx = p[0].x - p[3].x;
					int dy = p[0].y - p[3].y;
					int dz = p[0].z - p[3].z;

					int distance = dx + dy + dz;

					if (distance < SQUARE(sphere->r))
					{
						dx = SQUARE(sphere->x - start->x);
						dy = SQUARE(sphere->y - start->y);
						dz = SQUARE(sphere->z - start->z);

						distance = dx + dy + dz;

						if (distance < minDistance)
						{
							minDistance = distance;
							meshPtr = &g_Level.Meshes[obj->meshIndex + i];
							bit = 1 << i;
							sp = i;
						}
					}
				}
			}
		}

		if (sp < -1)
			return 0;
	}

	if (distance >= ClosestDist)
		return 0;

	// Setup test result
	ClosestCoord.x = hitPos->x + itemOrStaticPos->xPos;
	ClosestCoord.y = hitPos->y + itemOrStaticPos->yPos;
	ClosestCoord.z = hitPos->z + itemOrStaticPos->zPos;
	ClosestDist = distance;
	ClosestItem = closesItemNumber;

	// If collided object is an item, then setup the shatter item data struct
	if (sp >= 0)
	{
		ITEM_INFO *item = &g_Level.Items[closesItemNumber];

		GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD | SPHERES_SPACE_BONE_ORIGIN, Matrix::Identity);

		ShatterItem.yRot = item->pos.yRot;
		ShatterItem.meshp = meshPtr;
		ShatterItem.sphere.x = CreatureSpheres[sp].x;
		ShatterItem.sphere.y = CreatureSpheres[sp].y;
		ShatterItem.sphere.z = CreatureSpheres[sp].z;
		ShatterItem.bit = bit;
		ShatterItem.flags = 0;
	}

	return 1;
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
				effect_routines[effectID](item);

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

void DoFlipMap(short group)
{
	ROOM_INFO temp;

	for (size_t i = 0; i < g_Level.Rooms.size(); i++)
	{
		ROOM_INFO *r = &g_Level.Rooms[i];

		if (r->flippedRoom >= 0 && r->flipNumber == group)
		{
			RemoveRoomFlipItems(r);

			ROOM_INFO *flipped = &g_Level.Rooms[r->flippedRoom];

			temp = *r;
			*r = *flipped;
			*flipped = temp;

			r->flippedRoom = flipped->flippedRoom;
			flipped->flippedRoom = -1;

			r->itemNumber = flipped->itemNumber;
			r->fxNumber = flipped->fxNumber;

			AddRoomFlipItems(r);

			g_Renderer.flipRooms(static_cast<short>(i), r->flippedRoom);

			for (auto& fd : r->floor)
				fd.Room = i;
			for (auto& fd : flipped->floor)
				fd.Room = r->flippedRoom;
		}
	}

	int status = FlipStats[group] == 0;
	FlipStats[group] = status;
	FlipStatus = status;

	for (int i = 0; i < ActiveCreatures.size(); i++)
	{
		ActiveCreatures[i]->LOT.targetBox = NO_BOX;
	}
}

void AddRoomFlipItems(ROOM_INFO *r)
{
	for (short linkNum = r->itemNumber; linkNum != NO_ITEM; linkNum = g_Level.Items[linkNum].nextItem)
	{
		ITEM_INFO *item = &g_Level.Items[linkNum];

		//if (item->objectNumber == ID_RAISING_BLOCK1 && item->itemFlags[1])
		//	AlterFloorHeight(item, -1024);

		if (item->objectNumber == ID_RAISING_BLOCK2)
		{
			//if (item->itemFlags[1])
			//	AlterFloorHeight(item, -2048);
		}
	}
}

void RemoveRoomFlipItems(ROOM_INFO *r)
{
	for (short linkNum = r->itemNumber; linkNum != NO_ITEM; linkNum = g_Level.Items[linkNum].nextItem)
	{
		ITEM_INFO *item = &g_Level.Items[linkNum];

		if (item->flags & 0x100 && Objects[item->objectNumber].intelligent && item->hitPoints <= 0 && item->hitPoints != NOT_TARGETABLE)
		{
			KillItem(linkNum);
		}
	}
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

int is_object_in_room(short roomNumber, short objectNumber)
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

void InterpolateAngle(short angle, short *rotation, short *outAngle, int shift)
{
	int deltaAngle = angle - *rotation;

	if (deltaAngle < -32768)
		deltaAngle += 65536;
	else if (deltaAngle > 32768)
		deltaAngle -= 65536;

	if (outAngle)
		*outAngle = static_cast<short>(deltaAngle);

	*rotation += static_cast<short>(deltaAngle >> shift);
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

	// ClearSky resets lightning parameters so user won't see prev lightning in new level
	TEN::Effects::Sky::Sky.ClearSky();

	// Needs to be cleared because otherwise a list of active creatures from previous level
	// will spill into new level
	ActiveCreatures.clear();

	// Clear spotcam array
	ClearSpotCamSequences();
}

