#include "framework.h"
#include "Game/control/lot.h"

#include "Game/control/box.h"
#include "Game/camera.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/misc.h"
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
	auto* creature = GetCreatureInfo(item);

	if (!creature->LOT.Initialised)
	{
		creature->LOT.Node = std::vector<BoxNode>(g_Level.Boxes.size(), BoxNode{});
		creature->LOT.Initialised = true;
	}
}

bool EnableEntityAI(short itemNum, bool always, bool makeTarget)
{
	ItemInfo* item = &g_Level.Items[itemNum];

	if (item->IsCreature())
		return true;

	InitialiseSlot(itemNum, makeTarget);
	ActiveCreatures.push_back(item->Data);

	return item->IsCreature();
}

void DisableEntityAI(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!item->IsCreature())
		return;

	auto* creature = GetCreatureInfo(item);
	creature->ItemNumber = NO_ITEM;
	KillItem(creature->AITargetNumber);
	ActiveCreatures.erase(std::find(ActiveCreatures.begin(), ActiveCreatures.end(), creature));
	item->Data = nullptr;
}

void InitialiseSlot(short itemNumber, bool makeTarget)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* object = &Objects[item->ObjectNumber];
	item->Data = CreatureInfo();
	auto* creature = GetCreatureInfo(item);

	InitialiseLOTarray(itemNumber);
	creature->ItemNumber = itemNumber;
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
	creature->MaxTurn = ANGLE(1);
	creature->Flags = 0;
	creature->Enemy = nullptr;

	creature->LOT.CanJump = false;
	creature->LOT.CanMonkey = false;
	creature->LOT.IsAmphibious = false; // True for crocodile by default as only the crocodile can move in water and on land.
	creature->LOT.IsJumping = false;
	creature->LOT.IsMonkeying = false;
	creature->LOT.Fly = NO_FLYING;
	creature->LOT.BlockMask = BLOCKED;
	creature->AITargetNumber = NO_ITEM;
	creature->AITarget = nullptr;

	if (makeTarget)
	{
		creature->AITargetNumber = CreateItem();
		if (creature->AITargetNumber != NO_ITEM)
			creature->AITarget = &g_Level.Items[creature->AITargetNumber];
	}

	switch (object->LotType)
	{
		default:
		case LotType::Basic:
			creature->LOT.Step = CLICK(1);
			creature->LOT.Drop = -CLICK(2);
			creature->LOT.Zone = ZoneType::Basic;
			break;

		// Can jump.
		case LotType::Skeleton:
			creature->LOT.Step = CLICK(1);
			creature->LOT.Drop = -CLICK(2);
			creature->LOT.CanJump = true;
			creature->LOT.Zone = ZoneType::Skeleton;
			break;

		// Can fly.
		case LotType::Flyer:
			creature->LOT.Step = SECTOR(20);
			creature->LOT.Drop = -SECTOR(20);
			creature->LOT.Fly = DEFAULT_FLY_UPDOWN_SPEED;
			creature->LOT.Zone = ZoneType::Flyer;
			break;

		// Can swim.
		case LotType::Water:
			creature->LOT.Step = SECTOR(20);
			creature->LOT.Drop = -SECTOR(20);
			creature->LOT.Zone = ZoneType::Water;

			if (item->ObjectNumber == ID_CROCODILE)
			{
				creature->LOT.Fly = DEFAULT_SWIM_UPDOWN_SPEED / 2; // Slower than the other underwater creatures.
				creature->LOT.IsAmphibious = true;				   // Can walk and swim.
			}
			else if (item->ObjectNumber == ID_BIG_RAT)
			{
				creature->LOT.Fly = NO_FLYING;	   // Can't swim underwater, only on the surface.
				creature->LOT.IsAmphibious = true; // Can walk and swim.
			}
			else
			{
				creature->LOT.Fly = DEFAULT_SWIM_UPDOWN_SPEED;
			}

			break;

		// Can climb.
		case LotType::Human:
			creature->LOT.Step = BLOCK(1);
			creature->LOT.Drop = -BLOCK(1);
			creature->LOT.Zone = ZoneType::Human;
			break;

		// Can climb and jump.
		case LotType::HumanPlusJump:
			creature->LOT.Step = BLOCK(1);
			creature->LOT.Drop = -BLOCK(1);
			creature->LOT.CanJump = true;
			creature->LOT.Zone = ZoneType::Human;
			break;

		// Can climb, jump, monkeyswing.
		case LotType::HumanPlusJumpAndMonkey:
			creature->LOT.Step = BLOCK(1);
			creature->LOT.Drop = -BLOCK(1);
			creature->LOT.CanJump = true;
			creature->LOT.CanMonkey = true;
			creature->LOT.Zone = ZoneType::Human;
			break;

		case LotType::Spider:
			creature->LOT.Step = CLICK(2);
			creature->LOT.Drop = -BLOCK(1);
			creature->LOT.Zone = ZoneType::Human;
			break;

		case LotType::Blockable:
			creature->LOT.Step = CLICK(1);
			creature->LOT.Drop = -CLICK(2);
			creature->LOT.BlockMask = BLOCKABLE;
			creature->LOT.Zone = ZoneType::Basic;
			break;

		case LotType::Ape:
			creature->LOT.Step = CLICK(2);
			creature->LOT.Drop = -BLOCK(1);
			creature->LOT.Zone = ZoneType::Human;
			break;
	}

	ClearLOT(&creature->LOT);
	if (itemNumber != Lara.ItemNumber)
		CreateZone(item);

	SlotsUsed++;
}

void SetEntityTarget(short itemNum, short target)
{
	auto* item = &g_Level.Items[itemNum];
	auto* creature = GetCreatureInfo(item);

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

	auto* node = LOT->Node.data();
	for(auto& node : LOT->Node) 
	{
		node.exitBox = NO_BOX;
		node.nextExpansion = NO_BOX;
		node.searchNumber = 0;
	}
}

void CreateZone(ItemInfo* item)
{
	auto* creature = GetCreatureInfo(item);
	auto* room = &g_Level.Rooms[item->RoomNumber];

	item->BoxNumber = GetSector(room, item->Pose.Position.x - room->x, item->Pose.Position.z - room->z)->Box;

	if (creature->LOT.Fly)
	{
		auto* node = creature->LOT.Node.data();
		creature->LOT.ZoneCount = 0;

		for (int i = 0; i < (int)g_Level.Boxes.size(); i++)
		{
			node->boxNumber = i;
			node++;
			creature->LOT.ZoneCount++;
		}
	}
	else
	{
		int* zone = g_Level.Zones[(int)creature->LOT.Zone][0].data();
		int* flippedZone = g_Level.Zones[(int)creature->LOT.Zone][1].data();

		int zoneNumber = zone[item->BoxNumber];
		int flippedZoneNumber = flippedZone[item->BoxNumber];

		auto* node = creature->LOT.Node.data();
		creature->LOT.ZoneCount = 0;

		for (int i = 0; i < (int)g_Level.Boxes.size(); i++)
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
