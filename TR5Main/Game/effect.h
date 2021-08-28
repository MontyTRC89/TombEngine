#pragma once
#include "phd_global.h"
struct ITEM_INFO;

#define EffectFunction void(ITEM_INFO* item)

struct FX_INFO
{
	PHD_3DPOS pos;
	short roomNumber;
	short objectNumber;
	short nextFx;
	short nextActive;
	short speed;
	short fallspeed;
	short frameNumber;
	short counter;
	short shade;
	short flag1;
	short flag2;
};

extern std::function<EffectFunction> effect_routines[];
extern FX_INFO* EffectList;

bool ItemNearLara(PHD_3DPOS* pos, int radius);
bool ItemNearTarget(PHD_3DPOS* src, ITEM_INFO* target, int radius);
void StopSoundEffect(short sampleIndex);
short DoBloodSplat(int x, int y, int z, short speed, short yRot, short roomNumber);
void AddFootprint(ITEM_INFO* item);
void ControlWaterfallMist(short itemNumber);
void void_effect(ITEM_INFO* item);
void finish_level_effect(ITEM_INFO* item);
void turn180_effect(ITEM_INFO* item);
void floor_shake_effect(ITEM_INFO* item);
void SoundFlipEffect(ITEM_INFO* item);
void RubbleFX(ITEM_INFO* item);
void PoseidonSFX(ITEM_INFO* item);
void ActivateCamera(ITEM_INFO* item);
void ActivateKey(ITEM_INFO* item);
void SwapCrowbar(ITEM_INFO* item);
void ExplosionFX(ITEM_INFO* item);
void LaraLocation(ITEM_INFO* item);
void LaraLocationPad(ITEM_INFO* item);
void KillActiveBaddies(ITEM_INFO* item);
void lara_hands_free(ITEM_INFO* item);
void shoot_right_gun(ITEM_INFO* item);
void shoot_left_gun(ITEM_INFO* item);
void SetFog(ITEM_INFO* item);
void invisibility_on(ITEM_INFO* item);
void invisibility_off(ITEM_INFO* item);
void reset_hair(ITEM_INFO* item);
void Richochet(PHD_3DPOS* pos);
void DoLotsOfBlood(int x, int y, int z, int speed, short direction, short roomNumber, int count);
void pickup(ITEM_INFO* item);
void puzzle(ITEM_INFO* item);
void draw_right_pistol(ITEM_INFO* item);
void draw_left_pistol(ITEM_INFO* item);
void MeshSwapToPour(ITEM_INFO* item);
void MeshSwapFromPour(ITEM_INFO* item);
