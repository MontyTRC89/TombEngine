#include "debris.h"
#include "..\Global\global.h"
#include "../Specific/roomload.h"

short* CurrentShatterMeshPtr;

int ShatterObject(SHATTER_ITEM* shatterItem, MESH_INFO* meshInfo, int num, short roomNumber, int noXZvel)
{
	/*int v57 = 0;
	int v58 = 0;
	int v59 = 0;
	int v62 = 0;
	int negative = 0;

	if (num < 0)
	{
		num = -num;
		negative = 1;
	}

	int x, y, z;
	short* meshPtr;
	short yRot;
	short shade = 0;

	if (shatterItem)
	{
		meshPtr = shatterItem->meshp;
		yRot = shatterItem->yRot;
		x = shatterItem->sphere.x;
		y = shatterItem->sphere.y;
		z = shatterItem->sphere.z;
		v62 = 2 * ((shatterItem->flags >> 12) & 1);
	}
	else
	{
		yRot = meshInfo->yRot;
		meshPtr = Meshes[StaticObjects[meshInfo->staticNumber].meshNumber];
		x = meshInfo->x;
		y = meshInfo->y;
		z = meshInfo->z;
		shade = meshInfo->shade;
	}*/

	return 1;
}