#pragma once

struct ItemInfo;
struct CollisionInfo;

void InitializeZipLine(short itemNumber);
void ZipLineCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void ControlZipLine(short itemNumber);
