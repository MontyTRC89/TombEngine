#pragma once

struct CollisionInfo;
struct ITEM_INFO;

void InitialiseFallingBlock(short itemNumber);
void InitialiseWreckingBall(short itemNumber);
void WreckingBallCollision(short itemNumber, ITEM_INFO* l, CollisionInfo* coll);
void WreckingBallControl(short itemNumber);