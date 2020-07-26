#include "framework.h"
#include "laramisc.h"
#include "level.h"
#include "setup.h"
#include "GameFlowScript.h"
#include "effect.h"
#include "collide.h"
#include "lara.h"
#include "laraswim.h"
#include "larasurf.h"
#include "effect2.h"
#include "health.h"
#include "misc.h"
#include "rope.h"
#include "draw.h"
#include "savegame.h"
#include "inventory.h"
#include "camera.h"
#include "input.h"
#include "sound.h"
using T5M::Renderer::g_Renderer;
extern GameFlow* g_GameFlow;
extern short FXType;
COLL_INFO lara_coll;
short SubsuitAir = 0;
short cheatHitPoints;

void GetLaraDeadlyBounds() // (F) (D)
{
	BOUNDING_BOX* bounds;
	BOUNDING_BOX tbounds;

	bounds = GetBoundsAccurate(LaraItem);
	phd_RotBoundingBoxNoPersp(&LaraItem->pos, bounds, &tbounds);

	DeadlyBounds[0] = LaraItem->pos.xPos + tbounds.X1;
	DeadlyBounds[1] = LaraItem->pos.xPos + tbounds.X2;
	DeadlyBounds[2] = LaraItem->pos.yPos + tbounds.Y1;
	DeadlyBounds[3] = LaraItem->pos.yPos + tbounds.Y2;
	DeadlyBounds[4] = LaraItem->pos.zPos + tbounds.Z1;
	DeadlyBounds[5] = LaraItem->pos.zPos + tbounds.Z2;
}

void InitialiseLaraAnims(ITEM_INFO* item) // (F) (D)
{
	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		Lara.waterStatus = LW_UNDERWATER;
		item->goalAnimState = STATE_LARA_UNDERWATER_STOP;
		item->currentAnimState = STATE_LARA_UNDERWATER_STOP;
		item->fallspeed = 0;
		item->animNumber = ANIMATION_LARA_UNDERWATER_IDLE;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	}
	else
	{
		Lara.waterStatus = LW_ABOVE_WATER;
		item->goalAnimState = STATE_LARA_STOP;
		item->currentAnimState = STATE_LARA_STOP;
		item->animNumber = ANIMATION_LARA_STAY_SOLID;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	}
}

void InitialiseLaraLoad(short itemNum) // (F) (D)
{
	Lara.itemNumber = itemNum;
	LaraItem = &g_Level.Items[itemNum];
}

void DelsGiveLaraItemsCheat() // (AF) (D)
{
	int i;

	for (i = 0; i < 8; ++i)
	{
		if (Objects[ID_PUZZLE_ITEM1 + i].loaded)
			Lara.Puzzles[i] = 1;
		Lara.PuzzlesCombo[2 * i] = false;
		Lara.PuzzlesCombo[2 * i + 1] = false;
	}
	for (i = 0; i < 8; ++i)
	{
		if (Objects[ID_KEY_ITEM1 + i].loaded)
			Lara.Keys[i] = 1;
		Lara.KeysCombo[2 * i] = false;
		Lara.KeysCombo[2 * i + 1] = false;
	}
	for (i = 0; i < 3; ++i)
	{
		if (Objects[ID_PICKUP_ITEM1 + i].loaded)
			Lara.Pickups[i] = 1;
		Lara.PickupsCombo[2 * i] = false;
		Lara.PickupsCombo[2 * i + 1] = false;
	}

	g_Inventory.LoadObjects(false);

	/* Hardcoded code */
}

void LaraCheatGetStuff() // (F) (D)
{
	Lara.NumFlares = -1;
	Lara.NumSmallMedipacks = -1;
	Lara.NumLargeMedipacks = -1;

	if (Objects[ID_CROWBAR_ITEM].loaded)
		Lara.Crowbar = true;

	if (Objects[ID_LASERSIGHT_ITEM].loaded)
	Lara.Lasersight = true;

	if (Objects[ID_REVOLVER_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_REVOLVER].Present = true;
		Lara.Weapons[WEAPON_REVOLVER].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_REVOLVER].HasLasersight = false;
		Lara.Weapons[WEAPON_REVOLVER].HasSilencer = false;
		Lara.Weapons[WEAPON_REVOLVER].Ammo[WEAPON_AMMO1] = -1;
	}

	if (Objects[ID_UZI_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_UZI].Present = true;
		Lara.Weapons[WEAPON_UZI].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_UZI].HasLasersight = false;
		Lara.Weapons[WEAPON_UZI].HasSilencer = false;
		Lara.Weapons[WEAPON_UZI].Ammo[WEAPON_AMMO1] = -1;
	}

	if (Objects[ID_SHOTGUN_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_SHOTGUN].Present = true;
		Lara.Weapons[WEAPON_SHOTGUN].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_SHOTGUN].HasLasersight = false;
		Lara.Weapons[WEAPON_SHOTGUN].HasSilencer = false;
		Lara.Weapons[WEAPON_SHOTGUN].Ammo[WEAPON_AMMO1] = -1;
	}

