#pragma once
#include "phd_global.h"

void InitialiseBridge(short itemNumber);
int BridgeFloor(short itemNumber, int x, int y, int z);
int BridgeCeiling(short itemNumber, int x, int y, int z);
int GetOffset(PHD_3DPOS* pos, int x, int z);
