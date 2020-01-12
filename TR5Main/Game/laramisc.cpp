#include "laramisc.h"
#include "..\Global\global.h"
#include "..\Scripting\GameFlowScript.h"
#include "effects.h"
#include "collide.h"
#include "Lara.h"
#include "laraswim.h"
#include "larasurf.h"
#include "effect2.h"
#include "healt.h"
#include "misc.h"

extern LaraExtraInfo g_LaraExtra;
extern GameFlow* g_GameFlow;
extern void(*effect_routines[59])(ITEM_INFO* item);
extern short FXType;

COLL_INFO coll;
short SubsuitAir = 0;

/*void GetLaraDeadlyBounds()//4B408(<), 4B86C (F)
{
#if PSX_VERSION || PSXPC_VERSION///@TODO PC subs not there yet.
	short* bounds;
	short tbounds[6];

	bounds = GetBoundsAccurate(LaraItem);
	mPushUnitMatrix();
	mRotYXZ(LaraItem->pos.yRot, LaraItem->pos.xRot, LaraItem->pos.zRot);
	mSetTrans(0, 0, 0);
	mRotBoundingBoxNoPersp(bounds, &tbounds[0]);
	mPopMatrix();

	DeadlyBounds[0] = LaraItem->pos.xPos + tbounds[0];
	DeadlyBounds[1] = LaraItem->pos.xPos + tbounds[1];
	DeadlyBounds[2] = LaraItem->pos.yPos + tbounds[2];
	DeadlyBounds[3] = LaraItem->pos.yPos + tbounds[3];
	DeadlyBounds[4] = LaraItem->pos.zPos + tbounds[4];
	DeadlyBounds[5] = LaraItem->pos.zPos + tbounds[5];
#else
	UNIMPLEMENTED();
#endif
}

void DelAlignLaraToRope(ITEM_INFO* item)//4B3D8, 4B83C
{
	UNIMPLEMENTED();
}*/

void InitialiseLaraAnims(ITEM_INFO* item)//4B340(<), 4B7A4 (F)
{
	if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		Lara.waterStatus = LW_UNDERWATER;
		item->goalAnimState = STATE_LARA_UNDERWATER_STOP;
		item->currentAnimState = STATE_LARA_UNDERWATER_STOP;
		item->fallspeed = 0;
		item->animNumber = ANIMATION_LARA_UNDERWATER_IDLE;
		item->frameNumber = Anims[item->animNumber].frameBase;
	}
	else
	{
		Lara.waterStatus = LW_ABOVE_WATER;
		item->goalAnimState = STATE_LARA_STOP;
		item->currentAnimState = STATE_LARA_STOP;
		item->animNumber = ANIMATION_LARA_STAY_SOLID;
		item->frameNumber = Anims[item->animNumber].frameBase;
	}
}

void InitialiseLaraLoad(short itemNum)//4B308, 4B76C (F)
{
	Lara.itemNumber = itemNum;
	LaraItem = &Items[itemNum];
}

void LaraCheatyBits()
{
	if (TrInput & IN_D)
	{
		//LaraCheatGetStuff();
		//LaraItem->hitPoints = 1000;
	}

	if (TrInput & IN_PAUSE)
	{
		LaraItem->pos.yPos -= 128;
		if (Lara.waterStatus != LW_FLYCHEAT)
		{
			LaraItem->hitPoints = 1000;
			Lara.waterStatus = LW_FLYCHEAT;
			LaraItem->animNumber = ANIMATION_LARA_UNDERWATER_SWIM_SOLID;
			LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = STATE_LARA_UNDERWATER_FORWARD;
			LaraItem->goalAnimState = STATE_LARA_UNDERWATER_FORWARD;
			LaraItem->gravityStatus = 0;
			LaraItem->pos.xRot = ANGLE(30);
			LaraItem->fallspeed = 30;
			Lara.air = 1800;
			Lara.deathCount = 0;
			Lara.torsoXrot = Lara.torsoYrot = 0;
			Lara.headXrot = Lara.headYrot = 0;
		}
	}
}

