#pragma once
#include "Game/Lara/lara_struct.h"

void AnimatePistols(ItemInfo* laraItem, LaraWeaponType weaponType);
void PistolHandler(ItemInfo* laraItem, LaraWeaponType weaponType);
void ReadyPistols(ItemInfo* laraItem, LaraWeaponType weaponType);
void DrawPistols(ItemInfo* laraItem, LaraWeaponType weaponType);
void UndrawPistols(ItemInfo* laraItem, LaraWeaponType weaponType);
void SetArmInfo(ItemInfo* laraItem, ArmInfo* arm, int frame);
void DrawPistolMeshes(ItemInfo* laraItem, LaraWeaponType weaponType);
void UndrawPistolMeshRight(ItemInfo* laraItem, LaraWeaponType weaponType);
void UndrawPistolMeshLeft(ItemInfo* laraItem, LaraWeaponType weaponType);
