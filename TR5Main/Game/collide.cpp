#include "framework.h"
#include "collide.h"
#include "draw.h"
#include "Lara.h"
#include "items.h"
#include "effect.h"
#include "sphere.h"
#include "misc.h"
#include "setup.h"
#include "sound.h"
#include "trmath.h"

char LM[] =
{
	LM_HIPS,
	LM_LTHIGH,
	LM_LSHIN,
	LM_LFOOT,
	LM_RTHIGH,
	LM_RSHIN,
	LM_RFOOT,
	LM_TORSO,
	LM_RINARM,
	LM_ROUTARM,
	LM_RHAND,
	LM_LINARM,
	LM_LOUTARM,
	LM_LHAND,
	LM_HEAD,
};

extern int SplitFloor, SplitCeiling;
int XFront, ZFront;
BOUNDING_BOX GlobalCollisionBounds;
ITEM_INFO* CollidedItems[1024];
MESH_INFO* CollidedMeshes[1024];

int CollideStaticObjects(COLL_INFO* coll, int x, int y, int z, short roomNumber, int hite)
{
	ROOM_INFO* room;
	MESH_INFO* mesh;
	short roomList[255];
	short numRooms = 0;
	int xMin = 0, xMax = 0, zMin = 0, zMax = 0;
	int inXmin, inXmax, inZmin, inZmax, inYmin;

	coll->hitStatic = false;

	inXmin = x - coll->radius;
	inXmax = x + coll->radius;
	inZmin = z - coll->radius;
	inZmax = z + coll->radius;
	inYmin = y - hite;

	// Collect all the rooms where to check
	GetRoomList(roomNumber, roomList, &numRooms);

	if (numRooms <= 0)
		return 0;

	for (int i = 0; i < numRooms; i++)
	{
		room = &Rooms[roomList[i]];
		mesh = room->mesh;

		for (int j = room->numMeshes; j > 0; j--, mesh++)
		{
			StaticInfo* sInfo = &StaticObjects[mesh->staticNumber];
			if ((sInfo->flags & 1)) // No collision
				continue;

			int yMin = mesh->y + sInfo->xMinc;
			int yMax = mesh->y + sInfo->yMaxc;
			short yRot = mesh->yRot;

			if (yRot == ANGLE(180))
			{
				xMin = mesh->x - sInfo->xMaxc;
				xMax = mesh->x - sInfo->xMinc;
				zMin = mesh->z - sInfo->zMaxc;
				zMax = mesh->z - sInfo->zMinc;
			}
			else if (yRot == -ANGLE(90))
			{
				xMin = mesh->x - sInfo->zMaxc;
				xMax = mesh->x - sInfo->zMinc;
				zMin = mesh->z + sInfo->xMinc;
				zMax = mesh->z + sInfo->xMaxc;
			}
			else if (yRot == ANGLE(90))
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

			if (inXmax <= xMin
			||  inXmin >= xMax
			||  y <= yMin
			||  inYmin >= yMax
			||  inZmax <= zMin
			||  inZmin >= zMax)		 
				continue;

			coll->hitStatic = true;
			return 1;
		}
	}

	return 0;
}

