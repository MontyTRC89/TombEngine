#pragma once

#include "..\Global\global.h"

extern int NumLaraSpheres;
extern bool GotLaraSpheres;
extern SPHERE LaraSpheres[34];
extern SPHERE SpheresList[34];

int TestCollision(ITEM_INFO* item, ITEM_INFO* l);
int GetSpheres(ITEM_INFO* item, SPHERE* ptr, char worldSpace, Matrix local);
void GetJointAbsPosition(ITEM_INFO* item, PHD_VECTOR* vec, int joint);
void GetMatrixFromTrAngle(Matrix* matrix, short* frameptr, int index);
