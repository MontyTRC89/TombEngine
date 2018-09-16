#include "objects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\effects.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"

void __cdecl FourBladesControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (frameNumber <= 5 || frameNumber >= 58 || frameNumber >= 8 && frameNumber <= 54)
			item->itemFlags[0] = 0;
		else
		{
			if (frameNumber >= 6 && frameNumber <= 7)
			{
				item->itemFlags[3] = 20;
				item->itemFlags[0] = 30;
			}
			else
			{
				if (frameNumber >= 55 && frameNumber <= 57)
				{
					item->itemFlags[3] = 200;
					item->itemFlags[0] = 30;
				}
			}
		}

		AnimateItem(item);
	}
}

void __cdecl BirdBladeControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->itemFlags[3] = 100;
	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (frameNumber <= 14 || frameNumber >= 31)
			item->itemFlags[0] = 0;
		else
			item->itemFlags[0] = 6;

		AnimateItem(item);
	}
}

void __cdecl CatwalkBlaldeControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
	else
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (item->frameNumber == Anims[item->animNumber].frameEnd || frameNumber < 38)
			item->itemFlags[3] = 0;
		else
			item->itemFlags[3] = 100;

		AnimateItem(item);
	}
}

void __cdecl PlinthBladeControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!TriggerActive(item))
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
	}
	else
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if (item->frameNumber == Anims[item->animNumber].frameEnd)
			item->itemFlags[3] = 0;
		else
			item->itemFlags[3] = 200;

		AnimateItem(item);
	}
}

void __cdecl InitialiseSethBlade(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->animNumber = Objects[ID_SETH_BLADE].animIndex + 1;
	item->goalAnimState = 2;
	item->currentAnimState = 2;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->itemFlags[2] = abs(item->triggerFlags);
}

void __cdecl SethBladeControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->itemFlags[0] = 0;
	if (TriggerActive(item))
	{
		if (item->currentAnimState == 2)
		{
			if (item->itemFlags[2] > 1)
			{
				item->itemFlags[2]--;
			}
			else if (item->itemFlags[2] == 1)
			{
				item->goalAnimState = 1;
				item->itemFlags[2] = 0;
			}
			else if (!item->itemFlags[2])
			{
				if (item->triggerFlags > 0)
				{
					item->itemFlags[2] = item->triggerFlags;
				}
			}
		}
		else
		{
			__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

			if (item->frameNumber != Anims[item->animNumber].frameBase && frameNumber <= 6)
			{
				item->itemFlags[0] = -1;
				item->itemFlags[3] = 1000;
			}
			else if (frameNumber >= 7 && frameNumber <= 15)
			{
				item->itemFlags[0] = 448;
				item->itemFlags[3] = 1000;
			}
			else
			{
				item->itemFlags[0] = 0;
				item->itemFlags[3] = 1000;
			}
		}

		AnimateItem(item);
	}
}

void __cdecl ChainControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->triggerFlags)
	{
		item->itemFlags[2] = 1;
		item->itemFlags[3] = 75;

		if (TriggerActive(item))
		{
			item->itemFlags[0] = 30846;
			AnimateItem(item);
			return;
		}
	}
	else
	{
		item->itemFlags[3] = 25;

		if (TriggerActive(item))
		{
			item->itemFlags[0] = 1920;
			AnimateItem(item);
			return;
		}
	}

	item->itemFlags[0] = 0;
}

void __cdecl PloughControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->itemFlags[3] = 50;
	if (TriggerActive(item))
	{
		item->itemFlags[0] = 258048;
		AnimateItem(item);
	}
	else
	{
		item->itemFlags[0] = 0;
	}
}

void __cdecl CogControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (TriggerActive(item))
	{
		item->status = ITEM_ACTIVE;
		// *(_DWORD *)&item->gap4C[5526] = *(_DWORD *)&item->gap4C[5526] & 0xFFFFFFFB | 2;
		AnimateItem(item);

		if (item->triggerFlags == 666)
		{
			PHD_VECTOR pos;
			GetJointAbsPosition(item, &pos, 0);
			SoundEffect(65, (PHD_3DPOS *)&pos, 0);

			if (item->frameNumber == Anims[item->animNumber].frameEnd)
				item->flags &= 0xC1;
		}
	}
	else if (item->triggerFlags == 2)
	{
		item->status |= ITEM_INVISIBLE;
	}
}

void __cdecl SpikeballControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (TriggerActive(item))
	{
		__int16 frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

		if ((frameNumber <= 14 || frameNumber >= 24) && (frameNumber < 138 || frameNumber > 140))
		{
			if (frameNumber < 141)
				item->itemFlags[0] = 0;
			else
			{
				item->itemFlags[3] = 50;
				item->itemFlags[0] = 0x7FF800;
			}
		}
		else
		{
			item->itemFlags[3] = 150;
			item->itemFlags[0] = 0x7FF800;
		}

		AnimateItem(item);
	}
	else
	{
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->itemFlags[0] = 0;
	}
}

void __cdecl StargateControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	item->itemFlags[3] = 50;

	if (TriggerActive(item))
	{
		SoundEffect(23, &item->pos, 0);
		item->itemFlags[0] = 57521664;
		AnimateItem(item);
	}
	else
		item->itemFlags[0] = 0;
}

