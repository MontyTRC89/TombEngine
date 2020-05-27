#pragma once

#include "global.h"

extern ITEM_INFO* WBItem;
extern short WBRoom;

void LaraBurn();
void LavaBurn(ITEM_INFO* item);
void FlameControl(short fxNumber);
void FlameEmitter2Control(short itemNumber);
void FlameEmitterControl(short itemNumber);
void InitialiseTrapDoor(short itemNumber);
void TrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void CeilingTrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void FloorTrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void TrapDoorControl(short itemNumber);
void CloseTrapDoor(ITEM_INFO* item);
void OpenTrapDoor(ITEM_INFO* item);
void InitialiseFallingBlock(short itemNumber);
void FallingBlockCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void FallingBlockControl(short itemNumber);
void FallingBlockFloor(ITEM_INFO* item, int x, int y, int z, int* height);
void FallingBlockCeiling(ITEM_INFO* item, int x, int y, int z, int* height);
void InitialiseWreckingBall(short itemNumber);
void WreckingBallCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void WreckingBallControl(short itemNumber);
void FlameEmitterCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void InitialiseFlameEmitter2(short itemNumber);
void InitialiseFlameEmitter(short itemNumber);
void FlameEmitter3Control(short itemNumber);