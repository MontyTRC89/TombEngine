#include "lara.h"

#include "control.h"
#include "items.h"
#include "collide.h"
#include "inventory.h"
#include "larafire.h"

#include "..\Objects\newobjects.h"
#include "..\Global\global.h"

#include <stdio.h>

extern Inventory* g_Inventory;

LaraExtraInfo g_LaraExtra;

void __cdecl LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->old.x = item->pos.xPos;
	coll->old.y = item->pos.yPos;
	coll->old.z = item->pos.zPos;
	coll->oldAnimState = item->currentAnimState;
	coll->enableBaddiePush = true;
	coll->enableSpaz = true;
	coll->slopesAreWalls = false;
	coll->slopesArePits = false;
	coll->lavaIsPit = false;
	coll->oldAnimNumber = item->animNumber;
	coll->oldFrameNumber = item->frameNumber;
	coll->radius = 100;
	coll->trigger = 0;

	if ((TrInput & IN_LOOK) && g_LaraExtra.ExtraAnim == -1 && Lara.look)
		LookLeftRight();
	else
		ResetLook();

	Lara.look = true;

	// Process Vehicles
	if (g_LaraExtra.Vehicle != NO_ITEM)
	{
		switch (Items[g_LaraExtra.Vehicle].objectNumber)
		{
			case ID_QUAD:
				if (QuadBikeControl())
					return;
				break;

			case ID_JEEP:
				if (JeepControl())
					return;
				break;

			case ID_KAYAK:
				if (KayakControl())
					return;
				break;

			case ID_SNOWMOBILE:
				if (SkidooControl())
					return;
				break;

			default:
				break;
		}
	}

	// Handle current Lara status
	(*LaraControlRoutines[item->currentAnimState])(item, coll);

	if (item->pos.zRot >= -ANGLE(1) && item->pos.zRot <= ANGLE(1)) 
		item->pos.zRot = 0;   
	else if (item->pos.zRot < -ANGLE(1))
		item->pos.zRot += ANGLE(1);
	else
		item->pos.zRot -= ANGLE(1);

	if (Lara.turnRate >= -ANGLE(2) && Lara.turnRate <= ANGLE(2)) 
		Lara.turnRate = 0; 
	else if (Lara.turnRate < -ANGLE(2))  
		Lara.turnRate += ANGLE(2); 
	else
		Lara.turnRate -= ANGLE(2);
	item->pos.yRot += Lara.turnRate;

	// Animate Lara
	AnimateLara(item);

	if (g_LaraExtra.ExtraAnim == -1)
	{
		// Check for collision with items
		LaraBaddieCollision(item, coll);

		// Handle Lara collision
		if (g_LaraExtra.Vehicle == NO_ITEM)
			(*LaraCollisionRoutines[item->currentAnimState])(item, coll);
	}

	UpdateLaraRoom(item, -381);

	//if (Lara.gunType == WEAPON_CROSSBOW && !LaserSight)
	//	TrInput &= ~IN_ACTION;

	// Handle weapons
	LaraGun();

	// Test if there's a trigger
	TestTriggers(coll->trigger, 0, 0);
}

__int32 __cdecl UseSpecialItem(ITEM_INFO* item)
{
	__int16 selectedObject = g_Inventory->GetSelectedObject();

	if (item->animNumber != ANIMATION_LARA_STAY_IDLE || Lara.gunStatus || selectedObject == NO_ITEM)
		return 0;

	if (selectedObject >= ID_WATERSKIN1_EMPTY && selectedObject <= ID_WATERSKIN2_5)
	{
		item->itemFlags[2] = 25;

		if (selectedObject != ID_WATERSKIN1_3 && selectedObject != ID_WATERSKIN2_5)
		{
			if (selectedObject >= ID_WATERSKIN2_EMPTY)
				g_LaraExtra.Waterskin2.Quantity = 5;
			else
				g_LaraExtra.Waterskin1.Quantity = 3;

			item->animNumber = ANIMATION_LARA_WATERSKIN_FILL;
		}
		else
		{
			if (selectedObject >= ID_WATERSKIN2_EMPTY)
			{
				item->itemFlags[3] = g_LaraExtra.Waterskin2.Quantity;
				g_LaraExtra.Waterskin2.Quantity = 1;
			}
			else
			{
				item->itemFlags[3] = g_LaraExtra.Waterskin1.Quantity;
				g_LaraExtra.Waterskin1.Quantity = 1;
			}

			item->animNumber = ANIMATION_LARA_WATERSKIN_EMPTY;
		}
	}
	else if (selectedObject == ID_CLOCKWORK_BEETLE)
	{
		item->animNumber = ANIMATION_LARA_BEETLE_PUT;
		//UseClockworkBeetle(1);
	}
	else
	{
		return 0;
	}

	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = STATE_LARA_MISC_CONTROL;
	item->currentAnimState = STATE_LARA_MISC_CONTROL;

	Lara.gunStatus = LG_HANDS_BUSY;
	g_Inventory->SetSelectedObject(NO_ITEM);

	return 1;
}

