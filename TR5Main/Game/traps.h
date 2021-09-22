#pragma once

struct COLL_INFO;
struct ITEM_INFO;

extern ITEM_INFO* WBItem;
extern short WBRoom;
void InitialiseFallingBlock(short itemNumber);
void FallingBlockCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void FallingBlockControl(short itemNumber);
std::optional<int> FallingBlockFloor(short itemNumber, int x, int y, int z);
std::optional<int> FallingBlockCeiling(short itemNumber, int x, int y, int z);
int FallingBlockFloorBorder(short itemNumber);
int FallingBlockCeilingBorder(short itemNumber);
void InitialiseWreckingBall(short itemNumber);
void WreckingBallCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void WreckingBallControl(short itemNumber);