/*	if (Objects[ID_HARPOON_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_HARPOON_GUN].Present = true;
		Lara.Weapons[WEAPON_HARPOON_GUN].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_HARPOON_GUN].HasLasersight = false;
		Lara.Weapons[WEAPON_HARPOON_GUN].HasSilencer = false;
		Lara.Weapons[WEAPON_HARPOON_GUN].Ammo[WEAPON_AMMO1] = -1;
	}

	if (Objects[ID_GRENADE_GUN_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Present = true;
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].HasSilencer = false;
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO1] = -1;
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO2] = -1;
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO3] = -1;
	}

	if (Objects[ID_ROCKET_LAUNCHER_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_ROCKET_LAUNCHER].Present = true;
		Lara.Weapons[WEAPON_ROCKET_LAUNCHER].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_ROCKET_LAUNCHER].HasLasersight = false;
		Lara.Weapons[WEAPON_ROCKET_LAUNCHER].HasSilencer = false;
		Lara.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[WEAPON_AMMO1] = -1;
	}*/

	if (Objects[ID_HK_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_HK].Present = true;
		Lara.Weapons[WEAPON_HK].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_HK].HasLasersight = false;
		Lara.Weapons[WEAPON_HK].HasSilencer = false;
		Lara.Weapons[WEAPON_HK].Ammo[WEAPON_AMMO1] = -1;
	}

/*	if (Objects[ID_CROSSBOW_ITEM].loaded)
	{
		Lara.Weapons[ID_CROSSBOW_ITEM].Present = true;
		Lara.Weapons[ID_CROSSBOW_ITEM].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[ID_CROSSBOW_ITEM].HasLasersight = false;
		Lara.Weapons[ID_CROSSBOW_ITEM].HasSilencer = false;
		Lara.Weapons[ID_CROSSBOW_ITEM].Ammo[WEAPON_AMMO1] = -1;
		Lara.Weapons[ID_CROSSBOW_ITEM].Ammo[WEAPON_AMMO2] = -1;
		Lara.Weapons[ID_CROSSBOW_ITEM].Ammo[WEAPON_AMMO3] = -1;
	}*/

	/*Commented out the blocks for weapons that don't work ingame, after they're fixed, it'll be okay to uncomment*/

	g_Inventory.LoadObjects(false);
}

void LaraCheatyBits() // (F) (D)
{
	if (g_GameFlow->FlyCheat)
	{
#ifdef _DEBUG
		if (TrInput & IN_PAUSE)
#else
		if (TrInput & IN_D)
#endif
		{
			LaraCheatGetStuff();
			LaraItem->hitPoints = 1000;
		}

#ifdef _DEBUG
		if (TrInput & IN_PAUSE)
#else
		if (TrInput & IN_CHEAT)
#endif
		{
			DelsGiveLaraItemsCheat();
			LaraItem->pos.yPos -= 128;
			if (Lara.waterStatus != LW_FLYCHEAT)
			{
				Lara.waterStatus = LW_FLYCHEAT;
				LaraItem->animNumber = ANIMATION_LARA_DOZY;
				LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
				LaraItem->currentAnimState = ANIMATION_LARA_ONWATER_IDLE_TO_SWIM;
				LaraItem->goalAnimState = ANIMATION_LARA_ONWATER_IDLE_TO_SWIM;
				LaraItem->gravityStatus = false;
				LaraItem->pos.xRot = ANGLE(30);
				LaraItem->fallspeed = 30;
				Lara.air = 1800;
				Lara.deathCount = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				cheatHitPoints = LaraItem->hitPoints;
			}
		}
	}
}

void LaraControl(short itemNumber) // (AF) (D)
{
	ITEM_INFO* item = LaraItem;

	LaraCheatyBits();

	if (Lara.isMoving)
	{
		if (Lara.moveCount > 90)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		} 
		++Lara.moveCount;
	} 

	if (!DisableLaraControl)
		Lara.locationPad = 128;

	int oldX = LaraItem->pos.xPos; 
	int oldY = LaraItem->pos.yPos; 
	int oldZ = LaraItem->pos.zPos; 

	if (Lara.gunStatus == LG_HANDS_BUSY && 
		LaraItem->currentAnimState == STATE_LARA_STOP &&
		LaraItem->goalAnimState == STATE_LARA_STOP && 
		LaraItem->animNumber == ANIMATION_LARA_STAY_IDLE &&
	   !LaraItem->gravityStatus)
	{
		Lara.gunStatus = LG_NO_ARMS;
	}

	if (item->currentAnimState != STATE_LARA_SPRINT && DashTimer < 120)
		DashTimer++;

	Lara.isDucked = false;

