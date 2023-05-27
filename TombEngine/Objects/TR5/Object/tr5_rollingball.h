#pragma once

struct ItemInfo;
struct CollisionInfo;

void RollingBallCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void RollingBallControl(short itemNumber);
void InitializeClassicRollingBall(short itemNumber);
void ClassicRollingBallCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void ClassicRollingBallControl(short itemNumber);
