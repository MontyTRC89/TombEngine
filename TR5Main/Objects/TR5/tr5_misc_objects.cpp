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
#include "../../Game/tomb4fx.h"
#include "../../Game/switch.h"

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

void InitialiseSmokeEmitter(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (item->triggerFlags == 111)
	{
		if (item->pos.yRot > 0)
		{
			if (item->pos.yRot == ANGLE(90))
				item->pos.xPos += 512;
		}
		else if (item->pos.yRot)
		{
			if (item->pos.yRot == -ANGLE(180))
			{
				item->pos.zPos -= 512;
			}
			else if (item->pos.yRot == -ANGLE(90))
			{
				item->pos.xPos -= 512;
			}
		}
		else
		{
			item->pos.zPos += 512;
		}
	}
	else if (item->objectNumber != ID_SMOKE_EMITTER)
		return;
	else if (item->triggerFlags & 8)
	{
		item->itemFlags[0] = item->triggerFlags >> 4;
		
		if (item->pos.yRot > 0)
		{
			if (item->pos.yRot == ANGLE(90))
				item->pos.xPos += 256;
		}
		else
		{
			if (item->pos.yRot == 0)
			{
				item->pos.zPos += 256;
			}
			else if (item->pos.yRot == -ANGLE(180))
			{
				item->pos.zPos -= 256;
			}
			else if (item->pos.yRot == -ANGLE(90))
				item->pos.xPos -= 256;
		}

		if ((signed short)(item->triggerFlags >> 4) <= 0)
		{
			item->itemFlags[2] = 4096;
			item->triggerFlags |= 4;
		}
	}
	else if (Rooms[item->roomNumber].flags & 1 && item->triggerFlags == 1)
	{
		item->itemFlags[0] = 20;
		item->itemFlags[1] = 1;
	}
}

