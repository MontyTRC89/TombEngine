#include "framework.h"
#include "misc.h"
#include "setup.h"
#include "level.h"
#include "lara.h"
#include "animation.h"
#include "itemdata/creature_info.h"
#include "items.h"

using std::vector;

CREATURE_INFO* GetCreatureInfo(ITEM_INFO* item)
{
    return (CREATURE_INFO*)item->data;
}

void TargetNearestEntity(ITEM_INFO* item, CREATURE_INFO* creature)
{
	ITEM_INFO* target;
	int bestdistance;
	int distance;
	int x, z;

	bestdistance = MAXINT;
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		target = &g_Level.Items[i];

		if (target == nullptr)
			continue;

		if (target != item && target->hitPoints > 0 && target->status != ITEM_INVISIBLE)
		{
			x = target->pos.xPos - item->pos.xPos;
			z = target->pos.zPos - item->pos.zPos;
			distance = SQUARE(z) + SQUARE(x);
			if (distance < bestdistance)
			{
				creature->enemy = target;
				bestdistance = distance;
			}
		}
	}
}