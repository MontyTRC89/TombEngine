#pragma once
#include "items.h"
#include "collision/collide_room.h"

void InitialiseDeathSlide(short itemNumber);
void DeathSlideCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void ControlDeathSlide(short itemNumber);