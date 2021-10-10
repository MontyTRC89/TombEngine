#pragma once
#include "items.h"

#define SPHERES_SPACE_LOCAL			0
#define SPHERES_SPACE_WORLD			1
#define SPHERES_SPACE_BONE_ORIGIN	2	
#define	MAX_SPHERES					34

struct SPHERE
{
	int x;
	int y;
	int z;
	int r;
};

extern bool GotLaraSpheres;
extern SPHERE LaraSpheres[MAX_SPHERES];
extern SPHERE CreatureSpheres[MAX_SPHERES];

int TestCollision(ITEM_INFO* item, ITEM_INFO* l);
int GetSpheres(ITEM_INFO* item, SPHERE* ptr, int worldSpace, Matrix local);
