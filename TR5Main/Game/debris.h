#pragma once

#include "..\Global\global.h"

//#define ShatterObject ((void (__cdecl*)(SHATTER_ITEM*, MESH_INFO*, short, short, int)) 0x0041D6B0)
int ShatterObject(SHATTER_ITEM* shatterItem, MESH_INFO* meshInfo, int num, short roomNumber, int noXZvel);