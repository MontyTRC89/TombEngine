#pragma once

void InitialiseRaisingBlock(short itemNumber);
void ControlRaisingBlock(short itemNumber);
std::optional<int> RaisingBlockFloor(short itemNumber, int x, int y, int z);
std::optional<int> RaisingBlockCeiling(short itemNumber, int x, int y, int z);
