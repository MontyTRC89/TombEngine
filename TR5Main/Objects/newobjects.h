#pragma once
#include "global.h"
#include "collide.h"

// TR4 object
void SarcophagusCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
void InitialiseLaraDouble(short itemNum);
void LaraDoubleControl(short itemNum);
void ScalesControl(short itemNum);
void ScalesCollision(short itemNum, ITEM_INFO* item, COLL_INFO* coll);
int RespawnAhmet(short itemNum);
void BubblesControl(short fxNum);
int BubblesShatterFunction(FX_INFO* fx, int param1, int param2);
void BubblesEffect1(short fxNum, short xVel, short yVel, short zVel);
void BubblesEffect2(short fxNum, short xVel, short yVel, short zVel);
void BubblesEffect3(short fxNum, short xVel, short yVel, short zVel);
void BubblesEffect4(short fxNum, short xVel, short yVel, short zVel);
