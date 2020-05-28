#pragma once
#include "types.h"
#include "collide.h"

void InitialiseSkidman(short itemNum);
void SkidManCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
void SkidManControl(short riderNum);