#if 1
	bool isWater = g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER;
#else
	bool isWater = g_Level.Rooms[item->roomNumber].flags & (ENV_FLAG_WATER|ENV_FLAG_SWAMP);
#endif

	int wd = GetWaterDepth(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
	int wh = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);

	int hfw;
	if (wh != NO_HEIGHT)
		hfw = item->pos.yPos - wh;
	else
		hfw = NO_HEIGHT;
	Lara.waterSurfaceDist = -hfw;
	
#if 0
	if (Lara.Vehicle == NO_ITEM)
#endif
		WadeSplash(item, wh, wd);
	
	short roomNumber;

#if 0
	if (Lara.Vehicle == NO_ITEM && Lara.ExtraAnim == 0)
	{
#endif
		switch (Lara.waterStatus)
		{
			case LW_ABOVE_WATER:
				if (hfw != NO_HEIGHT && hfw >= STEP_SIZE)
				{
					if (wd <= 474)
					{
						if (hfw > 256)
						{
							Lara.waterStatus = LW_WADE;
							if (!(item->gravityStatus))
							{
								item->goalAnimState = STATE_LARA_STOP;
							}
#if 0
							else if (isWater & ENV_FLAG_SWAMP)
							{
								if (item->currentAnimState == STATE_LARA_SWANDIVE_BEGIN || item->currentAnimState == STATE_LARA_SWANDIVE_END)			// Is Lara swan-diving?
									item->pos.yPos = wh + 1000;

								item->goalAnimState = STATE_LARA_WADE_FORWARD;
								item->currentAnimState = STATE_LARA_WADE_FORWARD;
								item->animNumber = ANIMATION_LARA_WADE;
								item->frameNumber = GF(ANIMATION_LARA_WADE, 0);
							}
#endif
						}
					}
#if 1
					else if (isWater)
#else
					else if (!(isWater & ENV_FLAG_SWAMP))
#endif
					{
						Lara.air = 1800;
						Lara.waterStatus = LW_UNDERWATER;
						item->gravityStatus = false;
						item->pos.yPos += 100;

						UpdateLaraRoom(LaraItem, 0);
						StopSoundEffect(SFX_LARA_FALL);

						if (item->currentAnimState == STATE_LARA_SWANDIVE_BEGIN)
						{
							item->pos.xRot = -ANGLE(45);
							item->goalAnimState = STATE_LARA_UNDERWATER_DIVING;
							AnimateLara(item);
							item->fallspeed *= 2;
						}
						else if (item->currentAnimState == STATE_LARA_SWANDIVE_END)
						{
							item->pos.xRot = -ANGLE(85);
							item->goalAnimState = STATE_LARA_UNDERWATER_DIVING;
							AnimateLara(item);
							item->fallspeed *= 2;
						}
						else
						{
							item->pos.xRot = -ANGLE(45);
							item->animNumber = ANIMATION_LARA_FREE_FALL_TO_UNDERWATER;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->currentAnimState = STATE_LARA_UNDERWATER_DIVING;
							item->goalAnimState = STATE_LARA_UNDERWATER_FORWARD;
							item->fallspeed = 3 * item->fallspeed / 2;
						}

						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;
						Lara.headYrot = 0;
						Lara.headXrot = 0;

						Splash(LaraItem);
					}

					Camera.targetElevation = -ANGLE(22);
					if (hfw >= 256) /* @ORIGINAL_BUG: checking hfw for equality with 256 results in the wade bug */
					{
						if (hfw > 730)
						{
							Lara.waterStatus = LW_SURFACE;
							item->pos.yPos += 1 - hfw;

							switch (item->currentAnimState)
							{
							case STATE_LARA_WALK_BACK:
								item->animNumber = ANIMATION_LARA_ONWATER_IDLE_TO_SWIM_BACK;
								item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
								item->goalAnimState = STATE_LARA_ONWATER_BACK;
								item->currentAnimState = STATE_LARA_ONWATER_BACK;
								break;

							case STATE_LARA_WALK_RIGHT:
								item->animNumber = ANIMATION_LARA_ONWATER_SWIM_RIGHT;
								item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
								item->goalAnimState = STATE_LARA_ONWATER_RIGHT;
								item->currentAnimState = STATE_LARA_ONWATER_RIGHT;
								break;

							case STATE_LARA_WALK_LEFT:
								item->animNumber = ANIMATION_LARA_ONWATER_SWIM_LEFT;
								item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
								item->goalAnimState = STATE_LARA_ONWATER_LEFT;
								item->currentAnimState = STATE_LARA_ONWATER_LEFT;
								break;

							default:
								item->animNumber = ANIMATION_LARA_ONWATER_SWIM_FORWARD;
								item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
								item->goalAnimState = STATE_LARA_ONWATER_FORWARD;
								item->currentAnimState = STATE_LARA_ONWATER_FORWARD;
								break;
							}

							item->gravityStatus = false;
							item->fallspeed = 0;
							Lara.diveCount = 0;
							LaraItem->pos.zRot = 0;
							LaraItem->pos.xRot = 0;
							Lara.torsoYrot = 0;
							Lara.torsoXrot = 0;
							Lara.headYrot = 0;
							Lara.headXrot = 0;

							UpdateLaraRoom(item, 0);
						}
					}
					else
					{
						Lara.waterStatus = LW_ABOVE_WATER;
						if (item->currentAnimState == STATE_LARA_WADE_FORWARD)
							item->goalAnimState = STATE_LARA_RUN_FORWARD;
					}
				}

				break;

			case LW_UNDERWATER:
				roomNumber = item->roomNumber;
				GetFloor(item->pos.xPos, item->pos.yPos - 256, item->pos.zPos, &roomNumber);

				if (wd == NO_HEIGHT
					|| abs(hfw) >= 256
					|| g_Level.Rooms[roomNumber].flags & ENV_FLAG_WATER
					|| item->animNumber == ANIMATION_LARA_UNDERWATER_TO_ONWATER
					|| item->animNumber == ANIMATION_LARA_FREE_FALL_TO_UNDERWATER_ALTERNATE)
				{
					if (!isWater)
					{
						if (wd == NO_HEIGHT || abs(hfw) >= 256)
						{
							Lara.waterStatus = LW_ABOVE_WATER;
							item->animNumber = ANIMATION_LARA_FREE_FALL_FORWARD;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->goalAnimState = STATE_LARA_JUMP_FORWARD;
							item->currentAnimState = STATE_LARA_JUMP_FORWARD;
							item->speed = item->fallspeed / 4;
							item->gravityStatus = true;

							item->fallspeed = 0;
							LaraItem->pos.zRot = 0;
							LaraItem->pos.xRot = 0;
							Lara.torsoYrot = 0;
							Lara.torsoXrot = 0;
							Lara.headYrot = 0;
							Lara.headXrot = 0;
						}
						else
						{
							Lara.waterStatus = LW_SURFACE;
							item->pos.yPos = wh;
							item->animNumber = ANIMATION_LARA_UNDERWATER_TO_ONWATER;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->goalAnimState = STATE_LARA_ONWATER_STOP;
							item->currentAnimState = STATE_LARA_ONWATER_STOP;
							item->fallspeed = 0;
							Lara.diveCount = 11;
							LaraItem->pos.zRot = 0;
							LaraItem->pos.xRot = 0;
							Lara.torsoYrot = 0;
							Lara.torsoXrot = 0;
							Lara.headYrot = 0;
							Lara.headXrot = 0;

							UpdateLaraRoom(item, -381);
							SoundEffect(SFX_LARA_BREATH, &LaraItem->pos, 2);
						}
					}
				}
				else
				{
					Lara.waterStatus = LW_SURFACE;
					item->pos.yPos = wh + 1;
					item->animNumber = ANIMATION_LARA_UNDERWATER_TO_ONWATER;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->goalAnimState = STATE_LARA_ONWATER_STOP;
					item->currentAnimState = STATE_LARA_ONWATER_STOP;
					item->fallspeed = 0;
					Lara.diveCount = 11;
					LaraItem->pos.zRot = 0;
					LaraItem->pos.xRot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.headYrot = 0;
					Lara.headXrot = 0;

					UpdateLaraRoom(item, 0);
					SoundEffect(SFX_LARA_BREATH, &LaraItem->pos, 2);
				}
				break;

			case LW_SURFACE:
				if (!isWater)
				{
					if (hfw <= 256)
					{
						Lara.waterStatus = LW_ABOVE_WATER;
						item->animNumber = ANIMATION_LARA_FREE_FALL_FORWARD;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->goalAnimState = STATE_LARA_JUMP_FORWARD;
						item->currentAnimState = STATE_LARA_JUMP_FORWARD;
						item->speed = item->fallspeed / 4;
						item->gravityStatus = true;
					}
					else
					{
						Lara.waterStatus = LW_WADE; /* @DEAD_CODE: Lara has to reach a room without water while in the surface but then GetWaterHeight() return value never will make hfw > 256 */
						item->animNumber = ANIMATION_LARA_STAY_IDLE;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->goalAnimState = STATE_LARA_WADE_FORWARD;
						item->currentAnimState = STATE_LARA_STOP;

						AnimateItem(item);
					}

					item->fallspeed = 0;
					LaraItem->pos.zRot = 0;
					LaraItem->pos.xRot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.headYrot = 0;
					Lara.headXrot = 0;
				}
				break;

			case LW_WADE:
				Camera.targetElevation = -ANGLE(22);
				if (hfw >= 256) /* @ORIGINAL_BUG: checking hfw for equality with 256 results in the wade bug */
				{
#if 1
					if (hfw > 730)
#else
					if (hfw > 730 && !(isWater & ENV_FLAG_SWAMP))
#endif
					{
						Lara.waterStatus = LW_SURFACE;
						item->pos.yPos += 1 - hfw;

						switch (item->currentAnimState)
						{
						case STATE_LARA_WALK_BACK:
							item->animNumber = ANIMATION_LARA_ONWATER_IDLE_TO_SWIM_BACK;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->goalAnimState = STATE_LARA_ONWATER_BACK;
							item->currentAnimState = STATE_LARA_ONWATER_BACK;
							break;

						case STATE_LARA_WALK_RIGHT:
							item->animNumber = ANIMATION_LARA_ONWATER_SWIM_RIGHT;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->goalAnimState = STATE_LARA_ONWATER_RIGHT;
							item->currentAnimState = STATE_LARA_ONWATER_RIGHT;
							break;

						case STATE_LARA_WALK_LEFT:
							item->animNumber = ANIMATION_LARA_ONWATER_SWIM_LEFT;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->goalAnimState = STATE_LARA_ONWATER_LEFT;
							item->currentAnimState = STATE_LARA_ONWATER_LEFT;
							break;

						default:
							item->animNumber = ANIMATION_LARA_ONWATER_SWIM_FORWARD;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->goalAnimState = STATE_LARA_ONWATER_FORWARD;
							item->currentAnimState = STATE_LARA_ONWATER_FORWARD;
							break;
						}

						item->gravityStatus = false;
						item->fallspeed = 0;
						Lara.diveCount = 0;
						LaraItem->pos.zRot = 0;
						LaraItem->pos.xRot = 0;
						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;
						Lara.headYrot = 0;
						Lara.headXrot = 0;

						UpdateLaraRoom(item, 0);
					}
				}
				else
				{
					Lara.waterStatus = LW_ABOVE_WATER;
					if (item->currentAnimState == STATE_LARA_WADE_FORWARD)
						item->goalAnimState = STATE_LARA_RUN_FORWARD;
				}
				break;
		}
#if 0
	}
