#pragma once
#include "items.h"
#include "collide.h"

void InitialiseDeathSlide(short itemNumber);
void DeathSlideCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void ControlDeathSlide(short itemNumber);