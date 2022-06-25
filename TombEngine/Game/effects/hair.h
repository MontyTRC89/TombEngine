#pragma once
#include "Specific/phd_global.h"

constexpr auto HAIR_MAX = 2; // HAIR_NORMAL = 0, HAIR_YOUNG = 1
constexpr auto HAIR_SEGMENTS = 6; // classic = 7, young = 14
constexpr auto HAIR_SPHERE = 6; // current hair max collision

struct ANIM_FRAME;
struct ItemInfo;

struct HAIR_STRUCT
{
	PHD_3DPOS pos;
	Vector3Int hvel;
	Vector3Int unknown;

	bool initialised = false;
};
extern HAIR_STRUCT Hairs[HAIR_MAX][HAIR_SEGMENTS + 1];

void InitialiseHair();
void HairControl(ItemInfo* item, bool young);
void HairControl(ItemInfo* item, int ponytail, ANIM_FRAME* framePtr);
