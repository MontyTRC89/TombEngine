#pragma once

struct CollisionInfo;
struct ItemInfo;

void ProcessExplodingSwitchType8(ItemInfo* item);
void InitialiseShootSwitch(short itemNumber);
void ShootSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
