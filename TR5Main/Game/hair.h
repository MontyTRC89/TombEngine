#pragma once

#include "phd_global.h"

constexpr auto HAIR_MAX = 2; // HAIR_NORMAL = 0, HAIR_YOUNG = 1
constexpr auto HAIR_SEGMENTS = 7; // classic = 7, young = 14
constexpr auto HAIR_SPHERE = 5; // current hair max collision

struct HAIR_STRUCT
{
	PHD_3DPOS pos;
	PHD_VECTOR hvel;
	PHD_VECTOR unknown;
};
extern HAIR_STRUCT Hairs[HAIR_MAX][HAIR_SEGMENTS];

void InitialiseHair();
void HairControl(int cutscene, int ponytail, short* framePtr);
