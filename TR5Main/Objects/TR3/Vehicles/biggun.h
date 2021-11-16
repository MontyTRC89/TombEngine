#pragma once

struct COLL_INFO;
struct ITEM_INFO;

void FireBigGun(ITEM_INFO* obj);
void BigGunInitialise(short itemNum);
static bool CanUseGun (ITEM_INFO* lara, ITEM_INFO* bigGun);
void BigGunInitialise(short itemNum);
void BigGunCollision(short itemNum, ITEM_INFO* lara, COLL_INFO* coll);
bool BigGunControl(ITEM_INFO* lara, COLL_INFO* coll);