void SmokeEmitterControl(short itemNumber)
{
	/*ITEM_INFO* item = &Items[itemNumber];

	if (!TriggerActive(item))
		return;

	if (item->objectNumber != ID_SMOKE_EMITTER)
		goto LABEL_61;

	if (Rooms[item->roomNumber].flags & 1)
	{
		if (item->itemFlags[0] || !(GetRandomControl() & 0x1F) || item->triggerFlags == 1)
		{
			if (!(GetRandomControl() & 3) || item->itemFlags[1])
			{
				PHD_VECTOR pos;
				pos.x = (GetRandomControl() & 0x3F) + item->pos.xPos - 32;
				pos.y = item->pos.yPos - (GetRandomControl() & 0x1F) - 16;
				pos.z = (GetRandomControl() & 0x3F) + item->pos.zPos - 32;

				if (item->triggerFlags == 1)
				{
					CreateBubble(&pos, item->roomNumber, 15, 15, 0, 0, 0, 0);
				}
				else
				{
					CreateBubble(&pos, item->roomNumber, 8, 7, 0, 0, 0, 0);
				}
				
				if (item->itemFlags[0])
				{
					item->itemFlags[0]--;
					if (!item->itemFlags[0])
						item->itemFlags[1] = 0;
				}
			}
		}
		else
		{
			if (!(GetRandomControl() & 0x1F))
			{
				item->itemFlags[0] = (GetRandomControl() & 3) + 4;
			}
		}

		return;
	}

	if (!(item->triggerFlags & 8) || item->triggerFlags == 111)
		goto LABEL_61;
	
	if (item->triggerFlags & 4)
	{
		if (GlobalCounter & 1)
			return;
	}
	else
	{
		if (item->itemFlags[0])
		{
			item->itemFlags[0]--;
			if (!item->itemFlags[0])
				item->itemFlags[1] = (GetRandomControl() & 0x3F) + 30;
			
			v41 = 1;
			if (item->itemFlags[2])
				item->itemFlags[2] -= 256;
			if (!item->itemFlags[2])
				goto LABEL_61;
		}
		else
		{
			if (item->itemFlags[2] < 4096)
				item->itemFlags[2] += 256;
		}
	}

	int dx = LaraItem->pos.xPos - item->pos.xPos;
	int dz = LaraItem->pos.zPos - item->pos.zPos;
	
	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = 1;
		spark->dR = 48;
		spark->dG = 48;
		spark->dB = 48;
		spark->transType = COLADD;
		spark->x = (GetRandomControl() & 0x3F) + item->pos.xPos - 32;
		spark->y = (GetRandomControl() & 0x3F) + item->pos.yPos - 32;
		spark->z = (GetRandomControl() & 0x3F) + item->pos.zPos - 32;
		
		int flags = item->itemFlags[2];
		if (flags == 4096)
		{
			if (item->triggerFlags & 4)
				flags = (GetRandomControl() & 0xFFF) + 256;
			else
				flags = (GetRandomControl() & 0x7FF) + 2048;
		}

		if (item->triggerFlags >= 0)
		{
			spark->xVel = flags * SIN(item->pos.yRot - ANGLE(180)) >> W2V_SHIFT; 
			spark->yVel = -16 - (GetRandomControl() & 0xF);
			spark->zVel = flags * COS(item->pos.yRot - ANGLE(180)) >> W2V_SHIFT;
		}
		else
		{
			v17 = GetRandomControl();
			v18 = v17;
			LOWORD(v17) = item->pos.yRot;
			f = (v18 & 0x7F) + 2048;
			spark->Xvel = v15 * 4 * rcossin_tbl[((v17 + 20480) >> 3) & 0x1FFE] >> 14;
			spark->Yvel = -128 - (unsigned __int8)GetRandomControl();
			spark->Zvel = v15 * 4 * rcossin_tbl[((((unsigned __int16)item->pos.yRot + 20480) >> 3) & 0x1FFE) + 1] >> 14;
		}
		spark->Flags = 538;
		if (!(GlobalCounter & 3) && !(item->triggerFlags & 4))
			spark->Flags = 1562;
		spark->RotAng = GetRandomControl() & 0xFFF;
		spark->RotAdd = GetRandomControl() & 1 ? -8 - (GetRandomControl() & 7) : (GetRandomControl() & 7) + 8;
		spark->Gravity = -8 - (GetRandomControl() & 0xF);
		spark->MaxYvel = -8 - (GetRandomControl() & 7);
		v20 = (GetRandomControl() & 0x1F) + 128;
		if (item->triggerFlags & 4)
		{
			spark->sB = 0;
			spark->sG = 0;
			spark->sR = 0;
			spark->ColFadeSpeed = 2;
			spark->FadeToBlack = 2;
			v21 = (GetRandomControl() & 3) + 16;
			spark->Life = v21;
			spark->sLife = v21;
			if (item->triggerFlags >= 0)
			{
				spark->Xvel *= 4;
				spark->Zvel *= 4;
				spark->Scalar = 3;
				spark->Friction = 4;
			}
			else
			{
				spark->Scalar = 1;
				spark->Friction = 51;
				v15 >>= 1;
			}
			v22 = v15 * v20 >> 10;
			if (v22 > 255)
				v22 = 255;
			spark->dSize = v22;
			spark->sSize = v22 >> 2;
			spark->Size = v22 >> 2;
		}
		else
		{
			spark->sR = 96;
			spark->sG = 96;
			spark->sB = 96;
			spark->fadeToBlack = 6;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 6;
			spark->life = spark->sLife = (GetRandomControl() & 7) + 8;
			spark->friction = 4 - (item->triggerFlags & 4);
			v25 = (((item->triggerFlags & 0xFF) >> 2) & 1) + 2;
			spark->dSize = v20;
			spark->scalar = (((item->triggerFlags & 0xFF) >> 2) & 1) + 2;
			spark->sSize = v20 >> 1;
			spark->Size = v20 >> 1;
			v26 = item->itemFlags[1];
			if (v26)
				item->itemFlags[1] = v26 - 1;
			else
				item->itemFlags[0] = item->triggerFlags >> 4;
		}
		LOBYTE(v4) = v41;
		if (v41)
		{
		LABEL_61:
			LOBYTE(v4) = wibble;
			if (!(wibble & 0xF) && (item->objectNumber != 365 || !(wibble & 0x1F)))
			{
				SPARKS* spark = &Sparks[GetFreeSpark()];
				
				spark->on = 1;
				spark->sR = 0;
				spark->sG = 0;
				spark->sB = 0;
				spark->dR = 64;
				spark->dG = 64;
				spark->dB = 64;
				spark->fadeToBlack = 16;
				spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
				spark->life = spark->sLife = (GetRandomControl() & 7) + 28;
				if (item->objectNumber == ID_SMOKE_EMITTER_WHITE)
					spark->transType = COLSUB;
				else
					spark->transType = COLADD;
				spark->x = (GetRandomControl() & 0x3F) + item->pos.xPos - 32;
				spark->y = (GetRandomControl() & 0x3F) + item->pos.yPos - 32;
				spark->z = (GetRandomControl() & 0x3F) + item->pos.zPos - 32;
				if (item->triggerFlags == 111)
				{
					spark->xVel = 512 * SIN(item->pos.yRot - ANGLE(180)) >> W2V_SHIFT;
					spark->zVel = 512 * COS(item->pos.yRot - ANGLE(180)) >> W2V_SHIFT;
					spark->friction = 5;
				}
				else
				{
					spark->xVel = (byte)GetRandomControl() - 128;
					spark->zVel = (byte)GetRandomControl() - 128;
					spark->friction = 3;
				}
				v33 = GetRandomControl();
				spark->Flags = 538;
				spark->Yvel = -16 - (v33 & 0xF);
				if (room[item->roomNumber].flags & 8)
					spark->Flags = 794;
				spark->RotAng = GetRandomControl() & 0xFFF;
				if (GetRandomControl() & 1)
					spark->RotAdd = -8 - (GetRandomControl() & 7);
				else
					spark->RotAdd = (GetRandomControl() & 7) + 8;
				spark->Scalar = 2;
				spark->Gravity = -8 - (GetRandomControl() & 0xF);
				spark->MaxYvel = -8 - (GetRandomControl() & 7);
				v4 = (GetRandomControl() & 0x1F) + 128;
				spark->dSize = v4;
				spark->sSize = v4 >> 2;
				spark->Size = v4 >> 2;
				if (item->objectNumber == 365)
				{
					v34 = spark->MaxYvel;
					v35 = spark->Life;
					v36 = spark->sLife + 16;
					spark->Gravity >>= 1;
					spark->Yvel >>= 1;
					spark->MaxYvel = v34 >> 1;
					spark->Life = v35 + 16;
					LOBYTE(v4) = 32;
					spark->sLife = v36;
					spark->dR = 32;
					spark->dG = 32;
					spark->dB = 32;
				}
			}
			return v4;
		}
	}
	return v4;*/
}