int GetCollidedObjects(ITEM_INFO* collidingItem, int radius, int onlyVisible, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int ignoreLara)
{
	ROOM_INFO* room;
	short roomsArray[255];
	short numRooms;
	short numItems = 0, numMeshes = 0;
	int c, s;
	int rx, rz;

	// Collect all the rooms where to check
	GetRoomList(collidingItem->roomNumber, roomsArray, &numRooms);

	if (collidedMeshes)
	{
		for (int i = 0; i < numRooms; i++)
		{
			room = &Rooms[roomsArray[i]];

			for (int j = 0; j < room->numMeshes; j++)
			{
				MESH_INFO* mesh = &room->mesh[j];
				StaticInfo* staticMesh = &StaticObjects[mesh->staticNumber];

				if (mesh->Flags & 1)
				{
					if (collidingItem->pos.yPos + radius + STEP_SIZE/2 >= mesh->y + staticMesh->yMinc)
					{
						if (collidingItem->pos.yPos <= mesh->y + staticMesh->yMaxc)
						{
							s = phd_sin(mesh->yRot);
							c = phd_cos(mesh->yRot);
							rx = ((collidingItem->pos.xPos - mesh->x) * c - s * (collidingItem->pos.zPos - mesh->z)) >> W2V_SHIFT;
							rz = ((collidingItem->pos.zPos - mesh->z) * c + s * (collidingItem->pos.xPos - mesh->x)) >> W2V_SHIFT;

							if (radius + rx + STEP_SIZE/2 >= staticMesh->xMinc && rx - radius - STEP_SIZE/2 <= staticMesh->xMaxc)
							{
								if (radius + rz + STEP_SIZE/2 >= staticMesh->zMinc && rz - radius - STEP_SIZE/2 <= staticMesh->zMaxc)
								{
									collidedMeshes[numMeshes++] = mesh;
									if (!radius)
									{
										collidedItems[0] = NULL;
										return 1;
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
						int s = phd_sin(item->pos.yRot);
						int c = phd_cos(item->pos.yRot);

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
									return 1;
							}
						}
					}

					itemNumber = item->nextItem;

				} while (itemNumber != NO_ITEM);
			}
		}

		collidedItems[numItems] = NULL;
	}

	return (numItems | numMeshes);
}

int TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll)
{
	short* framePtr = GetBestFrame(lara);

	if (item->pos.yPos + GlobalCollisionBounds.Y2 <= lara->pos.yPos + framePtr[3])
		return false;

	if (item->pos.yPos + GlobalCollisionBounds.Y1 >= framePtr[4])
		return false;

	int s = phd_sin(item->pos.yRot);
	int c = phd_cos(item->pos.yRot);

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

		GetLaraJointPosition((PHD_VECTOR*)&s, LM_HEAD);
		s.roomNumber = LaraItem->roomNumber;

		GAME_VECTOR d;
		d.x = 0;
		d.x = (s.x + ((phd_sin(LaraItem->pos.yRot) << 1) + phd_sin(LaraItem->pos.yRot))) >> 4;
		d.y = s.y;
		d.z = (s.z + ((phd_cos(LaraItem->pos.yRot) << 1) + phd_cos(LaraItem->pos.yRot))) >> 4;
		
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

short GetTiltType(FLOOR_INFO* floor, int x, int y, int z)
{
	ROOM_INFO* r;
	short* data;
	short func;
	int tilt, t0, t1, t2, t3;
	int dx, dz;
	short xOff, yOff;

	while (floor->pitRoom != NO_ROOM)
	{
		if (CheckNoColFloorTriangle(floor, x, z) == TRUE)
			break;
		r = &Rooms[floor->pitRoom];
		floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
	}

	if ((y + CLICK(2)) < (floor->floor * CLICK(1)))
		return FLOOR_TYPE;

	if (!floor->index)
		return FLOOR_TYPE;

	data = &FloorData[floor->index];
	func = *data & DATA_TYPE;
	
	if (func == TILT_TYPE)
		return data[1];
	
	if (func != SPLIT1
	&&  func != SPLIT2
	&&  func != NOCOLF1T
	&&  func != NOCOLF2T
	&&  func != NOCOLF1B
	&&  func != NOCOLF2B)
	{
		return FLOOR_TYPE;
	}

	tilt = data[1];
	t0 = tilt & DATA_TILT;
	t1 = (tilt >> 4) & DATA_TILT;
	t2 = (tilt >> 8) & DATA_TILT;
	t3 = (tilt >> 12) & DATA_TILT;
	dx = x & (WALL_SIZE - 1);
	dz = z & (WALL_SIZE - 1);
	xOff = 0;
	yOff = 0;

	if (func == SPLIT1 || func == NOCOLF1T || func == NOCOLF1B)
	{
		if (dx > (SECTOR(1) - dz))
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

	return ((xOff << 8) | (yOff & DATA_STATIC));
}

int FindGridShift(int x, int z)
{
	if ((x >> WALL_SHIFT) == (z >> WALL_SHIFT))
		return 0;

	if ((z >> WALL_SHIFT) <= (x >> WALL_SHIFT))
		return (-1 - (x & (WALL_SIZE - 1)));
	else
		return ((WALL_SIZE + 1) - (x & (WALL_SIZE - 1)));
}

int TestBoundsCollideStatic(short* bounds, PHD_3DPOS* pos, int radius)
{
	if (!(bounds[5] | bounds[4] | bounds[0] | bounds[1] | bounds[2] | bounds[3]))
		return FALSE;

	short* frame = GetBestFrame(LaraItem);
	if (pos->yPos + bounds[3] <= LaraItem->pos.yPos + frame[2])
		return FALSE;

	if (pos->yPos + bounds[2] >= LaraItem->pos.yPos + frame[3])
		return FALSE;

	int c, s;
	int x, z, dx, dz;

	c = phd_cos(pos->yRot);
	s = phd_sin(pos->yRot);
	x = LaraItem->pos.xPos - pos->xPos;
	z = LaraItem->pos.zPos - pos->zPos;
	dx = (c * x - s * z) >> W2V_SHIFT;
	dz = (c * z + s * x) >> W2V_SHIFT;
	
	if (dx <= radius + bounds[1]
	&&  dx >= bounds[0] - radius
	&&  dz <= radius + bounds[5]
	&&  dz >= bounds[4] - radius)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

int ItemPushLaraStatic(ITEM_INFO* item, short* bounds, PHD_3DPOS* pos, COLL_INFO* coll)
{
	int c, s;
	int dx, dz, rx, rz, minX, maxX, minZ, maxZ;
	int left, right, top, bottom;
	short oldFacing;

	c = phd_cos(pos->yRot);
	s = phd_sin(pos->yRot);
	dx = LaraItem->pos.xPos - pos->xPos;
	dz = LaraItem->pos.zPos - pos->zPos;
	rx = (c * dx - s * dz) >> W2V_SHIFT;
	rz = (c * dz + s * dx) >> W2V_SHIFT;
	minX = bounds[0] - coll->radius;
	maxX = bounds[1] + coll->radius;
	minZ = bounds[4] - coll->radius;
	maxZ = bounds[5] + coll->radius;
	
	if (abs(dx) > 4608
	||  abs(dz) > 4608
	||  rx <= minX
	||  rx >= maxX
	||  rz <= minZ
	||  rz >= maxZ)
		return FALSE;

	left = rx - minX;
	top = maxZ - rz;
	bottom = rz - minZ;
	right = maxX - rx;

	if (right <= left && right <= top && right <= bottom)
		rx += right;
	else if (left <= right && left <= top && left <= bottom)
		rx -= left;
	else if (top <= left && top <= right && top <= bottom)
		rz += top;
	else
		rz -= bottom;

	item->pos.xPos = pos->xPos + ((c * rx + s * rz) >> W2V_SHIFT);
	item->pos.zPos = pos->zPos + ((c * rz - s * rx) >> W2V_SHIFT);
	
	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	oldFacing = coll->facing;
	coll->facing = phd_atan(item->pos.zPos - coll->old.z, item->pos.xPos - coll->old.x);
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
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

	return TRUE;
}

int ItemPushLara(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, int spazon, char bigpush)
{
	int c, s;
	int dx, dz, rx, rz, minX, maxX, minZ, maxZ;
	int left, right, bottom, top;
	short* bounds;
	short facing;

	c = phd_cos(item->pos.yRot);
	s = phd_sin(item->pos.yRot);
	dx = LaraItem->pos.xPos - item->pos.xPos;
	dz = LaraItem->pos.zPos - item->pos.zPos;
	rx = (c * dx - s * dz) >> W2V_SHIFT;
	rz = (c * dz + s * dx) >> W2V_SHIFT;

	if (bigpush & 2)
		bounds = (short*)&GlobalCollisionBounds;
	else
		bounds = GetBestFrame(item);

	minX = bounds[0];
	maxX = bounds[1];
	minZ = bounds[4];
	maxZ = bounds[5];

	if (bigpush & 1)
	{
		minX -= coll->radius;
		maxX += coll->radius;
		minZ -= coll->radius;
		maxZ += coll->radius;
	}

	if (abs(dx) > 4608
	||  abs(dz) > 4608
	||  rx <= minX
	||  rx >= maxX
	||  rz <= minZ
	||  rz >= maxZ)
		return FALSE;

	left = rx - minX;
	top = maxZ - rz;
	bottom = rz - minZ;
	right = maxX - rx;

	if (right <= left && right <= top && right <= bottom)
		rx += right;
	else if (left <= right && left <= top && left <= bottom)
		rx -= left;
	else if (top <= left && top <= right && top <= bottom)
		rz += top;
	else
		rz -= bottom;

	l->pos.xPos = item->pos.xPos + ((c * rx + s * rz) >> W2V_SHIFT);
	l->pos.zPos = item->pos.zPos + ((c * rz - s * rx) >> W2V_SHIFT);

	if (spazon && bounds[3] - bounds[2] > STEP_SIZE)
	{
		rx = (bounds[0] + bounds[1]) / 2;	 
		rz = (bounds[4] + bounds[5]) / 2;

		dx -= (c * rx + s * rz) >> W2V_SHIFT;  
		dz -= (c * rz - s * rx) >> W2V_SHIFT;

		Lara.hitDirection = (l->pos.yRot - phd_atan(dz, dz) - ANGLE(135)) >> W2V_SHIFT;

		if (!Lara.hitFrame)
			SoundEffect(SFX_LARA_INJURY_RND, &l->pos, 0);

		Lara.hitFrame++;
		if (Lara.hitFrame > 34) 
			Lara.hitFrame = 34;
	}

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	facing = coll->facing;
	coll->facing = phd_atan(l->pos.zPos - coll->old.z, l->pos.xPos - coll->old.x);
	GetCollisionInfo(coll, l->pos.xPos, l->pos.yPos, l->pos.zPos, l->roomNumber, LARA_HITE);
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

	return TRUE;
}

void AIPickupCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c)
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
				ItemPushLara(item, l, c, FALSE, TRUE);
		}
	}
}

