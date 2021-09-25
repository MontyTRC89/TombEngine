#include "framework.h"
#include "tr4_burningfloor.h"
#include "level.h"
#include "items.h"

void InitialiseBurningFloor(short itemNum)
{
	g_Level.Items[itemNum].requiredAnimState = 127;
}

void BurningFloorControl(short itemNum)
{

}