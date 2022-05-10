#pragma once
#include "Specific/phd_global.h"

constexpr auto HAIR_MAX = 2; // HAIR_NORMAL = 0, HAIR_YOUNG = 1
constexpr auto HAIR_SEGMENTS = 6; // classic = 7, young = 14
constexpr auto HAIR_SPHERE = 6; // current hair max collision

class EulerAngles;
struct ANIM_FRAME;
struct ItemInfo;

struct HairData
{
	PoseData Pose;
	Vector3  Velocity;

	bool Initialized = false;
};
extern HairData Hairs[HAIR_MAX][HAIR_SEGMENTS + 1];

void InitialiseHair();
void HairControl(ItemInfo* item, bool young);
void HairControl(ItemInfo* item, int braid, ANIM_FRAME* framePtr);
