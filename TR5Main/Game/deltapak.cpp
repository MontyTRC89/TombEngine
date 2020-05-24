#include "framework.h"
#include "deltapak.h"
#include "global.h"
#include "level.h"

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
