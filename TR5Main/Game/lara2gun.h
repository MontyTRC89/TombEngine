#pragma once
#include "..\Global\global.h"

void __cdecl AnimatePistols(__int32 weaponType);
void __cdecl PistolHandler(__int32 weaponType);
void __cdecl undraw_pistol_mesh_right(__int32 weaponType);
void __cdecl undraw_pistol_mesh_left(__int32 weaponType);
void __cdecl draw_pistol_meshes(__int32 weaponType);
void __cdecl ready_pistols(__int32 weaponType);
void __cdecl undraw_pistols(__int32 weaponType);
void __cdecl set_arm_info(LARA_ARM* arm, __int32 frame);
void __cdecl draw_pistols(__int32 weaponType);
