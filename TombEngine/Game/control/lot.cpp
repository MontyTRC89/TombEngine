#include "framework.h"
#include "Game/control/lot.h"

#include "Game/control/box.h"
#include "Game/camera.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#define DEFAULT_FLY_UPDOWN_SPEED 16
#define DEFAULT_SWIM_UPDOWN_SPEED 32

int SlotsUsed;
std::vector<CreatureInfo*> ActiveCreatures;

void InitialiseLOTarray(int itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* creature = (CreatureInfo*)item->Data;

	if(!creature->LOT.Initialised)
	{
		creature->LOT.Node = std::vector<BOX_NODE>(g_Level.Boxes.size(), BOX_NODE{});
		creature->LOT.Initialised = true;
	}
}

int EnableEntityAI(short itemNum, int always, bool makeTarget)
{
	ItemInfo* item = &g_Level.Items[itemNum];

	if (item->Data.is<CreatureInfo>())
		return true;
	   	
	/*
	if (SlotsUsed >= NUM_SLOTS)
	{
		int cameraDistance = 0;
		if (!always)
		{
			int deltaX = (item->pos.Position.x - Camera.pos.x) >> 8;
			int deltaY = (item->pos.Position.y - Camera.pos.y) >> 8;
			int deltaZ = (item->pos.Position.z - Camera.pos.z) >> 8;
			cameraDistance = SQUARE(deltaX) + SQUARE(deltaY) + SQUARE(deltaZ);
		}

		int slotToDisable = -1;


		for (int slot = 0; slot < ActiveCreatures.size(); slot++)
		{
			CREATURE_INFO* creature = ActiveCreatures[slot];
			item = &g_Level.Items[creature->itemNum];

			int deltaX = (item->pos.Position.x - Camera.pos.x) >> 8;
			int deltaY = (item->pos.Position.y - Camera.pos.y) >> 8;
			int deltaZ = (item->pos.Position.z - Camera.pos.z) >> 8;
			int distance = SQUARE(deltaX) + SQUARE(deltaY) + SQUARE(deltaZ);

			if (distance > cameraDistance)
			{
				cameraDistance = distance;
				slotToDisable = slot;
			}
		}

		if (slotToDisable < 0 || slotToDisable > NUM_SLOTS)
			return false;

		ItemInfo* itemToDisable = &g_Level.Items[ActiveCreatures[slotToDisable].itemNum];
		CREATURE_INFO* creatureToDisable = &ActiveCreatures[slotToDisable];

		itemToDisable->status = ITEM_INVISIBLE;
		DisableBaddyAI(creatureToDisable->itemNum);
		InitialiseSlot(itemNum, slotToDisable);
		return true;
	}
	else*/
	{
		/*
		CREATURE_INFO* creature = ActiveCreatures.data();
		for (int slot = 0; slot < NUM_SLOTS; slot++, creature++)
		{
			if (creature->itemNum == NO_ITEM)
			{
				InitialiseSlot(itemNum, slot);
				return true;
			}
		}
		*/
		InitialiseSlot(itemNum, 0, makeTarget);
		ActiveCreatures.push_back(item->Data);
	}

	return true;
}

void DisableEntityAI(short itemNumber)
{
	ItemInfo* item = &g_Level.Items[itemNumber];

	if (!item->IsCreature())
		return;

	auto* creature = (CreatureInfo*)item->Data;
	creature->ItemNumber = NO_ITEM;
	KillItem(creature->AITargetNumber);
	ActiveCreatures.erase(std::find(ActiveCreatures.begin(), ActiveCreatures.end(), creature));
	item->Data = nullptr;
}