void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l)
{
	int x, y, z;

	l->pos.xRot = item->pos.xRot;
	l->pos.yRot = item->pos.yRot;
	l->pos.zRot = item->pos.zRot;
	
	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->pos.yRot),
		TO_RAD(item->pos.xRot),
		TO_RAD(item->pos.zRot)
	);

	Vector3 pos = Vector3::Transform(Vector3(vec->x, vec->y, vec->z), matrix);

	l->pos.xPos = item->pos.xPos + pos.x;
	l->pos.yPos = item->pos.yPos + pos.y;
	l->pos.zPos = item->pos.zPos + pos.z;
}

void TriggerLaraBlood() 
{
	int i;
	int node = 1;

	for (i = 0; i < 14; i++)
	{
		if (node & LaraItem->touchBits)
		{
			PHD_VECTOR vec;
			vec.x = (GetRandomControl() & 31) - 16;
			vec.y = (GetRandomControl() & 31) - 16;
			vec.z = (GetRandomControl() & 31) - 16;

			GetLaraJointPosition(&vec, LM[i]);
			DoBloodSplat(vec.x, vec.y, vec.z, (GetRandomControl() & 7) + 8, 2 * GetRandomControl(), LaraItem->roomNumber);
		}

		node <<= 1;
	}
}

int TestLaraPosition(short* bounds, ITEM_INFO* item, ITEM_INFO* l)
{
	int x, y, z, rx, ry, rz;
	short xRotRel, yRotRel, zRotRel;

	xRotRel = l->pos.xRot - item->pos.xRot;
	yRotRel = l->pos.yRot - item->pos.yRot;
	zRotRel = l->pos.zRot - item->pos.zRot;

	if (xRotRel < bounds[6])
		return FALSE;
	if (xRotRel > bounds[7])
		return FALSE;
	if (yRotRel < bounds[8])
		return FALSE;
	if (yRotRel > bounds[9])
		return FALSE;
	if (zRotRel < bounds[10])
		return FALSE;
	if (zRotRel > bounds[11])
		return FALSE;
	
	Vector3 pos = Vector3(l->pos.xPos - item->pos.xPos, l->pos.yPos - item->pos.yPos, l->pos.zPos - item->pos.zPos);

	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->pos.yRot),
		TO_RAD(item->pos.xRot),
		TO_RAD(item->pos.zRot)
	);

	pos = Vector3::Transform(pos, matrix);

	rx = pos.x;
	ry = pos.y;
	rz = pos.z;

	if (rx < bounds[0] || rx > bounds[1] || ry < bounds[2] || ry > bounds[3] || rz < bounds[4] || rz > bounds[5])
		return FALSE;

	return TRUE;
}

int Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angAdd)
{
	int x, y, z;
	int distance, direction;
	int angle;

	x = dest->xPos - src->xPos;
	y = dest->yPos - src->yPos;
	z = dest->zPos - src->zPos;
	distance = sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));

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
			angle = mGetAngle(dest->xPos, dest->zPos, src->xPos, src->zPos);
			direction = ((unsigned short) (angle + ANGLE(45)) / ANGLE(90) - (unsigned short) (dest->yRot + ANGLE(45)) / ANGLE(90)) & 3;

			switch (direction)
			{
			case 0:
				LaraItem->animNumber = ANIMATION_LARA_WALK_LEFT;
				LaraItem->frameNumber = GF(ANIMATION_LARA_WALK_LEFT, 0);
				LaraItem->goalAnimState = STATE_LARA_WALK_LEFT;
				LaraItem->currentAnimState = STATE_LARA_WALK_LEFT;
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			case 1:
				LaraItem->animNumber = ANIMATION_LARA_WALK_FORWARD;
				LaraItem->frameNumber = GF(ANIMATION_LARA_WALK_FORWARD, 0);
				LaraItem->goalAnimState = STATE_LARA_WALK_FORWARD;
				LaraItem->currentAnimState = STATE_LARA_WALK_FORWARD;
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			case 2:
				LaraItem->animNumber = ANIMATION_LARA_WALK_RIGHT;
				LaraItem->frameNumber = GF(ANIMATION_LARA_WALK_RIGHT, 0);
				LaraItem->goalAnimState = STATE_LARA_WALK_RIGHT;
				LaraItem->currentAnimState = STATE_LARA_WALK_RIGHT;
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			case 3:
			default:
				LaraItem->animNumber = ANIMATION_LARA_WALK_BACK;
				LaraItem->frameNumber = GF(ANIMATION_LARA_WALK_BACK, 0);
				LaraItem->goalAnimState = STATE_LARA_WALK_BACK;
				LaraItem->currentAnimState = STATE_LARA_WALK_BACK;
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			}
		}

		Lara.isMoving = true;
		Lara.moveCount = 0;
	}

	if ((short) (dest->xRot - src->xRot) <= angAdd)
	{
		if ((short) (dest->xRot - src->xRot) >= -angAdd)
			src->xRot = dest->xRot;
		else
			src->xRot = src->xRot - angAdd;
	}
	else
	{
		src->xRot = angAdd + src->xRot;
	}

	if ((short) (dest->yRot - src->yRot) <= angAdd)
	{
		if ((short) (dest->yRot - src->yRot) >= -angAdd)
			src->yRot = dest->yRot;
		else
			src->yRot = src->yRot - angAdd;
	}
	else
	{
		src->yRot = angAdd + src->yRot;
	}

	if ((short) (dest->zRot - src->zRot) <= angAdd)
	{
		if ((short) (dest->zRot - src->zRot) >= -angAdd)
			src->zRot = dest->zRot;
		else
			src->zRot = src->zRot - angAdd;
	}
	else
	{
		src->zRot = angAdd + src->zRot;
	}

	return (src->xPos == dest->xPos
		&&  src->yPos == dest->yPos
		&&  src->zPos == dest->zPos
		&&  src->xRot == dest->xRot
		&&  src->yRot == dest->yRot
		&&  src->zRot == dest->zRot);
}

int MoveLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l)
{
	FLOOR_INFO* floor;
	PHD_3DPOS dest;
	int height;
	short roomNumber;

	dest.xRot = item->pos.xRot;
	dest.yRot = item->pos.yRot;
	dest.zRot = item->pos.zRot;

	Vector3 pos = Vector3(vec->x, vec->y, vec->z);

	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->pos.yRot),
		TO_RAD(item->pos.xRot),
		TO_RAD(item->pos.zRot)
	);

	pos = Vector3::Transform(pos, matrix);
	
	dest.xPos = item->pos.xPos + pos.x;
	dest.yPos = item->pos.yPos + pos.y;
	dest.zPos = item->pos.zPos + pos.z;

	if (item->objectNumber != ID_FLARE_ITEM && item->objectNumber != ID_BURNING_TORCH_ITEM)
		return Move3DPosTo3DPos(&l->pos, &dest, LARA_VELOCITY, ANGLE(2));

	roomNumber = l->roomNumber;
	floor = GetFloor(dest.xPos, dest.yPos, dest.zPos, &roomNumber);
	height = GetFloorHeight(floor, dest.xPos, dest.yPos, dest.zPos);
	
	if (abs(height - l->pos.yPos) <= CLICK(2))
	{
		if (sqrt(SQUARE(dest.xPos - l->pos.xPos) + SQUARE(dest.yPos - l->pos.yPos) + SQUARE(dest.zPos - l->pos.zPos)) < (STEP_SIZE/2))
			return TRUE;

		return Move3DPosTo3DPos(&l->pos, &dest, LARA_VELOCITY, ANGLE(2));
	}

	if (Lara.isMoving)
	{
		Lara.isMoving = false;
		Lara.gunStatus = LG_NO_ARMS;
	}

	return FALSE;
}

int TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int radius)
{
	short* bounds;
	short* laraBounds;
	int c, s;
	int x, z;
	int dx, dz;

	bounds = GetBestFrame(item);
	laraBounds = GetBestFrame(l);

	if (item->pos.yPos + bounds[3] > l->pos.yPos + laraBounds[2])
	{
		if (item->pos.yPos + bounds[2] < l->pos.yPos + laraBounds[3])
		{
			c = phd_cos(item->pos.yRot);
			s = phd_sin(item->pos.yRot);
			x = l->pos.xPos - item->pos.xPos;
			z = l->pos.zPos - item->pos.zPos;
			dx = (c * x - s * z) >> W2V_SHIFT;
			dz = (c * z + s * x) >> W2V_SHIFT;

			if (dx >= bounds[0] - radius
			&&  dx <= radius + bounds[1]
			&&  dz >= bounds[4] - radius
			&&  dz <= radius + bounds[5])
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

void CreatureCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];
	int c, s;
	int x, z, rx, rz;
	short* frame;

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
					x = l->pos.xPos - item->pos.xPos;
					z = l->pos.zPos - item->pos.zPos;
					c = phd_cos(item->pos.yRot);
					s = phd_sin(item->pos.yRot);
					frame = GetBestFrame(item);
					rx = (frame[0] + frame[1]) / 2;
					rz = (frame[4] + frame[5]) / 2;
					
					if (frame[3] - frame[2] > STEP_SIZE)
					{
						int angle = (l->pos.yRot - phd_atan(z - ((c * rx - s * rz) >> W2V_SHIFT), x - ((c * rx + s * rz) >> W2V_SHIFT)) - ANGLE(135)) >> W2V_SHIFT;
						Lara.hitDirection = (short)angle;
						// TODO: check if a second Lara.hitFrame++; is required there !
						Lara.hitFrame++; 
						if (Lara.hitFrame > 30)	 
							Lara.hitFrame = 30;
					}
				}
			}
		}
	}
}