void InitialiseTeleporter(short itemNumber)
{
	/*ITEM_INFO* item = &Items[itemNumber];

	if (item->triggerFlags == 512)
	{
		ITEM_INFO* puzzleHoleItem = find_a_fucking_item(ID_PUZZLE_HOLE2);
		v4 = (signed int)((unsigned __int64)(391146079i64 * ((char*)v3 - (char*)items)) >> 32) >> 9;
		result = (unsigned int)((unsigned __int64)(391146079i64 * ((char*)v3 - (char*)items)) >> 32) >> 31;
		item->itemFlags[1] = result + v4;
	}*/
}

void ControlTeleporter(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (!TriggerActive(item))
		return;

	/*if (item->triggerFlags == 512)
	{
		if (item->itemFlags[2])
		{
			g_LaraExtra.Puzzles[1] = 1;
			RemoveActiveItem(itemNumber);
			item->flags &= 0xC1FF;
		}
		else
		{
			item->itemFlags[0] += 2;

			if (item->itemFlags[0] <= 255)
			{
				int flags = item->itemFlags[0] >> 3;
				if (flags >= 4)
				{
					if (flags > 31)
						flags = 31;
				}
				else
				{
					flags = 4;
				}

				ITEM_INFO* targetItem = &Items[item->itemFlags[1]];
				SoundEffect(SFX_RICH_TELEPORT, &targetItem->pos, (flags << 8) | 8);

				if (FlashFader > 4)
				{
					FlashFader = (FlashFader >> 1) & 0xFE;
				}

				if (GlobalCounter & 1)
				{
					PHD_VECTOR src;
					pos.x = targetItem->pos.xPos;
					pos.y = targetItem->pos.yPos - 496;
					pos.z = targetItem->pos.zPos + 472;

					int dl = 4 * item->itemFlags[0] + 256;

					PHD_VECTOR dest;
					dest.x = src.x + GetRandomControl() % dl - (dl >> 1);
					dest.y = src.y + GetRandomControl() % dl - (dl >> 1);
					dest.z = src.z + GetRandomControl() % dl - (dl >> 1);

					int color = (item->itemFlags[0] >> 2) | (((item->itemFlags[0] - (GetRandomControl() % (item->itemFlags[0] >> 1))) | (item->itemFlags[0] << 8)) << 8);
					color |= 0x18; // BYTE1

					//TriggerEnergyArc(&src, &dest, (GetRandomControl() & 0x1F) + 16, color, 15, 40, 5);

					v20 = v16;
					v21 = v12 & 0xFFFFFFFE;
					LOBYTE(v20) = v16 & 0xFE;
					BYTE1(v21) |= 0x80u;
					TriggerLightningGlow(src.x, src.y, src.z, (item->itemFlags[0] >> 3) | ((v20 | (v21 << 8)) << 7));
					v22 = GetRandomControl();
					TriggerDynamicLight(src.x, src.y, src.z, (v22 & 3) + (item->itemFlags[0] >> 5) + 8, v12, v16, v13);
				}
				LOBYTE(v3) = GetRandomControl();
				if (v3 & 1)
				{
					v23 = item->itemFlags[0];
					v24 = item->itemFlags[0];
					v25 = GetRandomControl();
					FlashFadeR = v23;
					FlashFadeB = v24 >> 2;
					FlashFader = 32;
					FlashFadeG = v24 - v25 % (v24 >> 1);
					LOBYTE(v3) = SoundEffect(399, 0, 0);
				}
				if (!(GlobalCounter & 3))
				{
					v26 = GetRandomControl();
					v27 = 0;
					v28 = v26 & 3;
					v29 = 0;
					if (v28)
					{
						if (v28 == 1)
							v29 = 512;
						else
							v27 = v28 != 2 ? 512 : -512;
					}
					else
					{
						v29 = -512;
					}
					v30 = item->itemFlags[0];
					v31 = &items[item->itemFlags[1]];
					src.xPos = v29 + v31->pos.xPos;
					src.yPos = v31->pos.yPos - 2328;
					src.zPos = v27 + v31->pos.zPos;
					*(_DWORD*)& src.xRot = v31->pos.xPos;
					v32 = item->itemFlags[0];
					*(_DWORD*)& src.zRot = v31->pos.yPos - 496;
					v45 = v31->pos.zPos + 472;
					v33 = (v30 >> 2) | (((v30 - GetRandomControl() % (v30 >> 1)) | ((v32 | 0x2400) << 8)) << 8);
					v34 = GetRandomControl();
					TriggerEnergyArc((PHD_VECTOR*)& src, (PHD_VECTOR*)& src.xRot, (v34 & 0xF) + 16, v33, 13, 56, 5);
					v35 = &spark[GetFreeSpark()];
					v35->On = 1;
					v36 = item->itemFlags[0];
					v35->dR = v36;
					v35->sR = v36;
					v37 = item->itemFlags[0] >> 1;
					v35->dG = v37;
					v35->sG = v37;
					v38 = item->itemFlags[0];
					v35->ColFadeSpeed = 20;
					v38 >>= 2;
					v35->dB = v38;
					v35->sB = v38;
					v35->FadeToBlack = 4;
					v35->Life = 24;
					v35->sLife = 24;
					v35->TransType = 2;
					v35->x = src.xPos;
					v35->y = src.yPos;
					v35->z = src.zPos;
					v35->Zvel = 0;
					v35->Yvel = 0;
					v35->Xvel = 0;
					v35->Flags = 10;
					v39 = objects[458].mesh_index;
					v35->Scalar = 3;
					v35->MaxYvel = 0;
					v35->Def = v39 + 11;
					v35->Gravity = 0;
					v3 = (GetRandomControl() & 3) + 24;
					v35->dSize = v3;
					v35->sSize = v3;
					v35->Size = v3;
				}
				return v3;
			}
			FlashFadeR = 255;
			FlashFadeG = 255;
			FlashFadeB = 64;
			FlashFader = 32;
			item->itemFlags[2] = 1;
			SoundEffect(400, 0, (int)& unk_800004);
		}
	}*/

	DisableLaraControl = false;

	if (item->triggerFlags == 666)
	{
		if (item->itemFlags[0] == 15)
		{
			IsAtmospherePlaying = 0;
			S_CDPlay(CDA_XA12_Z_10, 0);
		}
		else if (item->itemFlags[0] == 70)
		{
			SoundEffect(SFX_LIFT_HIT_FLOOR1, 0, 0);
			SoundEffect(SFX_LIFT_HIT_FLOOR2, 0, 0);
		}

		LaraItem->animNumber = ANIMATION_LARA_ELEVATOR_RECOVER;
		LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
		LaraItem->goalAnimState = STATE_LARA_MISC_CONTROL;
		LaraItem->currentAnimState = STATE_LARA_MISC_CONTROL;

		item->itemFlags[0]++;
		if (item->itemFlags[0] >= 150)
			KillItem(itemNumber);
	}
	else
	{
		Camera.fixedCamera = true;
		LaraItem->pos.xPos = item->pos.xPos;
		LaraItem->pos.zPos = item->pos.zPos;
		LaraItem->pos.yRot = item->pos.yRot - ANGLE(180);

		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		LaraItem->pos.yPos = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

		if (LaraItem->roomNumber != roomNumber)
			ItemNewRoom(Lara.itemNumber, roomNumber);

		if (item->flags & ONESHOT)
		{
			KillItem(itemNumber);
		}
		else if (item->triggerFlags != 512)
		{
			RemoveActiveItem(itemNumber);
			item->flags &= 0xC1FFu;
		}
	}
}

