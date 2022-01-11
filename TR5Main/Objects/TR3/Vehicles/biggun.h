#pragma once

struct COLL_INFO;
struct ITEM_INFO;

void BigGunInitialise(short itemNum);
static bool BigGunTestMount(ITEM_INFO* bGunItem, ITEM_INFO* laraItem);
void BigGunFire(ITEM_INFO* bGunItem, ITEM_INFO* laraItem);
void BigGunCollision(short itemNum, ITEM_INFO* laraItem, COLL_INFO* coll);
bool BigGunControl(ITEM_INFO* laraItem, COLL_INFO* coll);