#endif

	if (item->hitPoints <= 0)
	{
		item->hitPoints = -1;

		if (Lara.deathCount == 0)
			S_CDStop();

		Lara.deathCount++;
		if ((LaraItem->flags & 0x100))
		{
			Lara.deathCount++;
			return;
		}
	}

	switch (Lara.waterStatus)
	{
	case LW_ABOVE_WATER:
	case LW_WADE:
#if 0
		if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && Lara.waterSurfaceDist < -775)
		{
			if (item->hitPoints >= 0)
			{
				Lara.air -= 6;

				if (Lara.air < 0)
				{
					Lara.air = -1;
					item->hitPoints -= 10;
				}
			}
		}
		else if (Lara.gassed)
#else
		if (Lara.gassed)
#endif
		{
			if (item->hitPoints >= 0 && --Lara.air < 0)
			{
				Lara.air = -1;
				item->hitPoints -= 5;
			}
		}
		else if (Lara.air < 1800 && item->hitPoints >= 0)
		{
#if 0
			/* lara is not equipped with any vehicle */
			if (Lara.Vehicle == NO_ITEM) // only for the upv !!
			{
#endif
				Lara.air += 10;
				if (Lara.air > 1800)
					Lara.air = 1800;
#if 0
			}
#endif
		}
		LaraAboveWater(item, &lara_coll);
		break;

	case LW_UNDERWATER:
		if (item->hitPoints >= 0)
		{
			if (LaraDrawType == LARA_DIVESUIT)
			{
				/* Hardcoded code */
			}
			else
			{
				Lara.air--;
			}
			if (Lara.air < 0)
			{
				if (LaraDrawType == LARA_DIVESUIT && Lara.anxiety < 251)
					Lara.anxiety += 4;
				Lara.air = -1;
				item->hitPoints -= 5;
			}
		}		
		LaraUnderWater(item, &lara_coll);
		break;

	case LW_SURFACE:
		if (item->hitPoints >= 0)
		{
			Lara.air += 10;
			if (Lara.air > 1800)
				Lara.air = 1800;
		}
		LaraSurface(item, &lara_coll);
		break;

	case LW_FLYCHEAT:
		LaraCheat(item, &lara_coll);
		break;
	}

	Savegame.Game.Distance += sqrt(
		SQUARE(item->pos.xPos - oldX) +
		SQUARE(item->pos.yPos - oldY) +
		SQUARE(item->pos.zPos - oldZ));
}