void InitialiseHighObject1(short itemNumber)
{
	int x = 0;
	int y = 0;
	int z = 0;

	ITEM_INFO* item = &Items[itemNumber];

	for (int i = 0; i < LevelItems; i++)
	{
		ITEM_INFO* currentItem = &Items[i];

		if (currentItem->objectNumber != ID_TRIGGER_TRIGGERER)
		{
			if (currentItem->objectNumber == ID_PUZZLE_ITEM4_COMBO2)
			{
				item->itemFlags[3] |= (i << 8);
				currentItem->pos.yPos = item->pos.yPos - 512;
				continue;
			}
		}

		if (currentItem->triggerFlags == 111)
		{
			item->itemFlags[3] |= i;
			continue;
		}

		if (currentItem->triggerFlags != 112)
		{
			if (currentItem->objectNumber == ID_PUZZLE_ITEM4_COMBO2)
			{
				item->itemFlags[3] |= (i << 8);
				currentItem->pos.yPos = item->pos.yPos - 512;
				continue;
			}
		}
		else
		{
			x = currentItem->pos.xPos;
			y = currentItem->pos.yPos;
			z = currentItem->pos.zPos;
		}

	}

	for (int i = 0; i < LevelItems; i++)
	{
		ITEM_INFO* currentItem = &Items[i];

		if (currentItem->objectNumber == ID_PULLEY
			&& currentItem->pos.xPos == x
			&& currentItem->pos.yPos == y
			&& currentItem->pos.zPos == z)
		{
			item->itemFlags[2] |= i;
			break;
		}
	}
}

