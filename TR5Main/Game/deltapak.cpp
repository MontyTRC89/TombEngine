#include "deltapak.h"
#include "..\Global\global.h"
#include <stdio.h>
#include "../Specific/level.h"

ITEM_INFO* FindItem(short objectNumber)
{
#ifdef _DEBUG
	printf("Called FindItem()\n");
#endif

	//DB_Log(0, "FindItem - DLL");

	if (LevelItems > 0)
	{
		for (int i = 0; i < LevelItems; i++)
		{
			if (Items[i].objectNumber == objectNumber)
				return &Items[i];
		}
	}

	return NULL;
}

void Inject_DeltaPak()
{
	INJECT(0x00423470, FindItem);
}