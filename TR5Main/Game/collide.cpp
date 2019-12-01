#include "collide.h"
#include "draw.h"
#include "Lara.h"

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

__int32 __cdecl GetCollidedObjects(ITEM_INFO* collidingItem, __int32 radius, __int32 onlyVisible, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, __int32 ignoreLara)
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

					if (item == collidingItem || !ignoreLara && item == LaraItem)
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
						&& (!onlyVisible || item->status != ITEM_INVISIBLE)
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

__int32 __cdecl TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll)
{
	__int16* framePtr = GetBestFrame(lara);

	if (item->pos.yPos + GlobalCollisionBounds.Y2 <= lara->pos.yPos + framePtr[3])
		return false;

	if (item->pos.yPos + GlobalCollisionBounds.Y1 >= framePtr[4])
		return false;

	__int32 s = SIN(item->pos.yRot);
	__int32 c = COS(item->pos.yRot);

	__int32 dx = lara->pos.xPos - item->pos.xPos;
	__int32 dz = lara->pos.zPos - item->pos.zPos;

	__int32 x = (c * dx - s * dz) >> W2V_SHIFT;
	__int32 z = (c * dz + s * dx) >> W2V_SHIFT;

	if (x < GlobalCollisionBounds.X1 - coll->radius ||
		x > GlobalCollisionBounds.X2 + coll->radius ||
		z < GlobalCollisionBounds.Z1 - coll->radius ||
		z > GlobalCollisionBounds.Z2 + coll->radius)
		return false;

	return true;
}

void __cdecl TrapCollision(__int16 itemNumber, ITEM_INFO* l, COLL_INFO* c)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (item->status == ITEM_ACTIVE)
	{
		if (!TestBoundsCollide(item, l, c->radius))
			return;

		TestCollision(item, LaraItem);    

		/*if (item->object_number == FAN && item->currentAnimState == 1)	// Is the fan moving slow ?
			ObjectCollision(item_num, laraitem, coll);*/
	}
	else if (item->status != ITEM_INVISIBLE)
		ObjectCollision(itemNumber, l, c);
}

void __cdecl TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll)//2A940(<), 2AB68(<) (F)
{
	for (__int32 i = 0; i < 2; i++)
	{
		GAME_VECTOR s;
		s.x = (i << 8) - 0x80;
		s.y = -256;
		s.z = 0;

		GetLaraJointPosition((PHD_VECTOR*)&s, LJ_HEAD);
		s.roomNumber = LaraItem->roomNumber;

		GAME_VECTOR d;
		d.x = 0;
		d.x = (s.x + ((SIN(LaraItem->pos.yRot) << 1) + SIN(LaraItem->pos.yRot))) >> 4;
		d.y = s.y;
		d.z = (s.z + ((COS(LaraItem->pos.yRot) << 1) + COS(LaraItem->pos.yRot))) >> 4;
		
		LOS(&s, &d);
		
		PHD_VECTOR v;
		MESH_INFO* mesh;

		// CHECK
		/*if (ObjectOnLOS2(&s, &d, &v, &mesh) != 999)
		{
			coll->hitStatic = true;
		}*/
	}
}

void Inject_Collide()
{
	INJECT(0x00411DB0, CollideStaticObjects);
	INJECT(0x00413CF0, GetCollidedObjects);
}