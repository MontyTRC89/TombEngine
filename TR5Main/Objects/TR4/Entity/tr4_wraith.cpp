#include "framework.h"
#include "tr4_wraith.h"
#include "level.h"
#include "effect2.h"
#include "control.h"
#include "objectslist.h"
#include "trmath.h"
#include <sound.h>
#include <lara.h>

constexpr auto WRAITH_COUNT = 8;

short WraithSpeed = 64;

struct WRAITH_INFO
{
	int xPos;
	int yPos;
	int zPos;
	short xRot;
	short yRot;
	short zRot;
	unsigned char unknown01;
	unsigned char unknown02;
	unsigned char unknown03;
};

void InitialiseWraith(short itemNumber)
{
	ITEM_INFO* item;

	item = &g_Level.Items[itemNumber];
	WRAITH_INFO* wraithData;
	
	wraithData = game_malloc<WRAITH_INFO>(WRAITH_COUNT);
	item->data = wraithData;
	item->itemFlags[0] = 0;
	item->hitPoints = 0;
	item->speed = WraithSpeed;

	for (int i = 0; i < WRAITH_COUNT; i++)
	{
		wraithData->xPos = item->pos.xPos;
		wraithData->yPos = item->pos.yPos;
		wraithData->zPos = item->pos.zPos;
		wraithData->zRot = 0;
		wraithData->yPos = 0;
		wraithData->xPos = 0;
		wraithData->unknown01 = 0;
		wraithData->unknown02 = 0;
		wraithData->unknown03 = 0;

		wraithData++;
	}
}

void WraithControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	SoundEffect(SFX_TR4_WRAITH_WHISPERS, &item->pos, 0);

	ITEM_INFO* target;
	if (item->hitPoints)
		target = &g_Level.Items[item->hitPoints];
	else
		target = LaraItem;

	int x, y, z, distance, dx, dy, dz;

	if (target == LaraItem || target->objectNumber == 445)
	{
		x = target->pos.xPos - item->pos.xPos;
		y = target->pos.yPos;
		z = target->pos.zPos - item->pos.zPos;
		distance = SQUARE(x) + SQUARE(z);
		dy = abs((distance >> 13) - 512);
	}
	else
	{
		ROOM_INFO* room = &g_Level.Rooms[LaraItem->roomNumber];

		x = room->x + room->ySize * 1024 / 2 - item->pos.xPos;
		z = room->z + room->xSize * 1024 / 2 - item->pos.zPos;

		distance = SQUARE(x) + SQUARE(z);
		dy = abs((distance >> 13) - 768);
		y = room->y + ((room->minfloor - room->maxceiling) / 2);
	}

	dy = y - item->pos.yPos - dy - 128;
	short angleH = phd_atan(z, x) - item->pos.yRot;

	short angleV = 0;
	if (abs(x) <= abs(z))
		angleV = phd_atan(abs(x) + (abs(z) / 2), dy);
	else
		angleV = phd_atan(abs(z) + (abs(x) / 2), dy);

	angleV -= item->pos.xRot;

	int speed = 8 * WraithSpeed / item->speed;

	if (abs(angleH) >= item->itemFlags[2] || angleH > 0 != item->itemFlags[2] > 0)
	{
		if (angleH >= 0)
		{
			if (item->itemFlags[2] <= 0)
			{
				item->itemFlags[2] = 1;
			}
			else
			{
				item->itemFlags[2] += speed;
				item->pos.yRot += item->itemFlags[2];
			}
		}
		else if (item->itemFlags[2] >= 0)
		{
			item->itemFlags[2] = -1;
		}
		else
		{
			item->itemFlags[2] -= speed;
			item->pos.yRot += item->itemFlags[2];
		}
	}
	else
	{
		item->pos.yRot += angleH;
	}

	if (abs(angleV) >= item->itemFlags[3] || angleV > 0 != item->itemFlags[3] > 0)
	{
		if (angleV >= 0)
		{
			if (item->itemFlags[3] <= 0)
			{
				item->itemFlags[3] = 1;
			}
			else
			{
				item->itemFlags[3] += speed;
				item->pos.xRot += item->itemFlags[3];
			}
		}
		else if (item->itemFlags[3] >= 0)
		{
			item->itemFlags[3] = -1;
		}
		else
		{
			item->itemFlags[3] -= speed;
			item->pos.xRot += item->itemFlags[3];
		}
	}
	else
	{
		item->pos.xRot += angleV;
	}

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	int ceiling = GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	int doEffect = 0;
	if (height < item->pos.yPos || ceiling > item->pos.yPos)
	{
		doEffect = 1;
	}

	item->pos.xPos += item->speed * phd_sin(item->pos.yRot) >> W2V_SHIFT;
	item->pos.yPos += item->speed * phd_sin(item->pos.xRot) >> W2V_SHIFT;
	item->pos.zPos += item->speed * phd_cos(item->pos.yRot) >> W2V_SHIFT;

	IsRoomOutsideNo = NO_ROOM;
	if (item->roomNumber != IsRoomOutsideNo && IsRoomOutsideNo != NO_ROOM)
	{
		ItemNewRoom(itemNumber, IsRoomOutsideNo);

		ROOM_INFO* r = &g_Level.Rooms[IsRoomOutsideNo];
		short linkNum = NO_ITEM;
		for (linkNum = r->itemNumber; linkNum != NO_ITEM; linkNum = g_Level.Items[linkNum].nextItem)
		{
			ITEM_INFO* target = &g_Level.Items[linkNum];

			if (target->active)
			{
				if (item->objectNumber == ID_WRAITH1 && target->objectNumber == ID_WRAITH2 || 
					item->objectNumber == ID_WRAITH2 && target->objectNumber == ID_WRAITH1 || 
					item->objectNumber == ID_WRAITH3 && target->objectNumber == ID_ANIMATING10)
				{
					break;
				}
			}
		}

		if (linkNum != NO_ITEM)
		{
			item->hitPoints = linkNum;
		}
	}
}

