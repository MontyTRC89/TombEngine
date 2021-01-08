#include "framework.h"
#include "tr5_explosion.h"
#include "level.h"

void InitialiseExplosion(short itemNumber)
{
	ITEM_INFO* item;

	item = &g_Level.Items[itemNumber];
	if (item->triggerFlags >= 30000)
	{
		item->itemFlags[1] = 3;
		item->triggerFlags -= 30000;
	}
	else if (item->triggerFlags >= 20000)
	{
		item->itemFlags[1] = 2;
		item->triggerFlags -= 20000;
	}
	else if (item->triggerFlags >= 10000)
	{
		item->itemFlags[1] = 1;
		item->triggerFlags -= 10000;
	}

	if (item->triggerFlags >= 1000)
	{
		item->itemFlags[3] = 1;
		item->triggerFlags -= 1000;
	}

	item->itemFlags[2] = item->triggerFlags / 100;
	item->triggerFlags = 7 * (item->triggerFlags % 100);
}

void ExplosionControl(short itemNumber)
{

}
