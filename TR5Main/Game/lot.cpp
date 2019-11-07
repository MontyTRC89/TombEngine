#include "lot.h"
#include "..\Global\global.h"
#include <stdio.h>

void __cdecl InitialiseLOTarray(__int32 allocMem)
{
	DB_Log(0, "InitialiseLOTarray - DLL");

	if (allocMem)
		BaddieSlots = (CREATURE_INFO*)GameMalloc(sizeof(CREATURE_INFO) * NUM_SLOTS);

	CREATURE_INFO* creature = BaddieSlots;
	for (__int32 i = 0; i < NUM_SLOTS; i++, creature++)
	{
		creature->itemNum = NO_ITEM;
		creature->LOT.node = (BOX_NODE*)GameMalloc(sizeof(BOX_NODE) * NumberBoxes);
	}

	SlotsUsed = 0;
}

__int32 __cdecl EnableBaddieAI(__int16 itemNum, __int32 always)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->data != NULL)
		return true;
	else if (SlotsUsed >= NUM_SLOTS)
	{
		__int32 cameraDistance = 0;
		if (!always)
		{
			__int32 deltaX = (item->pos.xPos - Camera.pos.x) >> 8;
			__int32 deltaY = (item->pos.yPos - Camera.pos.y) >> 8;
			__int32 deltaZ = (item->pos.zPos - Camera.pos.z) >> 8;
			cameraDistance = SQUARE(deltaX) + SQUARE(deltaY) + SQUARE(deltaZ);
		}

		__int32 slotToDisable = -1;
		CREATURE_INFO* creature = BaddieSlots;
		for (__int32 slot = 0; slot < NUM_SLOTS; slot++, creature++)
		{
			item = &Items[creature->itemNum];

			__int32 deltaX = (item->pos.xPos - Camera.pos.x) >> 8;
			__int32 deltaY = (item->pos.yPos - Camera.pos.y) >> 8;
			__int32 deltaZ = (item->pos.zPos - Camera.pos.z) >> 8;
			__int32 distance = SQUARE(deltaX) + SQUARE(deltaY) + SQUARE(deltaZ);

			if (distance > cameraDistance)
			{
				cameraDistance = distance;
				slotToDisable = slot;
			}
		}

		if (slotToDisable < 0)
			return false;

		Items[BaddieSlots[slotToDisable].itemNum].status = ITEM_INVISIBLE;
		DisableBaddieAI(BaddieSlots[slotToDisable].itemNum);
		InitialiseSlot(itemNum, slotToDisable);

		return true;
	}
	else
	{
		CREATURE_INFO* creature = BaddieSlots;
		for (__int32 slot = 0; slot < NUM_SLOTS; slot++, creature++)
		{
			if (creature->itemNum == NO_ITEM)
			{
				InitialiseSlot(itemNum, slot);
				return true;
			}
		}
	}

	return false;
}

void __cdecl DisableBaddieAI(__int16 itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	
	item->data = NULL;
	if (creature)
	{
		creature->itemNum = NO_ITEM;
		SlotsUsed--;
	}
}

void InitialiseCustomObjects(__int16 itemNum, __int16 slot)
{
	CREATURE_INFO* creature = &BaddieSlots[slot];
	ITEM_INFO* item = &Items[itemNum];

	switch (item->objectNumber)
	{
	case ID_SCUBA_DIVER:
	case ID_SHARK:
	case ID_BARRACUDA:
		creature->LOT.step = 20480;
		creature->LOT.drop = -20480;
		creature->LOT.fly = 32;
		creature->LOT.zone = 4;
		break;

	case ID_TRIBESMAN_WITH_AX:
		creature->LOT.step = 256;
		creature->LOT.drop = -256;
		creature->LOT.isAmphibious = false;
		break;

	case ID_APE:
		// can climb only 2 click and fall 2 click
		creature->LOT.step = 512;
		creature->LOT.drop = -512;
		creature->LOT.zone = 3;
		break;
	}
}

