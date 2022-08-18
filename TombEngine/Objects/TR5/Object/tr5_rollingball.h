#pragma once

struct CollisionInfo;
struct ItemInfo;

void InitialiseClassicRollingBall(short itemNumber);
void RollingBallCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void RollingBallControl(short itemNumber);
void ClassicRollingBallCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void ClassicRollingBallControl(short itemNumber);
