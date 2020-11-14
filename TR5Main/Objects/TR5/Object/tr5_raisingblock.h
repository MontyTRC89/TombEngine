#pragma once

void InitialiseRaisingBlock(short itemNumber);
void ControlRaisingBlock(short itemNumber);
std::tuple<std::optional<int>, bool> RaisingBlockFloor(short itemNumber, int x, int y, int z);
std::tuple<std::optional<int>, bool> RaisingBlockCeiling(short itemNumber, int x, int y, int z);
