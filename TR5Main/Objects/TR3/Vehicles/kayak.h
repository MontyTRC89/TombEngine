#pragma once
struct ITEM_INFO;
struct COLL_INFO;
struct PHD_VECTOR;
struct KAYAK_INFO;


void KayakDoWake(ITEM_INFO* v, short xoff, short zoff, short rotate);
void KayakDoRipple(ITEM_INFO* v, short xoff, short zoff);
void KayakUpdateWakeFX();
int KayakGetIn(short itemNumber, COLL_INFO* coll);
int KayakGetCollisionAnim(ITEM_INFO* v, int xdiff, int zdiff);
int KayakDoDynamics(int height, int fallspeed, int* y);
void KayakDoCurrent(ITEM_INFO* item);
int KayakTestHeight(ITEM_INFO* item, int x, int z, PHD_VECTOR* pos);
bool KayakCanGetOut(ITEM_INFO* v, int direction);
int KayakDoShift(ITEM_INFO* v, PHD_VECTOR* pos, PHD_VECTOR* old);
void KayakToBackground(ITEM_INFO* v, KAYAK_INFO* Kayak);
void KayakUserInput(ITEM_INFO* v, ITEM_INFO* l, KAYAK_INFO* Kayak);
void KayakToBaddieCollision(ITEM_INFO* v);
void KayakLaraRapidsDrown();
void InitialiseKayak(short itemNumber);
void KayakDraw(ITEM_INFO* v);
void KayakCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);

void InitialiseKayak(short itemNumber);
void KayakCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
int KayakControl(void);