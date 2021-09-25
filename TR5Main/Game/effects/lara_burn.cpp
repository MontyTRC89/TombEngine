#include "framework.h"
#include "Game/Lara/lara.h"
#include "Game/effects/effects.h"
#include "Game/floordata.h"
#include "Game/effects/lara_burn.h"
#include "Game/items.h"
#include "Game/control/control.h"
#include "item.h"

namespace TEN::Effects::Fire
{
	void LaraBurn()
	{
		if (!Lara.burn && !Lara.burnSmoke)
		{
			short fxNum = CreateNewEffect(LaraItem->roomNumber);
			if (fxNum != NO_ITEM)
			{
				EffectList[fxNum].objectNumber = ID_FLAME;
				Lara.burn = true;
			}
		}
	}

	void LavaBurn(ITEM_INFO* item)
	{
		if (item->hitPoints >= 0 && Lara.waterStatus != LW_FLYCHEAT)
		{
			short roomNumber = item->roomNumber;
			FLOOR_INFO* floor = GetFloor(item->pos.xPos, 32000, item->pos.zPos, &roomNumber);
			if (item->floor == GetFloorHeight(floor, item->pos.xPos, 32000, item->pos.zPos))
			{
				//			if (Objects[ID_KAYAK].loaded && Objects[ID_KAYAK_LARA_ANIMS].loaded)		//TEMPORARILY ADDING THIS HACK FOR TESTING-// KayakLaraRapidsDrown works fine.
				//				KayakLaraRapidsDrown();
				//			else
				//			{
				item->hitPoints = -1;
				item->hitStatus = true;
				LaraBurn();
				//			}
			}
		}
	}
}