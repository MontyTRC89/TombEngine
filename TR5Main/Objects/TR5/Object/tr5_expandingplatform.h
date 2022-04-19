#pragma once

void InitialiseExpandingPlatform(short itemNumber);
void ControlExpandingPlatform(short itemNumber);
std::optional<int> ExpandingPlatformFloor(short itemNumber, int x, int y, int z);
std::optional<int> ExpandingPlatformCeiling(short itemNumber, int x, int y, int z);
int ExpandingPlatformFloorBorder(short itemNumber);
int ExpandingPlatformCeilingBorder(short itemNumber);
void ExpandingPlatformUpdateMutators(short itemNumber);
