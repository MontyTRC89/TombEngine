#include "framework.h"
#include "tr4_burningfloor.h"
#include "Specific/level.h"
#include "Game/items.h"

void InitialiseBurningFloor(short itemNumber)
{
	g_Level.Items[itemNumber].RequiredState = 127;
}

void BurningFloorControl(short itemNumber)
{

}
