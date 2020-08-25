#pragma once
#include "lara_struct.h"

typedef struct {
	short xRot;
	short yRot;
	short startYRot;
	char flags;
	signed char fireCount;
}BIGGUNINFO;
void FireBigGun(ITEM_INFO *obj);
void BigGunInitialise(short itemNum);
static int CanUseGun(ITEM_INFO *obj, ITEM_INFO *lara);
void BigGunInitialise(short itemNum);
void BigGunCollision(short itemNum, ITEM_INFO* lara, COLL_INFO* coll);
int BigGunControl(COLL_INFO *coll);
