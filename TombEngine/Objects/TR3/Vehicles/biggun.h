#pragma once

struct CollisionInfo;
struct ItemInfo;

void BigGunInitialise(short itemNumber);
static bool BigGunTestMount(ItemInfo* laraItem, ItemInfo* bigGunItem);
void BigGunFire(ItemInfo* laraItem, ItemInfo* bigGunItem);
void BigGunCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
bool BigGunControl(ItemInfo* laraItem, CollisionInfo* coll);
