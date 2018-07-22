#include "collide.h"
#include "..\Global\global.h"
#include <stdio.h>

__int32 __cdecl CollideStaticObjects(COLL_INFO* coll, __int32 x, __int32 y, __int32 z, __int16 roomNumber, __int32 hite)
{
	__int16 roomsArray[22];
	memset(&roomsArray[0], 0, 44);

	coll->hitStatic = false;

	__int32 inXmin = x - coll->radius;
	__int32 inXmax = x + coll->radius;
	__int32 inZmin = z - coll->radius;
	__int32 inZmax = z + coll->radius;
	__int32 inYmin = y - hite;

	roomsArray[0] = roomNumber;

	__int16* doors = Rooms[roomNumber].door;
	__int32 numRooms = 1;

	// Check for connected rooms
	if (doors != NULL)
	{
		__int16 numDoors = *doors;
		__int16* currentDoor = doors + 1;

		for (__int32 i = 0; i < numDoors; i++)
		{
			__int32 j = 0;

			for (j = 0; j < numRooms; j++)
			{
				if (roomsArray[i] == *currentDoor)
					break;
			}

			if (j == numRooms)
				roomsArray[numRooms++] = *currentDoor;

			currentDoor += 16;
		}
	}

	if (numRooms <= 0)
		return false;

	__int32 xMin = 0;
	__int32 xMax = 0;
	__int32 zMin = 0;
	__int32 zMax = 0;

	for (__int32 i = 0; i < numRooms; i++)
	{
		ROOM_INFO* room = &Rooms[roomsArray[i]];
		MESH_INFO* mesh = room->mesh;

		for (__int32 j = room->numMeshes; j > 0; j--, mesh++)
		{
			STATIC_INFO* sInfo = &StaticObjects[mesh->staticNumber];
			if ((sInfo->flags & 1)) // No collision
				continue;

			__int32 yMin = mesh->y + sInfo->xMinc;
			__int32 yMax = mesh->y + sInfo->yMaxc;
			__int16 yRot = mesh->yRot;

			if (yRot == -32768)
			{
				xMin = mesh->x - sInfo->xMaxc;
				xMax = mesh->x - sInfo->xMinc;
				zMin = mesh->z - sInfo->zMaxc;
				zMax = mesh->z - sInfo->zMinc;
			}
			else if (yRot == -16384)
			{
				xMin = mesh->x - sInfo->zMaxc;
				xMax = mesh->x - sInfo->zMinc;
				zMin = mesh->z + sInfo->xMinc;
				zMax = mesh->z + sInfo->xMaxc;
			}
			else if (yRot == 16384)
			{

				xMin = mesh->x + sInfo->zMinc;
				xMax = mesh->x + sInfo->zMaxc;
				zMin = mesh->z - sInfo->xMaxc;
				zMax = mesh->z - sInfo->xMinc;
			}
			else
			{
				xMin = mesh->x + sInfo->xMinc;
				xMax = mesh->x + sInfo->xMaxc;
				zMin = mesh->z + sInfo->zMinc;
				zMax = mesh->z + sInfo->zMaxc;
			}

			if (inXmax <= xMin || inXmin >= xMax ||
				y <= yMin || inYmin >= yMax ||
				inZmax <= zMin || inZmin >= zMax)		 
				continue;

			coll->hitStatic = true;
			return true;
		}
	}

	return false;
}

void __cdecl j_GetCollisionInfo(COLL_INFO* coll, __int32 x, __int32 y, __int32 z, __int16 roomNumber, __int32 objHeight)
{
	printf("Radius: %d\n", coll->radius);
	GetCollisionInfo(coll, x, y, z, roomNumber, objHeight);
	printf("Floor: %d\n", coll->frontFloor);
}

void Inject_Collide()
{
	INJECT(0x00411DB0, CollideStaticObjects);
	//INJECT(0x00401A32, j_GetCollisionInfo);
}