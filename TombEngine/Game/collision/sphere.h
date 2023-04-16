#pragma once
#include "Game/items.h"

#define SPHERES_SPACE_LOCAL			0
#define SPHERES_SPACE_WORLD			1
#define SPHERES_SPACE_BONE_ORIGIN	2	
#define	MAX_SPHERES					34

struct SPHERE
{
	int x = 0;
	int y = 0;
	int z = 0;
	int r = 0;

	SPHERE() {};

	SPHERE(const Vector3& pos, float radius)
	{
		this->x = (int)round(pos.x);
		this->y = (int)round(pos.y);
		this->z = (int)round(pos.z);
		this->r = (int)round(radius);
	}
};

extern SPHERE LaraSpheres[MAX_SPHERES];
extern SPHERE CreatureSpheres[MAX_SPHERES];

int TestCollision(ItemInfo* item, ItemInfo* laraItem);
int GetSpheres(ItemInfo* item, SPHERE* ptr, int worldSpace, Matrix local);
