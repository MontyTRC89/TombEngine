#pragma once
#include "Game/items.h"

constexpr auto NUM_SPIDERS = 64;

struct SPIDER_STRUCT
{
	PHD_3DPOS pos; // size=20, offset=0
	short roomNumber; // size=0, offset=20
	short speed; // size=0, offset=22
	short fallspeed; // size=0, offset=24
	byte on; // size=0, offset=26
	byte flags; // size=0, offset=27
};

extern int NextSpider;
extern SPIDER_STRUCT Spiders[NUM_SPIDERS];

short GetNextSpider();
void ClearSpiders();
void ClearSpidersPatch(ITEM_INFO* item);
void InitialiseSpiders(short itemNumber);
void SpidersEmitterControl(short itemNumber);
void UpdateSpiders();