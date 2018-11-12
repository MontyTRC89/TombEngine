#include "lara.h"

#include "control.h"
#include "items.h"
#include "collide.h"
#include "larafire.h"

#include "..\Objects\objects.h"
#include "..\Global\global.h"

#include <stdio.h>

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

	if ((TrInput & IN_LOOK) && !g_LaraExtra.extraAnim && Lara.look)
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

	if (Lara.gunType == WEAPON_CROSSBOW && !LaserSight)
		TrInput &= ~IN_ACTION;

	// Handle weapons
	LaraGun();

	// Test if there's a trigger
	TestTriggers(coll->trigger, 0, 0);
}

void Inject_Lara()
{
	INJECT(0x00442E70, LaraAboveWater);
}