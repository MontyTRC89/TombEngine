#include "framework.h"
#include "Game/misc.h"

#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Specific/setup.h"
#include "Specific/level.h"

using std::vector;

CREATURE_INFO* GetCreatureInfo(ITEM_INFO* item)
{
    return (CREATURE_INFO*)item->data;
}

void TargetNearestEntity(ITEM_INFO* item, CREATURE_INFO* creature)
{
	int bestDistance = MAXINT;
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		auto* target = &g_Level.Items[i];

		if (target == nullptr)
			continue;

		if (target != item &&
			target->hitPoints > 0 &&
			target->status != ITEM_INVISIBLE)
		{
			int x = target->pos.xPos - item->pos.xPos;
			int y = target->pos.yPos - item->pos.yPos;
			int z = target->pos.zPos - item->pos.zPos;

			int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);
			if (distance < bestDistance)
			{
				creature->enemy = target;
				bestDistance = distance;
			}
		}
	}
}
