#pragma once

struct CollisionInfo;
struct ITEM_INFO;

void ProcessExplodingSwitchType8(ITEM_INFO* item);
void InitialiseShootSwitch(short itemNumber);
void ShootSwitchCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
