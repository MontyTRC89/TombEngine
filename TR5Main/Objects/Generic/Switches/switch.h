#pragma once

struct COLL_INFO;
struct ITEM_INFO;
void ProcessExplodingSwitchType8(ITEM_INFO* item);
void InitialiseShootSwitch(short itemNumber);
void ShootSwitchCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
