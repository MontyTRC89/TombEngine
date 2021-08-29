#pragma once
struct COLL_INFO;
struct ITEM_INFO;

void FireBigGun(ITEM_INFO *obj);
void BigGunInitialise(short itemNum);
static int CanUseGun(ITEM_INFO *obj, ITEM_INFO *lara);
void BigGunInitialise(short itemNum);
void BigGunCollision(short itemNum, ITEM_INFO* lara, COLL_INFO* coll);
int BigGunControl(COLL_INFO *coll);
