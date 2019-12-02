#include "traps.h"
#include "..\Global\global.h"
#include "items.h"

void LaraBurn()
{
	if (!Lara.burn && !Lara.burnSmoke)
	{
		short fxNum = CreateNewEffect(LaraItem->roomNumber);
		if (fxNum != NO_ITEM)
		{
			Effects[fxNum].objectNumber = ID_FLAME;
			Lara.burn = true;
		}
	}
}
