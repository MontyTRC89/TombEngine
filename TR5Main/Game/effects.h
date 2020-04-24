#pragma once

#include "..\Global\global.h"
#include "control.h"

extern FX_INFO* Effects;

int ItemNearLara(PHD_3DPOS* pos, int radius);
void StopSoundEffect(short sampleIndex);
short DoBloodSplat(int x, int y, int z, short a4, short a5, short roomNumber);
//void SoundEffects();
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
void ResetTest(ITEM_INFO* item);
void KillActiveBaddies(ITEM_INFO* item);
void lara_hands_free(ITEM_INFO* item);
void shoot_right_gun(ITEM_INFO* item);
void shoot_left_gun(ITEM_INFO* item);
void SetFog(ITEM_INFO* item);
void invisibility_on(ITEM_INFO* item);
void invisibility_off(ITEM_INFO* item);
void reset_hair(ITEM_INFO* item);
void TL_1(ITEM_INFO* item);
void TL_2(ITEM_INFO* item);
void TL_3(ITEM_INFO* item);
void TL_4(ITEM_INFO* item);
void TL_5(ITEM_INFO* item);
void TL_6(ITEM_INFO* item);
void TL_7(ITEM_INFO* item);
void TL_8(ITEM_INFO* item);
void TL_9(ITEM_INFO* item);
void TL_10(ITEM_INFO* item);
void TL_11(ITEM_INFO* item);
void TL_12(ITEM_INFO* item);
void Richochet(PHD_3DPOS* pos);
void DoLotsOfBlood(int x, int y, int z, int speed, short direction, short roomNumber, int count);
