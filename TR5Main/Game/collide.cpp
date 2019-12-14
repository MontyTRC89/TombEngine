#include "collide.h"
#include "draw.h"
#include "Lara.h"

#include "..\Global\global.h"

#include <stdio.h>
#include "items.h"
#include "effects.h"
#include "sphere.h"

char LM[] =
{
	LJ_HIPS,
	LJ_LTHIGH,
	LJ_LSHIN,
	LJ_LFOOT,
	LJ_RTHIGH,
	LJ_RSHIN,
	LJ_RFOOT,
	LJ_TORSO,
	LJ_RINARM,
	LJ_ROUTARM,
	LJ_RHAND,
	LJ_LINARM,
	LJ_LOUTARM,
	LJ_LHAND,
	LJ_HEAD,
};

int XFront, ZFront;

int CollideStaticObjects(COLL_INFO* coll, int x, int y, int z, short roomNumber, int hite)
{
	short roomsArray[22];
	memset(&roomsArray[0], 0, 44);

	coll->hitStatic = false;

	int inXmin = x - coll->radius;
	int inXmax = x + coll->radius;
	int inZmin = z - coll->radius;
	int inZmax = z + coll->radius;
	int inYmin = y - hite;

	roomsArray[0] = roomNumber;

	short* doors = Rooms[roomNumber].door;
	int numRooms = 1;

	// Check for connected rooms
	if (doors != NULL)
	{
		short numDoors = *doors;
		short* currentDoor = doors + 1;

		for (int i = 0; i < numDoors; i++)
		{
			int j = 0;

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

	int xMin = 0;
	int xMax = 0;
	int zMin = 0;
	int zMax = 0;

	for (int i = 0; i < numRooms; i++)
	{
		ROOM_INFO* room = &Rooms[roomsArray[i]];
		MESH_INFO* mesh = room->mesh;

		for (int j = room->numMeshes; j > 0; j--, mesh++)
		{
			STATIC_INFO* sInfo = &StaticObjects[mesh->staticNumber];
			if ((sInfo->flags & 1)) // No collision
				continue;

			int yMin = mesh->y + sInfo->xMinc;
			int yMax = mesh->y + sInfo->yMaxc;
			short yRot = mesh->yRot;

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

int GetCollidedObjects(ITEM_INFO* collidingItem, int radius, int onlyVisible, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int ignoreLara)
{
	// Collect all the rooms where to check
	short roomsArray[22];
	int numRooms = 1;
	roomsArray[0] = collidingItem->roomNumber;

	short* doors = Rooms[roomsArray[0]].door;
	if (doors)
	{
		int numDoors = *doors;
		doors++;

		for (int i = 0; i < numDoors; i++)
		{
			short adjoiningRoom = *doors;
			bool found = false;
			for (int j = 0; j < numRooms; j++)
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

	int numItems = 0;
	int numMeshes = 0;

	if (collidedMeshes)
	{
		for (int i = 0; i < numRooms; i++)
		{
			ROOM_INFO* room = &Rooms[roomsArray[i]];

			for (int j = 0; j < room->numMeshes; j++)
			{
				MESH_INFO* mesh = &room->mesh[j];
				STATIC_INFO* staticMesh = &StaticObjects[mesh->staticNumber];

				if (mesh->Flags & 1)
				{
					if (collidingItem->pos.yPos + radius + 128 >= mesh->y + staticMesh->yMinc)
					{
						if (collidingItem->pos.yPos <= mesh->y + staticMesh->yMaxc)
						{
							int s = SIN(mesh->yRot);
							int c = COS(mesh->yRot);

							int rx = ((collidingItem->pos.xPos - mesh->x) * c - s * (collidingItem->pos.zPos - mesh->z)) >> W2V_SHIFT;
							int rz = ((collidingItem->pos.zPos - mesh->z) * c + s * (collidingItem->pos.xPos - mesh->x)) >> W2V_SHIFT;

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
		for (int i = 0; i < numRooms; i++)
		{
			ROOM_INFO* room = &Rooms[roomsArray[i]];

			int itemNumber = room->itemNumber;
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

					int dx = collidingItem->pos.xPos - item->pos.xPos;
					int dy = collidingItem->pos.yPos - item->pos.yPos;
					int dz = collidingItem->pos.zPos - item->pos.zPos;

					short* framePtr = GetBestFrame(item);

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
						int s = SIN(item->pos.yRot);
						int c = COS(item->pos.yRot);

						int rx = (dx * c - s * dz) >> W2V_SHIFT;
						int rz = (dz * c + s * dx) >> W2V_SHIFT;

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

int TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll)
{
	short* framePtr = GetBestFrame(lara);

	if (item->pos.yPos + GlobalCollisionBounds.Y2 <= lara->pos.yPos + framePtr[3])
		return false;

	if (item->pos.yPos + GlobalCollisionBounds.Y1 >= framePtr[4])
		return false;

	int s = SIN(item->pos.yRot);
	int c = COS(item->pos.yRot);

	int dx = lara->pos.xPos - item->pos.xPos;
	int dz = lara->pos.zPos - item->pos.zPos;

	int x = (c * dx - s * dz) >> W2V_SHIFT;
	int z = (c * dz + s * dx) >> W2V_SHIFT;

	if (x < GlobalCollisionBounds.X1 - coll->radius ||
		x > GlobalCollisionBounds.X2 + coll->radius ||
		z < GlobalCollisionBounds.Z1 - coll->radius ||
		z > GlobalCollisionBounds.Z2 + coll->radius)
		return false;

	return true;
}

void TrapCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c)
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

void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll)//2A940(<), 2AB68(<) (F)
{
	for (int i = 0; i < 2; i++)
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

void ShiftItem(ITEM_INFO* item, COLL_INFO* coll)
{
	item->pos.xPos += coll->shift.x;
	item->pos.yPos += coll->shift.y;
	item->pos.zPos += coll->shift.z;
	coll->shift.z = 0;
	coll->shift.y = 0;
	coll->shift.x = 0;
}

void UpdateLaraRoom(ITEM_INFO* item, int height)
{
	int x = item->pos.xPos;
	int y = height + item->pos.yPos;
	int z = item->pos.zPos;
	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	item->floor = GetFloorHeight(floor, x, y, z);
	if (item->roomNumber != roomNumber)
		ItemNewRoom(Lara.itemNumber, roomNumber);
}

int GetTiltType(FLOOR_INFO* floor, int x, int y, int z)
{
	while (floor->pitRoom != NO_ROOM)
	{
		if (CheckNoColFloorTriangle(floor, x, z) == 1)
			break;
		ROOM_INFO* r = &Rooms[floor->pitRoom];
		floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
	}

	if (y + 512 < floor->floor * 256)
		return 0;

	if (!floor->index)
		return 0;

	short* data = &FloorData[floor->index];
	short func = *data & DATA_TYPE;
	
	if (func == TILT_TYPE)
	{
		return *(data + 1);
	}
	
	if (func != SPLIT1 && func != SPLIT2 
		&& func != NOCOLF1T && func != NOCOLF2T 
		&& func != NOCOLF1B && func != NOCOLF2B)
	{
		return 0;
	}

	int tilts = *(data + 1);
	
	int t0 = tilts & 0xF;
	int t1 = (tilts >> 4) & 0xF;
	int t2 = (tilts >> 8) & 0xF;
	int t3 = (tilts >> 12) & 0xF;
	
	int dx = x & 1023;
	int dz = z & 1023;

	short xOff = 0;
	short yOff = 0;

	if (func == SPLIT1 || func == NOCOLF1T || func == NOCOLF1B)
	{
		if (dx > 1024 - dz)
		{
			xOff = t3 - t0;
			yOff = t3 - t2;
		}
		else
		{
			xOff = t2 - t1;
			yOff = t0 - t1;
		}
	}
	else if (dx > dz)
	{
		xOff = t3 - t0;
		yOff = t0 - t1;
	}
	else
	{
		xOff = t2 - t1;
		yOff = t3 - t2;
	}

	return ((xOff << 8) | (yOff & 0xFF));
}

int FindGridShift(int x, int z)
{
	if (x >> 10 == z >> 10)
		return 0;

	if (z >> 10 <= x >> 10)
		return (-1 - (x & 0x3FF));
	else
		return (1025 - (x & 0x3FF));
}

int TestBoundsCollideStatic(short* bounds, PHD_3DPOS* pos, int radius)
{
	if (!(bounds[5] | bounds[4] | bounds[0] | bounds[1] | bounds[2] | bounds[3]))
		return 0;

	short* frame = GetBestFrame(LaraItem);
	
	if (pos->yPos + bounds[3] <= LaraItem->pos.yPos + frame[2])
		return 0;

	if (pos->yPos + bounds[2] >= LaraItem->pos.yPos + frame[3])
		return 0;

	int c = COS(pos->yRot);  
	int s = SIN(pos->yRot);

	int dx = LaraItem->pos.xPos - pos->xPos;
	int dz = LaraItem->pos.zPos - pos->zPos;
	
	dx = (c * dx - s * dz) >> W2V_SHIFT;
	dz = (c * dz + s * dx) >> W2V_SHIFT;
	
	if (dx >= bounds[0] - radius && dx <= radius + bounds[1] 
		&& dz >= bounds[4] - radius && dz <= radius + bounds[5])
		return 1;
	else
		return 0;
}

int ItemPushLaraStatic(ITEM_INFO* item, short* bounds, PHD_3DPOS* pos, COLL_INFO* coll)
{
	int c = COS(pos->yRot);
	int s = SIN(pos->yRot);

	int dx = LaraItem->pos.xPos - pos->xPos;
	int dz = LaraItem->pos.zPos - pos->zPos;

	int rx = (c * dx - s * dz) >> W2V_SHIFT;
	int rz = (c * dz + s * dx) >> W2V_SHIFT;

	int minX = bounds[0] - coll->radius;
	int maxX = bounds[1] + coll->radius;
	int minZ = bounds[4] - coll->radius;
	int maxZ = bounds[5] + coll->radius;
	
	if (abs(dx) > 4608
		|| abs(dz) > 4608
		|| rx <= minX
		|| rx >= maxX
		|| rz <= minZ
		|| rz >= maxZ)
		return 0;

	int left = rx - minX;
	int top = maxZ - rz;
	int bottom = rz - minZ;
	int right = maxX - rx;

	if (left <= right && left <= top && left <= bottom)   
		rx -= left;                            
	else if (right <= left && right <= top && right <= bottom)
		rx += right;
	else if (top <= left && top <= right && top <= bottom)
		rz += top;
	else
		rz -= bottom;

	item->pos.xPos = pos->xPos + ((c * rx + s * rz) >> W2V_SHIFT);
	item->pos.zPos = pos->zPos + ((c * rz - s * rx) >> W2V_SHIFT);
	
	coll->badPos = 32512;
	coll->badNeg = -384;
	coll->badCeiling = 0;

	int oldFacing = coll->facing;
	coll->facing = ATAN(item->pos.zPos - coll->old.z, item->pos.xPos - coll->old.x);

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 762);
	
	coll->facing = oldFacing;

	if (coll->collType == CT_NONE)
	{
		coll->old.x = item->pos.xPos;
		coll->old.y = item->pos.yPos;
		coll->old.z = item->pos.zPos;

		UpdateLaraRoom(item, -10);
	}
	else
	{
		item->pos.xPos = coll->old.x;
		item->pos.zPos = coll->old.z;
	}

	if (item == LaraItem && Lara.isMoving && Lara.moveCount > 15)
	{
		Lara.isMoving = false;
		Lara.gunStatus = LG_NO_ARMS;
	}

	return 1;
}

int ItemPushLara(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, int spazon, char bigpush)
{
	int c = COS(item->pos.yRot);
	int s = SIN(item->pos.yRot);

	int dx = LaraItem->pos.xPos - item->pos.xPos;
	int dz = LaraItem->pos.zPos - item->pos.zPos;

	int rx = (c * dx - s * dz) >> W2V_SHIFT;
	int rz = (c * dz + s * dx) >> W2V_SHIFT;

	short* bounds;

	if (bigpush & 2)
		bounds = (short*)&GlobalCollisionBounds;
	else
		bounds = GetBestFrame(item);

	int minX = bounds[0];
	int maxX = bounds[1];
	int minZ = bounds[4];
	int maxZ = bounds[5];

	if (bigpush & 1)
	{
		minX -= coll->radius;
		maxX += coll->radius;
		minZ -= coll->radius;
		maxZ += coll->radius;
	}

	if (abs(dx) > 4608
		|| abs(dz) > 4608
		|| rx <= minX
		|| rx >= maxX
		|| rz <= minZ
		|| rz >= maxZ)
		return 0;

	int left = rx - minX;
	int top = maxZ - rz;
	int bottom = rz - minZ;
	int right = maxX - rx;

	if (left <= right && left <= top && left <= bottom)
		rx -= left;
	else if (right <= left && right <= top && right <= bottom)
		rx += right;
	else if (top <= left && top <= right && top <= bottom)
		rz += top;
	else
		rz -= bottom;

	l->pos.xPos = item->pos.xPos + ((c * rx + s * rz) >> W2V_SHIFT);
	l->pos.zPos = item->pos.zPos + ((c * rz - s * rx) >> W2V_SHIFT);

	if (spazon && bounds[3] - bounds[2] > 256)
	{
		rx = (bounds[0] + bounds[1]) / 2;	 
		rz = (bounds[4] + bounds[5]) / 2;

		dx -= (c * rx + s * rz) >> W2V_SHIFT;  
		dz -= (c * rz - s * rx) >> W2V_SHIFT;

		Lara.hitDirection = (l->pos.yRot - ATAN(dz, dz) - ANGLE(135)) >> W2V_SHIFT;

		if (!Lara.hitFrame)
			SoundEffect(SFX_LARA_INJURY_RND, &l->pos, 0);

		Lara.hitFrame++;
		if (Lara.hitFrame > 34) 
			Lara.hitFrame = 34;
	}

	coll->badPos = 32512;
	coll->badNeg = -384;
	coll->badCeiling = 0;

	int facing = coll->facing;
	coll->facing = ATAN(l->pos.zPos - coll->old.z, l->pos.xPos - coll->old.x);
	GetCollisionInfo(coll, l->pos.xPos, l->pos.yPos, l->pos.zPos, l->roomNumber, 762);
	coll->facing = facing;

	if (coll->collType == CT_NONE)
	{
		coll->old.x = l->pos.xPos;
		coll->old.y = l->pos.yPos;
		coll->old.z = l->pos.zPos;

		UpdateLaraRoom(l, -10);
	}
	else
	{
		l->pos.xPos = coll->old.x;
		l->pos.zPos = coll->old.z;
	}

	if (Lara.isMoving && Lara.moveCount > 15)
	{
		Lara.isMoving = false;
		Lara.gunStatus = LG_NO_ARMS;
	}

	return 1;
}

void AIPickupCollision(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	if (item->objectNumber == ID_SHOOT_SWITCH1 && !(item->meshBits & 1))
		item->status = ITEM_INVISIBLE;
}

void ObjectCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (TestBoundsCollide(item, l, c->radius))
	{
		if (TestCollision(item, l))
		{
			if (c->enableBaddiePush)
				ItemPushLara(item, l, c, 0, 1);
		}
	}
}

void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l)
{
	l->pos.xRot = item->pos.xRot;
	l->pos.yRot = item->pos.yRot;
	l->pos.zRot = item->pos.zRot;
	
	phd_PushUnitMatrix();
	phd_RotYXZ(item->pos.yRot, item->pos.xRot, item->pos.zRot);
	
	int x = item->pos.xPos + ((*(MatrixPtr + M00) * vec->x + *(MatrixPtr + M01) * vec->y + *(MatrixPtr + M02) * vec->z) >> W2V_SHIFT);
	int y = item->pos.yPos + ((*(MatrixPtr + M10) * vec->x + *(MatrixPtr + M11) * vec->y + *(MatrixPtr + M12) * vec->z) >> W2V_SHIFT);
	int z = item->pos.zPos + ((*(MatrixPtr + M20) * vec->x + *(MatrixPtr + M21) * vec->y + *(MatrixPtr + M22) * vec->z) >> W2V_SHIFT);

	MatrixPtr -= 12;
	DxMatrixPtr = (DxMatrixPtr - 48);

	l->pos.xPos = x;
	l->pos.yPos = y;
	l->pos.zPos = z;
}

void TriggerLaraBlood() 
{
	int node = 1;

	for (int i = 0; i < 15; i++)
	{
		if (node & LaraItem->touchBits)
		{
			PHD_VECTOR vec;

			vec.x = (GetRandomControl() & 0x1F) - 16;
			vec.y = (GetRandomControl() & 0x1F) - 16;
			vec.z = (GetRandomControl() & 0x1F) - 16;

			GetLaraJointPosition(&vec, LM[i]);
			DoBloodSplat(vec.x, vec.y, vec.z, (GetRandomControl() & 7) + 8, 2 * GetRandomControl(), LaraItem->roomNumber);
		}

		node <<= 1;
	}
}

int TestLaraPosition(__int16* bounds, ITEM_INFO* item, ITEM_INFO* l)
{
	int xRotRel = l->pos.xRot - item->pos.xRot;
	int yRotRel = l->pos.yRot - item->pos.yRot;
	int zRotRel = l->pos.zRot - item->pos.zRot;

	if (xRotRel < bounds[6])
		return 0;
	if (xRotRel > bounds[7])
		return 0;
	if (yRotRel < bounds[8])
		return 0;
	if (yRotRel > bounds[9])
		return 0;
	if (zRotRel < bounds[10])
		return 0;
	if (zRotRel > bounds[11])
		return 0;

	phd_PushUnitMatrix();
	phd_RotYXZ(item->pos.yRot, item->pos.xRot, item->pos.zRot);
	
	int x = l->pos.xPos - item->pos.xPos;
	int y = l->pos.yPos - item->pos.yPos;
	int z = l->pos.zPos - item->pos.zPos;

	int rx = (x * *MatrixPtr + y * MatrixPtr[4] + z * MatrixPtr[8]) >> W2V_SHIFT;
	int ry = (x * MatrixPtr[1] + y * MatrixPtr[5] + z * MatrixPtr[9]) >> W2V_SHIFT;
	int rz = (x * MatrixPtr[2] + y * MatrixPtr[6] + z * MatrixPtr[10]) >> W2V_SHIFT;
	
	MatrixPtr -= 12;
	DxMatrixPtr -= 48;
	
	if (rx < bounds[0] || rx > bounds[1] || ry < bounds[2] || ry > bounds[3] || rz < bounds[4] || rz > bounds[5])
		return 0;

	return 1;
}

int Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angAdd)
{
	int x = dest->xPos - src->xPos;
	int y = dest->yPos - src->yPos;
	int z = (dest->zPos - src->zPos);

	int distance = SQRT_ASM(SQUARE(x) + SQUARE(y) + SQUARE(z));
	if (velocity < distance)
	{
		src->xPos += x * velocity / distance;
		src->yPos += y * velocity / distance;
		src->zPos += z * velocity / distance;
	}
	else
	{
		src->xPos = dest->xPos;
		src->yPos = dest->yPos;
		src->zPos = dest->zPos;
	}

	if (!Lara.isMoving)
	{
		if (Lara.waterStatus != LW_UNDERWATER)
		{
			int angle = mGetAngle(dest->xPos, dest->zPos, src->xPos, src->zPos);
			int direction = (((angle + ANGLE(45)) >> W2V_SHIFT) - ((dest->yRot + ANGLE(45)) >> W2V_SHIFT)) & 3;

			switch (direction)
			{
			case 0:
				LaraItem->animNumber = ANIMATION_LARA_WALK_LEFT;
				LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
				LaraItem->goalAnimState = STATE_LARA_WALK_LEFT;
				LaraItem->currentAnimState = STATE_LARA_WALK_LEFT;
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			case 1:
				LaraItem->animNumber = ANIMATION_LARA_WALK_FORWARD;
				LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
				LaraItem->goalAnimState = STATE_LARA_WALK_FORWARD;
				LaraItem->currentAnimState = STATE_LARA_WALK_FORWARD;
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			case 2:
				LaraItem->animNumber = ANIMATION_LARA_WALK_RIGHT;
				LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
				LaraItem->goalAnimState = STATE_LARA_WALK_RIGHT;
				LaraItem->currentAnimState = STATE_LARA_WALK_RIGHT;
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			case 3:
			default:
				LaraItem->animNumber = ANIMATION_LARA_WALK_BACK;
				LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
				LaraItem->goalAnimState = STATE_LARA_WALK_BACK;
				LaraItem->currentAnimState = STATE_LARA_WALK_BACK;
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			}
		}

		Lara.isMoving = true;
		Lara.moveCount = 0;
	}

	if ((dest->xRot - src->xRot) <= angAdd)
	{
		if ((dest->xRot - src->xRot) >= -angAdd)
			src->xRot = dest->xRot;
		else
			src->xRot = src->xRot - angAdd;
	}
	else
	{
		src->xRot = angAdd + src->xRot;
	}

	if ((dest->yRot - src->yRot) <= angAdd)
	{
		if ((dest->yRot - src->yRot) >= -angAdd)
			src->yRot = dest->yRot;
		else
			src->yRot = src->yRot - angAdd;
	}
	else
	{
		src->yRot = angAdd + src->yRot;
	}

	if ((dest->zRot - src->zRot) <= angAdd)
	{
		if ((dest->zRot - src->zRot) >= -angAdd)
			src->zRot = dest->zRot;
		else
			src->zRot = src->zRot - angAdd;
	}
	else
	{
		src->zRot = angAdd + src->zRot;
	}

	return (src->xPos == dest->xPos
		&& src->yPos == dest->yPos
		&& src->zPos == dest->zPos
		&& src->xRot == dest->xRot
		&& src->yRot == dest->yRot
		&& src->zRot == dest->zRot);
}

int MoveLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l)
{
	PHD_3DPOS dest;

	dest.xRot = item->pos.xRot;
	dest.yRot = item->pos.yRot;
	dest.zRot = item->pos.zRot;
	
	phd_PushUnitMatrix();
	phd_RotYXZ(item->pos.yRot, item->pos.xRot, item->pos.zRot);

	dest.xPos = item->pos.xPos + ((MatrixPtr[M00] * vec->x + MatrixPtr[M01] * vec->y + MatrixPtr[M02] * vec->z) >> W2V_SHIFT);
	dest.yPos = item->pos.yPos + ((MatrixPtr[M10] * vec->x + MatrixPtr[M11] * vec->y + MatrixPtr[M12] * vec->z) >> W2V_SHIFT);
	dest.zPos = item->pos.zPos + ((MatrixPtr[M20] * vec->x + MatrixPtr[M21] * vec->y + MatrixPtr[M22] * vec->z) >> W2V_SHIFT);

	MatrixPtr -= 12;
	DxMatrixPtr = (DxMatrixPtr - 48);

	if (item->objectNumber != ID_FLARE_ITEM && item->objectNumber != ID_BURNING_TORCH_ITEM)
		return Move3DPosTo3DPos(&l->pos, &dest, 12, 364);

	short roomNumber = l->roomNumber;
	FLOOR_INFO* floor = GetFloor(dest.xPos, dest.yPos, dest.zPos, &roomNumber);
	int height = GetFloorHeight(floor, dest.xPos, dest.yPos, dest.zPos);
	
	if (abs(height - l->pos.yPos) <= 512)
	{
		if (SQRT_ASM(SQUARE(dest.xPos - l->pos.xPos) + SQUARE(dest.yPos - l->pos.yPos) + SQUARE(dest.zPos - l->pos.zPos)) < 128)
			return 1;

		return Move3DPosTo3DPos(&l->pos, &dest, 12, 364);
	}

	if (Lara.isMoving)
	{
		Lara.isMoving = false;
		Lara.gunStatus = LG_NO_ARMS;
	}

	return 0;
}

int TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int radius)
{
	short* bounds = GetBestFrame(item);
	short* laraBounds = GetBestFrame(l);

	if (item->pos.yPos + bounds[3] > l->pos.yPos + laraBounds[2])
	{
		if (item->pos.yPos + bounds[2] < l->pos.yPos + laraBounds[3])
		{
			int c = COS(item->pos.yRot);
			int s = SIN(item->pos.yRot);

			int dx = (c * (l->pos.xPos - item->pos.xPos) - s * (l->pos.zPos - item->pos.zPos)) >> W2V_SHIFT;
			int dz = (c * (l->pos.zPos - item->pos.zPos) + s * (l->pos.xPos - item->pos.xPos)) >> W2V_SHIFT;

			if (dx >= bounds[0] - radius
				&& dx <= radius + bounds[1]
				&& dz >= bounds[4] - radius
				&& dz <= radius + bounds[5])
			{
				return 1;
			}
		}
	}

	return 0;
}

void CreatureCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->objectNumber != ID_HITMAN || item->currentAnimState != STATE_LARA_INSERT_PUZZLE)
	{
		if (TestBoundsCollide(item, l, coll->radius))
		{
			if (TestCollision(item, l))
			{
				if (coll->enableBaddiePush || Lara.waterStatus == LW_UNDERWATER || Lara.waterStatus == LW_SURFACE)
				{
					ItemPushLara(item, l, coll, coll->enableSpaz, 0);
				}
				else if (coll->enableSpaz)
				{
					int x = l->pos.xPos - item->pos.xPos;
					int z = l->pos.zPos - item->pos.zPos;

					int c = COS(item->pos.yRot);
					int s = SIN(item->pos.yRot);

					short* frame = GetBestFrame(item);
	
					int rx = (frame[0] + frame[1]) / 2;
					int rz = (frame[4] + frame[5]) / 2;
					
					if (frame[3] - frame[2] > 256)
					{
						int angle = (l->pos.yRot
									- ATAN(z - ((c * rx - s * rz) >> W2V_SHIFT), x - ((c * rx + s * rz) >> W2V_SHIFT))
									- ANGLE(135)) >> W2V_SHIFT;
						Lara.hitDirection = (short)angle;

						Lara.hitFrame++;
						Lara.hitFrame++; 
						if (Lara.hitFrame > 30)	 
							Lara.hitFrame = 30;
					}
				}
			}
		}
	}
}

void _GetCollisionInfo(COLL_INFO* coll, int xPos, int yPos, int zPos, int roomNumber, int objectHeight)
{
	int resetRoom;
	if (objectHeight >= 0)
	{
		resetRoom = 0;
	}
	else
	{
		objectHeight = -objectHeight;
		resetRoom = 1;
	}

	coll->collType = 0;
	coll->shift.x = 0;
	coll->shift.y = 0;
	coll->shift.z = 0;
	coll->quadrant = (unsigned short) (coll->facing + ANGLE(45)) / ANGLE(90);

	int x = xPos;
	int y = yPos - objectHeight;
	int yTop = y - 160;
	int z = zPos;

	short tRoomNumber = roomNumber;
	FLOOR_INFO* floor = GetFloor(xPos, yTop, zPos, &tRoomNumber);
	
	int height, mheight;
	mheight = height = GetFloorHeight(floor, xPos, yTop, zPos);
	if (height != NO_HEIGHT)
		height -= yPos;

	int ceiling = GetCeiling(floor, xPos, yTop - LaraItem->fallspeed, zPos);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->midCeiling = ceiling;
	coll->midFloor = height;
	coll->midType = HeightType;

	coll->trigger = TriggerIndex;

	coll->tiltX = GetTiltType(floor, xPos, LaraItem->pos.yPos, zPos);
	coll->tiltZ = coll->tiltX >> 8;
	
	int xright, xleft, zright, zleft;

	switch (coll->quadrant)
	{
	case 0:
		XFront = (SIN(coll->facing) * coll->radius) >> (W2V_SHIFT);
		ZFront = coll->radius;
		xleft = -(coll->radius);
		zleft = coll->radius;
		xright = coll->radius;
		zright = coll->radius;
		break;

	case 1:
		XFront = coll->radius;
		ZFront = (COS(coll->facing) * coll->radius) >> (W2V_SHIFT);
		xleft = coll->radius;
		zleft = coll->radius;
		xright = coll->radius;
		zright = -(coll->radius);
		break;

	case 2:
		XFront = (SIN(coll->facing) * coll->radius) >> (W2V_SHIFT);
		ZFront = -coll->radius;
		xleft = coll->radius;
		zleft = -(coll->radius);
		xright = -(coll->radius);
		zright = -(coll->radius);
		break;

	case 3:
		XFront = -(coll->radius);
		ZFront = (COS(coll->facing) * coll->radius) >> (W2V_SHIFT);
		xleft = -(coll->radius);
		zleft = -(coll->radius);
		xright = -(coll->radius);
		zright = coll->radius;
		break;

	default:
		xleft = zleft = 0;
		xright = zright = 0;
		XFront = ZFront = 0;
		break;
	}

	x = XFront + xPos;
	z = ZFront + zPos;

	if (resetRoom)
		tRoomNumber = roomNumber;
	
	floor = GetFloor(x, yTop, z, &tRoomNumber);
	
	int fheight = height = GetFloorHeight(floor, x, yTop, z);
	if (height != NO_HEIGHT)
		height -= yPos;

	ceiling = GetCeiling(floor, x, yTop, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->frontCeiling = ceiling;
	coll->frontFloor = height;
	coll->frontType = HeightType;

	floor = GetFloor(x + XFront, yTop, z + ZFront, &tRoomNumber);
	height = GetFloorHeight(floor, x + XFront, yTop, z + ZFront);
	if (height != NO_HEIGHT)
		height -= yPos;

	if ((coll->slopesAreWalls)
		&& ((coll->frontType == BIG_SLOPE) || (coll->frontType == DIAGONAL))
		&& (coll->frontFloor < coll->midFloor)
		&& (height < coll->frontFloor)
		&& (coll->frontFloor < 0))
	{
		coll->frontFloor = -32767;
	}
	else if (coll->slopesArePits
		&& ((coll->frontType == BIG_SLOPE) || (coll->frontType == DIAGONAL))
		&& (coll->frontFloor > coll->midFloor))
	{
		coll->frontFloor = 512;
	}
	else if ((coll->lavaIsPit)
		&& (coll->frontFloor > 0)
		&& (TriggerIndex)
		&& ((*(TriggerIndex) & DATA_TYPE) == LAVA_TYPE))
	{
		coll->frontFloor = 512;
	}

	x = xPos + xleft;
	z = zPos + zleft;
	short lrRoomNumber = roomNumber;
	floor = GetFloor(x, yTop, z, &lrRoomNumber);

	height = GetFloorHeight(floor, x, yTop, z);
	if (height != NO_HEIGHT)
		height -= yPos;

	ceiling = GetCeiling(floor, x, yTop, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->leftFloor = height;
	coll->leftCeiling = ceiling;
	coll->leftType = HeightType;

	if (coll->slopesAreWalls == 1 && (coll->leftType == BIG_SLOPE || coll->leftType == DIAGONAL) && coll->leftFloor < 0)
		coll->leftFloor = -32767;
	else if (coll->slopesArePits && (coll->leftType == BIG_SLOPE || coll->leftType == DIAGONAL) && coll->leftFloor > 0)
		coll->leftFloor = 512;
	else if (coll->lavaIsPit && coll->leftFloor > 0 && TriggerIndex && (*(TriggerIndex)& DATA_TYPE) == LAVA_TYPE)
		coll->leftFloor = 512;

	x = xPos + xleft;
	z = zPos + zleft;
	floor = GetFloor(x, yTop, z, &tRoomNumber);

	height = GetFloorHeight(floor, x, yTop, z);
	if (height != NO_HEIGHT)
		height -= yPos;

	ceiling = GetCeiling(floor, x, yTop, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->leftFloor2 = height;
	coll->leftCeiling2 = ceiling;
	coll->leftType2 = HeightType;

	if (coll->slopesAreWalls == 1 && (coll->leftType2 == BIG_SLOPE || coll->leftType2 == DIAGONAL) && coll->leftFloor2 < 0)
		coll->leftFloor2 = -32767;
	else if (coll->slopesArePits && (coll->leftType2 == BIG_SLOPE || coll->leftType2 == DIAGONAL) && coll->leftFloor2 > 0)
		coll->leftFloor2 = 512;
	else if (coll->lavaIsPit && coll->leftFloor2 > 0 && TriggerIndex && (*(TriggerIndex)& DATA_TYPE) == LAVA_TYPE)
		coll->leftFloor2 = 512;

	x = xPos + xright;
	z = zPos + zright;
	lrRoomNumber = roomNumber;
	floor = GetFloor(x, yTop, z, &lrRoomNumber);

	height = GetFloorHeight(floor, x, yTop, z);
	if (height != NO_HEIGHT)
		height -= yPos;

	ceiling = GetCeiling(floor, x, yTop, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->rightFloor = height;
	coll->rightCeiling = ceiling;
	coll->rightType = HeightType;

	if (coll->slopesAreWalls == 1 && (coll->rightType == BIG_SLOPE || coll->rightType == DIAGONAL) && coll->rightFloor < 0)
		coll->rightFloor = -32767;
	else if (coll->slopesArePits && (coll->rightType == BIG_SLOPE || coll->rightType == DIAGONAL) && coll->rightFloor > 0)
		coll->rightFloor = 512;
	else if (coll->lavaIsPit && coll->rightFloor > 0 && TriggerIndex && (*(TriggerIndex)& DATA_TYPE) == LAVA_TYPE)
		coll->rightFloor = 512;

	x = xPos + xright;
	z = zPos + zright;
	floor = GetFloor(x, yTop, z, &tRoomNumber);

	height = GetFloorHeight(floor, x, yTop, z);
	if (height != NO_HEIGHT)
		height -= yPos;

	ceiling = GetCeiling(floor, x, yTop, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->rightFloor2 = height;
	coll->rightCeiling2 = ceiling;
	coll->rightType2 = HeightType;

	if (coll->slopesAreWalls == 1 && (coll->rightType2 == BIG_SLOPE || coll->rightType2 == DIAGONAL) && coll->rightFloor2 < 0)
		coll->rightFloor2 = -32767;
	else if (coll->slopesArePits && (coll->rightType2 == BIG_SLOPE || coll->rightType2 == DIAGONAL) && coll->rightFloor2 > 0)
		coll->rightFloor2 = 512;
	else if (coll->lavaIsPit && coll->rightFloor2 > 0 && TriggerIndex && (*(TriggerIndex)& DATA_TYPE) == LAVA_TYPE)
		coll->rightFloor2 = 512;

	CollideStaticObjects(coll, xPos, yPos, zPos, tRoomNumber, objectHeight);
	
	if (coll->midFloor == NO_HEIGHT)	 
	{
		coll->shift.x = coll->old.x - xPos;
		coll->shift.y = coll->old.y - yPos;
		coll->shift.z = coll->old.z - zPos;
		coll->collType = CT_FRONT;
		return;
	}

	if (coll->midFloor - coll->midCeiling <= 0)
	{
		coll->shift.x = coll->old.x - xPos;	 
		coll->shift.y = coll->old.y - yPos;	 
		coll->shift.z = coll->old.z - zPos;
		coll->collType = CT_CLAMP;
		return;
	}

	if (coll->midCeiling >= 0)
	{
		coll->shift.y = coll->midCeiling;	
		coll->collType = CT_TOP;
		coll->hitCeiling = true;
	}

	if ((coll->frontFloor > coll->badPos)
		|| (coll->frontFloor < coll->badNeg)
		|| (coll->frontCeiling > coll->badCeiling))
	{
		if ((coll->frontType == DIAGONAL)
			|| (coll->frontType == SPLIT_TRI))
		{
			coll->shift.x = coll->old.x - xPos; 
			coll->shift.z = coll->old.z - zPos; 
		}
		else
		{
			switch (coll->quadrant)
			{
			case 0:
			case 2:
				coll->shift.x = coll->old.x - xPos;  
				coll->shift.z = FindGridShift(zPos + ZFront, zPos); 
				break;

			case 1:
			case 3:
				coll->shift.x = FindGridShift(xPos + XFront, xPos);	 
				coll->shift.z = coll->old.z - zPos;  
				break;

			}
		}

		coll->collType = CT_FRONT;
		return;
	}

	if (coll->frontCeiling >= coll->badCeiling)
	{
		coll->shift.x = coll->old.x - xPos; 
		coll->shift.y = coll->old.y - yPos;
		coll->shift.z = coll->old.z - zPos;
		coll->collType = CT_TOP_FRONT;
		return;
	}

	if (coll->leftFloor > coll->badPos ||
		coll->leftFloor < coll->badNeg ||
		coll->leftCeiling > coll->badCeiling)
	{
		if (coll->leftType == SPLIT_TRI)
		{
			coll->shift.x = coll->old.x - xPos; 
			coll->shift.z = coll->old.z - zPos;   
		}
		else
		{
			switch (coll->quadrant)
			{
			case 0:
			case 2:
				coll->shift.x = FindGridShift(xPos + xleft, xPos + XFront);
				break;

			case 1:
			case 3:
				coll->shift.z = FindGridShift(zPos + zleft, zPos + ZFront);
				break;
			}
		}
		coll->collType = CT_LEFT;
		return;
	}

	if (coll->rightFloor > coll->badPos ||
		coll->rightFloor < coll->badNeg ||
		coll->rightCeiling > coll->badCeiling)
	{
		if (coll->rightType == SPLIT_TRI)
		{
			coll->shift.x = coll->old.x - xPos;   
			coll->shift.z = coll->old.z - zPos;  
		}
		else
		{
			switch (coll->quadrant)
			{
			case 0:
			case 2:
				coll->shift.x = FindGridShift(xPos + xright, xPos + XFront);
				break;

			case 1:
			case 3:
				coll->shift.z = FindGridShift(zPos + zright, zPos + ZFront);
				break;
			}
		}
		coll->collType = CT_RIGHT;
		return;
	}
}

void LaraBaddieCollision(ITEM_INFO* l, COLL_INFO* coll)
{
	l->hitStatus = false;
	Lara.hitDirection = -1;

	if (l->hitPoints > 0)
	{
		short roomsToCheck[255];
		int numRoomsToCheck = 1;
		roomsToCheck[0] = l->roomNumber;

		short* door = Rooms[l->roomNumber].door;	 
		if (door != NULL)
		{
			for (int i = (int) *(door++); i > 0; i--)
			{
				roomsToCheck[numRoomsToCheck++] = *(door);
				door += 16;
			}
		}

		for (int i = 0; i < numRoomsToCheck; i++)
		{
			short itemNumber = Rooms[roomsToCheck[i]].itemNumber; 
			while (itemNumber != NO_ITEM)
			{
				ITEM_INFO* item = &Items[itemNumber];
				if (item->collidable && item->status != ITEM_INVISIBLE)		 
				{
					OBJECT_INFO* obj = &Objects[item->objectNumber];
					if (obj->collision) 
					{
						int x = l->pos.xPos - item->pos.xPos; 
						int y = l->pos.yPos - item->pos.yPos;
						int z = l->pos.zPos - item->pos.zPos;
						
						if (x > -3072 && x < 3072 &&  
							z > -3072 && z < 3072 &&   
							y > -3072 && y < 3072)  
							(*obj->collision)(itemNumber, l, coll);
					}
				}
				itemNumber = item->nextItem;
			}

			if (coll->enableSpaz)
			{
				MESH_INFO* mesh = Rooms[roomsToCheck[i]].mesh;
				int numMeshes = Rooms[roomsToCheck[i]].numMeshes;

				for (int j = 0; j < numMeshes; j++)
				{
					if (mesh->Flags & 1)
					{
						int x = l->pos.xPos - mesh->x;
						int y = l->pos.yPos - mesh->y;
						int z = l->pos.zPos - mesh->z;

						if (x > -3072 && x < 3072 && y > -3072 && y < 3072 && z > -3072 && z < 3072)
						{
							PHD_3DPOS pos;
							pos.xPos = mesh->x;
							pos.yPos = mesh->y;
							pos.zPos = mesh->z;
							pos.yRot = mesh->yRot;

							if (TestBoundsCollideStatic(&StaticObjects[mesh->staticNumber].xMinc, &pos, coll->radius))
								ItemPushLaraStatic(l, &StaticObjects[mesh->staticNumber].xMinc, &pos, coll);
						}
					}

					mesh++;
				}
			}
		}

		if (Lara.hitDirection == -1)
			Lara.hitFrame = 0;
	}
}

void Inject_Collide()
{
	INJECT(0x00411DB0, CollideStaticObjects);
	INJECT(0x00413CF0, GetCollidedObjects);
	INJECT(0x00410EF0, GetTiltType);
	//INJECT(0x00411100, _GetCollisionInfo);
}