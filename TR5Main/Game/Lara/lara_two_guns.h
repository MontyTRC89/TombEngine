#pragma once
#include "Game/Lara/lara_struct.h"

void AnimatePistols(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void PistolHandler(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void ReadyPistols(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void DrawPistols(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void UndrawPistols(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void SetArmInfo(ITEM_INFO* laraItem, ArmInfo* arm, int frame);
void DrawPistolMeshes(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void UndrawPistolMeshRight(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void UndrawPistolMeshLeft(ITEM_INFO* laraItem, LaraWeaponType weaponType);
