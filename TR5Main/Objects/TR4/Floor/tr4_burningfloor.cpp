#include "framework.h"
#include "tr4_burningfloor.h"
#include "Specific/level.h"
#include "Game/items.h"

void InitialiseBurningFloor(short itemNum)
{
	g_Level.Items[itemNum].requiredState = 127;
}

void BurningFloorControl(short itemNum)
{

}