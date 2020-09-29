#pragma once
#include "phd_global.h"

void InitialiseBridge(short itemNumber);
std::optional<int> BridgeFloor(short itemNumber, int x, int y, int z);
std::optional<int> BridgeCeiling(short itemNumber, int x, int y, int z);
int GetOffset(PHD_3DPOS* pos, int x, int z);
