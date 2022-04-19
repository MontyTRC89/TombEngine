#pragma once

struct CollisionInfo;
struct ITEM_INFO;

void BigGunInitialise(short itemNumber);
static bool BigGunTestMount(ITEM_INFO* laraItem, ITEM_INFO* bigGunItem);
void BigGunFire(ITEM_INFO* laraItem, ITEM_INFO* bigGunItem);
void BigGunCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
bool BigGunControl(ITEM_INFO* laraItem, CollisionInfo* coll);
