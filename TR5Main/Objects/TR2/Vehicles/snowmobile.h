#pragma once
#include "items.h"
#include "collide.h"



void InitialiseSkidoo(short itemNum);
void SkidooCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll);
bool SkidooControl();
void DrawSkidoo(ITEM_INFO* item);
void DoSnowEffect(ITEM_INFO* skidoo);
void SkidooGuns();
void SkidooBaddieCollision(ITEM_INFO* skidoo);
void SkidooExplode(ITEM_INFO* skidoo);
bool SkidooCheckGetOffOK(int direction);
bool SkidooCheckGetOff();
void SkidooAnimation(ITEM_INFO* skidoo, int collide, bool dead);
int GetSkidooCollisionAnim(ITEM_INFO* skidoo, PHD_VECTOR* moved);
bool SkidooUserControl(ITEM_INFO* skidoo, int height, int* pitch);
int DoSkidooDynamics(int height, int fallspeed, int* y);
int SkidooCheckGetOn(short itemNum, COLL_INFO* coll);
int TestSkidooHeight(ITEM_INFO* item, int z_off, int x_off, PHD_VECTOR* pos);
short DoSkidooShift(ITEM_INFO* skidoo, PHD_VECTOR* pos, PHD_VECTOR* old);
int SkidooDynamics(ITEM_INFO* skidoo);