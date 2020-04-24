#pragma once

#include "..\Global\global.h"

extern bool GotLaraSpheres;
extern SPHERE BaddieSpheres[34];
extern SPHERE SphereList[34];

int TestCollision(ITEM_INFO* item, ITEM_INFO* l);
int GetSpheres(ITEM_INFO* item, SPHERE* ptr, char worldSpace);
void GetJointAbsPosition(ITEM_INFO* item, PHD_VECTOR* vec, int joint);
void GetMatrixFromTrAngle(Matrix* matrix, short* frameptr, int index);
