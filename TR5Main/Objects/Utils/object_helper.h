#pragma once
#include "Specific/setup.h"

#define InitFunction void(short itemNumber)

void InitSmashObject(OBJECT_INFO* obj, int objectNumber);
void InitKeyHole(OBJECT_INFO* obj, int objectNumber);
void InitPuzzleHole(OBJECT_INFO* obj, int objectNumber);
void InitPuzzleDone(OBJECT_INFO* obj, int objectNumber);
void InitAnimating(OBJECT_INFO* obj, int objectNumber);
void InitPickup(OBJECT_INFO* obj, int objectNumber);
void InitPickupItem(OBJECT_INFO* obj, std::function<InitFunction> func, int objectNumber, bool useDrawAnimItem = false);
void InitProjectile(OBJECT_INFO* obj, std::function<InitFunction> func, int objectNumber, bool noLoad = false);
void InitSearchObject(OBJECT_INFO* obj, int objectNumber);
void InitPushableObject(OBJECT_INFO* obj, int objectNumber);