void LaraCheat(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	LaraItem->hitPoints = 1000;
	LaraUnderWater(item, coll);
	if (TrInput & IN_WALK && !(TrInput & IN_LOOK))
	{
		Lara.waterStatus = LW_ABOVE_WATER;
		item->animNumber = ANIMATION_LARA_STAY_SOLID;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->pos.zRot = 0;
		item->pos.xRot = 0;
		Lara.torsoYrot = 0;
		Lara.torsoXrot = 0;
		Lara.headYrot = 0;
		Lara.headXrot = 0;
		Lara.gunStatus = LG_NO_ARMS;
		LaraInitialiseMeshes();
		Lara.meshEffects = 0;
		LaraItem->hitPoints = cheatHitPoints;
	}
}

void LaraInitialiseMeshes() // (AF) (D)
{
	for (int i = 0; i < NUM_LARA_MESHES; i++)
	{
		//Meshes[i] = Meshes[MESHES(ID_LARA_SKIN, i)];
		//LARA_MESHES(ID_LARA, MESHES(ID_LARA_SKIN, i));
		Lara.meshPtrs[i] = Objects[ID_LARA_SKIN].meshIndex + i;
	}

	/* Hardcoded code */

	if (Lara.gunType == WEAPON_HK)
	{
		Lara.backGun = WEAPON_HK;
	}
	else if (!Lara.Weapons[WEAPON_SHOTGUN].Present)
	{
		if (Lara.Weapons[WEAPON_HK].Present)
			Lara.backGun = WEAPON_HK;
	}
	else
	{
		Lara.backGun = WEAPON_UZI;
	}

	Lara.gunStatus = LG_NO_ARMS;
	Lara.leftArm.frameNumber = 0;
	Lara.rightArm.frameNumber = 0;
	Lara.target = NULL;
	Lara.rightArm.lock = false;
	Lara.leftArm.lock = false;
}

