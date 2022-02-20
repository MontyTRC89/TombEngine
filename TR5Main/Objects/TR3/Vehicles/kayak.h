#pragma once

struct ITEM_INFO;
struct COLL_INFO;
struct PHD_VECTOR;

void InitialiseKayak(short itemNumber);
void KayakDraw(ITEM_INFO* kayakItem);

void KayakDoWake(ITEM_INFO* kayakItem, int xOffset, int zOffset, short rotate);
void KayakDoRipple(ITEM_INFO* kayakItem, int xOffset, int zOffset);
void KayakUpdateWakeFX();

int KayakGetCollisionAnim(ITEM_INFO* kayakItem, int xDiff, int zDiff);
int KayakDoDynamics(int height, int verticalVelocity, int* y);
void KayakDoCurrent(ITEM_INFO* laraItem, ITEM_INFO* kayakItem);
int KayakTestHeight(ITEM_INFO* kayakItem, int x, int z, PHD_VECTOR* pos);
bool KayakCanGetOut(ITEM_INFO* kayakItem, int dir);
int KayakDoShift(ITEM_INFO* kayakItem, PHD_VECTOR* pos, PHD_VECTOR* old);
void KayakToBackground(ITEM_INFO* laraItem, ITEM_INFO* kayakItem);
void KayakUserInput(ITEM_INFO* laraItem, ITEM_INFO* kayakItem);
void KayakToItemCollision(ITEM_INFO* laraItem, ITEM_INFO* kayakItem);
void KayakLaraRapidsDrown(ITEM_INFO* laraItem);

void KayakCollision(short itemNum, ITEM_INFO* laraItem, COLL_INFO* coll);
bool KayakControl(ITEM_INFO* laraItem);
