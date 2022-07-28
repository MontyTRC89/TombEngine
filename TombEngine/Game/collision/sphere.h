#pragma once
#include "Game/items.h"

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

extern SPHERE LaraSpheres[MAX_SPHERES];
extern SPHERE CreatureSpheres[MAX_SPHERES];

int TestCollision(ItemInfo* item, ItemInfo* laraItem);
int GetSpheres(ItemInfo* item, SPHERE* ptr, int worldSpace, Matrix local);
