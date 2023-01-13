#pragma once
#include "Specific/setup.h"

#define InitFunction void(short itemNumber)
#define ControlFunction void(short itemNumber)

void AssignObjectMeshSwap(ObjectInfo* obj, int requiredMeshSwap, const std::string& baseName, const std::string& requiredName);
void CheckIfSlotExists(int requiredObj, const std::string& baseName, const std::string& requiredName);
bool AssignObjectAnimations(ObjectInfo* obj, int requiredObj, const std::string& baseName = "NOT_SET", const std::string& requiredName = "NOT_SET");
void InitSmashObject(ObjectInfo* obj, int objectNumber);
void InitKeyHole(ObjectInfo* obj, int objectNumber);
void InitPuzzleHole(ObjectInfo* obj, int objectNumber);
void InitPuzzleDone(ObjectInfo* obj, int objectNumber);
void InitAnimating(ObjectInfo* obj, int objectNumber);
void InitPickup(ObjectInfo* obj, int objectNumber);
void InitPickup(ObjectInfo* obj, int objectNumber, std::function<ControlFunction> func);
void InitFlare(ObjectInfo* obj, int objectNumber);
void InitProjectile(ObjectInfo* obj, std::function<InitFunction> func, int objectNumber, bool noLoad = false);
void InitSearchObject(ObjectInfo* obj, int objectNumber);
void InitPushableObject(ObjectInfo* obj, int objectNumber);
