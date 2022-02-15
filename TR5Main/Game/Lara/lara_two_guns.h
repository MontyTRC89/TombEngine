#pragma once
#include "Game/Lara/lara_struct.h"

void AnimatePistols(ITEM_INFO* laraItem, LARA_WEAPON_TYPE weaponType);
void PistolHandler(ITEM_INFO* laraItem, LARA_WEAPON_TYPE weaponType);
void ReadyPistols(ITEM_INFO* laraItem, LARA_WEAPON_TYPE weaponType);
void DrawPistols(ITEM_INFO* laraItem, LARA_WEAPON_TYPE weaponType);
void UndrawPistols(ITEM_INFO* laraItem, LARA_WEAPON_TYPE weaponType);
void SetArmInfo(ITEM_INFO* laraItem, ArmInfo* arm, int frame);
void DrawPistolMeshes(ITEM_INFO* laraItem, LARA_WEAPON_TYPE weaponType);
void UndrawPistolMeshRight(ITEM_INFO* laraItem, LARA_WEAPON_TYPE weaponType);
void UndrawPistolMeshLeft(ITEM_INFO* laraItem, LARA_WEAPON_TYPE weaponType);
