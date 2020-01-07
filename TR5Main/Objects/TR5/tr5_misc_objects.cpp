#include "../newobjects.h"
#include "../oldobjects.h"
#include "../../Game/lara.h"
#include "../../Game/draw.h"
#include "../../Global/global.h"
#include "../../Game/items.h"
#include "../../Game/collide.h"
#include "../../Game/effects.h"
#include "../../Game/laramisc.h"
#include "../../Game/Box.h"
#include "../../Game/sphere.h"
#include "../../Game/effect2.h"

void InitialiseRaisingBlock(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	short boxIndex = floor->box;
	Boxes[boxIndex].overlapIndex &= ~BLOCKED;
	
	if (item->triggerFlags < 0)
	{
		item->aiBits |= (GUARD | FOLLOW | AMBUSH | PATROL1 | MODIFY);
		AddActiveItem(itemNumber);
		item->status = ITEM_ACTIVE;
	}
}

void ControlRaisingBlock(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (TriggerActive(item))
	{
		if (!item->itemFlags[2])
		{
			if (item->objectNumber == ID_RAISING_BLOCK1)
			{
				if (item->triggerFlags == -1)
				{
					AlterFloorHeight(item, -255);
				}
				else if (item->triggerFlags == -3)
				{
					AlterFloorHeight(item, -1023);
				}
				else
				{
					AlterFloorHeight(item, -1024);
				}
			}
			else
			{
				AlterFloorHeight(item, -2048);
			}

			item->itemFlags[2] = 1;
		}

		if (item->triggerFlags < 0)
		{
			item->itemFlags[1] = 1;
		}
		else if (item->itemFlags[1] < 4096)
		{
			SoundEffect(SFX_BLK_PLAT_RAISE_LOW, &item->pos, 0);

			item->itemFlags[1] += 64;
			
			if (item->triggerFlags > 0)
			{
				if (abs(item->pos.xPos - Camera.pos.x) < 10240 &&
					abs(item->pos.xPos - Camera.pos.x) < 10240 &&
					abs(item->pos.xPos - Camera.pos.x) < 10240)
				{
					if (item->itemFlags[1] == 64 || item->itemFlags[1] == 4096)
						Camera.bounce = -32;
					else
						Camera.bounce = -16;
				}
			}
		}
	}
	else if (item->itemFlags[1] <= 0 || item->triggerFlags < 0)
	{
		if (item->itemFlags[2])
		{
			item->itemFlags[1] = 0;

			if (item->objectNumber == ID_RAISING_BLOCK1)
			{
				if (item->triggerFlags == -1)
				{
					AlterFloorHeight(item, 255);
					item->itemFlags[2] = 0;
				}
				else if (item->triggerFlags == -3)
				{
					AlterFloorHeight(item, 1023);
					item->itemFlags[2] = 0;
				}
				else
				{
					AlterFloorHeight(item, 1024);
				}
			}
			else
			{
				AlterFloorHeight(item, 2048);
			}

			item->itemFlags[2] = 0;
		}
	}
	else
	{
		SoundEffect(SFX_BLK_PLAT_RAISE_LOW, &item->pos, 0);

		if (item->triggerFlags >= 0)
		{
			if (abs(item->pos.xPos - Camera.pos.x) < 10240 &&
				abs(item->pos.xPos - Camera.pos.x) < 10240 &&
				abs(item->pos.xPos - Camera.pos.x) < 10240)
			{
				if (item->itemFlags[1] == 64 || item->itemFlags[1] == 4096)
					Camera.bounce = -32;
				else
					Camera.bounce = -16;
			}
		}

		item->itemFlags[1] -= 64;
	}
}

void PulseLightControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (TriggerActive(item))
	{
		item->itemFlags[0] -= 1024;

		long pulse = 256 * SIN(item->itemFlags[0] + 4 * (item->pos.yPos & 0x3FFF)) >> W2V_SHIFT;
		pulse = (HIDWORD(pulse) ^ pulse) - HIDWORD(pulse);
		if (pulse > 255)
			pulse = 255;
		
		TriggerDynamicLight(
			item->pos.xPos,
			item->pos.yPos,
			item->pos.zPos,
			24,
			pulse * 8 * (item->triggerFlags & 0x1F) >> 9,
			pulse* ((item->triggerFlags >> 2) & 0xF8) >> 9,
			pulse* ((item->triggerFlags >> 7) & 0xF8) >> 9);
	}
}

