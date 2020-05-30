#pragma once
#include "setup.h"

void InitSmashObject(ObjectInfo* obj, int objectNumber);
void InitKeyHole(ObjectInfo* obj, int objectNumber);
void InitPuzzleHole(ObjectInfo* obj, int objectNumber);
void InitPuzzleDone(ObjectInfo* obj, int objectNumber);
void InitAnimating(ObjectInfo* obj, int objectNumber);
void InitPickup(ObjectInfo* obj, int objectNumber);
void InitPickupItem(ObjectInfo* obj, function<void(short itemNumber)> func, int objectNumber, bool useDrawAnimItem = false);
void InitProjectile(ObjectInfo* obj, function<void(short itemNumber)> func, int objectNumber, bool noLoad = false);
void InitSearchObject(ObjectInfo* obj, int objectNumber);
void InitPushableObject(ObjectInfo* obj, int objectNumber);