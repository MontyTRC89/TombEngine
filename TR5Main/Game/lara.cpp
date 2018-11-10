#include "lara.h"
#include "..\Global\global.h"
#include "control.h"
#include "items.h"
#include "collide.h"
#include "..\Objects\objects.h"

#include <stdio.h>

LaraExtraInfo g_LaraExtra;

void lara_as_walk(ITEM_INFO* item, COLL_INFO* coll)
{
	printf("lara_as_walk\n");

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = 2;

		return;
	}

	if (!Lara.isMoving)
	{
		if (TrInput & 4)
		{
			Lara.turnRate -= 409;
			if (Lara.turnRate < ANGLE(-4))
				Lara.turnRate = ANGLE(-4);
		}
		else if (TrInput & 8)
		{
			Lara.turnRate += 409;
			if (Lara.turnRate > ANGLE(4))
				Lara.turnRate = ANGLE(4);
		}

		if (TrInput & 1)
		{
			if (Lara.waterStatus == 4)
			{
				item->goalAnimState = 65;
			}
			else if (TrInput & 0x80)
			{
				item->goalAnimState = 0;
			}
			else
			{
				item->goalAnimState = 1;
			}
		}
		else
		{
			item->goalAnimState = 2;
		}
	}
}

#define AnimateLara ((__int32 (__cdecl*)(ITEM_INFO*)) 0x004563F0)

void __cdecl AnimateLaraNew(ITEM_INFO* item)
{
	Lara.ropePtr = -1;

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
			__int16* command = &Commands[anim->commandIndex];
			for (__int32 i = anim->numberCommands; i > 0; i--)
			{
				switch (*(command++))
				{
				case 1:
					TranslateItem(item, (__int32)*(command), (__int32)*(command + 1), (__int32)*(command + 2));
					UpdateLaraRoom(item, -381);
					command += 3;
					break;

				case 2:
					item->gravityStatus = 1;
					item->fallspeed = *(command++);
					item->speed = *(command++);
					if (Lara.calcFallSpeed)
					{
						item->fallspeed = Lara.calcFallSpeed;
						Lara.calcFallSpeed = 0;
					}
					break;

				case 3:
					if (Lara.gunStatus != 5)
						Lara.gunStatus = 0;
					break;

				case 4:
					break;

				case 5:
				case 6:
					command += 2;
					break;
				}
			}
		}

		item->animNumber = anim->jumpAnimNum;
		item->frameNumber = anim->jumpFrameNum;
		anim = &Anims[anim->jumpAnimNum];
		item->currentAnimState = anim->currentAnimState;
	}

	//EffectRoutines[0](item);

	__int32 xAccel = anim->Xacceleration;
	__int32 xVelocity = anim->Xvelocity;
	if (xAccel)
		xVelocity += xAccel * (item->frameNumber - anim->frameBase);
	xVelocity >>= 16;

	if (item->gravityStatus)
	{
		__int32 speed = anim->velocity + anim->acceleration * (item->frameNumber - anim->frameBase - 1);
		item->speed -= (__int16)(speed >> 16);
		speed += anim->acceleration;
		item->speed += (__int16)(speed >> 16);
		item->fallspeed += (item->fallspeed >= 128) ? 1 : 6;
		item->pos.yPos += item->fallspeed;
	}
	else
	{
		__int32 acceleration = anim->acceleration;
		__int32 speed = anim->velocity;
		if (acceleration)
			speed += acceleration * (item->frameNumber - anim->frameBase);
		item->speed = speed >> 16;
	}

	//if (!Lara.isMoving)
	//{
		item->pos.xPos += (SIN((Lara.moveAngle)) * item->speed) >> W2V_SHIFT;
		item->pos.zPos += (COS((Lara.moveAngle)) * item->speed) >> W2V_SHIFT;

		item->pos.xPos += (SIN((Lara.moveAngle + 0x4000)) * xVelocity) >> W2V_SHIFT;
		item->pos.zPos += (COS((Lara.moveAngle + 0x4000)) * xVelocity) >> W2V_SHIFT;
	//}
}

void __cdecl j_AnimateLara(ITEM_INFO* item)
{
	printf("ANIMSTATE: %d - FRAME: %d\n", item->currentAnimState, item->frameNumber);
	printf("BEFORE: %d %d %d\n", item->pos.xPos, item->pos.yPos, item->pos.zPos);
	Lara.ropePtr = -1;
	AnimateLara(item);
	printf("AFTER: %d %d %d\n", item->pos.xPos, item->pos.yPos, item->pos.zPos);
}

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

	if ((TrInput & IN_LOOK)
		/*&& (!lara.extra_anim)*/
		&& (Lara.look))
			LookLeftRight();
	else
		ResetLook();

	Lara.look = true;

	// Process vehicles
	if (g_LaraExtra.vehicle != NO_ITEM)
	{
		if (Items[g_LaraExtra.vehicle].objectNumber == ID_QUAD)  
		{
			if (QuadBikeControl())
				return;
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

	// Check for collision with items
	LaraBaddieCollision(item, coll);

	// Handle Lara collision
	(*LaraCollisionRoutines[item->currentAnimState])(item, coll);

	UpdateLaraRoom(item, -381);

	/*if (lara.gun_type == 6 && !LaserSight && gfLevelFlags & 0x80)
	{
		v7 = input;
		LOBYTE(v7) = input & 0xBF;
		input = v7;
	}*/

	// Handle weapons
	LaraGun();

	// Test if there's a trigger
	TestTriggers(coll->trigger, 0, 0);
}

void Inject_Lara()
{
	//INJECT(0x00442E70, LaraAboveWater);
	//INJECT(0x00449260, lara_as_walk);
	//INJECT(0x00402AC2, j_AnimateLara);
	//INJECT(0x004563F0, AnimateLaraNew);
}