void __cdecl InitialiseSlot(__int16 itemNum, __int16 slot)
{
	CREATURE_INFO* creature = &BaddieSlots[slot];
	ITEM_INFO* item = &Items[itemNum];

	item->data = creature;
	creature->itemNum = itemNum;
	creature->mood = BORED_MOOD;
	creature->jointRotation[0] = 0;
	creature->jointRotation[1] = 0;
	creature->jointRotation[2] = 0;
	creature->jointRotation[3] = 0;
	creature->alerted = false;
	creature->headLeft = false;
	creature->headRight = false;
	creature->reachedGoal = false;
	creature->hurtByLara = false;
	creature->patrol2 = false;
	creature->jumpAhead = false;
	creature->monkeyAhead = false;
	creature->alerted = false;
	creature->LOT.canJump = false;
	creature->LOT.canMonkey = false;
	creature->LOT.isAmphibious = true;
	creature->LOT.isJumping = false;
	creature->LOT.isMonkeying = false;
	creature->maximumTurn = ONE_DEGREE;
	creature->flags = 0;
	creature->enemy = NULL;
	creature->LOT.step = 256;
	creature->LOT.drop = -512;
	creature->LOT.blockMask = 0x4000;
	creature->LOT.fly = 0;
	creature->LOT.zone = 1;
	  
	switch (item->objectNumber)
	{
	case ID_MP_WITH_STICK:
	case ID_MONKEY:
		// Can climb
		creature->LOT.step = 1024;
		creature->LOT.drop = -1024;
		creature->LOT.zone = 3;
		break;

	case ID_SAS:
	case ID_BLUE_GUARD:
	case ID_MAFIA2:
	case ID_SAILOR:
		// Can climb and jjump
		creature->LOT.step = 1024;
		creature->LOT.drop = -1024;
		creature->LOT.canJump = true;
		creature->LOT.zone = 3;
		break;

	case ID_HITMAN:
	case ID_BADDY1:
	case ID_BADDY2:
		// Can climb, jump, monkey
		creature->LOT.step = 1024;
		creature->LOT.drop = -1024;
		creature->LOT.canJump = true;
		creature->LOT.canMonkey = true;
		creature->LOT.zone = 3;
		break;

	case ID_SKELETON:
		// Can jump
		creature->LOT.step = 256;
		creature->LOT.drop = -256;
		creature->LOT.canJump = true;
		break;

	case ID_CROW:
	case ID_EAGLE:
	case ID_WILLOWISP:
	case ID_REAPER:
	case ID_GREEN_TEETH:
	case ID_ATTACK_SUB:
	case ID_BAT:
	case ID_HARPY:
		// Can fly
		creature->LOT.step = 20480;
		creature->LOT.drop = -20480;
		creature->LOT.fly = 16;
		creature->LOT.zone = 4;
		break;

	case ID_SCUBA_DIVER:
	case ID_SHARK:
	case ID_BARRACUDA:
	case ID_CROCODILE:
		// Can swim
		creature->LOT.step = 20480;
		creature->LOT.drop = -20480;
		creature->LOT.fly = 32;
		creature->LOT.zone = 4;
		break;
	}

	// Hook for initialising custom objects in a separate function
	InitialiseCustomObjects(itemNum, slot);

	ClearLOT(&creature->LOT);

	if (itemNum != Lara.itemNumber)
		CreateZone(item);

	SlotsUsed++;
}

void __cdecl ClearLOT(LOT_INFO* LOT)
{
	LOT->head = NO_BOX;
	LOT->tail = NO_BOX;
	LOT->searchNumber = 0;
	LOT->targetBox = NO_BOX;
	LOT->requiredBox = NO_BOX;

	BOX_NODE* node = LOT->node;
	for (__int32 i = 0; i < NumberBoxes; i++)
	{
		node->exitBox = NO_BOX;
		node->nextExpansion = NO_BOX;
		node->searchNumber = 0;
		node++;
	}
}

void __cdecl CreateZone(ITEM_INFO* item)
{
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ROOM_INFO* r = &Rooms[item->roomNumber];

	item->boxNumber = XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z).box;

	if (creature->LOT.fly)
	{
		BOX_NODE* node = creature->LOT.node;
		creature->LOT.zoneCount = 0;

		for (__int32 i = 0; i < NumberBoxes; i++)
		{
			node->boxNumber = i;
			node++;
			creature->LOT.zoneCount++;
		}
	}
	else
	{
		__int16* zone = GroundZones[creature->LOT.zone];
		__int16* flippedZone = GroundZones[creature->LOT.zone + 1];

		__int16 zoneNumber = zone[item->boxNumber];
		__int16 flippedZoneNumber = flippedZone[item->boxNumber];

		BOX_NODE* node = creature->LOT.node;
		creature->LOT.zoneCount = 0;

		for (__int32 i = 0; i < NumberBoxes; i++)
		{
			if (*zone == zoneNumber || *flippedZone == flippedZoneNumber)
			{
				node->boxNumber = i;
				node++;
				creature->LOT.zoneCount++;
			}

			zone++;
			flippedZone++;
		}
	}
}

void Inject_Lot()
{
	INJECT(0x0045B0C0, InitialiseLOTarray);
	INJECT(0x0045B1A0, EnableBaddieAI);
	INJECT(0x0045B150, DisableBaddieAI);
	INJECT(0x0045B740, ClearLOT);
	INJECT(0x0045B5E0, CreateZone);
}