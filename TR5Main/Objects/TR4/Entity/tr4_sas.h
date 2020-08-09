#pragma once
#include <collide.h>

void InitialiseSas(short itemNumber);
void SasControl(short itemNumber);
void InitialiseSasDying(short itemNumber);
void SasDyingControl(short itemNumber);
void SasDragBlokeCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);