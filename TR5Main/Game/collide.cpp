#include "framework.h"
#include "control.h"
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
#include "prng.h"
using std::vector;
using namespace T5M::Math::Random;
using namespace T5M::Floordata;
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
int hitSoundTimer;
int XFront, ZFront;
BOUNDING_BOX GlobalCollisionBounds;
ITEM_INFO* CollidedItems[MAX_COLLIDED_OBJECTS];
MESH_INFO* CollidedMeshes[MAX_COLLIDED_OBJECTS];

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

	for (int i = 0; i < numRooms; i++)
	{
		room = &g_Level.Rooms[roomList[i]];
		for (int j = 0; j < room->mesh.size(); j++, mesh++)
		{
			mesh = &room->mesh[j];
			STATIC_INFO* sInfo = &StaticObjects[mesh->staticNumber];

			if ((sInfo->flags & 1)) // No collision
				continue;

			int yMin = mesh->y + sInfo->collisionBox.Y1;
			int yMax = mesh->y + sInfo->collisionBox.Y2;
			short yRot = mesh->yRot;

			if (yRot == ANGLE(180))
			{
				xMin = mesh->x - sInfo->collisionBox.X2;
				xMax = mesh->x - sInfo->collisionBox.X1;
				zMin = mesh->z - sInfo->collisionBox.Z2;
				zMax = mesh->z - sInfo->collisionBox.Z1;
			}
			else if (yRot == -ANGLE(90))
			{
				xMin = mesh->x - sInfo->collisionBox.Z2;
				xMax = mesh->x - sInfo->collisionBox.Z1;
				zMin = mesh->z + sInfo->collisionBox.X1;
				zMax = mesh->z + sInfo->collisionBox.X2;
			}
			else if (yRot == ANGLE(90))
			{

				xMin = mesh->x + sInfo->collisionBox.Z1;
				xMax = mesh->x + sInfo->collisionBox.Z2;
				zMin = mesh->z - sInfo->collisionBox.X2;
				zMax = mesh->z - sInfo->collisionBox.X1;
			}
			else
			{
				xMin = mesh->x + sInfo->collisionBox.X1;
				xMax = mesh->x + sInfo->collisionBox.X2;
				zMin = mesh->z + sInfo->collisionBox.Z1;
				zMax = mesh->z + sInfo->collisionBox.Z2;
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
	float c, s;
	int rx, rz;

	// Collect all the rooms where to check
	GetRoomList(collidingItem->roomNumber, roomsArray, &numRooms);

	if (collidedMeshes)
	{
		for (int i = 0; i < numRooms; i++)
		{
			room = &g_Level.Rooms[roomsArray[i]];

			for (int j = 0; j < room->mesh.size(); j++)
			{
				MESH_INFO* mesh = &room->mesh[j];
				STATIC_INFO* staticMesh = &StaticObjects[mesh->staticNumber];

				if (mesh->flags & 1)
				{
					if (collidingItem->pos.yPos + radius + STEP_SIZE/2 >= mesh->y + staticMesh->collisionBox.Y1)
					{
						if (collidingItem->pos.yPos <= mesh->y + staticMesh->collisionBox.Y2)
						{
							s = phd_sin(mesh->yRot);
							c = phd_cos(mesh->yRot);
							rx = (collidingItem->pos.xPos - mesh->x) * c - s * (collidingItem->pos.zPos - mesh->z);
							rz = (collidingItem->pos.zPos - mesh->z) * c + s * (collidingItem->pos.xPos - mesh->x);

							if (radius + rx + STEP_SIZE/2 >= staticMesh->collisionBox.X1 && rx - radius - STEP_SIZE/2 <= staticMesh->collisionBox.X2)
							{
								if (radius + rz + STEP_SIZE/2 >= staticMesh->collisionBox.Z1 && rz - radius - STEP_SIZE/2 <= staticMesh->collisionBox.Z2)
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
			ROOM_INFO* room = &g_Level.Rooms[roomsArray[i]];

			int itemNumber = room->itemNumber;
			if (itemNumber != NO_ITEM)
			{
				do
				{
					ITEM_INFO* item = &g_Level.Items[itemNumber];

					if ((item == collidingItem || ignoreLara && item == LaraItem) 
						|| (item->flags & 0x8000)
						|| (item->meshBits == 0)
						|| (Objects[item->objectNumber].drawRoutine == NULL)
						|| (Objects[item->objectNumber].collision == NULL && item->objectNumber != ID_LARA) 
						|| (onlyVisible && item->status == ITEM_INVISIBLE) 
						|| item->objectNumber == ID_BURNING_FLOOR)
					{
						itemNumber = item->nextItem;
						continue;
					}

					/*this is awful*/
					/*if (item->objectNumber == ID_UPV && item->hitPoints == 1)
					{
						itemNumber = item->nextItem;
						continue;
					}
					if (item->objectNumber == ID_BIGGUN && item->hitPoints == 1)
					{
						itemNumber = item->nextItem;
						continue;
					}*/
					/*we need a better system*/

					int dx = collidingItem->pos.xPos - item->pos.xPos;
					int dy = collidingItem->pos.yPos - item->pos.yPos;
					int dz = collidingItem->pos.zPos - item->pos.zPos;

					ANIM_FRAME* framePtr = GetBestFrame(item);

					if (dx >= -2048
						&& dx <= 2048
						&& dy >= -2048
						&& dy <= 2048
						&& dz >= -2048
						&& dz <= 2048
						&& collidingItem->pos.yPos + radius + 128 >= item->pos.yPos + framePtr->boundingBox.Y1
						&& collidingItem->pos.yPos - radius - 128 <= item->pos.yPos + framePtr->boundingBox.Y2
						&& collidingItem->floor >= item->pos.yPos)
					{
						float s = phd_sin(item->pos.yRot);
						float c = phd_cos(item->pos.yRot);

						int rx = dx * c - s * dz;
						int rz = dz * c + s * dx;

						if (item->objectNumber == ID_TURN_SWITCH)
						{
							framePtr->boundingBox.X1 = -256;
							framePtr->boundingBox.X2 = 256;
							framePtr->boundingBox.Z1 = -256;
							framePtr->boundingBox.Z1 = 256;
						}

						if (radius + rx + 128 >= framePtr->boundingBox.X1 && rx - radius - 128 <= framePtr->boundingBox.X2)
						{
							if (radius + rz + 128 >= framePtr->boundingBox.Z1 && rz - radius - 128 <= framePtr->boundingBox.Z2)
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

	return (numItems || numMeshes);
}

int TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll)
{
	ANIM_FRAME* framePtr = GetBestFrame(lara);

	if (item->pos.yPos + GlobalCollisionBounds.Y2 <= lara->pos.yPos + framePtr->boundingBox.Y1)
		return false;

	if (item->pos.yPos + GlobalCollisionBounds.Y1 >= framePtr->boundingBox.Y2)
		return false;

	float s = phd_sin(item->pos.yRot);
	float c = phd_cos(item->pos.yRot);

	int dx = lara->pos.xPos - item->pos.xPos;
	int dz = lara->pos.zPos - item->pos.zPos;

	int x = c * dx - s * dz;
	int z = c * dz + s * dx;

	if (x < GlobalCollisionBounds.X1 - coll->radius ||
		x > GlobalCollisionBounds.X2 + coll->radius ||
		z < GlobalCollisionBounds.Z1 - coll->radius ||
		z > GlobalCollisionBounds.Z2 + coll->radius)
		return false;

	return true;
}

void TrapCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->status == ITEM_ACTIVE)
	{
		if (!TestBoundsCollide(item, l, c->radius))
			return;

		TestCollision(item, LaraItem);    
	}
	else if (item->status != ITEM_INVISIBLE)
		ObjectCollision(itemNumber, l, c);
}

void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll)
{
	for (int i = 0; i < 2; i++)
	{
		GAME_VECTOR s;
		s.x = (i * 256) - 0x80;
		s.y = -256;
		s.z = 0;

		GetLaraJointPosition((PHD_VECTOR*)&s, LM_HEAD);
		s.roomNumber = LaraItem->roomNumber;

		GAME_VECTOR d;
		d.x = s.x + phd_sin(LaraItem->pos.yRot) * 768;
		d.y = s.y;
		d.z = s.z + phd_cos(LaraItem->pos.yRot) * 768;

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
	item->location = GetRoom(item->location, x, y, z);
	item->floor = GetFloorHeight(item->location, x, z).value_or(NO_HEIGHT);
	if (item->roomNumber != item->location.roomNumber)
		ItemNewRoom(Lara.itemNumber, item->location.roomNumber);
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
		r = &g_Level.Rooms[floor->pitRoom];
		floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
	}

	if ((y + CLICK(2)) < (floor->floor * CLICK(1)))
		return FLOOR_TYPE;

	if (!floor->index)
		return FLOOR_TYPE;

	data = &g_Level.FloorData[floor->index];
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
	t1 = (tilt / 16) & DATA_TILT;
	t2 = (tilt / 256) & DATA_TILT;
	t3 = (tilt / 4096) & DATA_TILT;
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

	return ((xOff * 256) | (yOff & DATA_STATIC));
}

int FindGridShift(int x, int z)
{
	if ((x / SECTOR(1)) == (z / SECTOR(1)))
		return 0;

	if ((z / SECTOR(1)) <= (x / SECTOR(1)))
		return (-1 - (x & (WALL_SIZE - 1)));
	else
		return ((WALL_SIZE + 1) - (x & (WALL_SIZE - 1)));
}

int TestBoundsCollideStatic(BOUNDING_BOX* bounds, PHD_3DPOS* pos, int radius)
{
	if (!(bounds->Z2 != 0 || bounds->Z1 != 0 || bounds->X1 != 0 || bounds->X2 != 0 || bounds->Y1 != 0 || bounds->Y2 != 0))
		return false;

	ANIM_FRAME* frame = GetBestFrame(LaraItem);
	if (pos->yPos + bounds->Y2 <= LaraItem->pos.yPos + frame->boundingBox.Y1)
		return false;

	if (pos->yPos + bounds->Y1 >= LaraItem->pos.yPos + frame->boundingBox.Y2)
		return false;

	float c, s;
	int x, z, dx, dz;

	c = phd_cos(pos->yRot);
	s = phd_sin(pos->yRot);
	x = LaraItem->pos.xPos - pos->xPos;
	z = LaraItem->pos.zPos - pos->zPos;
	dx = c * x - s * z;
	dz = c * z + s * x;
	
	if (dx <= radius + bounds->X2
	&&  dx >= bounds->X1 - radius
	&&  dz <= radius + bounds->Z2
	&&  dz >= bounds->Z1 - radius)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int ItemPushLaraStatic(ITEM_INFO* item, BOUNDING_BOX* bounds, PHD_3DPOS* pos, COLL_INFO* coll)
{
	float c, s;
	int dx, dz, rx, rz, minX, maxX, minZ, maxZ;
	int left, right, top, bottom;
	short oldFacing;

	c = phd_cos(pos->yRot);
	s = phd_sin(pos->yRot);
	dx = LaraItem->pos.xPos - pos->xPos;
	dz = LaraItem->pos.zPos - pos->zPos;
	rx = c * dx - s * dz;
	rz = c * dz + s * dx;
	minX = bounds->X1 - coll->radius;
	maxX = bounds->X2 + coll->radius;
	minZ = bounds->Z1 - coll->radius;
	maxZ = bounds->Z2 + coll->radius;
	
	if (abs(dx) > 4608
	||  abs(dz) > 4608
	||  rx <= minX
	||  rx >= maxX
	||  rz <= minZ
	||  rz >= maxZ)
		return false;

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

	item->pos.xPos = pos->xPos + c * rx + s * rz;
	item->pos.zPos = pos->zPos + c * rz - s * rx;
	
	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	oldFacing = coll->facing;
	coll->facing = phd_atan(item->pos.zPos - coll->old.z, item->pos.xPos - coll->old.x);
	if (item == LaraItem)
	{
		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	}
	else
	{
		GetObjectCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	}
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

	return true;
}

int ItemPushLara(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, int spazon, char bigpush)
{
	float c, s;
	int dx, dz, rx, rz, minX, maxX, minZ, maxZ;
	int left, right, bottom, top;
	BOUNDING_BOX* bounds;
	short facing;

	dx = l->pos.xPos - item->pos.xPos;		// Get Vector from Baddie to Lara
	dz = l->pos.zPos - item->pos.zPos;
	c = phd_cos(item->pos.yRot);					// Get Baddies Rotation
	s = phd_sin(item->pos.yRot);
	rx = (c * dx - s * dz);            	// Rotate Lara Vector into Baddie Frame
	rz = (c * dz + s * dx);

	if (bigpush & 2)
		bounds = &GlobalCollisionBounds;
	else
		bounds = (BOUNDING_BOX*)GetBestFrame(item);

	minX = bounds->X1;
	maxX = bounds->X2;
	minZ = bounds->Z1;
	maxZ = bounds->Z2;

	if (bigpush & 1)
	{
		minX -= coll->radius;
		maxX += coll->radius;
		minZ -= coll->radius;
		maxZ += coll->radius;
	}

	// Big enemies
	if (abs(dx) > 4608
	||  abs(dz) > 4608
	||  rx <= minX
	||  rx >= maxX
	||  rz <= minZ
	||  rz >= maxZ)
		return false;

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

	l->pos.xPos = item->pos.xPos + c * rx + s * rz;
	l->pos.zPos = item->pos.zPos + c * rz - s * rx;

	if (spazon && bounds->Y2 - bounds->Y1 > STEP_SIZE)
	{
		rx = (bounds->X1 + bounds->X2) / 2;	 
		rz = (bounds->Z1 + bounds->Z2) / 2;

		dx -= c * rx + s * rz;
		dz -= c * rz - s * rx;

		Lara.hitDirection = (l->pos.yRot - phd_atan(dz, dz) - ANGLE(135)) / 16384;

		if ((!Lara.hitFrame) && (!hitSoundTimer))
		{
				SoundEffect(SFX_TR4_LARA_INJURY, &l->pos, 0);
				hitSoundTimer = generateFloat(5, 15);
		}

		if (hitSoundTimer)
			hitSoundTimer--;

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

	return true;
}

void AIPickupCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (item->objectNumber == ID_SHOOT_SWITCH1 && !(item->meshBits & 1))
		item->status = ITEM_INVISIBLE;
}

void ObjectCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TestBoundsCollide(item, l, c->radius))
	{
		if (TestCollision(item, l))
		{
			if (c->enableBaddiePush)
				ItemPushLara(item, l, c, false, true);
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

		node *= 2;
	}
}

int TestLaraPosition(OBJECT_COLLISION_BOUNDS* bounds, ITEM_INFO* item, ITEM_INFO* l)
{
	int x, y, z, rx, ry, rz;
	short xRotRel, yRotRel, zRotRel;

	xRotRel = l->pos.xRot - item->pos.xRot;
	yRotRel = l->pos.yRot - item->pos.yRot;
	zRotRel = l->pos.zRot - item->pos.zRot;

	if (xRotRel < bounds->rotX1)
		return false;
	if (xRotRel > bounds->rotX2)
		return false;
	if (yRotRel < bounds->rotY1)
		return false;
	if (yRotRel > bounds->rotY2)
		return false;
	if (zRotRel < bounds->rotZ1)
		return false;
	if (zRotRel > bounds->rotZ2)
		return false;

	x = l->pos.xPos - item->pos.xPos; 
	y = l->pos.yPos - item->pos.yPos;
	z = l->pos.zPos - item->pos.zPos;

	Vector3 pos = Vector3(x, y, z);

	// HACK (REMOVED FOR NOW): it seems that a minus sign is required here. I don't know why, but it just works (tm) but we must 
	// do more tests
	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(-item->pos.yRot),
		TO_RAD(item->pos.xRot),
		TO_RAD(item->pos.zRot)
	);

	pos = Vector3::Transform(pos, matrix);

	rx = pos.x;
	ry = pos.y;
	rz = pos.z;

	if (rx < bounds->boundingBox.X1 || rx > bounds->boundingBox.X2
		|| ry < bounds->boundingBox.Y1 || ry > bounds->boundingBox.Y2
		|| rz < bounds->boundingBox.Z1 || rz > bounds->boundingBox.Z2)
		return false;

	return true;
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
			//direction = (GetQuadrant(angle) - GetQuadrant(dest->yRot)) & 3;
			
			angle = (angle + 0x2000) / 0x4000;
			angle = (angle - ((unsigned short)(dest->yRot + 0x2000) / 0x4000));
			angle &= 3;
			
			switch (angle)
			{
			case 0:
				LaraItem->animNumber = LA_SIDESTEP_LEFT;
				LaraItem->frameNumber = GF(LA_SIDESTEP_LEFT, 0);
				LaraItem->goalAnimState = LS_STEP_LEFT;
				LaraItem->currentAnimState = LS_STEP_LEFT;
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			case 1:
				LaraItem->animNumber = LA_WALK;
				LaraItem->frameNumber = GF(LA_WALK, 0);
				LaraItem->goalAnimState = LS_WALK_FORWARD;
				LaraItem->currentAnimState = LS_WALK_FORWARD;
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			case 2:
				LaraItem->animNumber = LA_WALK;
				LaraItem->frameNumber = GF(LA_SIDESTEP_RIGHT, 0);
				LaraItem->goalAnimState = LS_STEP_RIGHT;
				LaraItem->currentAnimState = LS_STEP_RIGHT;
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			case 3:
			default:
				LaraItem->animNumber = LA_WALK_BACK;
				LaraItem->frameNumber = GF(LA_WALK_BACK, 0);
				LaraItem->goalAnimState = LS_WALK_BACK;
				LaraItem->currentAnimState = LS_WALK_BACK;
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
			return true;

		return Move3DPosTo3DPos(&l->pos, &dest, LARA_VELOCITY, ANGLE(2));
	}

	if (Lara.isMoving)
	{
		Lara.isMoving = false;
		Lara.gunStatus = LG_NO_ARMS;
	}

	return false;
}

int TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int radius)
{
	BOUNDING_BOX* bounds;
	BOUNDING_BOX* laraBounds;
	float c, s;
	int x, z;
	int dx, dz;

	bounds = (BOUNDING_BOX*)GetBestFrame(item);
	laraBounds = (BOUNDING_BOX*)GetBestFrame(l);

	if (item->pos.yPos + bounds->Y2 > l->pos.yPos + laraBounds->Y1)
	{
		if (item->pos.yPos + bounds->Y1 < l->pos.yPos + laraBounds->Y2)
		{
			c = phd_cos(item->pos.yRot);
			s = phd_sin(item->pos.yRot);
			x = l->pos.xPos - item->pos.xPos;
			z = l->pos.zPos - item->pos.zPos;
			dx = c * x - s * z;
			dz = c * z + s * x;

			if (dx >= bounds->X1 - radius
			&&  dx <= radius + bounds->X2
			&&  dz >= bounds->Z1 - radius
			&&  dz <= radius + bounds->Z2)
			{
				return true;
			}
		}
	}

	return false;
}

void CreatureCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	float c, s;
	int x, z, rx, rz;
	ANIM_FRAME* frame;

	if (item->objectNumber != ID_HITMAN || item->currentAnimState != LS_INSERT_PUZZLE)
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
					rx = (frame->boundingBox.X1 + frame->boundingBox.X2) / 2;
					rz = (frame->boundingBox.X2 + frame->boundingBox.Z2) / 2;
					
					if (frame->boundingBox.Y2 - frame->boundingBox.Y1 > STEP_SIZE)
					{
						int angle = (l->pos.yRot - phd_atan(z - c * rx - s * rz, x - c * rx + s * rz) - ANGLE(135)) / 16384;
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

void DoFloorThings(FLOOR_INFO* floor, int x, int y, int z)
{
	TiltYOffset = 0;
	TiltXOffset = 0;
	OnObject = 0;
	HeightType = WALL;
	SplitFloor = 0;

	ROOM_INFO* r;
	while (floor->pitRoom != NO_ROOM)
	{
		if (CheckNoColFloorTriangle(floor, x, z) == 1)
			break;
		r = &g_Level.Rooms[floor->pitRoom];
		floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
	}

	int height = floor->floor * 256;
	if (height != NO_HEIGHT)
	{
		TriggerIndex = NULL;

		if (floor->index != 0)
		{
			short* data = &g_Level.FloorData[floor->index];
			short type, hadj;

			int xOff, yOff, trigger;
			ITEM_INFO* item;
			OBJECT_INFO* obj;
			int tilts, t0, t1, t2, t3, t4, dx, dz, h1, h2;
			do
			{
				type = *(data++);

				switch (type & DATA_TYPE)
				{
				case DOOR_TYPE:
				case ROOF_TYPE:
				case SPLIT3:
				case SPLIT4:
				case NOCOLC1T:
				case NOCOLC1B:
				case NOCOLC2T:
				case NOCOLC2B:
					data++;
					break;

				case TILT_TYPE:
					TiltXOffset = xOff = (*data >> 8);
					TiltYOffset = yOff = *(char*)data;

					if ((abs(xOff)) > 2 || (abs(yOff)) > 2)
						HeightType = BIG_SLOPE;
					else
						HeightType = SMALL_SLOPE;

					if (xOff >= 0)
						height += (xOff * ((-1 - z) & 1023) >> 2);
					else
						height -= (xOff * (z & 1023) >> 2);

					if (yOff >= 0)
						height += yOff * ((-1 - x) & 1023) >> 2;
					else
						height -= yOff * (x & 1023) >> 2;

					data++;
					break;

				case TRIGGER_TYPE:
					if (!TriggerIndex)
						TriggerIndex = data - 1;

					data++;

					do
					{
						trigger = *(data++);

						if (TRIG_BITS(trigger) != TO_OBJECT)
						{
							if (TRIG_BITS(trigger) == TO_CAMERA ||
								TRIG_BITS(trigger) == TO_FLYBY)
							{
								trigger = *(data++);
							}
						}

					} while (!(trigger & END_BIT));
					break;

				case LAVA_TYPE:
					TriggerIndex = data - 1;
					break;

				case CLIMB_TYPE:
				case MONKEY_TYPE:
				case TRIGTRIGGER_TYPE:
				case MINER_TYPE:
					if (!TriggerIndex)
						TriggerIndex = data - 1;
					break;

				case SPLIT1:
				case SPLIT2:
				case NOCOLF1T:
				case NOCOLF1B:
				case NOCOLF2T:
				case NOCOLF2B:
					tilts = *data;
					t0 = tilts & 15;
					t1 = (tilts >> 4) & 15;
					t2 = (tilts >> 8) & 15;
					t3 = (tilts >> 12) & 15;

					dx = x & 1023;
					dz = z & 1023;

					xOff = yOff = 0;

					HeightType = SPLIT_TRI;
					SplitFloor = (type & DATA_TYPE);

					if ((type & DATA_TYPE) == SPLIT1 ||
						(type & DATA_TYPE) == NOCOLF1T ||
						(type & DATA_TYPE) == NOCOLF1B)
					{
						if (dx <= (1024 - dz))
						{
							hadj = (type >> 10) & 0x1F;
							if (hadj & 0x10)
								hadj |= 0xfff0;
							height += 256 * hadj;
							xOff = t2 - t1;
							yOff = t0 - t1;
						}
						else
						{
							hadj = (type >> 5) & 0x1F;
							if (hadj & 0x10)
								hadj |= 0xFFF0;
							height += 256 * hadj;
							xOff = t3 - t0;
							yOff = t3 - t2;
						}
					}
					else
					{
						if (dx <= dz)
						{
							hadj = (type >> 10) & 0x1f;
							if (hadj & 0x10)
								hadj |= 0xfff0;
							height += 256 * hadj;
							xOff = t2 - t1;
							yOff = t3 - t2;
						}
						else
						{
							hadj = (type >> 5) & 0x1f;
							if (hadj & 0x10)
								hadj |= 0xfff0;
							height += 256 * hadj;
							xOff = t3 - t0;
							yOff = t0 - t1;
						}
					}

					TiltXOffset = xOff;
					TiltYOffset = yOff;

					if ((abs(xOff)) > 2 || (abs(yOff)) > 2)
						HeightType = DIAGONAL;
					else if (HeightType != SPLIT_TRI)
						HeightType = SMALL_SLOPE;

					if (xOff >= 0)
						height += xOff * ((-1 - z) & 1023) >> 2;
					else
						height -= xOff * (z & 1023) >> 2;

					if (yOff >= 0)
						height += yOff * ((-1 - x) & 1023) >> 2;
					else
						height -= yOff * (x & 1023) >> 2;

					data++;
					break;

				default:
					break;
				}
			} while (!(type & END_BIT));
		}
	}
}

void GetCollisionInfo(COLL_INFO* coll, int xPos, int yPos, int zPos, int roomNumber, int objectHeight)
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
	coll->quadrant = GetQuadrant(coll->facing);

	int x = xPos;
	int y = yPos - objectHeight;
	int yTop = y - 160;
	int z = zPos;

	short tRoomNumber = roomNumber;
	FLOOR_INFO* floor = GetFloor(x, yTop, z, &tRoomNumber);

	ROOM_VECTOR tfLocation = GetRoom(LaraItem->location, x, yTop, z);

	DoFloorThings(floor, x, yTop, z);
	int height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	ROOM_VECTOR tcLocation = GetRoom(LaraItem->location, x, yTop - LaraItem->fallspeed, z);

	int ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
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
	coll->tiltZ = tilt / 256;

	int xright, xleft, zright, zleft;

	switch (coll->quadrant)
	{
	case 0:
		XFront = phd_sin(coll->facing) * coll->radius;
		ZFront = coll->radius;
		xleft = -(coll->radius);
		zleft = coll->radius;
		xright = coll->radius;
		zright = coll->radius;
		break;

	case 1:
		XFront = coll->radius;
		ZFront = phd_cos(coll->facing) * coll->radius;
		xleft = coll->radius;
		zleft = coll->radius;
		xright = coll->radius;
		zright = -(coll->radius);
		break;

	case 2:
		XFront = phd_sin(coll->facing) * coll->radius;
		ZFront = -coll->radius;
		xleft = coll->radius;
		zleft = -(coll->radius);
		xright = -(coll->radius);
		zright = -(coll->radius);
		break;

	case 3:
		XFront = -(coll->radius);
		ZFront = phd_cos(coll->facing) * coll->radius;
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

	//XFront = phd_sin(coll->facing) * coll->radius;
	//ZFront = phd_cos(coll->facing) * coll->radius;
	//xleft = -ZFront;
	//zleft = XFront;
	//xright = ZFront;
	//zright = -XFront;

	x = XFront + xPos;
	z = ZFront + zPos;

	if (resetRoom)
	{
		tfLocation = LaraItem->location;
		tcLocation = LaraItem->location;
	}

	tfLocation = GetRoom(tfLocation, x, yTop, z);

	DoFloorThings(floor, x, yTop, z);
	height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	tcLocation = GetRoom(tcLocation, x, yTop - LaraItem->fallspeed, z);

	ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->frontCeiling = ceiling;
	coll->frontFloor = height;
	coll->frontType = HeightType;
	coll->frontSplitFloor = SplitFloor;
	coll->frontSplitCeil = SplitCeiling;

	floor = GetFloor(x + XFront, yTop, z + ZFront, &tRoomNumber);
	DoFloorThings(floor, x + XFront, yTop, z + ZFront);

	tfLocation = GetRoom(tfLocation, x + XFront, yTop, z + ZFront);
	height = GetFloorHeight(tfLocation, x + XFront, z + ZFront).value_or(NO_HEIGHT);
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
		&& ((*(TriggerIndex)& DATA_TYPE) == LAVA_TYPE))
	{
		coll->frontFloor = 512;
	}

	x = xPos + xleft;
	z = zPos + zleft;

	ROOM_VECTOR lrfLocation = GetRoom(LaraItem->location, x, yTop, z);

	short lrRoomNumber = roomNumber;
	floor = GetFloor(x, yTop, z, &lrRoomNumber);
	DoFloorThings(floor, x, yTop, z);

	height = GetFloorHeight(lrfLocation, x, z).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	ROOM_VECTOR lrcLocation = GetRoom(LaraItem->location, x, yTop - LaraItem->fallspeed, z);

	ceiling = GetCeilingHeight(lrcLocation, x, z).value_or(NO_HEIGHT);
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

	tfLocation = GetRoom(tfLocation, x, yTop, z);

	DoFloorThings(floor, x, yTop, z);

	height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	tcLocation = GetRoom(tcLocation, x, yTop - LaraItem->fallspeed, z);

	ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
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

	lrfLocation = GetRoom(LaraItem->location, x, yTop, z);

	lrRoomNumber = roomNumber;
	floor = GetFloor(x, yTop, z, &lrRoomNumber);
	DoFloorThings(floor, x, yTop, z);

	height = GetFloorHeight(lrfLocation, x, z).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	lrcLocation = GetRoom(LaraItem->location, x, yTop - LaraItem->fallspeed, z);

	ceiling = GetCeilingHeight(lrcLocation, x, z).value_or(NO_HEIGHT);
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

	tfLocation = GetRoom(tfLocation, x, yTop, z);

	floor = GetFloor(x, yTop, z, &tRoomNumber);
	DoFloorThings(floor, x, yTop, z);

	height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	tcLocation = GetRoom(tcLocation, x, yTop - LaraItem->fallspeed, z);

	ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
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

void GetObjectCollisionInfo(COLL_INFO* coll, int xPos, int yPos, int zPos, int roomNumber, int objectHeight)
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
	coll->quadrant = GetQuadrant(coll->facing);

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
	coll->tiltZ = tilt / 256;
	
	int xright, xleft, zright, zleft;

	switch (coll->quadrant)
	{
	case 0:
		XFront = phd_sin(coll->facing) * coll->radius;
		ZFront = coll->radius;
		xleft = -(coll->radius);
		zleft = coll->radius;
		xright = coll->radius;
		zright = coll->radius;
		break;

	case 1:
		XFront = coll->radius;
		ZFront = phd_cos(coll->facing) * coll->radius;
		xleft = coll->radius;
		zleft = coll->radius;
		xright = coll->radius;
		zright = -(coll->radius);
		break;

	case 2:
		XFront = phd_sin(coll->facing) * coll->radius;
		ZFront = -coll->radius;
		xleft = coll->radius;
		zleft = -(coll->radius);
		xright = -(coll->radius);
		zright = -(coll->radius);
		break;

	case 3:
		XFront = -(coll->radius);
		ZFront = phd_cos(coll->facing) * coll->radius;
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

	//XFront = phd_sin(coll->facing) * coll->radius;
	//ZFront = phd_cos(coll->facing) * coll->radius;
	//xleft = -ZFront;
	//zleft = XFront;
	//xright = ZFront;
	//zright = -XFront;

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
	OBJECT_INFO* obj;

	l->hitStatus = false;
	Lara.hitDirection = -1;

	if (l->hitPoints > 0)
	{
		short* door, numDoors;
		short roomsToCheck[128];
		short numRoomsToCheck = 0;
		roomsToCheck[numRoomsToCheck++] = l->roomNumber;

		ROOM_INFO* room = &g_Level.Rooms[l->roomNumber];
		for (int i = 0; i < room->doors.size(); i++)
		{
			roomsToCheck[numRoomsToCheck++] = room->doors[i].room;
		}

		for (int i = 0; i < numRoomsToCheck; i++)
		{
			short itemNumber = g_Level.Rooms[roomsToCheck[i]].itemNumber;
			while (itemNumber != NO_ITEM)
			{
				item = &g_Level.Items[itemNumber];
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
				for (int j = 0; j < g_Level.Rooms[roomsToCheck[i]].mesh.size(); j++)
				{
					MESH_INFO* mesh = &g_Level.Rooms[roomsToCheck[i]].mesh[j];

					if (mesh->flags & 1)
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

							if (TestBoundsCollideStatic(&StaticObjects[mesh->staticNumber].collisionBox, &pos, coll->radius))
								ItemPushLaraStatic(l, &StaticObjects[mesh->staticNumber].collisionBox, &pos, coll);
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
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (item->status != ITEM_INVISIBLE)
	{
		if (TestBoundsCollide(item, l, coll->radius))
		{
			int collidedBits = TestCollision(item, l);
			if (collidedBits)
			{
				short oldRot = item->pos.yRot;

				item->pos.yRot = 0;
				GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
				item->pos.yRot = oldRot;

				int deadlyBits = item->itemFlags[0];
				SPHERE* sphere = &CreatureSpheres[0];

				if (item->itemFlags[2] != 0)
				{
					collidedBits &= ~1;
				}

				while (collidedBits)
				{
					if (collidedBits & 1)
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

						if (ItemPushLara(item, l, coll, ((deadlyBits & 1) & coll->enableSpaz), 3) && (deadlyBits & 1))
						{
							l->hitPoints -= item->itemFlags[3];

							int dx = x - l->pos.xPos;
							int dy = y - l->pos.yPos;
							int dz = z - l->pos.zPos;

							if (dx || dy || dz)
							{
								if (TriggerActive(item))
									TriggerLaraBlood();
							}

							if (!coll->enableBaddiePush)
							{
								l->pos.xPos += dx;
								l->pos.yPos += dy;
								l->pos.zPos += dz;
							}
						}
					}

					collidedBits >>= 1;
					deadlyBits >>= 1;
					sphere++;
				}
			}
		}
	}
}

void CalcItemToFloorRotation(ITEM_INFO* item, int radiusDivide)
{
	FLOOR_INFO* floor;
	BOUNDING_BOX* bounds;
	GAME_VECTOR pos;
	int ratioXZ, frontHDif, sideHDif;
	int frontX, frontZ, leftX, leftZ, rightX, rightZ;
	int frontHeight, backHeight, leftHeight, rightHeight;
	int radiusZ, radiusX;

	bounds = GetBoundsAccurate(item);
	pos.x = item->pos.xPos;
	pos.y = item->pos.yPos;
	pos.z = item->pos.zPos;
	pos.roomNumber = item->roomNumber;
	radiusX = bounds->X2;
	radiusZ = bounds->Z2 / radiusDivide; // need divide in any case else it's too much !

	ratioXZ = radiusZ / radiusX;
	frontX = phd_sin(item->pos.yRot) * radiusZ;
	frontZ = phd_cos(item->pos.yRot) * radiusZ;
	leftX = -frontZ * ratioXZ;
	leftZ = frontX * ratioXZ;
	rightX = frontZ * ratioXZ;
	rightZ = -frontX * ratioXZ;

	floor = GetFloor(pos.x + frontX, pos.y, pos.z + frontZ, &pos.roomNumber);
	frontHeight = GetFloorHeight(floor, pos.x + frontX, pos.y, pos.z + frontZ);
	floor = GetFloor(pos.x - frontX, pos.y, pos.z - frontZ, &pos.roomNumber);
	backHeight = GetFloorHeight(floor, pos.x - frontX, pos.y, pos.z - frontZ);
	floor = GetFloor(pos.x + leftX, pos.y, pos.z + leftZ, &pos.roomNumber);
	leftHeight = GetFloorHeight(floor, pos.x + leftX, pos.y, pos.z + leftZ);
	floor = GetFloor(pos.x + rightX, pos.y, pos.z + rightZ, &pos.roomNumber);
	rightHeight = GetFloorHeight(floor, pos.x + rightX, pos.y, pos.z + rightZ);

	frontHDif = backHeight - frontHeight;
	sideHDif = rightHeight - leftHeight;
	// NOTE: float(atan2()) is required, else warning about double !
	item->pos.xRot = ANGLE(float(atan2(frontHDif, 2 * radiusZ)) / RADIAN);
	item->pos.zRot = ANGLE(float(atan2(sideHDif, 2 * radiusX)) / RADIAN);
}

bool SnapToQuadrant(short& angle, int interval)
{
	if (abs(angle) <= ANGLE(interval))
	{
		angle = 0;
		return true;
	}
	else if (angle >= ANGLE(90 - interval) && angle <= ANGLE(interval + 90))
	{
		angle = ANGLE(90);
		return true;
	}
	else if (angle >= ANGLE(180 - interval) || angle <= -ANGLE(180 - interval))
	{
		angle = ANGLE(180);
		return true;
	}
	else if (angle >= -ANGLE(interval + 90) && angle <= -ANGLE(90 - interval))
	{
		angle = -ANGLE(90);
		return true;
	}
	return false;
}

int GetQuadrant(short angle)
{
	return (unsigned short) (angle + ANGLE(45)) / ANGLE(90);
}

bool SnapToDiagonal(short& angle, int interval)
{
	if (angle >= ANGLE(45 - interval) && angle <= ANGLE(interval + 45))
	{
		angle = ANGLE(45);
		return true;
	}
	else if (angle >= ANGLE(135 - interval) && angle <= ANGLE(interval + 135))
	{
		angle = ANGLE(135);
		return true;
	}
	else if (angle >= -ANGLE(interval + 135) && angle <= -ANGLE(135 - interval))
	{
		angle = -ANGLE(135);
		return true;
	}
	else if (angle >= -ANGLE(interval + 45) && angle <= -ANGLE(45 - interval))
	{
		angle = -ANGLE(45);
		return true;
	}
	return false;
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
		yRot = floor(yRot / 16384.0f) * ANGLE(90) + ANGLE(45);
		xPos -= int(radius * sin(TO_RAD(yRot)));
		zPos -= int(radius * cos(TO_RAD(yRot)));
	}

	vect.x = xPos;
	vect.y = zPos;

	return vect;
}

Vector2 GetOrthogonalIntersect(int xPos, int zPos, int radius, short yRot)
{
	Vector2 vect;

	int xGrid = xPos - (xPos % WALL_SIZE);
	int zGrid = zPos - (zPos % WALL_SIZE);

	int dir = (unsigned short)(yRot + ANGLE(45)) / ANGLE(90);
	switch (dir)
	{
	case NORTH:
		zPos = zGrid + (WALL_SIZE - 1) - radius;
		break;
	case EAST:
		xPos = xGrid + (WALL_SIZE - 1) - radius;
		break;
	case SOUTH:
		zPos = zGrid + radius;
		break;
	case WEST:
		xPos = xGrid + radius;
		break;
	}

	vect.x = xPos;
	vect.y = zPos;

	return vect;
}
