#include "framework.h"
#include "Objects/TR5/Object/tr5_raisingblock.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Setup.h"
#include "Objects/objectslist.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;

void InitializeRaisingBlock(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	short roomNumber = item->RoomNumber;
	auto* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);

	if (floor->Box != NO_BOX)
		g_Level.Boxes[floor->Box].flags &= ~BLOCKED;

	// Set mutators to EulerAngles identity by default.
	for (auto& mutator : item->Model.Mutators)
		mutator.Scale.y = 0;

	if (item->TriggerFlags < 0)
	{
		item->AIBits |= ALL_AIOBJ;
		AddActiveItem(itemNumber);
		item->Status = ITEM_ACTIVE;
	}

	TEN::Collision::Floordata::UpdateBridgeItem(itemNumber);
}

void ShakeRaisingBlock(ItemInfo* item)
{
	SoundEffect(SFX_TR4_RAISING_BLOCK, &item->Pose);

	if (item->TriggerFlags == 0)
		return;

	if ((item->Pose.Position.ToVector3() - Camera.pos.ToVector3()).Length() < BLOCK(10))
	{
		if (item->ItemFlags[1] == 64 || item->ItemFlags[1] == 4096)
			Camera.bounce = -32;
		else
			Camera.bounce = -16;
	}
}

void ControlRaisingBlock(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (!item->ItemFlags[2])
		{
			item->ItemFlags[2] = 1;
		}

		if (item->TriggerFlags < 0)
		{
			item->ItemFlags[1] = 1;
		}
		else if (item->ItemFlags[1] < 4096)
		{
			ShakeRaisingBlock(item);
			item->ItemFlags[1] += 64;
		}
	}
	else if (item->ItemFlags[1] <= 0 || item->TriggerFlags < 0)
	{
		if (item->ItemFlags[2])
		{
			item->ItemFlags[1] = 0;

			if (item->ObjectNumber == ID_RAISING_BLOCK1)
			{
				if (item->TriggerFlags == -1)
				{
					item->ItemFlags[2] = 0;
				}
				else if (item->TriggerFlags == -3)
				{
					item->ItemFlags[2] = 0;
				}
			}

			item->ItemFlags[2] = 0;
		}
	}
	else
	{
		ShakeRaisingBlock(item);
		item->ItemFlags[1] -= 64;
	}

	// Update bone mutators.
	if (item->TriggerFlags > -1)
	{
		for (auto& mutator : item->Model.Mutators)
			mutator.Scale = Vector3(1.0f, item->ItemFlags[1] / 4096.0f, 1.0f);
	}
}

std::optional<int> RaisingBlockFloor(short itemNumber, int x, int y, int z)
{
	auto bboxHeight = GetBridgeItemIntersect(Vector3i(x, y, z), itemNumber, false);

	if (bboxHeight.has_value())
	{
		auto* item = &g_Level.Items[itemNumber];

		auto bounds = GameBoundingBox(item);
		int height = bounds.GetHeight();

		int currentHeight = item->Pose.Position.y - height * item->ItemFlags[1] / 4096;
		return std::optional{ currentHeight };
	}

	return bboxHeight;
}

std::optional<int> RaisingBlockCeiling(short itemNumber, int x, int y, int z)
{
	auto bboxHeight = GetBridgeItemIntersect(Vector3i(x, y, z), itemNumber, true);

	if (bboxHeight.has_value())
		return std::optional{ bboxHeight.value() + 1 };
	else 
		return bboxHeight;
}

int RaisingBlockFloorBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, false);
}

int RaisingBlockCeilingBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, true);
}