void GetCollisionInfo(COLL_INFO* coll, int xPos, int yPos, int zPos, int roomNumber, int objectHeight) // (F) (D)
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
	coll->octant = (unsigned short) (coll->facing + ANGLE(22.5)) / ANGLE(45);

	int x = xPos;
	int y = yPos - objectHeight;
	int yTop = y - 160;
	int z = zPos;

	short tRoomNumber = roomNumber;
	FLOOR_INFO* floor = GetFloor(x, yTop, z, &tRoomNumber);
	
	int height = GetFloorHeight(floor, x, yTop, z);
	if (height != NO_HEIGHT)
		height -= yPos;

	int ceiling = GetCeiling(floor, x, yTop - LaraItem->fallspeed, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->midCeiling = ceiling;
	coll->midFloor = height;
	coll->midType = HeightType;
	coll->midSplitFloor = SplitFloor;
	coll->midSplitCeil = SplitCeiling;
	coll->trigger = TriggerIndex;

	int tilt = GetTiltType(floor, x, LaraItem->pos.yPos, z);
	coll->tiltX = tilt;
	coll->tiltZ = tilt >> 8;
	
	int xright, xleft, zright, zleft;

	/*switch (coll->quadrant)
	{
	case 0:
		XFront = (phd_sin(coll->facing) * coll->radius) >> (W2V_SHIFT);
		ZFront = coll->radius;
		xleft = -(coll->radius);
		zleft = coll->radius;
		xright = coll->radius;
		zright = coll->radius;
		break;

	case 1:
		XFront = coll->radius;
		ZFront = (phd_cos(coll->facing) * coll->radius) >> (W2V_SHIFT);
		xleft = coll->radius;
		zleft = coll->radius;
		xright = coll->radius;
		zright = -(coll->radius);
		break;

	case 2:
		XFront = (phd_sin(coll->facing) * coll->radius) >> (W2V_SHIFT);
		ZFront = -coll->radius;
		xleft = coll->radius;
		zleft = -(coll->radius);
		xright = -(coll->radius);
		zright = -(coll->radius);
		break;

	case 3:
		XFront = -(coll->radius);
		ZFront = (phd_cos(coll->facing) * coll->radius) >> (W2V_SHIFT);
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
	}*/

	XFront = (phd_sin(coll->facing) * coll->radius) >> (W2V_SHIFT);
	ZFront = (phd_cos(coll->facing) * coll->radius) >> (W2V_SHIFT);
	xleft = -ZFront;
	zleft = XFront;
	xright = ZFront;
	zright = -XFront;

	x = XFront + xPos;
	z = ZFront + zPos;

	if (resetRoom)
		tRoomNumber = roomNumber;
	
	floor = GetFloor(x, yTop, z, &tRoomNumber);
	
	height = GetFloorHeight(floor, x, yTop, z);
	if (height != NO_HEIGHT)
		height -= yPos;

	ceiling = GetCeiling(floor, x, yTop - LaraItem->fallspeed, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->frontCeiling = ceiling;
	coll->frontFloor = height;
	coll->frontType = HeightType;
	coll->frontSplitFloor = SplitFloor;
	coll->frontSplitCeil = SplitCeiling;

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

	ceiling = GetCeiling(floor, x, yTop - LaraItem->fallspeed, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->leftFloor = height;
	coll->leftCeiling = ceiling;
	coll->leftType = HeightType;
	coll->leftSplitFloor = SplitFloor;
	coll->leftSplitCeil = SplitCeiling;

	if (coll->slopesAreWalls == 1 && (coll->leftType == BIG_SLOPE || coll->leftType == DIAGONAL) && coll->leftFloor < 0)
		coll->leftFloor = -32767;
	else if (coll->slopesArePits && (coll->leftType == BIG_SLOPE || coll->leftType == DIAGONAL) && coll->leftFloor > 0)
		coll->leftFloor = 512;
	else if (coll->lavaIsPit && coll->leftFloor > 0 && TriggerIndex && (*(TriggerIndex)& DATA_TYPE) == LAVA_TYPE)
		coll->leftFloor = 512;

	floor = GetFloor(x, yTop, z, &tRoomNumber);

	height = GetFloorHeight(floor, x, yTop, z);
	if (height != NO_HEIGHT)
		height -= yPos;

	ceiling = GetCeiling(floor, x, yTop - LaraItem->fallspeed, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->leftFloor2 = height;
	coll->leftCeiling2 = ceiling;
	coll->leftType2 = HeightType;
	coll->leftSplitFloor2 = SplitFloor;
	coll->leftSplitCeil2 = SplitCeiling;

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

	ceiling = GetCeiling(floor, x, yTop - LaraItem->fallspeed, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->rightFloor = height;
	coll->rightCeiling = ceiling;
	coll->rightType = HeightType;
	coll->rightSplitFloor = SplitFloor;
	coll->rightSplitCeil = SplitCeiling;

	if (coll->slopesAreWalls == 1 && (coll->rightType == BIG_SLOPE || coll->rightType == DIAGONAL) && coll->rightFloor < 0)
		coll->rightFloor = -32767;
	else if (coll->slopesArePits && (coll->rightType == BIG_SLOPE || coll->rightType == DIAGONAL) && coll->rightFloor > 0)
		coll->rightFloor = 512;
	else if (coll->lavaIsPit && coll->rightFloor > 0 && TriggerIndex && (*(TriggerIndex)& DATA_TYPE) == LAVA_TYPE)
		coll->rightFloor = 512;

	floor = GetFloor(x, yTop, z, &tRoomNumber);

	height = GetFloorHeight(floor, x, yTop, z);
	if (height != NO_HEIGHT)
		height -= yPos;

	ceiling = GetCeiling(floor, x, yTop - LaraItem->fallspeed, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->rightFloor2 = height;
	coll->rightCeiling2 = ceiling;
	coll->rightType2 = HeightType;
	coll->rightSplitFloor2 = SplitFloor;
	coll->rightSplitCeil2 = SplitCeiling;

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
		if (coll->leftType == SPLIT_TRI && coll->midType == SPLIT_TRI)
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
		
		if (coll->leftSplitFloor && coll->leftSplitFloor == coll->midSplitFloor)
		{
			int quarter = (unsigned short)(coll->facing) / ANGLE(90); // different from coll->quadrant!
			quarter %= 2;

			switch (coll->leftSplitFloor)
			{
			case SPLIT1:
			case NOCOLF1T:
			case NOCOLF1B:
				if (quarter)
					coll->collType = CT_LEFT;
				break;
			case SPLIT2:
			case NOCOLF2T:
			case NOCOLF2B:
				if (!quarter)
					coll->collType = CT_LEFT;
				break;
			}
		}
		else
		{
			coll->collType = CT_LEFT;
		}

		return;
	}

	if (coll->rightFloor > coll->badPos ||
		coll->rightFloor < coll->badNeg ||
		coll->rightCeiling > coll->badCeiling)
	{
		if (coll->rightType == SPLIT_TRI && coll->midType == SPLIT_TRI)
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
		
		if (coll->rightSplitFloor && coll->rightSplitFloor == coll->midSplitFloor)
		{
			int quarter = (unsigned short)(coll->facing) / ANGLE(90); // different from coll->quadrant!
			quarter %= 2;

			switch (coll->rightSplitFloor)
			{
			case SPLIT1:
			case NOCOLF1T:
			case NOCOLF1B:
				if (quarter)
					coll->collType = CT_RIGHT;
				break;
			case SPLIT2:
			case NOCOLF2T:
			case NOCOLF2B:
				if (!quarter)
					coll->collType = CT_RIGHT;
				break;
			}
		}
		else
		{
			coll->collType = CT_RIGHT;
		}

		return;
	}
}

void LaraBaddieCollision(ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;
	ObjectInfo* obj;

	l->hitStatus = false;
	Lara.hitDirection = -1;

	if (l->hitPoints > 0)
	{
		// Crash when using GetRoomList() with vector there but work without :x
		vector<short> roomsList;
		short* door, numDoors;

		roomsList.push_back(l->roomNumber);
		door = Rooms[l->roomNumber].door;
		if (door)
		{
			numDoors = *door;
			door++;
			for (int i = 0; i < numDoors; i++)
			{
				roomsList.push_back(*door);
				door += 16;
			}
		}

		for (int i = 0; i < roomsList.size(); i++)
		{
			short itemNumber = Rooms[roomsList[i]].itemNumber;
			while (itemNumber != NO_ITEM)
			{
				item = &Items[itemNumber];
				if (item->collidable && item->status != ITEM_INVISIBLE)		 
				{
					obj = &Objects[item->objectNumber];
					if (obj->collision != nullptr)
					{
						int x = l->pos.xPos - item->pos.xPos; 
						int y = l->pos.yPos - item->pos.yPos;
						int z = l->pos.zPos - item->pos.zPos;
						
						if (x > -3072 && x < 3072 && z > -3072 && z < 3072 && y > -3072 && y < 3072)  
							obj->collision(itemNumber, l, coll);
					}
				}
				itemNumber = item->nextItem;
			}

			if (coll->enableSpaz)
			{
				MESH_INFO* mesh = Rooms[roomsList[i]].mesh;
				int numMeshes = Rooms[roomsList[i]].numMeshes;

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

void GenericSphereBoxCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->status != ITEM_INVISIBLE)
	{
		if (TestBoundsCollide(item, l, coll->radius))
		{
			int collided = TestCollision(item, l);
			if (collided)
			{
				short oldRot = item->pos.yRot;

				item->pos.yRot = 0;
				GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
				item->pos.yRot = oldRot;
				
				SPHERE* sphere = &CreatureSpheres[0];
				while (collided)
				{
					if (collided & 1)
					{
						GlobalCollisionBounds.X1 = sphere->x - sphere->r - item->pos.xPos;
						GlobalCollisionBounds.X2 = sphere->x + sphere->r - item->pos.xPos;
						GlobalCollisionBounds.Y1 = sphere->y - sphere->r - item->pos.yPos;
						GlobalCollisionBounds.Y2 = sphere->y + sphere->r - item->pos.yPos;
						GlobalCollisionBounds.Z1 = sphere->z - sphere->r - item->pos.zPos;
						GlobalCollisionBounds.Z2 = sphere->z + sphere->r - item->pos.zPos;

						int x = l->pos.xPos;
						int y = l->pos.yPos;
						int z = l->pos.zPos;

						if (ItemPushLara(item, l, coll, (item->itemFlags[0] & (coll->enableSpaz >> 5) & 1), 3) && item->itemFlags[0] & 1)
						{
							l->hitPoints -= item->itemFlags[3];
							
							int dx = x - l->pos.xPos;
							int dy = y - l->pos.yPos;
							int dz = z - l->pos.zPos;

							if (!coll->enableBaddiePush)
							{
								l->pos.xPos += dx;
								l->pos.yPos += dy;
								l->pos.zPos += dz;
							}

							if (dx || dy || dz)
							{
								if (TriggerActive(item))
									TriggerLaraBlood();
							}
						}
					}

					collided >>= 1;
					sphere++;
				}
			}
		}
	}
}

void CalcItemToFloorRotation(ITEM_INFO* item, short rotY, int radiusZ, int radiusX)
{
	FLOOR_INFO* floor;
	float ratioXZ, frontHDif, sideHDif;
	int frontX, frontZ, leftX, leftZ, rightX, rightZ;
	int frontHeight, backHeight, leftHeight, rightHeight;
	int x, y, z;
	short troomNumber;

	// this part with x, y, z, roomNumber can be GAME_VECTOR struct
	x = item->pos.xPos;
	y = item->pos.yPos;
	z = item->pos.zPos;
	troomNumber = item->roomNumber;

	ratioXZ = radiusZ / float(radiusX);
	frontX = (phd_sin(rotY) * radiusZ) >> W2V_SHIFT;
	frontZ = (phd_cos(rotY) * radiusZ) >> W2V_SHIFT;
	leftX = -frontZ * int(ratioXZ);
	leftZ = frontX * int(ratioXZ);
	rightX = frontZ * int(ratioXZ);
	rightZ = -frontX * int(ratioXZ);

	floor = GetFloor(x + frontX, y, z + frontZ, &troomNumber);
	frontHeight = GetFloorHeight(floor, x + frontX, y, z + frontZ);
	floor = GetFloor(x - frontX, y, z - frontZ, &troomNumber);
	backHeight = GetFloorHeight(floor, x - frontX, y, z - frontZ);
	floor = GetFloor(x + leftX, y, z + leftZ, &troomNumber);
	leftHeight = GetFloorHeight(floor, x + leftX, y, z + leftZ);
	floor = GetFloor(x + rightX, y, z + rightZ, &troomNumber);
	rightHeight = GetFloorHeight(floor, x + rightX, y, z + rightZ);

	frontHDif = float(backHeight - frontHeight);
	sideHDif = float(rightHeight - leftHeight);
	item->pos.xRot = ANGLE(atan2(frontHDif, 2 * radiusZ) / RADIAN);
	item->pos.zRot = ANGLE(atan2(sideHDif, 2 * radiusX) / RADIAN);
}

Vector2 GetDiagonalIntersect(int xPos, int zPos, int splitType, int radius, short yRot)
{
	Vector2 vect;

	int dx = (xPos % WALL_SIZE) - WALL_SIZE/2;
	int dz = (zPos % WALL_SIZE) - WALL_SIZE/2;
	int xGrid = xPos - dx;
	int zGrid = zPos - dz;
	
	switch (splitType)
	{
	case SPLIT1:
	case NOCOLF1T:
	case NOCOLF1B:
		xPos = xGrid + (dx - dz) / 2;
		zPos = zGrid - (dx - dz) / 2;
		break;
	case SPLIT2:
	case NOCOLF2T:
	case NOCOLF2B:
		xPos = xGrid + (dx + dz) / 2;
		zPos = zGrid + (dx + dz) / 2;
		break;
	default:
		break;
	}

	if (splitType)
	{
		xPos -= int(radius * sin(TO_RAD(yRot)));
		zPos -= int(radius * cos(TO_RAD(yRot)));
	}

	vect.x = xPos;
	vect.y = zPos;

	return vect;
}