void ControlHighObject1(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (!TriggerActive(item))
	{
		if (item->itemFlags[0] == 4)
		{
			item->itemFlags[1]--;

			if (!item->itemFlags[1])
			{
				ITEM_INFO* targetItem = &Items[item->itemFlags[3] & 0xFF];
				targetItem->flags = (item->flags & 0xC1FF) | 0x20;
				item->itemFlags[0] = 6;
				item->itemFlags[1] = 768;
				TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 1, 0);
			}

			return;
		}
		
		if (item->itemFlags[0] == 6)
		{
			item->itemFlags[1] -= 8;
			
			if (item->itemFlags[1] >= 0)
			{
				int flags = 0;

				if (item->itemFlags[1] >= 256)
				{
					if (item->itemFlags[1] <= 512)
						flags = 31;
					else
						flags = (768 - item->itemFlags[1]) >> 3;
				}
				else
				{
					flags = item->itemFlags[1] >> 3;
				}

				SoundEffect(SFX_BLK_PLAT_RAISE_LOW, &item->pos, (flags << 8) | 8);

				item->pos.yPos += 8;

				ITEM_INFO* targetItem = &Items[(item->itemFlags[3] >> 8) & 0xFF];
				targetItem->flags |= 0x20u;
				targetItem->pos.yPos = item->pos.yPos - 560;
			}

			if (item->itemFlags[1] < -60)
			{
				ITEM_INFO* targetItem = &Items[item->itemFlags[2] & 0xFF];
				targetItem->itemFlags[1] = 0;
				targetItem->flags |= 0x20u;
				item->itemFlags[0] = 0;
				item->itemFlags[1] = 0;
				
				RemoveActiveItem(itemNumber);

				item->flags &= 0xC1FF;
				item->status = ITEM_INACTIVE;

				return;
			}
		}
	}
	else if (item->itemFlags[0] >= 3)
	{
		if (item->itemFlags[0] == 4)
		{
			item->itemFlags[0] = 5;
			item->itemFlags[1] = 0;
		}
		else if (item->itemFlags[0] == 5 
			&& !item->itemFlags[1] 
			&& Items[(item->itemFlags[3] >> 8) & 0xFF].flags < 0)
		{
			DoFlipMap(3);
			FlipMap[3] ^= 0x3E00u;
			item->itemFlags[1] = 1;
		}
	}
	else
	{
		if (item->itemFlags[1] >= 256)
		{
			item->itemFlags[1] = 0;
			item->itemFlags[0]++;

			if (item->itemFlags[0] == 3)
			{
				item->itemFlags[1] = 30 * item->triggerFlags;
				item->itemFlags[0] = 4;

				short targetItemNumber = item->itemFlags[3] & 0xFF;
				ITEM_INFO* targetItem = &Items[targetItemNumber];

				AddActiveItem(targetItemNumber);

				targetItem->flags |= 0x3E20u;
				targetItem->status = ITEM_ACTIVE;

				targetItemNumber = item->itemFlags[2] & 0xFF;
				targetItem = &Items[targetItemNumber];

				targetItem->itemFlags[1] = 1;
				targetItem->flags |= 0x20;
				targetItem->flags &= 0xC1FF;

				return;
			}

			RemoveActiveItem(itemNumber);

			item->flags &= 0xC1FF;
			item->status = ITEM_INACTIVE;

			return;
		}

		int flags = 0;
		
		if (item->itemFlags[1] >= 31)
		{
			if (item->itemFlags[1] <= 224)
				flags = 31;
			else
				flags = 255 - item->itemFlags[1];
		}
		else
		{
			flags = item->itemFlags[1];
		}

		SoundEffect(SFX_BLK_PLAT_RAISE_LOW, &item->pos, (flags << 8) | 8);

		item->itemFlags[1] += 16;
		item->pos.yPos -= 16;

		short targetItemNumber = (item->itemFlags[3] >> 8) & 0xFF;
		ITEM_INFO* targetItem = &Items[targetItemNumber];
		targetItem->flags |= 0x20;
		targetItem->pos.yPos = item->pos.yPos - 560;
	}
}