void TriggerAlertLight(int x, int y, int z, int r, int g, int b, int rot, __int16 roomNumber, __int16 falloff)
{
	GAME_VECTOR from;
	from.x = x;
	from.y = y;
	from.z = z;
	GetFloor(x, y, z, &roomNumber);
	from.roomNumber = roomNumber;
	
	GAME_VECTOR to;
	to.x = x + rcossin_tbl[2 * rot];
	to.y = y;
	to.z = z + rcossin_tbl[2 * rot + 1];

	if (!LOS(&from, &to))
		TriggerDynamicLight(to.x, to.y, to.z, falloff, r, g, b);
}

void StrobeLightControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (TriggerActive(item))
	{
		item->pos.yRot += ANGLE(16);

		byte r = 8 * (item->triggerFlags & 0x1F);
		byte g = (item->triggerFlags >> 2) & 0xF8;
		byte b = (item->triggerFlags >> 7) & 0xF8;

		TriggerAlertLight(
			item->pos.xPos, 
			item->pos.yPos - 512, 
			item->pos.zPos, 
			r, g, b, 
			((item->pos.yRot + 22528) >> 4) & 0xFFF,
			item->roomNumber, 
			12);
		
		TriggerDynamicLight(
			item->pos.xPos + 256 * SIN(item->pos.yRot + 22528) >> W2V_SHIFT,
			item->pos.yPos - 768,
			item->pos.zPos + 256 * COS(item->pos.yRot + 22528) >> W2V_SHIFT,
			8,
			r, g, b);
	}
}

void ColorLightControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (TriggerActive(item))
	{
		TriggerDynamicLight(
			item->pos.xPos,
			item->pos.yPos,
			item->pos.zPos,
			24,
			8 * (item->triggerFlags & 0x1F),
			(item->triggerFlags >> 2) & 0xF8,
			(item->triggerFlags >> 7) & 0xF8);
	}
}

void ElectricalLightControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (!TriggerActive(item))
	{
		item->itemFlags[0] = 0;
		return;
	}

	int intensity = 0;

	if (item->triggerFlags > 0)
	{
		if (item->itemFlags[0] < 16)
		{
			intensity = 4 * (GetRandomControl() & 0x3F);
			item->itemFlags[0]++;			
		}
		else if (item->itemFlags[0] >= 96)
		{
			if (item->itemFlags[0] >= 160)
			{
				intensity = 255 - (GetRandomControl() & 0x1F);
			}
			else
			{
				intensity = 96 - (GetRandomControl() & 0x1F);
				if (!(GetRandomControl() & 0x1F) && item->itemFlags[0] > 128)
				{
					item->itemFlags[0] = 160;
				}
				else
				{
					item->itemFlags[0]++;
				}
			}
		}
		else
		{
			if (Wibble & 0x3F && GetRandomControl() & 7)
			{
				intensity = GetRandomControl() & 0x3F;
				item->itemFlags[0]++;
			}
			else
			{
				intensity = 192 - (GetRandomControl() & 0x3F);
				item->itemFlags[0]++;
			}
		}		
	}
	else
	{
		if (item->itemFlags[0] <= 0)
		{
			item->itemFlags[0] = (GetRandomControl() & 3) + 4;
			item->itemFlags[1] = (GetRandomControl() & 0x7F) + 128;
			item->itemFlags[2] = GetRandomControl() & 1;
		}
		
		item->itemFlags[0]--;

		if (!item->itemFlags[2])
		{
			item->itemFlags[0]--;

			intensity = item->itemFlags[1] - (GetRandomControl() & 0x7F);
			if (intensity > 64)
				SoundEffect(SFX_ELEC_LIGHT_CRACKLES, &item->pos, 32 * (intensity & 0xFFFFFFF8) | 8);
		}
		else
		{
			return;
		}
	}

	TriggerDynamicLight(
		item->pos.xPos,
		item->pos.yPos,
		item->pos.zPos,
		24,
		intensity * 8 * (item->triggerFlags & 0x1F) >> 8,
		intensity * ((item->triggerFlags >> 2) & 0xF8) >> 8,
		intensity * ((item->triggerFlags >> 7) & 0xF8) >> 8);
}

void BlinkingLightControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (TriggerActive(item))
	{
		item->itemFlags[0]--;

		if (item->itemFlags[0] >= 3)
		{
			item->meshBits = 1;
		}
		else
		{
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 0;
			GetJointAbsPosition(item, &pos, 0);
			
			TriggerDynamicLight(
				pos.x,
				pos.y,
				pos.z,
				16,
				8 * (item->triggerFlags & 0x1F),
				(item->triggerFlags >> 2) & 0xF8,
				(item->triggerFlags >> 7) & 0xF8);

			item->meshBits = 2;

			if (item->itemFlags[0] < 0)
				item->itemFlags[0] = 30;
		}
	}
}

void InitialiseTwoBlocksPlatform(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	item->itemFlags[0] = item->pos.yPos;
	item->itemFlags[1] = 1;
}

void TwoBlocksPlatformControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (TriggerActive(item))
	{
		if (item->triggerFlags)
		{
			if (item->pos.yPos > (item->itemFlags[0] - 16 * (item->triggerFlags & 0xFFFFFFF0)))
			{
				item->pos.yPos -= item->triggerFlags & 0xF;
			}

			short roomNumber = item->roomNumber;
			FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
			item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

			if (roomNumber != item->roomNumber)
				ItemNewRoom(itemNumber, roomNumber);
		}
		else
		{
			OnFloor = false;

			int height = LaraItem->pos.yPos + 1;
			TwoBlocksPlatformFloor(item, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, &height);
			
			if (OnFloor && LaraItem->animNumber != ANIMATION_LARA_RUN_BACK)
				item->itemFlags[1] = 1;
			else
				item->itemFlags[1] = -1;
			
			if (item->itemFlags[1] <= 0)
			{
				if (item->itemFlags[1]<= 0)
				{
					if (item->pos.yPos <= item->itemFlags[0])
					{
						item->itemFlags[1] = 1;
					}
					else
					{
						SoundEffect(SFX_2GUNTEX_FALL_BIG, &item->pos, 0);
						item->pos.yPos -= 4;
					}
				}
			}
			else
			{
				if (item->pos.yPos >= item->itemFlags[0] + 128)
				{
					item->itemFlags[1] = -1;
				}
				else
				{
					SoundEffect(SFX_2GUNTEX_FALL_BIG, &item->pos, 0);
					item->pos.yPos+= 4;
				}
			}
		}
	}
}

void TwoBlocksPlatformFloor(ITEM_INFO* item, int x, int y, int z, int* height)
{
	if (IsOnTwoBlocksPlatform(item, x, z))
	{
		if (y <= item->pos.yPos + 32 && item->pos.yPos < *height)
		{
			*height = item->pos.yPos;
			OnFloor = 1;
			HeightType = 0;
		}
	}
}

void TwoBlocksPlatformCeiling(ITEM_INFO* item, int x, int y, int z, int* height)
{
	if (IsOnTwoBlocksPlatform(item, x, z))
	{
		if (y > item->pos.yPos + 32 && item->pos.yPos > * height)
		{
			*height = item->pos.yPos + 256;
		}
	}
}

int IsOnTwoBlocksPlatform(ITEM_INFO* item, int x, int z)
{
	if (!item->meshBits)
		return 0;

	short angle = item->pos.yRot;

	int xb = x >> 10;
	int zb = z >> 10;

	int itemxb = item->pos.xPos >> 10;
	int itemzb = item->pos.zPos >> 10;

	if (!angle && (xb == itemxb || xb == itemxb - 1) && (zb == itemzb || zb == itemzb + 1))
		return 1;
	if (angle == -ANGLE(180) && (xb == itemxb || xb == itemxb + 1) && (zb == itemzb || zb == itemzb - 1))
		return 1;
	if (angle == ANGLE(90) && (zb == itemzb || zb == itemzb - 1) && (xb == itemxb || xb == itemxb + 1))
		return 1;
	if (angle == -ANGLE(90) && (zb == itemzb || zb == itemzb - 1) && (xb == itemxb || xb == itemxb - 1))
		return 1;

	return 0;
}