#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"

void InitialiseSkidoo(short itemNumber);

int GetSkidooMountType(ITEM_INFO* laraItem, ITEM_INFO* skidooItem, CollisionInfo* coll);
bool TestSkidooDismountOK(ITEM_INFO* skidooItem, int direction);
bool TestSkidooDismount(ITEM_INFO* laraItem, ITEM_INFO* skidooItem);

int GetSkidooCollisionAnim(ITEM_INFO* skidooItem, Vector3Int* moved);
void SkidooCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void SkidooEntityCollision(ITEM_INFO* laraItem, ITEM_INFO* skidooItem);

void SkidooGuns(ITEM_INFO* laraItem, ITEM_INFO* skidooItem);
void SkidooExplode(ITEM_INFO* laraItem, ITEM_INFO* skidooItem);
void DoSnowEffect(ITEM_INFO* skidooItem);

bool SkidooControl(ITEM_INFO* laraItem, CollisionInfo* coll);
bool SkidooUserControl(ITEM_INFO* laraItem, ITEM_INFO* skidooItem, int height, int* pitch);
void SkidooAnimation(ITEM_INFO* laraItem, ITEM_INFO* skidooItem, int collide, bool dead);

int SkidooDynamics(ITEM_INFO* laraItem, ITEM_INFO* skidooItem);
int TestSkidooHeight(ITEM_INFO* skidooItem, int zOffset, int xOffset, Vector3Int* pos);
short DoSkidooShift(ITEM_INFO* skidooItem, Vector3Int* pos, Vector3Int* old);
int DoSkidooDynamics(int height, int verticalVelocity, int* y);

void DrawSkidoo(ITEM_INFO* skidooItem);