void InitialiseSlot(short itemNum, short slot, bool makeTarget)
{
	ItemInfo* item = &g_Level.Items[itemNum];
	ObjectInfo* obj = &Objects[item->ObjectNumber];

	item->Data = CreatureInfo();
	CreatureInfo* creature = item->Data;
	InitialiseLOTarray(itemNum);
	creature->ItemNumber = itemNum;
	creature->Mood = MoodType::Bored;
	creature->JointRotation[0] = 0;
	creature->JointRotation[1] = 0;
	creature->JointRotation[2] = 0;
	creature->JointRotation[3] = 0;
	creature->Alerted = false;
	creature->HeadLeft = false;
	creature->HeadRight = false;
	creature->ReachedGoal = false;
	creature->HurtByLara = false;
	creature->Patrol = false;
	creature->JumpAhead = false;
	creature->MonkeySwingAhead = false;
	creature->LOT.CanJump = false;
	creature->LOT.CanMonkey = false;
	creature->LOT.IsAmphibious = false; // only the crocodile can go water and land. (default: true)
	creature->LOT.IsJumping = false;
	creature->LOT.IsMonkeying = false;
	creature->MaxTurn = ANGLE(1);
	creature->Flags = 0;
	creature->Enemy = NULL;
	creature->LOT.Fly = NO_FLYING;
	creature->LOT.BlockMask = BLOCKED;

	if (makeTarget)
		creature->AITargetNumber = CreateItem();
	else
		creature->AITargetNumber = NO_ITEM;

	if (creature->AITargetNumber != NO_ITEM)
		creature->AITarget = &g_Level.Items[creature->AITargetNumber];
	else
		creature->AITarget = nullptr;

	switch (obj->zoneType)
	{
		default:
		case ZONE_NULL:
		    creature->LOT.Step = SECTOR(1) - CLICK(3);
			creature->LOT.Drop = -(SECTOR(1) - CLICK(3));
			obj->zoneType = ZONE_BASIC; // only entity that use CreatureActive() will reach InitialiseSlot() !
			break;

		case ZONE_SKELLY:
			// Can jump
			creature->LOT.Step = SECTOR(1) - CLICK(3);
			creature->LOT.Drop = -(SECTOR(1) - CLICK(3));
			creature->LOT.CanJump = true;
			creature->LOT.Zone = ZONE_SKELLY;
			break;

		case ZONE_BASIC:
			creature->LOT.Step = SECTOR(1) - CLICK(3);
			creature->LOT.Drop = -(SECTOR(1) - CLICK(3));
			creature->LOT.Zone = ZONE_BASIC;
			break;

		case ZONE_FLYER:
			// Can fly
			creature->LOT.Step = SECTOR(20);
			creature->LOT.Drop = -SECTOR(20);
			creature->LOT.Fly = DEFAULT_FLY_UPDOWN_SPEED;
			creature->LOT.Zone = ZONE_FLYER;
			break;

		case ZONE_WATER:
			// Can swim
			creature->LOT.Step = SECTOR(20);
			creature->LOT.Drop = -SECTOR(20);
			creature->LOT.Zone = ZONE_WATER;

			if (item->ObjectNumber == ID_CROCODILE)
			{
				creature->LOT.Fly = DEFAULT_SWIM_UPDOWN_SPEED / 2; // is more slow than the other underwater entity
				creature->LOT.IsAmphibious = true; // crocodile can walk and swim.
				creature->LOT.Zone = ZONE_FLYER;
			}
			else if (item->ObjectNumber == ID_BIG_RAT)
			{
				creature->LOT.Fly = NO_FLYING; // dont want the bigrat to be able to go in water (just the surface !)
				creature->LOT.IsAmphibious = true; // bigrat can walk and swim.
			}
			else
			{
				creature->LOT.Fly = DEFAULT_SWIM_UPDOWN_SPEED;
			}

			break;

		case ZONE_HUMAN_CLASSIC:
			// Can climb
			creature->LOT.Step = SECTOR(1);
			creature->LOT.Drop = -SECTOR(1);
			creature->LOT.Zone = ZONE_HUMAN_CLASSIC;
			break;

		case ZONE_HUMAN_JUMP:
			// Can climb and jump
			creature->LOT.Step = SECTOR(1);
			creature->LOT.Drop = -SECTOR(1);
			creature->LOT.CanJump = true;
			creature->LOT.Zone = ZONE_HUMAN_CLASSIC;
			break;

		case ZONE_HUMAN_JUMP_AND_MONKEY:
			// Can climb, jump, monkey
			creature->LOT.Step = SECTOR(1);
			creature->LOT.Drop = -SECTOR(1);
			creature->LOT.CanJump = true;
			creature->LOT.CanMonkey = true;
			creature->LOT.Zone = ZONE_HUMAN_CLASSIC;
			break;

		case ZONE_HUMAN_LONGJUMP_AND_MONKEY:
			// Can climb, jump, monkey, long jump
			creature->LOT.Step = 1792;
			creature->LOT.Drop = -1792;
			creature->LOT.CanJump = true;
			creature->LOT.CanMonkey = true;
			creature->LOT.Zone = ZONE_VON_CROY;
			break;

		case ZONE_SPIDER:
			creature->LOT.Step = SECTOR(1) - CLICK(2);
			creature->LOT.Drop = -(SECTOR(1) - CLICK(2));
			creature->LOT.Zone = ZONE_HUMAN_CLASSIC;
			break;

		case ZONE_BLOCKABLE:
			creature->LOT.BlockMask = BLOCKABLE;
			creature->LOT.Zone = ZONE_BASIC;
			break;

		case ZONE_SOPHIALEE:
			creature->LOT.Step = CLICK(4);
			creature->LOT.Drop = -CLICK(3);
			creature->LOT.Zone = ZONE_HUMAN_CLASSIC;
			break;
	}

	ClearLOT(&creature->LOT);
	if (itemNum != Lara.ItemNumber)
		CreateZone(item);

	SlotsUsed++;
}

