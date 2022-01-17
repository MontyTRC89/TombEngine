#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"

void InitialiseSkidoo(short itemNum);
void SkidooCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll);
bool SkidooControl(ITEM_INFO* lara, COLL_INFO* coll);
void DrawSkidoo(ITEM_INFO* item);
void DoSnowEffect(ITEM_INFO* skidoo);
void SkidooGuns(ITEM_INFO* lara, ITEM_INFO* skidoo);
void SkidooBaddieCollision(ITEM_INFO* skidoo);
void SkidooExplode(ITEM_INFO* lara, ITEM_INFO* skidoo);
bool SkidooCheckGetOffOK(ITEM_INFO* skidoo, int dir);
bool SkidooCheckGetOff(ITEM_INFO* lara, ITEM_INFO* skidoo);
void SkidooAnimation(ITEM_INFO* lara, ITEM_INFO* skidoo, int collide, bool dead);
int GetSkidooCollisionAnim(ITEM_INFO* skidoo, PHD_VECTOR* moved);
bool SkidooUserControl(ITEM_INFO* skidoo, int height, int* pitch);
int DoSkidooDynamics(int height, int fallspeed, int* y);
int SkidooCheckGetOn(ITEM_INFO* lara, ITEM_INFO* skidoo, COLL_INFO* coll);
int TestSkidooHeight(ITEM_INFO* skidoo, int z_off, int x_off, PHD_VECTOR* pos);
short DoSkidooShift(ITEM_INFO* skidoo, PHD_VECTOR* pos, PHD_VECTOR* old);
int SkidooDynamics(ITEM_INFO* lara, ITEM_INFO* skidoo);
