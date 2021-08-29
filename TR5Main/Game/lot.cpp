#include "framework.h"
#include "lot.h"
#include "box.h"
#include "setup.h"
#include "camera.h"
#include "lara.h"
#include "level.h"
#include "creature_info.h"
#define DEFAULT_FLY_UPDOWN_SPEED 16
#define DEFAULT_SWIM_UPDOWN_SPEED 32

int SlotsUsed;
std::vector<CREATURE_INFO> BaddieSlots;

void InitialiseLOTarray(int allocMem)
{
	if (allocMem)
	{
		BaddieSlots.clear();
		BaddieSlots.resize(NUM_SLOTS);
	}

	CREATURE_INFO* creature = BaddieSlots.data();
	for (int i = 0; i < NUM_SLOTS; i++, creature++)
	{
		creature->itemNum = NO_ITEM;
		if (allocMem)
		{
			creature->LOT.node.clear();
			creature->LOT.node.resize(g_Level.Boxes.size());
			for (int j = 0; j < g_Level.Boxes.size(); j++)
			{
				creature->LOT.node.emplace_back(BOX_NODE());
			}
		}
	}

	SlotsUsed = 0;
}

int EnableBaddieAI(short itemNum, int always)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (item->data != NULL)
		return true;

	if (SlotsUsed >= NUM_SLOTS)
	{
		int cameraDistance = 0;
		if (!always)
		{
			int deltaX = (item->pos.xPos - Camera.pos.x) / 256;
			int deltaY = (item->pos.yPos - Camera.pos.y) / 256;
			int deltaZ = (item->pos.zPos - Camera.pos.z) / 256;
			cameraDistance = SQUARE(deltaX) + SQUARE(deltaY) + SQUARE(deltaZ);
		}

		int slotToDisable = -1;
		CREATURE_INFO* creature = BaddieSlots.data();
		for (int slot = 0; slot < NUM_SLOTS; slot++, creature++)
		{
			item = &g_Level.Items[creature->itemNum];

			int deltaX = (item->pos.xPos - Camera.pos.x) / 256;
			int deltaY = (item->pos.yPos - Camera.pos.y) / 256;
			int deltaZ = (item->pos.zPos - Camera.pos.z) / 256;
			int distance = SQUARE(deltaX) + SQUARE(deltaY) + SQUARE(deltaZ);

			if (distance > cameraDistance)
			{
				cameraDistance = distance;
				slotToDisable = slot;
			}
		}

		if (slotToDisable < 0 || slotToDisable > NUM_SLOTS)
			return false;

		ITEM_INFO* itemToDisable = &g_Level.Items[BaddieSlots[slotToDisable].itemNum];
		CREATURE_INFO* creatureToDisable = &BaddieSlots[slotToDisable];

		itemToDisable->status = ITEM_INVISIBLE;
		DisableBaddieAI(creatureToDisable->itemNum);
		InitialiseSlot(itemNum, slotToDisable);
		return true;
	}
	else
	{
		CREATURE_INFO* creature = BaddieSlots.data();
		for (int slot = 0; slot < NUM_SLOTS; slot++, creature++)
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

void DisableBaddieAI(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	
	item->data = NULL;
	if (creature)
	{
		creature->itemNum = NO_ITEM;
		SlotsUsed--;
	}
}

void InitialiseSlot(short itemNum, short slot)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	OBJECT_INFO* obj = &Objects[item->objectNumber];
	CREATURE_INFO* creature = &BaddieSlots[slot];

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
	creature->LOT.canJump = false;
	creature->LOT.canMonkey = false;
	creature->LOT.isAmphibious = false; // only the crocodile can go water and land. (default: true)
	creature->LOT.isJumping = false;
	creature->LOT.isMonkeying = false;
	creature->maximumTurn = ANGLE(1);
	creature->flags = 0;
	creature->enemy = NULL;
	creature->LOT.fly = NO_FLYING;
	creature->LOT.blockMask = BLOCKED;

	if (obj->intelligent)
	{
		// simple check to set hitEffect to blood or smoke by default if intelligent enabled and no value assigned to hitEffect and the entity have HP !
		// undead have smoke instead of blood !
		if (obj->hitEffect == HIT_NONE)
		{
			if (obj->undead)
				obj->hitEffect = HIT_SMOKE;
			else if (!obj->undead && obj->hitPoints)
				obj->hitEffect = HIT_BLOOD;
		}
		
		obj->nonLot = false; // change to use pathfinding
	}

	switch (obj->zoneType)
	{
		default:
		case ZONE_NULL:
		    creature->LOT.step = SECTOR(1) - CLICK(3);
			creature->LOT.drop = -(SECTOR(1) - CLICK(3));
			obj->zoneType = ZONE_BASIC; // only entity that use CreatureActive() will reach InitialiseSlot() !
			break;

		case ZONE_SKELLY:
			// Can jump
			creature->LOT.step = SECTOR(1) - CLICK(3);
			creature->LOT.drop = -(SECTOR(1) - CLICK(3));
			creature->LOT.canJump = true;
			creature->LOT.zone = ZONE_SKELLY;
			break;

		case ZONE_BASIC:
			creature->LOT.step = SECTOR(1) - CLICK(3);
			creature->LOT.drop = -(SECTOR(1) - CLICK(3));
			creature->LOT.zone = ZONE_BASIC;
			break;

		case ZONE_FLYER:
			// Can fly
			creature->LOT.step = SECTOR(20);
			creature->LOT.drop = -SECTOR(20);
			creature->LOT.fly = DEFAULT_FLY_UPDOWN_SPEED;
			creature->LOT.zone = ZONE_FLYER;
			break;

		case ZONE_WATER:
			// Can swim
			creature->LOT.step = SECTOR(20);
			creature->LOT.drop = -SECTOR(20);
			creature->LOT.zone = ZONE_WATER;

			if (item->objectNumber == ID_CROCODILE)
			{
				creature->LOT.fly = DEFAULT_SWIM_UPDOWN_SPEED / 2; // is more slow than the other underwater entity
				creature->LOT.isAmphibious = true; // crocodile can walk and swim.
				creature->LOT.zone = ZONE_FLYER;
			}
			else if (item->objectNumber == ID_BIG_RAT)
			{
				creature->LOT.fly = NO_FLYING; // dont want the bigrat to be able to go in water (just the surface !)
				creature->LOT.isAmphibious = true; // bigrat can walk and swim.
			}
			else
			{
				creature->LOT.fly = DEFAULT_SWIM_UPDOWN_SPEED;
			}

			break;

		case ZONE_HUMAN_CLASSIC:
			// Can climb
			creature->LOT.step = SECTOR(1);
			creature->LOT.drop = -SECTOR(1);
			creature->LOT.zone = ZONE_HUMAN_CLASSIC;
			break;

		case ZONE_HUMAN_JUMP:
			// Can climb and jump
			creature->LOT.step = SECTOR(1);
			creature->LOT.drop = -SECTOR(1);
			creature->LOT.canJump = true;
			creature->LOT.zone = ZONE_HUMAN_CLASSIC;
			break;

		case ZONE_HUMAN_JUMP_AND_MONKEY:
			// Can climb, jump, monkey
			creature->LOT.step = SECTOR(1);
			creature->LOT.drop = -SECTOR(1);
			creature->LOT.canJump = true;
			creature->LOT.canMonkey = true;
			creature->LOT.zone = ZONE_HUMAN_CLASSIC;
			break;

		case ZONE_HUMAN_LONGJUMP_AND_MONKEY:
			// Can climb, jump, monkey, long jump
			creature->LOT.step = 1792;
			creature->LOT.drop = -1792;
			creature->LOT.canJump = true;
			creature->LOT.canMonkey = true;
			creature->LOT.zone = ZONE_VON_CROY;
			break;

		case ZONE_SPIDER:
			creature->LOT.step = SECTOR(1) - CLICK(2);
			creature->LOT.drop = -(SECTOR(1) - CLICK(2));
			creature->LOT.zone = ZONE_HUMAN_CLASSIC;
			break;

		case ZONE_BLOCKABLE:
			creature->LOT.blockMask = BLOCKABLE;
			creature->LOT.zone = ZONE_BASIC;
			break;

		case ZONE_SOPHIALEE:
			creature->LOT.step = CLICK(4);
			creature->LOT.drop = -CLICK(3);
			creature->LOT.zone = ZONE_HUMAN_CLASSIC;
			break;
	}

	ClearLOT(&creature->LOT);
	if (itemNum != Lara.itemNumber)
		CreateZone(item);

	SlotsUsed++;
}