void SetBaddyTarget(short itemNum, short target)
{
	ItemInfo* item = &g_Level.Items[itemNum];

	CreatureInfo* creature = item->Data;

	creature->AITargetNumber = target;

	if (creature->AITargetNumber != NO_ITEM)
		creature->AITarget = &g_Level.Items[creature->AITargetNumber];
	else
		creature->AITarget = nullptr;
}

void ClearLOT(LOTInfo* LOT)
{
	LOT->Head = NO_BOX;
	LOT->Tail = NO_BOX;
	LOT->SearchNumber = 0;
	LOT->TargetBox = NO_BOX;
	LOT->RequiredBox = NO_BOX;

	BOX_NODE* node = LOT->Node.data();
	for(auto& node : LOT->Node) 
	{
		node.exitBox = NO_BOX;
		node.nextExpansion = NO_BOX;
		node.searchNumber = 0;
	}
}

void CreateZone(ItemInfo* item)
{
	CreatureInfo* creature = (CreatureInfo*)item->Data;
	ROOM_INFO* r = &g_Level.Rooms[item->RoomNumber];

	item->BoxNumber = GetSector(r, item->Pose.Position.x - r->x, item->Pose.Position.z - r->z)->Box;

	if (creature->LOT.Fly)
	{
		BOX_NODE* node = creature->LOT.Node.data();
		creature->LOT.ZoneCount = 0;

		for (int i = 0; i < g_Level.Boxes.size(); i++)
		{
			node->boxNumber = i;
			node++;
			creature->LOT.ZoneCount++;
		}
	}
	else
	{
		int* zone = g_Level.Zones[creature->LOT.Zone][0].data();
		int* flippedZone = g_Level.Zones[creature->LOT.Zone][1].data();

		int zoneNumber = zone[item->BoxNumber];
		int flippedZoneNumber = flippedZone[item->BoxNumber];

		BOX_NODE* node = creature->LOT.Node.data();
		creature->LOT.ZoneCount = 0;

		for (int i = 0; i < g_Level.Boxes.size(); i++)
		{
			if (*zone == zoneNumber || *flippedZone == flippedZoneNumber)
			{
				node->boxNumber = i;
				node++;
				creature->LOT.ZoneCount++;
			}

			zone++;
			flippedZone++;
		}
	}
}
