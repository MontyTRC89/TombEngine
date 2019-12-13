#include "hair.h"
#include "..\Global\global.h"

void InitialiseHair()
{
	int* bone;

	FirstHair = 1;
	bone = &Bones[Objects[ID_HAIR].boneIndex];

	Hairs[0].pos.yRot = 0;
	Hairs[0].pos.xRot = -0x4000;

	// normal hair
	for (int i = 1; i < HAIR_SEGMENTS; i++, bone += 4)
	{
		Hairs[i].pos.xPos = *(bone + 1);
		Hairs[i].pos.yPos = *(bone + 2);
		Hairs[i].pos.zPos = *(bone + 3);
		Hairs[i].pos.xRot = -0x4000;
		Hairs[i].pos.yRot = 0;
		Hairs[i].pos.zRot = 0;
		//Hairs[i].hvel.x = 0;
		//Hairs[i].hvel.y = 0;
		//Hairs[i].hvel.z = 0;
	}
}

void Inject_Hair()
{
	
}