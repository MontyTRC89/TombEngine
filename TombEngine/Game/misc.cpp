#include "framework.h"
#include "Game/misc.h"

#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Specific/setup.h"
#include "Specific/level.h"

using std::vector;

CreatureInfo* GetCreatureInfo(ItemInfo* item)
{
    return (CreatureInfo*)item->Data;
}

void TargetNearestEntity(ItemInfo* item, CreatureInfo* creature)
{
	float bestDistance = FLT_MAX;
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		auto* targetEntity = &g_Level.Items[i];

		if (targetEntity == nullptr)
			continue;

		if (targetEntity != item &&
			targetEntity->HitPoints > 0 &&
			targetEntity->Status != ITEM_INVISIBLE)
		{
			float distance = Vector3i::Distance(item->Pose.Position, targetEntity->Pose.Position);
			if (distance < bestDistance)
			{
				creature->Enemy = targetEntity;
				bestDistance = distance;
			}
		}
	}
}