void InitialiseLara(int restore)
{
	if (Lara.itemNumber == NO_ITEM)
		return;

	short itemNumber = Lara.itemNumber;

	LaraItem->data = &Lara;
	LaraItem->collidable = false;

	if (restore)
	{
		LaraInfo backup;
		memcpy(&backup, &Lara, sizeof(LaraInfo));
		ZeroMemory(&Lara, sizeof(LaraInfo));
	}
	else
	{
		ZeroMemory(&Lara, sizeof(LaraInfo));
		Lara.ExtraAnim = NO_ITEM;
		Lara.Vehicle = NO_ITEM;
	}

	Lara.look = true;
	Lara.itemNumber = itemNumber;
	Lara.hitDirection = -1;
	Lara.air = 1800;
	Lara.weaponItem = NO_ITEM;
	PoisonFlag = 0;
	Lara.dpoisoned = 0;
	Lara.poisoned = 0;
	Lara.waterSurfaceDist = 100;
	Lara.holster = ID_LARA_HOLSTERS_PISTOLS;
	Lara.location = -1;
	Lara.highestLocation = -1;
	Lara.ropePtr = -1;
	LaraItem->hitPoints = 1000;
	Lara.gunStatus = LG_NO_ARMS;
	Lara.skelebob = 0;

	LARA_WEAPON_TYPE gun = WEAPON_NONE;

	if (Objects[ID_HK_ITEM].loaded)
		gun = WEAPON_HK;
	
	if (Objects[ID_PISTOLS_ITEM].loaded)
		gun = WEAPON_PISTOLS;
	
	Lara.lastGunType = Lara.gunType = Lara.requestGunType = gun;

	LaraInitialiseMeshes();

	if (gun == WEAPON_PISTOLS)
	{
		Lara.Weapons[WEAPON_PISTOLS].Present = true;
		Lara.Weapons[WEAPON_PISTOLS].Ammo[WEAPON_AMMO1] = -1;
	}
	else if (gun == WEAPON_HK)
	{
		Lara.Weapons[WEAPON_HK].Present = true;
		Lara.Weapons[WEAPON_HK].Ammo[WEAPON_AMMO1] = 100;
	}

	Lara.Binoculars = true;

	if (!restore)
	{
		if (Objects[ID_FLARE_INV_ITEM].loaded)
			Lara.NumFlares = 3;

		Lara.NumSmallMedipacks = 3;
		Lara.NumLargeMedipacks = 1;
	}

	InitialiseLaraAnims(LaraItem);

	DashTimer = 120;

	Lara.bottle = 0;
	Lara.wetcloth = CLOTH_MISSING;
}

