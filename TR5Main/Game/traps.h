#pragma once

struct COLL_INFO;
struct ITEM_INFO;

extern ITEM_INFO* WBItem;
extern short WBRoom;
void LaraBurn();
void LavaBurn(ITEM_INFO* item);
void FlameControl(short fxNumber);
void FlameEmitter2Control(short itemNumber);
void FlameEmitterControl(short itemNumber);
void InitialiseFallingBlock(short itemNumber);
void InitialiseWreckingBall(short itemNumber);
void WreckingBallCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void WreckingBallControl(short itemNumber);
void FlameEmitterCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void InitialiseFlameEmitter2(short itemNumber);
void InitialiseFlameEmitter(short itemNumber);
void FlameEmitter3Control(short itemNumber);