void LaraControl(short itemNumber)//4A838, 4AC9C
{
	ITEM_INFO* item = LaraItem;

	LaraCheatyBits();

	if (Lara.isMoving)
	{
		if (++Lara.moveCount > 90)
		{
			Lara.isMoving = 0;
			Lara.gunStatus = LG_NO_ARMS;
		} 
	} 

	if (!DisableLaraControl)
		Lara.locationPad = -128;

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

	Lara.isDucked = 0;

	bool isWater = Rooms[item->roomNumber].flags & (ENV_FLAG_WATER|ENV_FLAG_SWAMP);
	int wd = GetWaterDepth(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
	int wh = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);

	int hfw = NO_HEIGHT;
	if (wh != NO_HEIGHT)
		hfw = item->pos.yPos - wh;

	Lara.waterSurfaceDist = -hfw;
	WadeSplash(item, wh, wd);
	
	short roomNumber;
	short height = 0;

	switch (Lara.waterStatus)
	{
	case LW_ABOVE_WATER:
		if (hfw != NO_HEIGHT && hfw >= 256)
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
					else if (isWater & ENV_FLAG_SWAMP)
					{
						if (item->currentAnimState == STATE_LARA_SWANDIVE_BEGIN || item->currentAnimState == STATE_LARA_SWANDIVE_END)			// Is Lara swan-diving?
							item->pos.yPos = wh + 1000;

						item->goalAnimState = STATE_LARA_WADE_FORWARD;
						item->currentAnimState = STATE_LARA_WADE_FORWARD;
						item->animNumber = ANIMATION_LARA_WADE;
						item->frameNumber = GF(ANIMATION_LARA_WADE, 0);
					}
				}
			}
			else if (!(isWater & ENV_FLAG_SWAMP))
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
					item->frameNumber = Anims[item->animNumber].frameBase;
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
			if (hfw >= 256)
			{
				if (hfw > 730)
				{
					Lara.waterStatus = LW_SURFACE;
					item->pos.yPos += 1 - hfw;

					switch (item->currentAnimState)
					{
					case STATE_LARA_WALK_BACK:
						item->animNumber = ANIMATION_LARA_ONWATER_IDLE_TO_SWIM_BACK;
						item->frameNumber = Anims[item->animNumber].frameBase;
						item->goalAnimState = STATE_LARA_ONWATER_BACK;
						item->currentAnimState = STATE_LARA_ONWATER_BACK;
						break;

					case STATE_LARA_WALK_RIGHT:
						item->animNumber = ANIMATION_LARA_ONWATER_SWIM_RIGHT;
						item->frameNumber = Anims[item->animNumber].frameBase;
						item->goalAnimState = STATE_LARA_ONWATER_RIGHT;
						item->currentAnimState = STATE_LARA_ONWATER_RIGHT;
						break;

					case STATE_LARA_WALK_LEFT:
						item->animNumber = ANIMATION_LARA_ONWATER_SWIM_LEFT;
						item->frameNumber = Anims[item->animNumber].frameBase;
						item->goalAnimState = STATE_LARA_ONWATER_LEFT;
						item->currentAnimState = STATE_LARA_ONWATER_LEFT;
						break;

					default:
						item->animNumber = ANIMATION_LARA_ONWATER_SWIM_FORWARD;
						item->frameNumber = Anims[item->animNumber].frameBase;
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

		height = 0;
		if (wd != NO_HEIGHT)
		{
			height = -hfw;
			if (hfw >= 0)
				height = hfw;
		}

		if (height >= 256
			|| wd == NO_HEIGHT
			|| Rooms[roomNumber].flags & ENV_FLAG_WATER
			|| item->animNumber == ANIMATION_LARA_UNDERWATER_TO_ONWATER
			|| item->animNumber == ANIMATION_LARA_FREE_FALL_TO_UNDERWATER_ALTERNATE)
		{
			if (!isWater)
			{
				if (wd == NO_HEIGHT || abs(hfw) >= 256)
				{
					Lara.waterStatus = LW_ABOVE_WATER;
					item->animNumber = ANIMATION_LARA_FREE_FALL_FORWARD;
					item->frameNumber = Anims[item->animNumber].frameBase;
					item->goalAnimState = STATE_LARA_JUMP_FORWARD;
					item->currentAnimState = STATE_LARA_JUMP_FORWARD;
					item->speed = item->fallspeed / 4;
					item->gravityStatus = true;

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
					item->frameNumber = Anims[item->animNumber].frameBase;
					item->goalAnimState = STATE_LARA_ONWATER_STOP;
					item->currentAnimState = STATE_LARA_ONWATER_STOP;
					item->fallspeed = 0;
					Lara.diveCount = 11;
					LaraItem->pos.zRot = 0;
					item->pos.xRot = 0;
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
			item->frameNumber = Anims[item->animNumber].frameBase;
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
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->goalAnimState = STATE_LARA_JUMP_FORWARD;
				item->currentAnimState = STATE_LARA_JUMP_FORWARD;
				item->speed = item->fallspeed / 4;
				item->gravityStatus = true;
			}
			else
			{
				Lara.waterStatus = LW_WADE;
				item->animNumber = ANIMATION_LARA_STAY_IDLE;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->goalAnimState = STATE_LARA_STOP;
				item->currentAnimState = STATE_LARA_WADE_FORWARD;

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
		if (hfw >= 256)
		{
			if (hfw > 730 && !(isWater & ENV_FLAG_SWAMP))
			{
				Lara.waterStatus = LW_SURFACE;
				item->pos.yPos += 1 - hfw;

				switch (item->currentAnimState)
				{
				case STATE_LARA_WALK_BACK:
					item->animNumber = ANIMATION_LARA_ONWATER_IDLE_TO_SWIM_BACK;
					item->frameNumber = Anims[item->animNumber].frameBase;
					item->goalAnimState = STATE_LARA_ONWATER_BACK;
					item->currentAnimState = STATE_LARA_ONWATER_BACK;
					break;

				case STATE_LARA_WALK_RIGHT:
					item->animNumber = ANIMATION_LARA_ONWATER_SWIM_RIGHT;
					item->frameNumber = Anims[item->animNumber].frameBase;
					item->goalAnimState = STATE_LARA_ONWATER_RIGHT;
					item->currentAnimState = STATE_LARA_ONWATER_RIGHT;
					break;

				case STATE_LARA_WALK_LEFT:
					item->animNumber = ANIMATION_LARA_ONWATER_SWIM_LEFT;
					item->frameNumber = Anims[item->animNumber].frameBase;
					item->goalAnimState = STATE_LARA_ONWATER_LEFT;
					item->currentAnimState = STATE_LARA_ONWATER_LEFT;
					break;

				default:
					item->animNumber = ANIMATION_LARA_ONWATER_SWIM_FORWARD;
					item->frameNumber = Anims[item->animNumber].frameBase;
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

	case LW_FLYCHEAT:
		LaraCheat(item, &coll);
		break;
	}

	//S_SetReverbType(room[item->roomNumber].ReverbType);

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
		if (Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && Lara.waterSurfaceDist < -775)
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
		{
			if (item->hitPoints >= 0 && --Lara.air < 0)
			{
				Lara.air = -1;
				item->hitPoints -= 5;
				LaraAboveWater(item, &coll);
				break;
			}
		}
		else if (Lara.air < 1800 && item->hitPoints >= 0)
		{
			/* lara is not equipped with any vehicle */
			if (g_LaraExtra.Vehicle == NO_ITEM) // only for the upv !!
			{
				Lara.air += 10;
				if (Lara.air > 1800)
					Lara.air = 1800;
			}
		}
		LaraAboveWater(item, &coll);
		break;

	case LW_UNDERWATER:
		if (item->hitPoints >= 0)
		{
			/*if (LaraDrawType == LARA_DIVESUIT)
			{
				if (CheckCutPlayed(40))
				{
					v32 = Lara.Anxiety + 8;
					v33 = v32 + word_51CEE0;
					word_51CEE0 += v32;
					if (word_51CEE0 > 80)
					{
						v34 = (v33 - 1) / 0x50u;
						word_51CEE0 = -80 * v34 + v33;
						do
						{
							--Lara.air;
							--v34;
						} while (v34);
					}
				}
			}
			else
			{*/
			Lara.air--;
			//}
			if (Lara.air < 0)
			{
				if (LaraDrawType == LARA_DIVESUIT && Lara.anxiety < 251)
					Lara.anxiety += 4;
				Lara.air = -1;
				item->hitPoints -= 5;
			}
		}		
		LaraUnderWater(item, &coll);
		break;

	case LW_SURFACE:
		if (item->hitPoints >= 0)
		{
			Lara.air += 10;
			if (Lara.air > 1790)
				Lara.air = 1800;
		}
		LaraSurface(item, &coll);
		break;

	case LW_FLYCHEAT:
		LaraCheat(item, &coll);
		break;
	}

	Savegame.Game.Distance += SQRT_ASM(
		SQUARE(item->pos.xPos - oldX) +
		SQUARE(item->pos.yPos - oldY) +
		SQUARE(item->pos.zPos - oldZ));
}

void LaraCheat(ITEM_INFO* item, COLL_INFO* coll)//4A790(<), 4ABF4(<) (F)
{
	LaraItem->hitPoints = 1000;
	LaraUnderWater(item, coll);
	if (TrInput & IN_WALK && !(TrInput & IN_LOOK))
	{
		Lara.waterStatus = LW_ABOVE_WATER;
		item->animNumber = ANIMATION_LARA_STAY_SOLID;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->pos.zRot = 0;
		item->pos.xRot = 0;
		Lara.torsoYrot = 0;
		Lara.torsoXrot = 0;
		Lara.headYrot = 0;
		Lara.headXrot = 0;
		Lara.gunStatus = LG_NO_ARMS;
		LaraInitialiseMeshes();
		Lara.meshEffects = 0;
	}
}

void LaraInitialiseMeshes()//4A684, 4AAE8 (F)
{
	for (int i = 0; i < 15; i++)
	{
		INIT_LARA_MESHES(i, ID_LARA, ID_LARA_SKIN);
	}

	/*if (gfCurrentLevel >= LVL5_GALLOWS_TREE && gfCurrentLevel <= LVL5_OLD_MILL)
	{
		Lara.mesh_ptrs[LM_TORSO] = meshes[Objects[ANIMATING6_MIP].mesh_index + 2 * LM_TORSO];
	}*/

	if (Lara.gunType == WEAPON_HK)
	{
		Lara.backGun = WEAPON_HK;
	}
	else if (!g_LaraExtra.Weapons[WEAPON_SHOTGUN].Present)
	{
		if (g_LaraExtra.Weapons[WEAPON_HK].Present)
		{
			Lara.backGun = WEAPON_HK;
		}
	}
	else
	{
		Lara.backGun = WEAPON_UZI;
	}

	Lara.gunStatus = LG_NO_ARMS;
	Lara.leftArm.frameNumber = 0;
	Lara.rightArm.frameNumber = 0;
	Lara.target = 0;
	Lara.rightArm.lock = 0;
	Lara.leftArm.lock = 0;
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
		LARA_INFO backup;
		memcpy(&backup, &Lara, sizeof(LARA_INFO));
		memset(&Lara, 0, sizeof(LARA_INFO));
		memcpy(&Lara.Legacy_pistolsTypeCarried, &backup.Legacy_pistolsTypeCarried, 59);
	}
	else
	{
		memset(&Lara, 0, sizeof(LARA_INFO));
		ZeroMemory(&g_LaraExtra, sizeof(LaraExtraInfo));

		g_LaraExtra.ExtraAnim = -1;
		g_LaraExtra.Vehicle = NO_ITEM;
	}

	Lara.look = true;
	Lara.itemNumber = itemNumber;
	Lara.hitDirection = -1;
	Lara.air = 1800;
	Lara.weaponItem = -1;
	PoisonFlag = 0;
	Lara.dpoisoned = 0;
	Lara.poisoned = 0;
	Lara.waterSurfaceDist = 100;
	Lara.holster = 14;
	Lara.location = -1;
	Lara.highestLocation = -1;
	Lara.ropePtr = -1;
	LaraItem->hitPoints = 1000;
	Lara.gunStatus = LG_NO_ARMS;
	Lara.skelebob = 0;

	short gun = WEAPON_NONE;
	if (LaraDrawType != LARA_YOUNG)
	{
		if (Objects[ID_PISTOLS_ITEM].loaded)
			gun = WEAPON_PISTOLS;
		else if (Objects[ID_HK_ITEM].loaded)
			gun = WEAPON_HK;
	}

	Lara.lastGunType = Lara.gunType = Lara.requestGunType = gun;

	LaraInitialiseMeshes();

	if (gun == WEAPON_PISTOLS)
	{
		g_LaraExtra.Weapons[WEAPON_PISTOLS].Present = true;
		g_LaraExtra.Weapons[WEAPON_PISTOLS].Ammo[WEAPON_AMMO1] = -1;
	}
	else if (gun == WEAPON_HK)
	{
		g_LaraExtra.Weapons[WEAPON_HK].Present = true;
		g_LaraExtra.Weapons[WEAPON_HK].Ammo[WEAPON_AMMO1] = 100;
	}

	g_LaraExtra.Binoculars = true;

	if (!restore)
	{
		if (Objects[ID_FLARE_INV_ITEM].loaded)
			g_LaraExtra.NumFlares = 3;

		g_LaraExtra.NumSmallMedipacks = 3;
		g_LaraExtra.NumLargeMedipacks = 1;
	}

	InitialiseLaraAnims(LaraItem);

	DashTimer = 120;

	//weapons[WEAPON_REVOLVER].damage = gfCurrentLevel >= LVL5_BASE ? 15 : 6;

	Lara.bottle = 0;
	Lara.wetcloth = CLOTH_MISSING;
}

void AnimateLara(ITEM_INFO* item)
{
	item->frameNumber++;

	ANIM_STRUCT* anim = &Anims[item->animNumber];
	if (anim->numberChanges > 0 && GetChange(item, anim))
	{
		anim = &Anims[item->animNumber];
		item->currentAnimState = anim->currentAnimState;
	}

	if (item->frameNumber > anim->frameEnd)
	{
		if (anim->numberCommands > 0)
		{
			short* cmd = &Commands[anim->commandIndex];
			for (int i = anim->numberCommands; i > 0; i--)
			{
				switch (*(cmd++))
				{
				case COMMAND_MOVE_ORIGIN:
					TranslateItem(item, cmd[0], cmd[1], cmd[2]);
					UpdateLaraRoom(item, -381);
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
		
		anim = &Anims[item->animNumber];
		item->currentAnimState = anim->currentAnimState;
	}

	if (anim->numberCommands > 0)
	{
		short* cmd = &Commands[anim->commandIndex];
		int flags;

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
				   (flags == SFX_WATERONLY && Lara.waterSurfaceDist < 0 && Lara.waterSurfaceDist != NO_HEIGHT && !(Rooms[LaraItem->roomNumber].flags & ENV_FLAG_SWAMP)))
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
				(*effect_routines[(int)(cmd[1] & 0x3fff)])(item);

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
		if (Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
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

		if (Lara.waterStatus == LW_WADE && Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
		{
			velocity = anim->velocity >> 1;
			if (anim->acceleration)
				velocity += anim->acceleration * (item->frameNumber - anim->frameBase) >> 2;
		}
		else
		{
			velocity = anim->velocity;
			if (anim->acceleration)
				velocity += anim->acceleration * (item->frameNumber - anim->frameBase);
		}
		
		item->speed = velocity >> 16;
	}

	/*if (lara.RopePtr != -1)
		result = j_SomeRopeCollisionFunc(item);*/

	if (!Lara.isMoving)
	{
		item->pos.xPos += item->speed * SIN(Lara.moveAngle) >> W2V_SHIFT; 
		item->pos.zPos += item->speed * COS(Lara.moveAngle) >> W2V_SHIFT;

		item->pos.xPos += lateral * SIN(Lara.moveAngle + ANGLE(90)) >> W2V_SHIFT;  
		item->pos.zPos += lateral * COS(Lara.moveAngle + ANGLE(90)) >> W2V_SHIFT;
	}
}