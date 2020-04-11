#pragma once
#include "..\Global\global.h"

void TriggerTorchFlame(char fxObj, char node);
void DoFlameTorch();
void GetFlameTorch();
void TorchControl(short itemNumber);
void FireCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);