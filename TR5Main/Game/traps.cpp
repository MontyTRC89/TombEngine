#include "traps.h"
#include "..\Global\global.h"
#include "items.h"

void __cdecl LaraBurn()
{
	if (!Lara.burn && !Lara.burnSmoke)
	{
		__int16 fxNum = CreateNewEffect(LaraItem->roomNumber);
		if (fxNum != NO_ITEM)
		{
			Effects[fxNum].objectNumber = ID_FLAME;
			Lara.burn = true;
		}
	}
}