#pragma once
#include "..\Global\global.h"

#define LaraTorch ((void(__cdecl*)(PHD_VECTOR*,PHD_VECTOR*,int,int)) 0x00410550)

void TriggerTorchFlame(char fxObj, char node);
void DoFlameTorch();
void GetFlameTorch();
void TorchControl(short itemNumber);
void FireCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);