void ClearLOT(LOT_INFO* LOT)
{
	LOT->head = NO_BOX;
	LOT->tail = NO_BOX;
	LOT->searchNumber = 0;
	LOT->targetBox = NO_BOX;
	LOT->requiredBox = NO_BOX;

	BOX_NODE* node = LOT->node.data();
	for (int i = 0; i < g_Level.Boxes.size(); i++)
	{
		node->exitBox = NO_BOX;
		node->nextExpansion = NO_BOX;
		node->searchNumber = 0;
		node++;
	}
}

void CreateZone(ITEM_INFO* item)
{
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ROOM_INFO* r = &g_Level.Rooms[item->roomNumber];

	item->boxNumber = XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z).box;

	if (creature->LOT.fly)
	{
		BOX_NODE* node = creature->LOT.node.data();
		creature->LOT.zoneCount = 0;

		for (int i = 0; i < g_Level.Boxes.size(); i++)
		{
			node->boxNumber = i;
			node++;
			creature->LOT.zoneCount++;
		}
	}
	else
	{
		int* zone = g_Level.Zones[creature->LOT.zone][0].data();
		int* flippedZone = g_Level.Zones[creature->LOT.zone][1].data();

		int zoneNumber = zone[item->boxNumber];
		int flippedZoneNumber = flippedZone[item->boxNumber];

		BOX_NODE* node = creature->LOT.node.data();
		creature->LOT.zoneCount = 0;

		for (int i = 0; i < g_Level.Boxes.size(); i++)
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