void __cdecl lara_as_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	short fheight = -32512;
	short rheight = -32512;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = STATE_LARA_DEATH;

		return;
	}

	if (item->animNumber != ANIMATION_LARA_SPRINT_SLIDE_STAND_RIGHT &&
		item->animNumber != ANIMATION_LARA_SPRINT_SLIDE_STAND_LEFT)
		StopSoundEffect(SFX_LARA_SLIPPING);

	// Handles waterskin and clockwork beetle
	if (UseSpecialItem(item))
		return;

	if (TrInput & IN_ROLL && Lara.waterStatus != LW_WADE)
	{
		item->animNumber = ANIMATION_LARA_ROLL_BEGIN;
		item->frameNumber = Anims[ANIMATION_LARA_ROLL_BEGIN].frameBase + 2;

		item->currentAnimState = STATE_LARA_ROLL_FORWARD;
		item->goalAnimState = STATE_LARA_STOP;

		return;
	}

	if (TrInput & IN_DUCK &&
		Lara.waterStatus != LW_WADE &&
		item->currentAnimState == STATE_LARA_STOP &&
		(Lara.gunStatus == LG_NO_ARMS ||
			Lara.gunType == WEAPON_NONE ||
			Lara.gunType == WEAPON_PISTOLS ||
			Lara.gunType == WEAPON_REVOLVER ||
			Lara.gunType == WEAPON_UZI ||
			Lara.gunType == WEAPON_FLARE))
	{
		item->goalAnimState = STATE_LARA_CROUCH_IDLE;

		return;
	}

	item->goalAnimState = STATE_LARA_STOP;

	if (TrInput & IN_LOOK)
	{
		LookUpDown();
	}

	if (TrInput & IN_FORWARD)
	{
		fheight = LaraFloorFront(item, item->pos.yRot, 104);
	}
	else if (TrInput & IN_BACK)
	{
		rheight = LaraFloorFront(item, item->pos.yRot - ANGLE(180), 104);
	}

	if (TrInput & IN_LSTEP)
	{
		if (abs(LaraFloorFront(item, item->pos.yRot - ANGLE(90), 116)) < 128 &&
			HeightType != BIG_SLOPE &&
			LaraCeilingFront(item, item->pos.yRot - ANGLE(90), 116, 762) <= 0)
		{
			item->goalAnimState = STATE_LARA_WALK_LEFT;
		}
	}
	else if (TrInput & IN_RSTEP)
	{
		if (abs(LaraFloorFront(item, item->pos.yRot + ANGLE(90), 116)) < 128 &&
			HeightType != BIG_SLOPE &&
			LaraCeilingFront(item, item->pos.yRot + ANGLE(90), 116, 762) <= 0)
		{
			item->goalAnimState = STATE_LARA_WALK_RIGHT;
		}
	}
	else if (TrInput & IN_LEFT)
	{
		item->goalAnimState = STATE_LARA_TURN_LEFT_SLOW;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->goalAnimState = STATE_LARA_TURN_RIGHT_SLOW;
	}

	if (Lara.waterStatus == 4)
	{
		if (TrInput & IN_JUMP)
		{
			item->goalAnimState = STATE_LARA_JUMP_PREPARE;
		}

		if (TrInput & IN_FORWARD)
		{
			if (abs(fheight) >= 383)
			{
				Lara.moveAngle = item->pos.yRot;

				coll->badPos = 32512;
				coll->badNeg = -384;
				coll->badCeiling = 0;
				coll->radius = 102;
				coll->slopesAreWalls = true;

				GetLaraCollisionInfo(item, coll);
				if (!TestLaraVault(item, coll))
					coll->radius = 100;
			}
			else
			{
				lara_as_wade(item, coll);
			}
		}
		else if (TrInput & IN_BACK && abs(rheight) < 383)
		{
			lara_as_back(item, coll);
		}
	}
	else if (TrInput & IN_JUMP)
	{
		item->goalAnimState = STATE_LARA_JUMP_PREPARE;
	}
	else if (TrInput & IN_FORWARD)
	{
		__int32 ceiling = LaraCeilingFront(item, item->pos.yRot, 104, 762);
		__int32 height = LaraFloorFront(item, item->pos.yRot, 104);

		if ((HeightType == BIG_SLOPE || HeightType == DIAGONAL) && height < 0 || ceiling > 0)
		{
			item->goalAnimState = STATE_LARA_STOP;

			return;
		}

		if (height >= -256 || fheight >= -256)
		{
			if (TrInput & IN_WALK)
				lara_as_walk(item, coll);
			else
				lara_as_run(item, coll);
		}
		else
		{
			Lara.moveAngle = item->pos.yRot;

			coll->badPos = 32512;
			coll->badNeg = -384;
			coll->badCeiling = 0;

			coll->radius = 102;
			coll->slopesAreWalls = 1;

			GetLaraCollisionInfo(item, coll);

			if (!TestLaraVault(item, coll))
			{
				coll->radius = 100;

				item->goalAnimState = STATE_LARA_STOP;
			}
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (abs(rheight) < 383 && HeightType != BIG_SLOPE)
				lara_as_back(item, coll);
		}
		else if (rheight > -383)
		{
			item->goalAnimState = STATE_LARA_RUN_BACK;
		}
	}
}

void Inject_Lara()
{
	INJECT(0x00448010, lara_as_stop);
	INJECT(0x00442E70, LaraAboveWater);
}