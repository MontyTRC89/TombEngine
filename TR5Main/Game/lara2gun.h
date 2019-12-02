#pragma once
#include "..\Global\global.h"

void __cdecl AnimatePistols(int weaponType);
void __cdecl PistolHandler(int weaponType);
void __cdecl undraw_pistol_mesh_right(int weaponType);
void __cdecl undraw_pistol_mesh_left(int weaponType);
void __cdecl draw_pistol_meshes(int weaponType);
void __cdecl ready_pistols(int weaponType);
void __cdecl undraw_pistols(int weaponType);
void __cdecl set_arm_info(LARA_ARM* arm, int frame);
void __cdecl draw_pistols(int weaponType);
