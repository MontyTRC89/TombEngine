#pragma once
#include "Game/Lara/lara_struct.h"

void AnimatePistols(LARA_WEAPON_TYPE weaponType);
void PistolHandler(LARA_WEAPON_TYPE weaponType);
void undraw_pistol_mesh_right(LARA_WEAPON_TYPE weaponType);
void undraw_pistol_mesh_left(LARA_WEAPON_TYPE weaponType);
void draw_pistol_meshes(LARA_WEAPON_TYPE weaponType);
void ready_pistols(LARA_WEAPON_TYPE weaponType);
void undraw_pistols(LARA_WEAPON_TYPE weaponType);
void set_arm_info(LARA_ARM* arm, int frame);
void draw_pistols(LARA_WEAPON_TYPE weaponType);
