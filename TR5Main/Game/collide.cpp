#include "collide.h"
#include "draw.h"
#include "lara.h"

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

__int32 GetCollidedObjects(ITEM_INFO* collidingItem, __int32 radius, __int32 flag1, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, __int32 flag2)
{
	// Collect all the rooms where to check
	__int16 roomsArray[22];
	__int32 numRooms = 1;
	roomsArray[0] = collidingItem->roomNumber;

	__int16* doors = Rooms[roomsArray[0]].door;
	if (doors)
	{
		__int32 numDoors = *doors;
		doors++;

		for (__int32 i = 0; i < numDoors; i++)
		{
			__int16 adjoiningRoom = *doors;
			bool found = false;
			for (__int32 j = 0; j < numRooms; j++)
			{
				if (roomsArray[j] == adjoiningRoom)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				roomsArray[numRooms] = adjoiningRoom;
				numRooms++;
			}

			doors += 16;
		}
	}

	__int32 numItems = 0;
	__int32 numMeshes = 0;

	if (collidedMeshes)
	{
		for (__int32 i = 0; i < numRooms; i++)
		{
			ROOM_INFO* room = &Rooms[roomsArray[i]];

			for (__int32 j = 0; j < room->numMeshes; j++)
			{
				MESH_INFO* mesh = &room->mesh[j];
				STATIC_INFO* staticMesh = &StaticObjects[mesh->staticNumber];

				if (mesh->Flags & 1)
				{
					if (collidingItem->pos.yPos + radius + 128 >= mesh->y + staticMesh->yMinc)
					{
						if (collidingItem->pos.yPos <= mesh->y + staticMesh->yMaxc)
						{
							__int32 s = SIN(mesh->yRot);
							__int32 c = COS(mesh->yRot);

							__int32 rx = ((collidingItem->pos.xPos - mesh->x) * c - s * (collidingItem->pos.zPos - mesh->z)) >> W2V_SHIFT;
							__int32 rz = ((collidingItem->pos.zPos - mesh->z) * c + s * (collidingItem->pos.xPos - mesh->x)) >> W2V_SHIFT;

							if (radius + rx + 128 >= staticMesh->xMinc && rx - radius - 128 <= staticMesh->xMaxc)
							{
								if (radius + rz + 128 >= staticMesh->zMinc && rz - radius - 128 <= staticMesh->zMaxc)
								{
									collidedMeshes[numMeshes++] = mesh;
									if (!radius)
									{
										collidedItems[0] = NULL;
										return true;
									}
								}
							}
						}
					}
				}
			}
		}

		collidedMeshes[numMeshes] = NULL;
	}

	if (collidedItems)
	{
		for (__int32 i = 0; i < numRooms; i++)
		{
			ROOM_INFO* room = &Rooms[roomsArray[i]];

			__int32 itemNumber = room->itemNumber;
			if (itemNumber != NO_ITEM)
			{
				do
				{
					ITEM_INFO* item = &Items[itemNumber];

					if (item == collidingItem || !flag2 && item == LaraItem)
					{
						itemNumber = item->nextItem;
						continue;
					}

					if (item->flags & 0x8000)
					{
						itemNumber = item->nextItem;
						continue;
					}

					if (!Objects[item->objectNumber].collision && item->objectNumber != ID_LARA)
					{
						itemNumber = item->nextItem;
						continue;
					}

					__int32 dx = collidingItem->pos.xPos - item->pos.xPos;
					__int32 dy = collidingItem->pos.yPos - item->pos.yPos;
					__int32 dz = collidingItem->pos.zPos - item->pos.zPos;

					__int16* framePtr = GetBestFrame(item);

					if (Objects[item->objectNumber].drawRoutine
						&& item->meshBits
						&& (!flag1 || item->status != ITEM_INVISIBLE)
						&& dx >= -2048
						&& dx <= 2048
						&& dy >= -2048
						&& dy <= 2048
						&& dz >= -2048
						&& dz <= 2048
						&& collidingItem->pos.yPos + radius + 128 >= item->pos.yPos + framePtr[2]
						&& collidingItem->pos.yPos - radius - 128 <= item->pos.yPos + framePtr[3])
					{
						__int32 s = SIN(item->pos.yRot);
						__int32 c = COS(item->pos.yRot);

						__int32 rx = (dx * c - s * dz) >> W2V_SHIFT;
						__int32 rz = (dz * c + s * dx) >> W2V_SHIFT;

						if (item->objectNumber == ID_TURN_SWITCH)
						{
							// TODO: implement
							/*v59 = -256;
							  v57 = -256;
							  v60 = 256;
							  v58 = 256;
							  bounds = &v57;*/
						}

						if (radius + rx + 128 >= framePtr[0] && rx - radius - 128 <= framePtr[1])
						{
							if (radius + rz + 128 >= framePtr[4] && rz - radius - 128 <= framePtr[5])
							{
								collidedItems[numItems++] = item;
								if (!radius)
									return true;
							}
						}
					}

					itemNumber = item->nextItem;

				} while (itemNumber != NO_ITEM);
			}
		}
	}

	collidedItems[numItems] = NULL;

	return (numItems | numMeshes);
}

void Inject_Collide()
{
	INJECT(0x00411DB0, CollideStaticObjects);
	INJECT(0x00413CF0, GetCollidedObjects);
}