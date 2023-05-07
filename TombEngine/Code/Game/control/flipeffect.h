#pragma once
#include "Game/items.h"
#include "Game/control/control.h"

#define EffectFunction void(ItemInfo* item)

constexpr auto NUM_FLIPEFFECTS = 47;

extern int FlipEffect;

extern std::function<EffectFunction> effect_routines[];

void AddLeftFootprint(ItemInfo* item);
void AddRightFootprint(ItemInfo* item);
void VoidEffect(ItemInfo* item);
void FinishLevel(ItemInfo* item);
void Turn180(ItemInfo* item);
void FloorShake(ItemInfo* item);
void PlaySoundEffect(ItemInfo* item);
void RubbleFX(ItemInfo* item);
void PoseidonSFX(ItemInfo* item);
void ActivateCamera(ItemInfo* item);
void ActivateKey(ItemInfo* item);
void SwapCrowbar(ItemInfo* item);
void ExplosionFX(ItemInfo* item);
void LaraLocation(ItemInfo* item);
void LaraLocationPad(ItemInfo* item);
void KillActiveBaddys(ItemInfo* item);
void LaraHandsFree(ItemInfo* item);
void ShootRightGun(ItemInfo* item);
void ShootLeftGun(ItemInfo* item);
void SetFog(ItemInfo* item);
void InvisibilityOn(ItemInfo* item);
void InvisibilityOff(ItemInfo* item);
void ResetHair(ItemInfo* item);
void Pickup(ItemInfo* item);
void Puzzle(ItemInfo* item);
void DrawRightPistol(ItemInfo* item);
void DrawLeftPistol(ItemInfo* item);
void MeshSwapToPour(ItemInfo* item);
void MeshSwapFromPour(ItemInfo* item); 
void FlashOrange(ItemInfo* item);
void ClearSwarmEnemies(ItemInfo* item);

void DoFlipEffect(int number, ItemInfo* item = nullptr);