void WraithEffects(int x, int y, int z, short xVel, short yVel, short zVel, int objNumber)
{
	unsigned char size, life;
	BYTE color;
	SPARKS* spark;
	spark = &Sparks[GetFreeSpark()];
	spark->on = 1;

	if (objNumber == ID_WRAITH1)
	{
		spark->sR = (GetRandomControl() & 0x1F) + -128;
		spark->sB = 24;
		spark->sG = (GetRandomControl() & 0x1F) + 48;
		spark->dR = (GetRandomControl() & 0x1F) + -128;
		spark->dB = 24;
		spark->dG = (GetRandomControl() & 0x1F) + 64;
	}
	else if (objNumber == ID_WRAITH2) {
		spark->sB = (GetRandomControl() & 0x1F) + -128;
		spark->sR = 24;
		spark->sG = (GetRandomControl() & 0x1F) + -128;
		spark->dB = (GetRandomControl() & 0x1F) + -128;
		spark->dR = 24;
		spark->dG = (GetRandomControl() & 0x1F) + 64;
	}

	else {
		color = (GetRandomControl() & 0x1F) + 64;
		spark->dG = color;
		spark->dR = color;
		spark->sB = color;
		spark->sG = color;
		spark->sR = color;
		spark->dB = spark->sB + (GetRandomControl() & 0x1F);
	}

	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 7;
	spark->transType = COLADD;
	life = (GetRandomControl() & 7) + 12;
	spark->life = life;
	spark->sLife = life;
	spark->x = (GetRandomControl() & 0x1F) + x - 16;
	spark->y = y;
	spark->friction = 85;
	spark->flags = 522;
	spark->z = (GetRandomControl() & 0x1F) + z - 16;
	spark->xVel = xVel;
	spark->yVel = yVel;
	spark->zVel = zVel;
	spark->gravity = 0;
	spark->maxYvel = 0;
	spark->scalar = 2;
	spark->dSize = 2;
	size = (GetRandomControl() & 0x1F) + 48;
	spark->sSize = size;
	spark->size = size;
}