void GenSlot1Control(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (TriggerActive(item) && !item->triggerFlags)
	{
		int df = item->frameNumber - Anims[item->animNumber].frameBase;
		
		if (df == 10 || df == 11)
		{
			GetLaraDeadlyBounds();

			DeadlyBounds[0] -= 350;
			DeadlyBounds[1] += 350;
			DeadlyBounds[4] -= 350;
			DeadlyBounds[5] += 350;

			bool found = false;
			for (int i = 0; i < 6; i++)
			{
				PHD_VECTOR pos;
				pos.x = 0;
				pos.y = -350;
				pos.z = 0;

				GetJointAbsPosition(item, &pos, i + 1);

				if (pos.x > DeadlyBounds[0]
					&& pos.x < DeadlyBounds[1]
					&& pos.y > DeadlyBounds[2]
					&& pos.y < DeadlyBounds[3]
					&& pos.z > DeadlyBounds[4]
					&& pos.z < DeadlyBounds[5])
				{
					found = true;
				}
			}

			if (found)
			{
				for (int i = 0; i < 8; i++)
				{
					PHD_VECTOR pos;
					pos.x = 0;
					pos.y = 0;
					pos.z = 0;

					GetLaraJointPosition(&pos, i + 7);

					int x = pos.x + (GetRandomControl() & 0xFF) - 128;
					int y = pos.y + (GetRandomControl() & 0xFF) - 128;
					int z = pos.z + (GetRandomControl() & 0xFF) - 128;

					DoBloodSplat(x, y, z, 1, -1, LaraItem->roomNumber);
				}

				LaraItem->hitPoints = 0;
			}
		}
		
		AnimateItem(item);
	}
}

void InitialiseGenSlot3(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	if (CurrentLevel != 7)
		item->meshBits = item->triggerFlags;
}

void InitialiseGenSlot4(short itemNumber)
{
	/*ITEM_INFO* item = &Items[itemNumber];

	HIWORD(v1) = HIWORD(items);
	item = &items[itemNumber];
	LOWORD(v1) = item->pos.y_rot;
	v3 = item->pos.x_pos;
	v4 = 2 * ((v1 >> 3) & 0x1FFE);
	v5 = 5 * *(__int16*)((char*)rcossin_tbl + v4);
	v6 = item->pos.z_pos;
	v7 = v6 + (10240 * *(__int16*)((char*)& rcossin_tbl[1] + v4) >> 14);
	item->item_flags[2] = 1;
	BYTE1(v4) = v6 >> 9;
	LOBYTE(v4) = v3 >> 9;
	item->item_flags[0] = v4;
	LOBYTE(v6) = (item->pos.xPos + (v5 << 11 >> 14)) >> 9;
	BYTE1(v6) = v7 >> 9;

	item->itemFlags[1] = item->pos.xPos + 2560 * SIN(item->pos.yRot) >> W2V_SHIFT;
	item->itemFlags[3] = 0;
	item->triggerFlags = 0;*/
}