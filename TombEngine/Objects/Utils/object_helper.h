#pragma once
#include "Specific/setup.h"

#define InitFunction void(short itemNumber)

void InitSmashObject(ObjectInfo* obj, int objectNumber);
void InitKeyHole(ObjectInfo* obj, int objectNumber);
void InitPuzzleHole(ObjectInfo* obj, int objectNumber);
void InitPuzzleDone(ObjectInfo* obj, int objectNumber);
void InitAnimating(ObjectInfo* obj, int objectNumber);
void InitPickup(ObjectInfo* obj, int objectNumber);
void InitFlare(ObjectInfo* obj, int objectNumber);
void InitProjectile(ObjectInfo* obj, std::function<InitFunction> func, int objectNumber, bool noLoad = false);
void InitSearchObject(ObjectInfo* obj, int objectNumber);
void InitPushableObject(ObjectInfo* obj, int objectNumber);
