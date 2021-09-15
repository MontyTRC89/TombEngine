#pragma once
#include "items.h"
#include "control.h"

#define EffectFunction void(ITEM_INFO* item)

extern int FlipEffect;
extern short FXType;

extern std::function<EffectFunction> effect_routines[];

void AddFootprint(ITEM_INFO* item);
void VoidEffect(ITEM_INFO* item);
void FinishLevel(ITEM_INFO* item);
void Turn180(ITEM_INFO* item);
void FloorShake(ITEM_INFO* item);
void PlaySoundEffect(ITEM_INFO* item);
void RubbleFX(ITEM_INFO* item);
void PoseidonSFX(ITEM_INFO* item);
void ActivateCamera(ITEM_INFO* item);
void ActivateKey(ITEM_INFO* item);
void SwapCrowbar(ITEM_INFO* item);
void ExplosionFX(ITEM_INFO* item);
void LaraLocation(ITEM_INFO* item);
void LaraLocationPad(ITEM_INFO* item);
void KillActiveBaddies(ITEM_INFO* item);
void LaraHandsFree(ITEM_INFO* item);
void ShootRightGun(ITEM_INFO* item);
void ShootLeftGun(ITEM_INFO* item);
void SetFog(ITEM_INFO* item);
void InvisibilityOn(ITEM_INFO* item);
void InvisibilityOff(ITEM_INFO* item);
void ResetHair(ITEM_INFO* item);
void Pickup(ITEM_INFO* item);
void Puzzle(ITEM_INFO* item);
void DrawRightPistol(ITEM_INFO* item);
void DrawLeftPistol(ITEM_INFO* item);
void MeshSwapToPour(ITEM_INFO* item);
void MeshSwapFromPour(ITEM_INFO* item);