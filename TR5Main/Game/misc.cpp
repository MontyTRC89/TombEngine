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
	int bestDistance = MAXINT;
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		auto* target = &g_Level.Items[i];

		if (target == nullptr)
			continue;

		if (target != item &&
			target->HitPoints > 0 &&
			target->Status != ITEM_INVISIBLE)
		{
			int x = target->Pose.Position.x - item->Pose.Position.x;
			int y = target->Pose.Position.y - item->Pose.Position.y;
			int z = target->Pose.Position.z - item->Pose.Position.z;

			int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);
			if (distance < bestDistance)
			{
				creature->Enemy = target;
				bestDistance = distance;
			}
		}
	}
}
