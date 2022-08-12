#pragma once

struct CollisionInfo;
struct ItemInfo;

void InitialiseWreckingBall(short itemNumber);
void WreckingBallCollision(short itemNumber, ItemInfo* l, CollisionInfo* coll);
void WreckingBallControl(short itemNumber);