void AnimateLara(ITEM_INFO* item)
{
	item->frameNumber++;

	ANIM_STRUCT* anim = &g_Level.Anims[item->animNumber];
	if (anim->numberChanges > 0 && GetChange(item, anim))
	{
		anim = &g_Level.Anims[item->animNumber];
		item->currentAnimState = anim->currentAnimState;
	}

	if (item->frameNumber > anim->frameEnd)
	{
		if (anim->numberCommands > 0)
		{
			short* cmd = &g_Level.Commands[anim->commandIndex];
			for (int i = anim->numberCommands; i > 0; i--)
			{
				switch (*(cmd++))
				{
				case COMMAND_MOVE_ORIGIN:
					TranslateItem(item, cmd[0], cmd[1], cmd[2]);
					UpdateLaraRoom(item, -LARA_HITE/2);
					cmd += 3;
					break;

				case COMMAND_JUMP_VELOCITY:
					item->fallspeed = *(cmd++);
					item->speed = *(cmd++);
					item->gravityStatus = true;
					if (Lara.calcFallSpeed)
					{
						item->fallspeed = Lara.calcFallSpeed;
						Lara.calcFallSpeed = 0;
					}
					break;

				case COMMAND_ATTACK_READY:
					if (Lara.gunStatus != LG_SPECIAL)
						Lara.gunStatus = LG_NO_ARMS;
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
		item->currentAnimState = anim->currentAnimState;
	}

	if (anim->numberCommands > 0)
	{
		short* cmd = &g_Level.Commands[anim->commandIndex];
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
				if (flags == SFX_LANDANDWATER ||
				   (flags == SFX_LANDONLY && (Lara.waterSurfaceDist >= 0 || Lara.waterSurfaceDist == NO_HEIGHT)) ||
				   (flags == SFX_WATERONLY && Lara.waterSurfaceDist < 0 && Lara.waterSurfaceDist != NO_HEIGHT && !(g_Level.Rooms[LaraItem->roomNumber].flags & ENV_FLAG_SWAMP)))
				{
					SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
				}

				cmd += 2;
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

	int lateral = anim->Xvelocity;
	if (anim->Xacceleration)
		lateral += anim->Xacceleration * (item->frameNumber - anim->frameBase);
	lateral >>= 16;
	
	if (item->gravityStatus)             // If gravity ON (Do Up/Down movement)
	{
		if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
		{
			item->speed -= item->speed >> 3;
			if (abs(item->speed) < 8)
			{
				item->speed = 0;
				item->gravityStatus = false;
			}
			if (item->fallspeed > 128)
				item->fallspeed >>= 1;
			item->fallspeed -= item->fallspeed >> 2;
			if (item->fallspeed < 4)
				item->fallspeed = 4;
			item->pos.yPos += item->fallspeed;
		}
		else
		{
			int velocity = (anim->velocity + anim->acceleration * (item->frameNumber - anim->frameBase - 1));
			item->speed -= velocity >> 16;
			item->speed += (velocity + anim->acceleration) >> 16;
			item->fallspeed += (item->fallspeed >= 128 ? 1 : GRAVITY);
			item->pos.yPos += item->fallspeed;
		}
	}
	else                                 // if on the Ground...
	{
		int velocity;

		if (Lara.waterStatus == LW_WADE && g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
		{
			velocity = (anim->velocity >> 1);
			if (anim->acceleration)
				velocity += (anim->acceleration * (item->frameNumber - anim->frameBase)) >> 2;
		}
		else
		{
			velocity = anim->velocity;
			if (anim->acceleration)
				velocity += anim->acceleration * (item->frameNumber - anim->frameBase);
		}
		
		item->speed = velocity >> 16;
	}

	if (Lara.ropePtr != -1)
		DelAlignLaraToRope(item);

	if (!Lara.isMoving) // TokyoSU: i dont know why but it's wreid, in TR3 only the 2 first line there is used and worked fine !
	{
		item->pos.xPos += item->speed * phd_sin(Lara.moveAngle) >> W2V_SHIFT; 
		item->pos.zPos += item->speed * phd_cos(Lara.moveAngle) >> W2V_SHIFT;

		item->pos.xPos += lateral * phd_sin(Lara.moveAngle + ANGLE(90)) >> W2V_SHIFT;  
		item->pos.zPos += lateral * phd_cos(Lara.moveAngle + ANGLE(90)) >> W2V_SHIFT;
	}

	// Update matrices
	g_Renderer.UpdateLaraAnimations(true);
}

void DelAlignLaraToRope(ITEM_INFO* item) // (F) (D)
{
	ROPE_STRUCT* rope;
	short ropeY;
	PHD_VECTOR vec, vec2, vec3, vec4, vec5, pos, pos2, diff, diff2;
	int matrix[12];
	short angle[3];
	ANIM_FRAME* frame;

	vec.x = 4096;
	vec.y = 0;
	vec.z = 0;
	frame = (ANIM_FRAME*) GetBestFrame(item);
	ropeY = Lara.ropeY - ANGLE(90);
	rope = &Ropes[Lara.ropePtr];
	_0x0046D130(rope, (Lara.ropeSegment - 1 << 7) + frame->offsetY, &pos.x, &pos.y, &pos.z);
	_0x0046D130(rope, (Lara.ropeSegment - 1 << 7) + frame->offsetY - 192, &pos2.x, &pos2.y, &pos2.z);
	diff.x = pos.x - pos2.x << 16;
	diff.y = pos.y - pos2.y << 16;
	diff.z = pos.z - pos2.z << 16;
	NormaliseRopeVector(&diff);
	diff.x >>= 2;
	diff.y >>= 2;
	diff.z >>= 2;
	ScaleVector(&diff, DotProduct(&vec, &diff), &vec2);
	vec2.x = vec.x - vec2.x;
	vec2.y = vec.y - vec2.y;
	vec2.z = vec.z - vec2.z;
	vec3.x = vec2.x;
	vec3.y = vec2.y;
	vec3.z = vec2.z;
	vec4.x = vec2.x;
	vec4.y = vec2.y;
	vec4.z = vec2.z;
	diff2.x = diff.x;
	diff2.y = diff.y;
	diff2.z = diff.z;
	ScaleVector(&vec3, phd_cos(ropeY), &vec3);
	ScaleVector(&diff2, DotProduct(&diff2, &vec2), &diff2);
	ScaleVector(&diff2, 4096 - phd_cos(ropeY), &diff2);
	CrossProduct(&diff, &vec2, &vec4);
	ScaleVector(&vec4, phd_sin(ropeY), &vec4);
	diff2.x += vec3.x;
	diff2.y += vec3.y;
	diff2.z += vec3.z;
	vec2.x = diff2.x + vec4.x << 16;
	vec2.y = diff2.y + vec4.y << 16;
	vec2.z = diff2.z + vec4.z << 16;
	NormaliseRopeVector(&vec2);
	vec2.x >>= 2;
	vec2.y >>= 2;
	vec2.z >>= 2;
	CrossProduct(&diff, &vec2, &vec5);
	vec5.x <<= 16;
	vec5.y <<= 16;
	vec5.z <<= 16;
	NormaliseRopeVector(&vec5);
	vec5.x >>= 2;
	vec5.y >>= 2;
	vec5.z >>= 2;
	matrix[M00] = vec5.x;
	matrix[M01] = diff.x;
	matrix[M02] = vec2.x;
	matrix[M10] = vec5.y;
	matrix[M11] = diff.y;
	matrix[M12] = vec2.y;
	matrix[M20] = vec5.z;
	matrix[M21] = diff.z;
	matrix[M22] = vec2.z;
	_0x0046D420(matrix, angle);
	item->pos.xPos = rope->position.x + (rope->meshSegment[Lara.ropeSegment].x >> 16);
	item->pos.yPos = rope->position.y + (rope->meshSegment[Lara.ropeSegment].y >> 16) + Lara.ropeOffset;
	item->pos.zPos = rope->position.z + (rope->meshSegment[Lara.ropeSegment].z >> 16);

	Matrix rotMatrix = Matrix::CreateFromYawPitchRoll(
		TO_DEGREES(angle[1]),
		TO_DEGREES(angle[0]),
		TO_DEGREES(angle[2])
	);
	
	// PHD_MATH!
	item->pos.xPos += -112 * rotMatrix.m[0][2]; // MatrixPtr[M02] >> W2V_SHIFT;
	item->pos.yPos += -112 * rotMatrix.m[1][2]; // MatrixPtr[M12] >> W2V_SHIFT;
	item->pos.zPos += -112 * rotMatrix.m[2][2]; // MatrixPtr[M22] >> W2V_SHIFT;
	
	item->pos.xRot = angle[0];
	item->pos.yRot = angle[1];
	item->pos.zRot = angle[2];
}
