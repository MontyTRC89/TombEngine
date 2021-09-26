#pragma once

struct COLL_INFO;
struct ITEM_INFO;

enum FLAME_STATES : short {
	FLAME_NORMAL_STATE = 0,
	FLAME_PROJECTILE_STATE = 1
};
extern ITEM_INFO* WBItem;
extern short WBRoom;
void InitialiseFallingBlock(short itemNumber);
void InitialiseWreckingBall(short itemNumber);
void WreckingBallCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void WreckingBallControl(short itemNumber);