void __cdecl ControlSpikeWall(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	/* Move wall */
	if (TriggerActive(item) && item->status != ITEM_DEACTIVATED)
	{
		__int32 x = item->pos.xPos + SIN(item->pos.yRot) >> WALL_SHIFT;
		__int32 z = item->pos.zPos + COS(item->pos.yRot) >> WALL_SHIFT;
		
		__int16 roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);

		if (TrGetHeight(floor, x, item->pos.yPos, z) != item->pos.yPos)
		{
			item->status = ITEM_DEACTIVATED;
			StopSoundEffect(147);
		}
		else
		{
			item->pos.xPos = x;
			item->pos.zPos = z;
			if (roomNumber != item->roomNumber)
				ItemNewRoom(itemNum, roomNumber);
			SoundEffect(147, &item->pos, 0);
		}
	}

	if (item->touchBits)
	{
		LaraItem->hitPoints -= 15;
		LaraItem->hitStatus = true;

		DoLotsOfBlood(LaraItem->pos.xPos, LaraItem->pos.yPos - 512, LaraItem->pos.zPos, 4, item->pos.yRot, LaraItem->roomNumber, 3);
		item->touchBits = 0;

		SoundEffect(56, &item->pos, 0);
	}
}

void __cdecl InitialiseSpinningBlade(__int16 item_number)
{
	ITEM_INFO* item = &Items[item_number];

	item->animNumber = Objects[item->objectNumber].animIndex + 3;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = 1;
}

void __cdecl SpinningBlade(__int16 item_number)
{
	bool spinning = false;
	
	ITEM_INFO* item = &Items[item_number];
	
	if (item->currentAnimState == 2)
	{
		if (item->goalAnimState != 1)
		{
			__int32 x = item->pos.xPos + (WALL_SIZE * 3 / 2 * SIN(item->pos.yRot) >> W2V_SHIFT);
			__int32 z = item->pos.zPos + (WALL_SIZE * 3 / 2 * COS(item->pos.yRot) >> W2V_SHIFT);
			
			__int16 roomNumber = item->roomNumber;
			FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
			__int32 height = TrGetHeight(floor, x, item->pos.yPos, z);

			if (height == NO_HEIGHT)
				item->goalAnimState = 1;
		}
		
		spinning = true;

		if (item->touchBits)
		{
			LaraItem->hitStatus = true;
			LaraItem->hitPoints -= 100;

			DoLotsOfBlood(LaraItem->pos.xPos, LaraItem->pos.yPos - STEP_SIZE * 2, LaraItem->pos.zPos, (__int16)(item->speed * 2), LaraItem->pos.yRot, LaraItem->roomNumber, 2);
		}

		SoundEffect(231, &item->pos, 0);
	}
	else
	{
		if (TriggerActive(item))
			item->goalAnimState = 2;
		spinning = false;
	}

	AnimateItem(item);

	__int16 roomNumber = item->roomNumber;
	FLOOR_INFO*  floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	item->floor = item->pos.yPos = TrGetHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	if (roomNumber != item->roomNumber)
		ItemNewRoom(item_number, roomNumber);

	if (spinning && item->currentAnimState == 1)
		item->pos.yRot += -ANGLE(180);
}

void __cdecl InitialiseKillerStatue(__int16 item_number)
{
	ITEM_INFO* item = &Items[item_number];

	item->animNumber = Objects[item->objectNumber].animIndex + 3;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = 1;
}

void __cdecl KillerStatueControl(__int16 item_number)
{
	ITEM_INFO *item;
	__int32 x, y, z;
	__int16 d;

	item = &Items[item_number];

	if (TriggerActive(item) && item->currentAnimState == 1)
		item->goalAnimState = 2;
	else
		item->goalAnimState = 1;

	if ((item->touchBits & 0x80) && item->currentAnimState == 2)
	{
		LaraItem->hitStatus = 1;
		LaraItem->hitPoints -= 20;

		__int32 x = LaraItem->pos.xPos + (GetRandomControl() - 16384) / 256;
		__int32 z = LaraItem->pos.zPos + (GetRandomControl() - 16384) / 256;
		__int32 y = LaraItem->pos.yPos - GetRandomControl() / 44;
		__int32 d = (GetRandomControl() - 16384) / 8 + LaraItem->pos.yRot;
		DoBloodSplat(x, y, z, LaraItem->speed, d, LaraItem->roomNumber);
	}

	AnimateItem(item);
}

void __cdecl SpringBoardControl(__int16 item_number)
{
	ITEM_INFO* item = &Items[item_number];

	if (item->currentAnimState == 0 && LaraItem->pos.yPos == item->pos.yPos &&
		(LaraItem->pos.xPos >> WALL_SHIFT) == (item->pos.xPos >> WALL_SHIFT) &&
		(LaraItem->pos.zPos >> WALL_SHIFT) == (item->pos.zPos >> WALL_SHIFT))
	{
		if (LaraItem->hitPoints <= 0)
			return;

		if (LaraItem->currentAnimState == STATE_LARA_WALK_BACK || LaraItem->currentAnimState == STATE_LARA_RUN_BACK)
			LaraItem->speed = -LaraItem->speed;

		LaraItem->fallspeed = -240;
		LaraItem->gravityStatus = true;
		LaraItem->animNumber = ANIMATION_LARA_FREE_FALL_FORWARD;
		LaraItem->frameNumber = GF2(ID_LARA, ANIMATION_LARA_FREE_FALL_FORWARD, 0);
		LaraItem->currentAnimState = STATE_LARA_JUMP_FORWARD;
		LaraItem->goalAnimState = STATE_LARA_JUMP_FORWARD;

		item->goalAnimState = 1;
	}

	AnimateItem(item);
}