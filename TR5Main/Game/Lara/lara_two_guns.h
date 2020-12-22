#pragma once
#include "lara_struct.h"

void AnimatePistols(int weaponType);
void PistolHandler(int weaponType);
void undraw_pistol_mesh_right(int weaponType);
void undraw_pistol_mesh_left(int weaponType);
void draw_pistol_meshes(int weaponType);
void ready_pistols(int weaponType);
void undraw_pistols(int weaponType);
void set_arm_info(LARA_ARM* arm, int frame);
void draw_pistols(int weaponType);
