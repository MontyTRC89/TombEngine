#pragma once

#include "global.h"

extern HAIR_STRUCT Hairs[2][7];

void InitialiseHair();
void HairControl(int cutscene, int ponytail, short* framePtr);
