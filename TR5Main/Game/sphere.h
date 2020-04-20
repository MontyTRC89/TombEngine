#pragma once

#include "..\Global\global.h"

#define InterpolateMatrix ((int (__cdecl*)()) 0x0042C8F0)
#define InitInterpolate2 ((int (__cdecl*)(int, int)) 0x00479BB0)

extern bool GotLaraSpheres;
extern SPHERE BaddieSpheres[34];

int TestCollision(ITEM_INFO* item, ITEM_INFO* l);
int GetSpheres(ITEM_INFO* item, SPHERE* ptr, char worldSpace);
void GetJointAbsPosition(ITEM_INFO* item, PHD_VECTOR* vec, int joint);
void GetMatrixFromTrAngle(Matrix* matrix, short* frameptr, int index);

void Inject_Sphere();