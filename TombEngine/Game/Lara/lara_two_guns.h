#pragma once
#include "Game/Lara/lara_struct.h"

void SetArmInfo(ItemInfo& laraItem, ArmInfo& arm, int frame);
void ReadyPistols(ItemInfo& laraItem, LaraWeaponType weaponType);

void HandlePistols(ItemInfo& laraItem, LaraWeaponType weaponType);
void AnimatePistols(ItemInfo& laraItem, LaraWeaponType weaponType);
void AnimateWeaponArm(ItemInfo& laraItem, LaraWeaponType weaponType, bool& hasFired, bool isRightArm);

void DrawPistols(ItemInfo& laraItem, LaraWeaponType weaponType);
void DrawPistolMeshes(ItemInfo& laraItem, LaraWeaponType weaponType);

void UndrawPistols(ItemInfo& laraItem, LaraWeaponType weaponType);
void UndrawPistolMeshRight(ItemInfo& laraItem, LaraWeaponType weaponType);
void UndrawPistolMeshLeft(ItemInfo& laraItem, LaraWeaponType weaponType);
