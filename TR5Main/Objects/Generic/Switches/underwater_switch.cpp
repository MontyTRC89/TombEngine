#include "framework.h"
#include "control.h"
#include "input.h"
#include "lara.h"
#include "underwater_switch.h"
#include "newinv2.h"
#include "sound.h"
#include "generic_switch.h"
#include "camera.h"
#include "collide.h"
#include "level.h"

namespace TEN::Entities::Switches
{ 
	OBJECT_COLLISION_BOUNDS UnderwaterSwitchBounds =
	{
		-1024, 1024,
		-1024, 1024,
		-1024, 512,
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80)
	};

	PHD_VECTOR UnderwaterSwitchPos = { 0, 0, 108 };

	OBJECT_COLLISION_BOUNDS CeilingUnderwaterSwitchBounds1 =
	{
		-256, 256,
		-1280, -512,
		-512, 0,
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80)
	};

	PHD_VECTOR CeilingUnderwaterSwitchPos1 = { 0, -736, -416 };

	OBJECT_COLLISION_BOUNDS CeilingUnderwaterSwitchBounds2 =
	{
		-256, 256,
		-1280, -512,
		0, 512,
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80)
	};

	PHD_VECTOR CeilingUnderwaterSwitchPos2 = { 0, -736, 416 };

	void UnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (TrInput & IN_ACTION)
		{
			if (item->status == ITEM_NOT_ACTIVE
				&& Lara.waterStatus == LW_UNDERWATER
				&& !Lara.gunStatus
				&& l->currentAnimState == LS_UNDERWATER_STOP)
			{
				if (TestLaraPosition(&UnderwaterSwitchBounds, item, l))
				{
					if (item->currentAnimState == SWITCH_ON
						|| item->currentAnimState == SWITCH_OFF)
					{
						if (MoveLaraPosition(&UnderwaterSwitchPos, item, l))
						{
							l->fallspeed = 0;
							l->goalAnimState = LS_SWITCH_DOWN;

							do
							{
								AnimateLara(l);
							} while (l->goalAnimState != LS_SWITCH_DOWN);

							l->goalAnimState = LS_UNDERWATER_STOP;
							Lara.gunStatus = LG_HANDS_BUSY;
							item->goalAnimState = item->currentAnimState != SWITCH_ON;
							item->status = ITEM_ACTIVE;
							AddActiveItem(itemNum);
							AnimateItem(item);
						}
					}
				}
			}
		}
	}

	void CeilingUnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];
		int flag = 0;

		if (TrInput & IN_ACTION
			&& Lara.waterStatus == LW_UNDERWATER
			&& l->currentAnimState == LS_UNDERWATER_STOP
			&& l->animNumber == LA_UNDERWATER_IDLE
			&& !Lara.gunStatus
			&& (item->currentAnimState == SWITCH_OFF)
			|| Lara.isMoving && Lara.generalPtr == (void*)itemNum)
		{
			if (TestLaraPosition(&CeilingUnderwaterSwitchBounds1, item, l))
			{
				if (MoveLaraPosition(&CeilingUnderwaterSwitchPos1, item, l))
					flag = 1;
				else
					Lara.generalPtr = (void*)itemNum;
			}
			else
			{
				l->pos.yRot ^= 0x8000;

				if (TestLaraPosition(&CeilingUnderwaterSwitchBounds2, item, l))
				{
					if (MoveLaraPosition(&CeilingUnderwaterSwitchPos2, item, l))
						flag = 1;
					else
						Lara.generalPtr = (void*)itemNum;
				}

				l->pos.yRot ^= 0x8000;
			}

			if (flag)
			{
				l->currentAnimState = LS_SWITCH_DOWN;
				l->animNumber = LA_UNDERWATER_CEILING_SWITCH_PULL;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				l->fallspeed = 0;
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_BUSY;
				item->goalAnimState = SWITCH_ON;
				item->status = ITEM_ACTIVE;

				AddActiveItem(itemNum);

				ForcedFixedCamera.x = item->pos.xPos - 1024 * phd_sin(item->pos.yRot + ANGLE(90));
				ForcedFixedCamera.y = item->pos.yPos - 1024;
				ForcedFixedCamera.z = item->pos.zPos - 1024 * phd_cos(item->pos.yRot + ANGLE(90));
				ForcedFixedCamera.roomNumber = item->roomNumber;
			}
		}
	}
}