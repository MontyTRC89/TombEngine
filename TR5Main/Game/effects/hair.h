#pragma once

#include "Specific\phd_global.h"

constexpr auto HAIR_MAX = 2; // HAIR_NORMAL = 0, HAIR_YOUNG = 1
constexpr auto HAIR_SEGMENTS = 6; // classic = 7, young = 14
constexpr auto HAIR_SPHERE = 6; // current hair max collision

struct ANIM_FRAME;

struct HAIR_STRUCT
{
	PHD_3DPOS pos;
	PHD_VECTOR hvel;
	PHD_VECTOR unknown;

	bool initialized = false;
};
extern HAIR_STRUCT Hairs[HAIR_MAX][HAIR_SEGMENTS + 1];

void InitialiseHair();
void HairControl(int cutscene, int ponytail, ANIM_FRAME* framePtr);
