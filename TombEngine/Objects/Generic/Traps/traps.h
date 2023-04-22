#pragma once

struct CollisionInfo;
struct ItemInfo;

void InitializeFallingBlock(short itemNumber);
void InitializeWreckingBall(short itemNumber);
void WreckingBallCollision(short itemNumber, ItemInfo* l, CollisionInfo* coll);
void WreckingBallControl(short itemNumber);