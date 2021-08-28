#pragma once

#include "collide.h"

void ProcessExplodingSwitchType8(ITEM_INFO* item);
void InitialiseShootSwitch(short itemNumber);
void ShootSwitchCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
