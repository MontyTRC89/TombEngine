#pragma once
#include "Specific/phd_global.h"
#include "Specific/EulerAngle.h"

constexpr auto HAIR_MAX = 2; // HAIR_NORMAL = 0, HAIR_YOUNG = 1
constexpr auto HAIR_SEGMENTS = 6; // classic = 7, young = 14
constexpr auto HAIR_SPHERE = 6; // current hair max collision

class EulerAngle;
struct ANIM_FRAME;
struct ITEM_INFO;

struct HAIR_STRUCT
{
	PoseData Pose;
	Vector3Int hvel;
	Vector3Int unknown;

	bool initialized = false;
};
extern HAIR_STRUCT Hairs[HAIR_MAX][HAIR_SEGMENTS + 1];

void InitialiseHair();
void HairControl(ITEM_INFO* item, bool young);
void HairControl(ITEM_INFO* item, int braid, ANIM_FRAME* framePtr);