/*
void DrawWraithEffect(int x, int y, int z, short yrot, short objNumber)
{
	BYTE life;
	unsigned char size;
	signed int i;
	int objNumbera,v5,v6,v7,v8,v9,v10,v11,v13;
	SPARKS* spark;
	spark = &Sparks[GetFreeSpark()];
	
	if (objNumber == ID_WRAITH1)
	{
		v5 = GetRandomControl() << 8;
		v6 = ((GetRandomControl() | v5) << 32) | 0x18000001;
		v7 = ((GetRandomControl() & 0x1F) + 48) << 8;
		v8 = (((GetRandomControl() & 0x1F) + 128) << 16) | v7 | 0x18;
	}
	else if (objNumber == ID_WRAITH2) {
		v9 = ((GetRandomControl() & 0x1F) << 24) - 0x7FFFFFFF;
		v6 = (GetRandomControl() << 32) | v9 | 1;
		v10 = ((GetRandomControl() & 0x1F) + 48) << 8;
		v8 = ((GetRandomControl() & 0x1F) + 128) | v10 | 0x180000;
	}
	else {
		v11 = (GetRandomControl() & 0xF) + 64;
		v6 = ((v11 | ((v11 | (v11 << 8)) << 8)) << 24) | 1;
		v8 = ((v11 | (v11 << 8)) << 8) | (v11 + (GetRandomControl() & 0xF));
	}

	objNumbera = (v8 & 0xFF00) | (v8 << 16) | BYTE2(v8);

	for (int i = 0; i < 15; i++)
	{
		*&spark->on = v6;
		*&spark->dR = objNumbera;
		*&spark->colFadeSpeed = 1796;
		v13 = GetRandomControl();
		spark->transType = COLADD;
		life = (v13 & 7) + 32;
		spark->life = life;
		spark->sLife = life;
		spark->x = (GetRandomControl() & 0x1F) + x - 16;
		spark->y = (GetRandomControl() & 0x1F) + y - 16;
		spark->z = (GetRandomControl() & 0x1F) + z - 16;
		v15 = GetRandomControl() - 0x4000;
		v16 = (GetRandomControl() & 0x3FF) + 1024;
		v17 = ((yrot + v15) >> 3) & 0x1FFE;
		spark->xVel = v16 * 4 * rcossin_tbl[v17] >> 14;
		spark->yVel = (GetRandomControl() & 0x7F) - 64;
		spark->zVel = v16 * 4 * rcossin_tbl[v17 + 1] >> 14;
		spark->friction = 4;
		spark->flags = 522;
		v18 = GetRandomControl();
		spark->maxYvel = 0;
		spark->scalar = 3;
		spark->gravity = (v18 & 0x7F) - 64;
		size = (GetRandomControl() & 0x1F) + 48;
		spark->sSize = size;
		spark->size = size;
		spark->dSize = size >> 2;
	}

}
*/

void DrawWraith(ITEM_INFO* item)
{

}

void KillWraith(ITEM_INFO* item)
{
	ITEM_INFO* item2;
	item2 = nullptr;

	if (NextItemActive != NO_ITEM)
	{
		for (;NextItemActive != NO_ITEM;)
		{
			item2 = &g_Level.Items[NextItemActive];
			if (item2->objectNumber == ID_WRAITH3 && !item2->hitPoints)
			{
				break;
			}
			if (item2->nextActive == NO_ITEM)
			{
				FlipEffect = -1;
				return;
			}
		}
		item2->hitPoints = item - g_Level.Items.data();
	}
	FlipEffect = -1;
}
