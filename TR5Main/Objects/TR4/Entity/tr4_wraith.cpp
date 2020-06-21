#include "framework.h"
#include "tr4_wraith.h"
#include "level.h"
#include "effect2.h"
#include "control.h"
#include "objectslist.h"
#include "trmath.h"

constexpr auto WRAITH_COUNT = 8;


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

	item = &Items[itemNumber];
	WRAITH_INFO* wraithData;
	

	item->data = (WRAITH_INFO*)game_malloc(WRAITH_COUNT * sizeof(WRAITH_INFO));
	item->itemFlags[0] = 0;
	item->hitPoints = 0;
	item->speed = 0x40;

	for (int i = 0; i < WRAITH_COUNT; i++)
	{
		wraithData = (WRAITH_INFO*)item->data;
		wraithData->xPos = item->pos.xPos;
		wraithData->yPos = item->pos.yPos;
		wraithData->zPos = item->pos.zPos;
		wraithData->zRot = 0;
		wraithData->yPos = 0;
		wraithData->xPos = 0;
		wraithData->unknown01 = 0;
		wraithData->unknown02 = 0;
		wraithData->unknown03 = 0;

	}

}

void WraithControl(short itemNumber)
{
	// TODO;
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
			item2 = &Items[NextItemActive];
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
		item2->hitPoints = item - Items;
	}
	FlipEffect = -1;
}
