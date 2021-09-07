#include "framework.h"
#include "control.h"
#include "collide.h"
#include "draw.h"
#include "Lara.h"
#include "items.h"
#include "effect2.h"
#include "sphere.h"
#include "misc.h"
#include "setup.h"
#include "sound.h"
#include "trmath.h"
#include "prng.h"

using std::vector;
using namespace TEN::Math::Random;
using namespace TEN::Floordata;
using namespace TEN::Renderer;

BOUNDING_BOX GlobalCollisionBounds;
ITEM_INFO* CollidedItems[MAX_COLLIDED_OBJECTS];
MESH_INFO* CollidedMeshes[MAX_COLLIDED_OBJECTS];

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

					if ((item == collidingItem) 
					 || (item->objectNumber == ID_LARA && ignoreLara)
					 || (item->flags & 0x8000)
					 || (item->meshBits == 0)
					 || (Objects[item->objectNumber].drawRoutine == NULL && item->objectNumber != ID_LARA)
					 || (Objects[item->objectNumber].collision   == NULL && item->objectNumber != ID_LARA) 
					 || (onlyVisible && item->status == ITEM_INVISIBLE) 
					 || (item->objectNumber == ID_BURNING_FLOOR))
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

void CollideSolidStatics(ITEM_INFO* item, COLL_INFO* coll)
{
	short roomsToCheck[128];
	short numRoomsToCheck = 0;
	roomsToCheck[numRoomsToCheck++] = item->roomNumber;

	ROOM_INFO* room = &g_Level.Rooms[item->roomNumber];
	for (int i = 0; i < room->doors.size(); i++)
	{
		roomsToCheck[numRoomsToCheck++] = room->doors[i].room;
	}

	for (int i = 0; i < numRoomsToCheck; i++)
	{
		for (int j = 0; j < g_Level.Rooms[roomsToCheck[i]].mesh.size(); j++)
		{
			auto mesh = &g_Level.Rooms[roomsToCheck[i]].mesh[j];
			if ((mesh->flags & 1) && (mesh->flags & 2))
				CollideSolidStatic(item, mesh, coll);
		}
	}
}

bool CollideSolidStatic(ITEM_INFO* item, MESH_INFO* mesh, COLL_INFO* coll)
{
	auto stInfo = StaticObjects[mesh->staticNumber];

	// Ignore collision if flag is not set
	if (stInfo.flags & 1)
		return false;

	// Get DX static bounds in global coords
	auto staticPos = PHD_3DPOS(mesh->x, mesh->y, mesh->z, 0, mesh->yRot, 0);
	auto staticBounds = TO_DX_BBOX(&staticPos, &stInfo.collisionBox);

	// Get local TR bounds and DX item bounds in global coords
	auto itemBBox = GetBoundsAccurate(item);
	auto itemBounds = TO_DX_BBOX(&item->pos, itemBBox);

	// Extend bounds a bit for visual testing
	itemBounds.Extents = itemBounds.Extents + Vector3(WALL_SIZE / 3);

	// Filter out any further checks if static isn't nearby
	if (!staticBounds.Intersects(itemBounds))
		return false;

	// Bring back extents
	itemBounds.Extents = itemBounds.Extents - Vector3(WALL_SIZE / 3);

	// Draw static bounds
	g_Renderer.addDebugBox(staticBounds, Vector4(1, 0.3, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	// Calculate item coll bounds according to radius
	BOUNDING_BOX collBox;
	collBox.X1 = -coll->radius;
	collBox.X2 =  coll->radius;
	collBox.Z1 = -coll->radius;
	collBox.Z2 =  coll->radius;
	collBox.Y1 = itemBBox->Y1;
	collBox.Y2 = itemBBox->Y2;

	// Get DX item coll bounds
	auto collBounds = TO_DX_BBOX(&item->pos, &collBox);

	// Draw item coll bounds
	if (!staticBounds.Intersects(collBounds)) // Final generic check before further calculations
	{
		g_Renderer.addDebugBox(collBounds, Vector4(0, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);
		return false;
	}
	else
		g_Renderer.addDebugBox(collBounds, Vector4(1, 0, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);
	
	// Determine identity static collision bounds
	auto XMin = mesh->x + stInfo.collisionBox.X1;
	auto XMax = mesh->x + stInfo.collisionBox.X2;
	auto YMin = mesh->y + stInfo.collisionBox.Y1;
	auto YMax = mesh->y + stInfo.collisionBox.Y2;
	auto ZMin = mesh->z + stInfo.collisionBox.Z1;
	auto ZMax = mesh->z + stInfo.collisionBox.Z2;

	// Determine identity rotation/distance
	auto distance = Vector3(item->pos.xPos, item->pos.yPos, item->pos.zPos) - Vector3(mesh->x, mesh->y, mesh->z);
	auto c = phd_cos(mesh->yRot);
	auto s = phd_sin(mesh->yRot);

	// Rotate item to collision bounds identity
	auto x = round(distance.x * c - distance.z * s) + mesh->x;
	auto y = item->pos.yPos;
	auto z = round(distance.x * s + distance.z * c) + mesh->z;

	// Determine item collision bounds
	auto inXMin = x - coll->radius;
	auto inXMax = x + coll->radius;
	auto inYMin = y - (itemBBox->Y2 - itemBBox->Y1);
	auto inYMax = y;
	auto inZMin = z - coll->radius;
	auto inZMax = z + coll->radius;

	// Don't calculate shifts if not in bounds
	if (inXMax <= XMin || inXMin >= XMax ||
		inYMax <= YMin || inYMin >= YMax ||
		inZMax <= ZMin || inZMin >= ZMax)
		return false;
	
	// Calculate shifts

	PHD_VECTOR rawShift = {};

	auto shiftLeft = inXMax - XMin;
	auto shiftRight = XMax - inXMin;

	if (shiftLeft < shiftRight)
		rawShift.x = -shiftLeft;
	else
		rawShift.x = shiftRight;

	shiftLeft = inZMax - ZMin;
	shiftRight = ZMax - inZMin;

	if (shiftLeft < shiftRight)
		rawShift.z = -shiftLeft;
	else
		rawShift.z = shiftRight;

	// Rotate previous collision position to identity
	distance = Vector3(coll->old.x, coll->old.y, coll->old.z) - Vector3(mesh->x, mesh->y, mesh->z);
	auto ox = round(distance.x * c - distance.z * s) + mesh->x;
	auto oz = round(distance.x * s + distance.z * c) + mesh->z;

	// Calculate collisison type based on identity rotation
	switch (GetQuadrant(coll->facing - mesh->yRot))
	{
	case NORTH:
		if (rawShift.x > coll->radius || rawShift.x < -coll->radius)
		{
			coll->shift.z = rawShift.z;
			coll->shift.x = ox - x;
			coll->collType = CT_FRONT;
		}
		else if (rawShift.x > 0 && rawShift.x <= coll->radius)
		{
			coll->shift.x = rawShift.x;
			coll->shift.z = 0;
			coll->collType = CT_LEFT;
		}
		else if (rawShift.x < 0 && rawShift.x >= -coll->radius)
		{
			coll->shift.x = rawShift.x;
			coll->shift.z = 0;
			coll->collType = CT_RIGHT;
		}
		break;

	case SOUTH:
		if (rawShift.x > coll->radius || rawShift.x < -coll->radius)
		{
			coll->shift.z = rawShift.z;
			coll->shift.x = ox - x;
			coll->collType = CT_FRONT;
		}
		else if (rawShift.x > 0 && rawShift.x <= coll->radius)
		{
			coll->shift.x = rawShift.x;
			coll->shift.z = 0;
			coll->collType = CT_RIGHT;
		}
		else if (rawShift.x < 0 && rawShift.x >= -coll->radius)
		{
			coll->shift.x = rawShift.x;
			coll->shift.z = 0;
			coll->collType = CT_LEFT;
		}
		break;

	case EAST:
		if (rawShift.z > coll->radius || rawShift.z < -coll->radius)
		{
			coll->shift.x = rawShift.x;
			coll->shift.z = oz - z;
			coll->collType = CT_FRONT;
		}
		else if (rawShift.z > 0 && rawShift.z <= coll->radius)
		{
			coll->shift.z = rawShift.z;
			coll->shift.x = 0;
			coll->collType = CT_RIGHT;
		}
		else if (rawShift.z < 0 && rawShift.z >= -coll->radius)
		{
			coll->shift.z = rawShift.z;
			coll->shift.x = 0;
			coll->collType = CT_LEFT;
		}
		break;

	case WEST:
		if (rawShift.z > coll->radius || rawShift.z < -coll->radius)
		{
			coll->shift.x = rawShift.x;
			coll->shift.z = oz - z;
			coll->collType = CT_FRONT;
		}
		else if (rawShift.z > 0 && rawShift.z <= coll->radius)
		{
			coll->shift.z = rawShift.z;
			coll->shift.x = 0;
			coll->collType = CT_LEFT;
		}
		else if (rawShift.z < 0 && rawShift.z >= -coll->radius)
		{
			coll->shift.z = rawShift.z;
			coll->shift.x = 0;
			coll->collType = CT_RIGHT;
		}
		break;
	}

	// Determine final shifts rotation/distance
	distance = Vector3(x + coll->shift.x, y, z + coll->shift.z) - Vector3(mesh->x, mesh->y, mesh->z);
	c = phd_cos(0 - mesh->yRot);
	s = phd_sin(0 - mesh->yRot);

	// Calculate final shifts rotation/distance
	coll->shift.x = (round(distance.x * c - distance.z * s) + mesh->x) - item->pos.xPos;
	coll->shift.z = (round(distance.x * s + distance.z * c) + mesh->z) - item->pos.zPos;

	if (coll->shift.x == 0 && coll->shift.z == 0)
		coll->collType = CT_NONE; // Paranoid

	coll->hitStatic = true;
	return true;
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

int FindGridShift(int x, int z)
{
	if ((x / SECTOR(1)) == (z / SECTOR(1)))
		return 0;

	if ((z / SECTOR(1)) <= (x / SECTOR(1)))
		return (-1 - (x & (WALL_SIZE - 1)));
	else
		return ((WALL_SIZE + 1) - (x & (WALL_SIZE - 1)));
}

int TestBoundsCollideStatic(ITEM_INFO* item, MESH_INFO* mesh, int radius)
{
	PHD_3DPOS pos;
	pos.xPos = mesh->x;
	pos.yPos = mesh->y;
	pos.zPos = mesh->z;
	pos.yRot = mesh->yRot;

	auto bounds = StaticObjects[mesh->staticNumber].collisionBox;

	if (!(bounds.Z2 != 0 || bounds.Z1 != 0 || bounds.X1 != 0 || bounds.X2 != 0 || bounds.Y1 != 0 || bounds.Y2 != 0))
		return false;

	ANIM_FRAME* frame = GetBestFrame(LaraItem);
	if (pos.yPos + bounds.Y2 <= LaraItem->pos.yPos + frame->boundingBox.Y1)
		return false;

	if (pos.yPos + bounds.Y1 >= LaraItem->pos.yPos + frame->boundingBox.Y2)
		return false;

	float c, s;
	int x, z, dx, dz;

	c = phd_cos(pos.yRot);
	s = phd_sin(pos.yRot);
	x = LaraItem->pos.xPos - pos.xPos;
	z = LaraItem->pos.zPos - pos.zPos;
	dx = c * x - s * z;
	dz = c * z + s * x;
	
	if (dx <= radius + bounds.X2
	&&  dx >= bounds.X1 - radius
	&&  dz <= radius + bounds.Z2
	&&  dz >= bounds.Z1 - radius)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int ItemPushStatic(ITEM_INFO* l, MESH_INFO* mesh, COLL_INFO* coll) // previously ItemPushLaraStatic
{
	PHD_3DPOS pos;
	pos.xPos = mesh->x;
	pos.yPos = mesh->y;
	pos.zPos = mesh->z;
	pos.yRot = mesh->yRot;

	auto bounds = StaticObjects[mesh->staticNumber].collisionBox;

	auto c = phd_cos(pos.yRot);
	auto s = phd_sin(pos.yRot);

	auto dx = l->pos.xPos - pos.xPos;
	auto dz = l->pos.zPos - pos.zPos;
	auto rx = c * dx - s * dz;
	auto rz = c * dz + s * dx;
	auto minX = bounds.X1 - coll->radius;
	auto maxX = bounds.X2 + coll->radius;
	auto minZ = bounds.Z1 - coll->radius;
	auto maxZ = bounds.Z2 + coll->radius;
	
	if (abs(dx) > 4608
	||  abs(dz) > 4608
	||  rx <= minX
	||  rx >= maxX
	||  rz <= minZ
	||  rz >= maxZ)
		return false;

	auto left = rx - minX;
	auto top = maxZ - rz;
	auto bottom = rz - minZ;
	auto right = maxX - rx;

	if (right <= left && right <= top && right <= bottom)
		rx += right;
	else if (left <= right && left <= top && left <= bottom)
		rx -= left;
	else if (top <= left && top <= right && top <= bottom)
		rz += top;
	else
		rz -= bottom;

	l->pos.xPos = pos.xPos + c * rx + s * rz;
	l->pos.zPos = pos.zPos + c * rz - s * rx;
	
	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	auto oldFacing = coll->facing;
	coll->facing = phd_atan(l->pos.zPos - coll->old.z, l->pos.xPos - coll->old.x);
	if (l == LaraItem)
	{
		GetCollisionInfo(coll, l->pos.xPos, l->pos.yPos, l->pos.zPos, l->roomNumber, LARA_HEIGHT);
	}
	else
	{
		GetObjectCollisionInfo(coll, l->pos.xPos, l->pos.yPos, l->pos.zPos, l->roomNumber, LARA_HEIGHT);
	}
	coll->facing = oldFacing;

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

	if (l == LaraItem && Lara.isMoving && Lara.moveCount > 15)
	{
		Lara.isMoving = false;
		Lara.gunStatus = LG_NO_ARMS;
	}

	return true;
}

int ItemPushItem(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, int spazon, char bigpush) // previously ItemPushLara
{
	float c, s;
	int dx, dz, rx, rz, minX, maxX, minZ, maxZ;
	int left, right, bottom, top;
	BOUNDING_BOX* bounds;
	short facing;

	// Get item's rotation
	c = phd_cos(item->pos.yRot); 
	s = phd_sin(item->pos.yRot);

	// Get vector from item to Lara
	dx = LaraItem->pos.xPos - item->pos.xPos; 
	dz = LaraItem->pos.zPos - item->pos.zPos;

	// Rotate Lara vector into item frame
	rx = c * dx - s * dz; 
	rz = c * dz + s * dx;

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

		static int hitSoundTimer = 0;

		if ((!Lara.hitFrame) && (!hitSoundTimer))
		{
				SoundEffect(SFX_TR4_LARA_INJURY, &l->pos, 0);
				hitSoundTimer = generateFloat(15, 35);
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

	if (l == LaraItem)
	{
		GetCollisionInfo(coll, l->pos.xPos, l->pos.yPos, l->pos.zPos, l->roomNumber, LARA_HEIGHT);
	}
	else
	{
		GetObjectCollisionInfo(coll, l->pos.xPos, l->pos.yPos, l->pos.zPos, l->roomNumber, LARA_HEIGHT);
	}

	coll->facing = facing;

	if (coll->collType == CT_NONE)
	{
		coll->old.x = l->pos.xPos;
		coll->old.y = l->pos.yPos;
		coll->old.z = l->pos.zPos;

		// Commented because causes Lara to jump out of the water if she touches an object on the surface. re: "kayak bug"
		// UpdateLaraRoom(l, -10);
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
				ItemPushItem(item, l, c, false, true);
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

	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->pos.yRot),
		TO_RAD(item->pos.xRot),
		TO_RAD(item->pos.zRot)
	);

	// This solves once for all the minus sign hack of CreateFromYawPitchRoll.
	// In reality it should be the inverse, but the inverse of a rotation matrix is equal to the transpose 
	// and transposing a matrix is faster.
	// It's the only piece of code that does it, because we want Lara's location relative to the identity frame 
	// of the object we are test against.
	matrix = matrix.Transpose();

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
			direction = (GetQuadrant(angle) - GetQuadrant(dest->yRot)) & 3;
			
			switch (direction)
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
					LaraItem->animNumber = LA_SIDESTEP_RIGHT;
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

	short deltaAngle = dest->xRot - src->xRot;
	if (deltaAngle > angAdd)
		src->xRot += angAdd;
	else if (deltaAngle < -angAdd)
		src->xRot -= angAdd;
	else
		src->xRot = dest->xRot;

	deltaAngle = dest->yRot - src->yRot;
	if (deltaAngle > angAdd)
		src->yRot += angAdd;
	else if (deltaAngle < -angAdd)
		src->yRot -= angAdd;
	else
		src->yRot = dest->yRot;

	deltaAngle = dest->zRot - src->zRot;
	if (deltaAngle > angAdd)
		src->zRot += angAdd;
	else if (deltaAngle < -angAdd)
		src->zRot -= angAdd;
	else
		src->zRot = dest->zRot;

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
					ItemPushItem(item, l, coll, coll->enableSpaz, 0);
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

// A handy overload of GetCollisionResult which can be used to quickly get collision parameters
// such as floor height under specific item.

COLL_RESULT GetCollisionResult(ITEM_INFO* item)
{
	auto room = item->roomNumber;
	auto floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room);
	auto result = GetCollisionResult(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	result.RoomNumber = room;
	return result;
}

// This variation of GetCollisionResult is an universal wrapper to be used across whole
// collisional code to replace "holy trinity" of roomNumber-GetFloor-GetFloorHeight operations.
// The advantage of this wrapper is that it does NOT modify incoming roomNumber parameter,
// instead putting modified one returned by GetFloor into return COLL_RESULT structure.
// This way, function never modifies any external variables.

COLL_RESULT GetCollisionResult(int x, int y, int z, short roomNumber)
{
	auto room = roomNumber;
	auto floor = GetFloor(x, y, z, &room);
	auto result = GetCollisionResult(floor, x, y, z);
	
	result.RoomNumber = room;
	return result;
}

// GetCollisionResult is a reworked legacy GetFloorHeight function, which does not
// write any data into globals, but instead into special COLL_RESULT struct.
// Additionally, it writes ceiling height for same coordinates, so this function
// may be reused instead both GetFloorHeight and GetCeilingHeight calls to increase
// readability.

COLL_RESULT GetCollisionResult(FLOOR_INFO* floor, int x, int y, int z)
{
	COLL_RESULT result = {};

	// Return provided block into result as itself.
	result.Block = floor;

	// Floor and ceiling heights are directly borrowed from new floordata.
	result.FloorHeight = GetFloorHeight(ROOM_VECTOR{ floor->Room, y }, x, z).value_or(NO_HEIGHT);
	result.CeilingHeight = GetCeilingHeight(ROOM_VECTOR{ floor->Room, y }, x, z).value_or(NO_HEIGHT);

	// Probe bottom block through portals.
	// TODO: Check if it is really needed, as GetFloor should take care of it itself?
	ROOM_INFO* r;
	while (floor->pitRoom != NO_ROOM)
	{
		if (CheckNoColFloorTriangle(floor, x, z) == 1)
			break;
		r = &g_Level.Rooms[floor->pitRoom];
		floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
	}

	// Return probed bottom block into result.
	result.BottomBlock = floor;

	// Check if block isn't a wall or there is no floordata
	if (floor->floor * CLICK(1) != NO_HEIGHT && floor->index)
	{
		// TODO: For ChocolateFan: currently we use legacy floordata ONLY for getting TiltX/TiltY and
		// SplitFloor/SplitCeiling values. These values can be derived from new floordata. After this
		// we can remove this chain floordata parser.

		short* data = &g_Level.FloorData[floor->index];

		short type;
		int zOff, xOff, trigger;
		int tilts, t0, t1, t2, t3, t4, dx, dz;

		do
		{
			type = *(data++);

			switch (type & DATA_TYPE)
			{
			case DOOR_TYPE:
			case ROOF_TYPE:
				data++;
				break;

			case SPLIT3:
			case SPLIT4:
			case NOCOLC1T:
			case NOCOLC1B:
			case NOCOLC2T:
			case NOCOLC2B:
				result.SplitCeiling = type & DATA_TYPE;
				data++;
				break;

			case TILT_TYPE:
				result.TiltZ = zOff = (*data >> 8);
				result.TiltX = xOff = *(char*)data;

				if ((abs(zOff)) > 2 || (abs(xOff)) > 2)
					result.HeightType = BIG_SLOPE;
				else
					result.HeightType = SMALL_SLOPE;

				data++;
				break;

			case TRIGGER_TYPE:
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

				result.HeightType = SPLIT_TRI;
				result.SplitFloor = (type & DATA_TYPE);

				if ((type & DATA_TYPE) == SPLIT1 ||
					(type & DATA_TYPE) == NOCOLF1T ||
					(type & DATA_TYPE) == NOCOLF1B)
				{
					if (dx <= (1024 - dz))
					{
						zOff = t2 - t1;
						xOff = t0 - t1;
					}
					else
					{
						zOff = t3 - t0;
						xOff = t3 - t2;
					}
				}
				else
				{
					if (dx <= dz)
					{
						zOff = t2 - t1;
						xOff = t3 - t2;
					}
					else
					{
						zOff = t3 - t0;
						xOff = t0 - t1;
					}
				}

				result.TiltZ = zOff;
				result.TiltX = xOff;

				if ((abs(zOff)) > 2 || (abs(xOff)) > 2)
					result.HeightType = DIAGONAL;
				else if (result.HeightType != SPLIT_TRI)
					result.HeightType = SMALL_SLOPE;

				data++;
				break;

			default:
				break;
			}
		} while (!(type & END_BIT));
	}

	// TODO: check if we need to keep here this slope vs. bridge check from legacy GetTiltType
	if ((y + CLICK(2)) < (floor->floor * CLICK(1)))
		result.TiltZ = result.TiltX = 0;

	return result;
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
	coll->shift.x  = 0;
	coll->shift.y  = 0;
	coll->shift.z  = 0;
	coll->quadrant = GetQuadrant(coll->facing);

	int x = xPos;
	int y = yPos - objectHeight;
	int yTop = y - LARA_HEADROOM;
	int z = zPos;

	auto collResult = GetCollisionResult(x, yTop, z, roomNumber);
	auto topRoomNumber = collResult.RoomNumber; // Keep top room number as we need it to re-probe from origin room

	ROOM_VECTOR tfLocation = GetRoom(LaraItem->location, x, yTop, z);
	int height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	ROOM_VECTOR tcLocation = GetRoom(LaraItem->location, x, yTop - LaraItem->fallspeed, z);
	int ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->middle.Ceiling = ceiling;
	coll->middle.Floor = height;
	coll->middle.Type = collResult.HeightType;
	coll->middle.SplitFloor = collResult.SplitFloor;
	coll->middle.SplitCeiling = collResult.SplitCeiling;

	collResult = GetCollisionResult(x, LaraItem->pos.yPos, z, roomNumber);
	coll->tiltX = collResult.TiltX;
	coll->tiltZ = collResult.TiltZ;

	int xfront, xright, xleft, zfront, zright, zleft;

	switch (coll->quadrant)
	{
	case 0:
		xfront = phd_sin(coll->facing) * coll->radius;
		zfront = coll->radius;
		xleft = -(coll->radius);
		zleft = coll->radius;
		xright = coll->radius;
		zright = coll->radius;
		break;

	case 1:
		xfront = coll->radius;
		zfront = phd_cos(coll->facing) * coll->radius;
		xleft = coll->radius;
		zleft = coll->radius;
		xright = coll->radius;
		zright = -(coll->radius);
		break;

	case 2:
		xfront = phd_sin(coll->facing) * coll->radius;
		zfront = -coll->radius;
		xleft = coll->radius;
		zleft = -(coll->radius);
		xright = -(coll->radius);
		zright = -(coll->radius);
		break;

	case 3:
		xfront = -(coll->radius);
		zfront = phd_cos(coll->facing) * coll->radius;
		xleft = -(coll->radius);
		zleft = -(coll->radius);
		xright = -(coll->radius);
		zright = coll->radius;
		break;

	default:
		xleft = zleft = 0;
		xright = zright = 0;
		xfront = zfront = 0;
		break;
	}

	x = xfront + xPos;
	z = zfront + zPos;

	if (resetRoom)
	{
		tfLocation = LaraItem->location;
		tcLocation = LaraItem->location;
		topRoomNumber = roomNumber;
	}

	collResult = GetCollisionResult(x, yTop, z, topRoomNumber);

	tfLocation = GetRoom(tfLocation, x, yTop, z);
	height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	tcLocation = GetRoom(tcLocation, x, yTop - LaraItem->fallspeed, z);
	ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->front.Ceiling = ceiling;
	coll->front.Floor = height;
	coll->front.Type = collResult.HeightType;
	coll->front.SplitFloor = collResult.SplitFloor;
	coll->front.SplitCeiling = collResult.SplitCeiling;

	collResult = GetCollisionResult(x + xfront, yTop, z + zfront, topRoomNumber);

	tfLocation = GetRoom(tfLocation, x + xfront, yTop, z + zfront);
	height = GetFloorHeight(tfLocation, x + xfront, z + zfront).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	if ((coll->slopesAreWalls)
		&& ((coll->front.Type == BIG_SLOPE) || (coll->front.Type == DIAGONAL))
		&& (coll->front.Floor < coll->middle.Floor)
		&& (height < coll->front.Floor)
		&& (coll->front.Floor < 0))
	{
		coll->front.Floor = MAX_HEIGHT;
	}
	else if (coll->slopesArePits
		&& ((coll->front.Type == BIG_SLOPE) || (coll->front.Type == DIAGONAL))
		&& (coll->front.Floor > coll->middle.Floor))
	{
		coll->front.Floor = STOP_SIZE;
	}
	else if ((coll->lavaIsPit)
		     && (coll->front.Floor > 0)
		     && collResult.BottomBlock->Flags.Death)
	{
		coll->front.Floor = STOP_SIZE;
	}

	x = xPos + xleft;
	z = zPos + zleft;

	collResult = GetCollisionResult(x, yTop, z, roomNumber);

	ROOM_VECTOR lrfLocation = GetRoom(LaraItem->location, x, yTop, z);
	height = GetFloorHeight(lrfLocation, x, z).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	ROOM_VECTOR lrcLocation = GetRoom(LaraItem->location, x, yTop - LaraItem->fallspeed, z);
	ceiling = GetCeilingHeight(lrcLocation, x, z).value_or(NO_HEIGHT);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->middleLeft.Floor = height;
	coll->middleLeft.Ceiling = ceiling;
	coll->middleLeft.Type = collResult.HeightType;
	coll->middleLeft.SplitFloor = collResult.SplitFloor;
	coll->middleLeft.SplitCeiling = collResult.SplitCeiling;

	if (coll->slopesAreWalls && (coll->middleLeft.Type == BIG_SLOPE || coll->middleLeft.Type == DIAGONAL) && coll->middleLeft.Floor < 0)
		coll->middleLeft.Floor = MAX_HEIGHT;
	else if (coll->slopesArePits && (coll->middleLeft.Type == BIG_SLOPE || coll->middleLeft.Type == DIAGONAL) && coll->middleLeft.Floor > 0)
		coll->middleLeft.Floor = STOP_SIZE;
	else if (coll->lavaIsPit && coll->middleLeft.Floor > 0 && collResult.BottomBlock->Flags.Death)
		coll->middleLeft.Floor = STOP_SIZE;

	collResult = GetCollisionResult(x, yTop, z, topRoomNumber); // We use plain x/z values here, proposed by Choco

	tfLocation = GetRoom(tfLocation, x, yTop, z);
	height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	tcLocation = GetRoom(tcLocation, x, yTop - LaraItem->fallspeed, z);
	ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->frontLeft.Floor = height;
	coll->frontLeft.Ceiling = ceiling;
	coll->frontLeft.Type = collResult.HeightType;
	coll->frontLeft.SplitFloor = collResult.SplitFloor;
	coll->frontLeft.SplitCeiling = collResult.SplitCeiling;

	if (coll->slopesAreWalls && (coll->frontLeft.Type == BIG_SLOPE || coll->frontLeft.Type == DIAGONAL) && coll->frontLeft.Floor < 0)
		coll->frontLeft.Floor = MAX_HEIGHT;
	else if (coll->slopesArePits && (coll->frontLeft.Type == BIG_SLOPE || coll->frontLeft.Type == DIAGONAL) && coll->frontLeft.Floor > 0)
		coll->frontLeft.Floor = STOP_SIZE;
	else if (coll->lavaIsPit && coll->frontLeft.Floor > 0 && collResult.BottomBlock->Flags.Death)
		coll->frontLeft.Floor = STOP_SIZE;

	x = xPos + xright;
	z = zPos + zright;

	collResult = GetCollisionResult(x, yTop, z, roomNumber);

	lrfLocation = GetRoom(LaraItem->location, x, yTop, z);
	height = GetFloorHeight(lrfLocation, x, z).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	lrcLocation = GetRoom(LaraItem->location, x, yTop - LaraItem->fallspeed, z);
	ceiling = GetCeilingHeight(lrcLocation, x, z).value_or(NO_HEIGHT);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->middleRight.Floor = height;
	coll->middleRight.Ceiling = ceiling;
	coll->middleRight.Type = collResult.HeightType;
	coll->middleRight.SplitFloor = collResult.SplitFloor;
	coll->middleRight.SplitCeiling = collResult.SplitCeiling;

	if (coll->slopesAreWalls && (coll->middleRight.Type == BIG_SLOPE || coll->middleRight.Type == DIAGONAL) && coll->middleRight.Floor < 0)
		coll->middleRight.Floor = MAX_HEIGHT;
	else if (coll->slopesArePits && (coll->middleRight.Type == BIG_SLOPE || coll->middleRight.Type == DIAGONAL) && coll->middleRight.Floor > 0)
		coll->middleRight.Floor = STOP_SIZE;
	else if (coll->lavaIsPit && coll->middleRight.Floor > 0 && collResult.BottomBlock->Flags.Death)
		coll->middleRight.Floor = STOP_SIZE;

	collResult = GetCollisionResult(x, yTop, z, topRoomNumber);

	tfLocation = GetRoom(tfLocation, x, yTop, z);
	height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		height -= yPos;

	tcLocation = GetRoom(tcLocation, x, yTop - LaraItem->fallspeed, z);
	ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->frontRight.Floor = height;
	coll->frontRight.Ceiling = ceiling;
	coll->frontRight.Type = collResult.HeightType;
	coll->frontRight.SplitFloor = collResult.SplitFloor;
	coll->frontRight.SplitCeiling = collResult.SplitCeiling;

	if (coll->slopesAreWalls && (coll->frontRight.Type == BIG_SLOPE || coll->frontRight.Type == DIAGONAL) && coll->frontRight.Floor < 0)
		coll->frontRight.Floor = MAX_HEIGHT;
	else if (coll->slopesArePits && (coll->frontRight.Type == BIG_SLOPE || coll->frontRight.Type == DIAGONAL) && coll->frontRight.Floor > 0)
		coll->frontRight.Floor = STOP_SIZE;
	else if (coll->lavaIsPit && coll->frontRight.Floor > 0 && collResult.BottomBlock->Flags.Death)
		coll->frontRight.Floor = STOP_SIZE;

	CollideSolidStatics(LaraItem, coll);

	if (coll->middle.Floor == NO_HEIGHT)
	{
		coll->shift.x = coll->old.x - xPos;
		coll->shift.y = coll->old.y - yPos;
		coll->shift.z = coll->old.z - zPos;
		coll->collType = CT_FRONT;
		return;
	}

	if (coll->middle.Floor - coll->middle.Ceiling <= 0)
	{
		coll->shift.x = coll->old.x - xPos;
		coll->shift.y = coll->old.y - yPos;
		coll->shift.z = coll->old.z - zPos;
		coll->collType = CT_CLAMP;
		return;
	}

	if (coll->middle.Ceiling >= 0)
	{
		coll->shift.y = coll->middle.Ceiling;
		coll->collType = CT_TOP;
		coll->hitCeiling = true;
	}

	if ((coll->front.Floor > coll->badPos)
		|| (coll->front.Floor < coll->badNeg)
		|| (coll->front.Ceiling > coll->badCeiling))
	{
		if ((coll->front.Type == DIAGONAL)
			|| (coll->front.Type == SPLIT_TRI))
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
				coll->shift.z = FindGridShift(zPos + zfront, zPos);
				break;

			case 1:
			case 3:
				coll->shift.x = FindGridShift(xPos + xfront, xPos);
				coll->shift.z = coll->old.z - zPos;
				break;

			}
		}

		coll->collType = CT_FRONT;
		return;
	}

	if (coll->front.Ceiling >= coll->badCeiling)
	{
		coll->shift.x = coll->old.x - xPos;
		coll->shift.y = coll->old.y - yPos;
		coll->shift.z = coll->old.z - zPos;
		coll->collType = CT_TOP_FRONT;
		return;
	}

	if (coll->middleLeft.Floor > coll->badPos ||
		coll->middleLeft.Floor < coll->badNeg ||
		coll->middleLeft.Ceiling > coll->badCeiling)
	{
		if (coll->middleLeft.Type == SPLIT_TRI && coll->middle.Type == SPLIT_TRI)
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
				coll->shift.x = FindGridShift(xPos + xleft, xPos + xfront);
				break;

			case 1:
			case 3:
				coll->shift.z = FindGridShift(zPos + zleft, zPos + zfront);
				break;
			}
		}

		if (coll->middleLeft.SplitFloor && coll->middleLeft.SplitFloor == coll->middle.SplitFloor)
		{
			int quarter = (unsigned short)(coll->facing) / ANGLE(90); // different from coll->quadrant!
			quarter %= 2;

			switch (coll->middleLeft.SplitFloor)
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

	if (coll->middleRight.Floor > coll->badPos ||
		coll->middleRight.Floor < coll->badNeg ||
		coll->middleRight.Ceiling > coll->badCeiling)
	{
		if (coll->middleRight.Type == SPLIT_TRI && coll->middle.Type == SPLIT_TRI)
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
				coll->shift.x = FindGridShift(xPos + xright, xPos + xfront);
				break;

			case 1:
			case 3:
				coll->shift.z = FindGridShift(zPos + zright, zPos + zfront);
				break;
			}
		}

		if (coll->middleRight.SplitFloor && coll->middleRight.SplitFloor == coll->middle.SplitFloor)
		{
			int quarter = (unsigned short)(coll->facing) / ANGLE(90); // different from coll->quadrant!
			quarter %= 2;

			switch (coll->middleRight.SplitFloor)
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
	int yTop = y - LARA_HEADROOM;
	int z = zPos;

	auto collResult = GetCollisionResult(x, yTop, z, roomNumber);
	auto topRoomNumber = collResult.RoomNumber;

	if (collResult.FloorHeight != NO_HEIGHT)
		collResult.FloorHeight -= yPos;

	int ceiling = GetCeiling(collResult.Block, x, yTop - LaraItem->fallspeed, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->middle.Ceiling = ceiling;
	coll->middle.Floor = collResult.FloorHeight;
	coll->middle.Type = collResult.HeightType;
	coll->middle.SplitFloor = collResult.SplitFloor;
	coll->middle.SplitCeiling = collResult.SplitCeiling;

	collResult = GetCollisionResult(x, LaraItem->pos.yPos, z, roomNumber);
	coll->tiltX = collResult.TiltX;
	coll->tiltZ = collResult.TiltZ;

	int xfront, xright, xleft, zfront, zright, zleft;

	switch (coll->quadrant)
	{
	case 0:
		xfront = phd_sin(coll->facing) * coll->radius;
		zfront = coll->radius;
		xleft = -(coll->radius);
		zleft = coll->radius;
		xright = coll->radius;
		zright = coll->radius;
		break;

	case 1:
		xfront = coll->radius;
		zfront = phd_cos(coll->facing) * coll->radius;
		xleft = coll->radius;
		zleft = coll->radius;
		xright = coll->radius;
		zright = -(coll->radius);
		break;

	case 2:
		xfront = phd_sin(coll->facing) * coll->radius;
		zfront = -coll->radius;
		xleft = coll->radius;
		zleft = -(coll->radius);
		xright = -(coll->radius);
		zright = -(coll->radius);
		break;

	case 3:
		xfront = -(coll->radius);
		zfront = phd_cos(coll->facing) * coll->radius;
		xleft = -(coll->radius);
		zleft = -(coll->radius);
		xright = -(coll->radius);
		zright = coll->radius;
		break;

	default:
		xleft = zleft = 0;
		xright = zright = 0;
		xfront = zfront = 0;
		break;
	}

	x = xfront + xPos;
	z = zfront + zPos;

	if (resetRoom)
		topRoomNumber = roomNumber;

	collResult = GetCollisionResult(x, yTop, z, topRoomNumber);
	if (collResult.FloorHeight != NO_HEIGHT)
		collResult.FloorHeight -= yPos;

	ceiling = GetCeiling(collResult.Block, x, yTop - LaraItem->fallspeed, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->front.Ceiling = ceiling;
	coll->front.Floor = collResult.FloorHeight;
	coll->front.Type = collResult.HeightType;
	coll->front.SplitFloor = collResult.SplitFloor;
	coll->front.SplitCeiling = collResult.SplitCeiling;

	collResult = GetCollisionResult(x + xfront, yTop, z + zfront, topRoomNumber);
	if (collResult.FloorHeight != NO_HEIGHT)
		collResult.FloorHeight -= yPos;

	if ((coll->slopesAreWalls)
		&& ((coll->front.Type == BIG_SLOPE) || (coll->front.Type == DIAGONAL))
		&& (coll->front.Floor < coll->middle.Floor)
		&& (collResult.FloorHeight < coll->front.Floor)
		&& (coll->front.Floor < 0))
	{
		coll->front.Floor = MAX_HEIGHT;
	}
	else if (coll->slopesArePits
		&& ((coll->front.Type == BIG_SLOPE) || (coll->front.Type == DIAGONAL))
		&& (coll->front.Floor > coll->middle.Floor))
	{
		coll->front.Floor = STOP_SIZE;
	}
	else if ((coll->lavaIsPit)
		&& (coll->front.Floor > 0)
		&& collResult.BottomBlock->Flags.Death)
	{
		coll->front.Floor = STOP_SIZE;
	}

	x = xPos + xleft;
	z = zPos + zleft;

	collResult = GetCollisionResult(x + xfront, yTop, z + zfront, roomNumber);
	if (collResult.FloorHeight != NO_HEIGHT)
		collResult.FloorHeight -= yPos;

	ceiling = GetCeiling(collResult.Block, x, yTop - LaraItem->fallspeed, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->middleLeft.Floor = collResult.FloorHeight;
	coll->middleLeft.Ceiling = ceiling;
	coll->middleLeft.Type = collResult.HeightType;
	coll->middleLeft.SplitFloor = collResult.SplitFloor;
	coll->middleLeft.SplitCeiling = collResult.SplitCeiling;

	if (coll->slopesAreWalls && (coll->middleLeft.Type == BIG_SLOPE || coll->middleLeft.Type == DIAGONAL) && coll->middleLeft.Floor < 0)
		coll->middleLeft.Floor = MAX_HEIGHT;
	else if (coll->slopesArePits && (coll->middleLeft.Type == BIG_SLOPE || coll->middleLeft.Type == DIAGONAL) && coll->middleLeft.Floor > 0)
		coll->middleLeft.Floor = STOP_SIZE;
	else if (coll->lavaIsPit && coll->middleLeft.Floor > 0 && collResult.BottomBlock->Flags.Death)
		coll->middleLeft.Floor = STOP_SIZE;

	collResult = GetCollisionResult(x, yTop, z, topRoomNumber);
	if (collResult.FloorHeight != NO_HEIGHT)
		collResult.FloorHeight -= yPos;

	ceiling = GetCeiling(collResult.Block, x, yTop - LaraItem->fallspeed, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->frontLeft.Floor = collResult.FloorHeight;
	coll->frontLeft.Ceiling = ceiling;
	coll->frontLeft.Type = collResult.HeightType;
	coll->frontLeft.SplitFloor = collResult.SplitFloor;
	coll->frontLeft.SplitCeiling = collResult.SplitCeiling;

	if (coll->slopesAreWalls && (coll->frontLeft.Type == BIG_SLOPE || coll->frontLeft.Type == DIAGONAL) && coll->frontLeft.Floor < 0)
		coll->frontLeft.Floor = MAX_HEIGHT;
	else if (coll->slopesArePits && (coll->frontLeft.Type == BIG_SLOPE || coll->frontLeft.Type == DIAGONAL) && coll->frontLeft.Floor > 0)
		coll->frontLeft.Floor = STOP_SIZE;
	else if (coll->lavaIsPit && coll->frontLeft.Floor > 0 && collResult.BottomBlock->Flags.Death)
		coll->frontLeft.Floor = STOP_SIZE;

	x = xPos + xright;
	z = zPos + zright;

	collResult = GetCollisionResult(x, yTop, z, roomNumber);
	if (collResult.FloorHeight != NO_HEIGHT)
		collResult.FloorHeight -= yPos;

	ceiling = GetCeiling(collResult.Block, x, yTop - LaraItem->fallspeed, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->middleRight.Floor = collResult.FloorHeight;
	coll->middleRight.Ceiling = ceiling;
	coll->middleRight.Type = collResult.HeightType;
	coll->middleRight.SplitFloor = collResult.SplitFloor;
	coll->middleRight.SplitCeiling = collResult.SplitCeiling;

	if (coll->slopesAreWalls && (coll->middleRight.Type == BIG_SLOPE || coll->middleRight.Type == DIAGONAL) && coll->middleRight.Floor < 0)
		coll->middleRight.Floor = MAX_HEIGHT;
	else if (coll->slopesArePits && (coll->middleRight.Type == BIG_SLOPE || coll->middleRight.Type == DIAGONAL) && coll->middleRight.Floor > 0)
		coll->middleRight.Floor = STOP_SIZE;
	else if (coll->lavaIsPit && coll->middleRight.Floor > 0 && collResult.BottomBlock->Flags.Death)
		coll->middleRight.Floor = STOP_SIZE;

	collResult = GetCollisionResult(x, yTop, z, topRoomNumber);
	if (collResult.FloorHeight != NO_HEIGHT)
		collResult.FloorHeight -= yPos;

	ceiling = GetCeiling(collResult.Block, x, yTop - LaraItem->fallspeed, z);
	if (ceiling != NO_HEIGHT)
		ceiling -= y;

	coll->frontRight.Floor = collResult.FloorHeight;
	coll->frontRight.Ceiling = ceiling;
	coll->frontRight.Type = collResult.HeightType;
	coll->frontRight.SplitFloor = collResult.SplitFloor;
	coll->frontRight.SplitCeiling = collResult.SplitCeiling;

	if (coll->slopesAreWalls && (coll->frontRight.Type == BIG_SLOPE || coll->frontRight.Type == DIAGONAL) && coll->frontRight.Floor < 0)
		coll->frontRight.Floor = MAX_HEIGHT;
	else if (coll->slopesArePits && (coll->frontRight.Type == BIG_SLOPE || coll->frontRight.Type == DIAGONAL) && coll->frontRight.Floor > 0)
		coll->frontRight.Floor = STOP_SIZE;
	else if (coll->lavaIsPit && coll->frontRight.Floor > 0 && collResult.BottomBlock->Flags.Death)
		coll->frontRight.Floor = STOP_SIZE;
	
	if (coll->middle.Floor == NO_HEIGHT)	 
	{
		coll->shift.x = coll->old.x - xPos;
		coll->shift.y = coll->old.y - yPos;
		coll->shift.z = coll->old.z - zPos;
		coll->collType = CT_FRONT;
		return;
	}

	if (coll->middle.Floor - coll->middle.Ceiling <= 0)
	{
		coll->shift.x = coll->old.x - xPos;
		coll->shift.y = coll->old.y - yPos;
		coll->shift.z = coll->old.z - zPos;
		coll->collType = CT_CLAMP;
		return;
	}

	if (coll->middle.Ceiling >= 0)
	{
		coll->shift.y = coll->middle.Ceiling;
		coll->collType = CT_TOP;
		coll->hitCeiling = true;
	}

	if ((coll->front.Floor > coll->badPos)
		|| (coll->front.Floor < coll->badNeg)
		|| (coll->front.Ceiling > coll->badCeiling))
	{
		if ((coll->front.Type == DIAGONAL)
			|| (coll->front.Type == SPLIT_TRI))
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
				coll->shift.z = FindGridShift(zPos + zfront, zPos);
				break;

			case 1:
			case 3:
				coll->shift.x = FindGridShift(xPos + xfront, xPos);
				coll->shift.z = coll->old.z - zPos;
				break;

			}
		}

		coll->collType = CT_FRONT;
		return;
	}

	if (coll->front.Ceiling >= coll->badCeiling)
	{
		coll->shift.x = coll->old.x - xPos;
		coll->shift.y = coll->old.y - yPos;
		coll->shift.z = coll->old.z - zPos;
		coll->collType = CT_TOP_FRONT;
		return;
	}

	if (coll->middleLeft.Floor > coll->badPos ||
		coll->middleLeft.Floor < coll->badNeg ||
		coll->middleLeft.Ceiling > coll->badCeiling)
	{
		if (coll->middleLeft.Type == SPLIT_TRI && coll->middle.Type == SPLIT_TRI)
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
				coll->shift.x = FindGridShift(xPos + xleft, xPos + xfront);
				break;

			case 1:
			case 3:
				coll->shift.z = FindGridShift(zPos + zleft, zPos + zfront);
				break;
			}
		}
		
		if (coll->middleLeft.SplitFloor && coll->middleLeft.SplitFloor == coll->middle.SplitFloor)
		{
			int quarter = (unsigned short)(coll->facing) / ANGLE(90); // different from coll->quadrant!
			quarter %= 2;

			switch (coll->middleLeft.SplitFloor)
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

	if (coll->middleRight.Floor > coll->badPos ||
		coll->middleRight.Floor < coll->badNeg ||
		coll->middleRight.Ceiling > coll->badCeiling)
	{
		if (coll->middleRight.Type == SPLIT_TRI && coll->middle.Type == SPLIT_TRI)
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
				coll->shift.x = FindGridShift(xPos + xright, xPos + xfront);
				break;

			case 1:
			case 3:
				coll->shift.z = FindGridShift(zPos + zright, zPos + zfront);
				break;
			}
		}
		
		if (coll->middleRight.SplitFloor && coll->middleRight.SplitFloor == coll->middle.SplitFloor)
		{
			int quarter = (unsigned short)(coll->facing) / ANGLE(90); // different from coll->quadrant!
			quarter %= 2;

			switch (coll->middleRight.SplitFloor)
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

void DoProjectileDynamics(short itemNumber, int x, int y, int z, int xv, int yv, int zv) // previously DoProperDetection
{
	int bs, yang;

	ITEM_INFO* item = &g_Level.Items[itemNumber];

	auto oldCollResult = GetCollisionResult(x, y, z, item->roomNumber);
	auto collResult = GetCollisionResult(item);

	if (item->pos.yPos >= collResult.FloorHeight)
	{
		bs = 0;

		if ((collResult.HeightType == BIG_SLOPE || collResult.HeightType == DIAGONAL) && oldCollResult.FloorHeight < collResult.FloorHeight)
		{
			yang = (long)((unsigned short)item->pos.yRot);
			if (collResult.TiltX < 0)
			{
				if (yang >= 0x8000)
					bs = 1;
			}
			else if (collResult.TiltX > 0)
			{
				if (yang <= 0x8000)
					bs = 1;
			}

			if (collResult.TiltZ < 0)
			{
				if (yang >= 0x4000 && yang <= 0xc000)
					bs = 1;
			}
			else if (collResult.TiltZ > 0)
			{
				if (yang <= 0x4000 || yang >= 0xc000)
					bs = 1;
			}
		}

		/* If last position of item was also below this floor height, we've hit a wall, else we've hit a floor */

		if (y > (collResult.FloorHeight + 32) && bs == 0 &&
			(((x / SECTOR(1)) != (item->pos.xPos / SECTOR(1))) ||
			((z / SECTOR(1)) != (item->pos.zPos / SECTOR(1)))))
		{
			// Need to know which direction the wall is.

			long	xs;

			if ((x & (~(WALL_SIZE - 1))) != (item->pos.xPos & (~(WALL_SIZE - 1))) &&	// X crossed boundary?
				(z & (~(WALL_SIZE - 1))) != (item->pos.zPos & (~(WALL_SIZE - 1))))	// Z crossed boundary as well?
			{
				if (abs(x - item->pos.xPos) < abs(z - item->pos.zPos))
					xs = 1;	// X has travelled the shortest, so (maybe) hit first. (Seems to work ok).
				else
					xs = 0;
			}
			else
				xs = 1;

			if ((x & (~(WALL_SIZE - 1))) != (item->pos.xPos & (~(WALL_SIZE - 1))) && xs)	// X crossed boundary?
			{
				if (xv <= 0)	// Hit angle = 0xc000.
					item->pos.yRot = 0x4000 + (0xc000 - item->pos.yRot);
				else			// Hit angle = 0x4000.
					item->pos.yRot = 0xc000 + (0x4000 - item->pos.yRot);
			}
			else		// Z crossed boundary.
				item->pos.yRot = 0x8000 - item->pos.yRot;

			item->speed /= 2;

			/* Put item back in its last position */
			item->pos.xPos = x;
			item->pos.yPos = y;
			item->pos.zPos = z;
		}
		else if (collResult.HeightType == BIG_SLOPE || collResult.HeightType == DIAGONAL) 	// Hit a steep slope?
		{
			// Need to know which direction the slope is.

			item->speed -= (item->speed / 4);

			if (collResult.TiltX < 0 && ((abs(collResult.TiltX)) - (abs(collResult.TiltZ)) >= 2))	// Hit angle = 0x4000
			{
				if (((unsigned short)item->pos.yRot) > 0x8000)
				{
					item->pos.yRot = 0x4000 + (0xc000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= collResult.TiltX * 2;
						if ((unsigned short)item->pos.yRot > 0x4000 && (unsigned short)item->pos.yRot < 0xc000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0x4000)
								item->pos.yRot = 0x4000;
						}
						else if ((unsigned short)item->pos.yRot < 0x4000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0x4000)
								item->pos.yRot = 0x4000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltX > 0 && ((abs(collResult.TiltX)) - (abs(collResult.TiltZ)) >= 2))	// Hit angle = 0xc000
			{
				if (((unsigned short)item->pos.yRot) < 0x8000)
				{
					item->pos.yRot = 0xc000 + (0x4000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += collResult.TiltX * 2;
						if ((unsigned short)item->pos.yRot > 0xc000 || (unsigned short)item->pos.yRot < 0x4000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0xc000)
								item->pos.yRot = 0xc000;
						}
						else if ((unsigned short)item->pos.yRot < 0xc000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0xc000)
								item->pos.yRot = 0xc000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltZ < 0 && ((abs(collResult.TiltZ)) - (abs(collResult.TiltX)) >= 2))	// Hit angle = 0
			{
				if (((unsigned short)item->pos.yRot) > 0x4000 && ((unsigned short)item->pos.yRot) < 0xc000)
				{
					item->pos.yRot = (0x8000 - item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= collResult.TiltZ * 2;

						if ((unsigned short)item->pos.yRot < 0x8000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot > 0xf000)
								item->pos.yRot = 0;
						}
						else if ((unsigned short)item->pos.yRot >= 0x8000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot < 0x1000)
								item->pos.yRot = 0;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltZ > 0 && ((abs(collResult.TiltZ)) - (abs(collResult.TiltX)) >= 2))	// Hit angle = 0x8000
			{
				if (((unsigned short)item->pos.yRot) > 0xc000 || ((unsigned short)item->pos.yRot) < 0x4000)
				{
					item->pos.yRot = (0x8000 - item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += collResult.TiltZ * 2;

						if ((unsigned short)item->pos.yRot > 0x8000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0x8000)
								item->pos.yRot = 0x8000;
						}
						else if ((unsigned short)item->pos.yRot < 0x8000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0x8000)
								item->pos.yRot = 0x8000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltX < 0 && collResult.TiltZ < 0)	// Hit angle = 0x2000
			{
				if (((unsigned short)item->pos.yRot) > 0x6000 && ((unsigned short)item->pos.yRot) < 0xe000)
				{
					item->pos.yRot = 0x2000 + (0xa000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += (-collResult.TiltX) + (-collResult.TiltZ);
						if ((unsigned short)item->pos.yRot > 0x2000 && (unsigned short)item->pos.yRot < 0xa000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0x2000)
								item->pos.yRot = 0x2000;
						}
						else if ((unsigned short)item->pos.yRot != 0x2000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0x2000)
								item->pos.yRot = 0x2000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltX < 0 && collResult.TiltZ > 0)	// Hit angle = 0x6000
			{
				if (((unsigned short)item->pos.yRot) > 0xa000 || ((unsigned short)item->pos.yRot) < 0x2000)
				{
					item->pos.yRot = 0x6000 + (0xe000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += (-collResult.TiltX) + collResult.TiltZ;
						if ((unsigned short)item->pos.yRot < 0xe000 && (unsigned short)item->pos.yRot > 0x6000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0x6000)
								item->pos.yRot = 0x6000;
						}
						else if ((unsigned short)item->pos.yRot != 0x6000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0x6000)
								item->pos.yRot = 0x6000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltX > 0 && collResult.TiltZ > 0)	// Hit angle = 0xa000
			{
				if (((unsigned short)item->pos.yRot) > 0xe000 || ((unsigned short)item->pos.yRot) < 0x6000)
				{
					item->pos.yRot = 0xa000 + (0x2000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += collResult.TiltX + collResult.TiltZ;
						if ((unsigned short)item->pos.yRot < 0x2000 || (unsigned short)item->pos.yRot > 0xa000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0xa000)
								item->pos.yRot = 0xa000;
						}
						else if ((unsigned short)item->pos.yRot != 0xa000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0xa000)
								item->pos.yRot = 0xa000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltX > 0 && collResult.TiltZ < 0)	// Hit angle = 0xe000
			{
				if (((unsigned short)item->pos.yRot) > 0x2000 && ((unsigned short)item->pos.yRot) < 0xa000)
				{
					item->pos.yRot = 0xe000 + (0x6000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += collResult.TiltX + (-collResult.TiltZ);
						if ((unsigned short)item->pos.yRot < 0x6000 || (unsigned short)item->pos.yRot > 0xe000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0xe000)
								item->pos.yRot = 0xe000;
						}
						else if ((unsigned short)item->pos.yRot != 0xe000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0xe000)
								item->pos.yRot = 0xe000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}

			/* Put item back in its last position */
			item->pos.xPos = x;
			item->pos.yPos = y;
			item->pos.zPos = z;
		}
		else
		{
			/* Hit the floor; bounce and slow down */
			if (item->fallspeed > 0)
			{
				if (item->fallspeed > 16)
				{
					if (item->objectNumber == ID_GRENADE)
						item->fallspeed = -(item->fallspeed - (item->fallspeed / 2));
					else
					{
						item->fallspeed = -(item->fallspeed / 2);
						if (item->fallspeed < -100)
							item->fallspeed = -100;
					}
				}
				else
				{
					/* Roll on floor */
					item->fallspeed = 0;
					if (item->objectNumber == ID_GRENADE)
					{
						item->requiredAnimState = 1;
						item->pos.xRot = 0;
						item->speed--;
					}
					else
						item->speed -= 3;

					if (item->speed < 0)
						item->speed = 0;
				}
			}
			item->pos.yPos = collResult.FloorHeight;
		}
	}
	else	// Check for on top of object.
	{
		if (yv >= 0)
		{
			oldCollResult = GetCollisionResult(item->pos.xPos, y, item->pos.zPos, item->roomNumber);
			collResult = GetCollisionResult(item);

			// Bounce off floor.

			// Removed weird OnObject global check from here which didnt make sense because OnObject
			// was always set to 0 by GetHeight() function which was called before the check.
			// Possibly a mistake or unfinished feature by Core? -- Lwmte, 27.08.21

			if (item->pos.yPos >= oldCollResult.FloorHeight) 
			{
				/* Hit the floor; bounce and slow down */
				if (item->fallspeed > 0)
				{
					if (item->fallspeed > 16)
					{
						if (item->objectNumber == ID_GRENADE)
							item->fallspeed = -(item->fallspeed - (item->fallspeed / 2));
						else
						{
							item->fallspeed = -(item->fallspeed / 4);
							if (item->fallspeed < -100)
								item->fallspeed = -100;
						}
					}
					else
					{
						/* Roll on floor */
						item->fallspeed = 0;
						if (item->objectNumber == ID_GRENADE)
						{
							item->requiredAnimState = 1;
							item->pos.xRot = 0;
							item->speed--;
						}
						else
							item->speed -= 3;

						if (item->speed < 0)
							item->speed = 0;
					}
				}
				item->pos.yPos = oldCollResult.FloorHeight;
			}
		}
		//		else
		{
			/* Bounce off ceiling */
			collResult = GetCollisionResult(item);

			if (item->pos.yPos < collResult.CeilingHeight)
			{
				if (y < collResult.CeilingHeight &&
					(((x / SECTOR(1)) != (item->pos.xPos / SECTOR(1))) ||
					((z / SECTOR(1)) != (item->pos.zPos / SECTOR(1)))))
				{
					// Need to know which direction the wall is.

					if ((x & (~(WALL_SIZE - 1))) != (item->pos.xPos & (~(WALL_SIZE - 1))))	// X crossed boundary?
					{
						if (xv <= 0)	// Hit angle = 0xc000.
							item->pos.yRot = 0x4000 + (0xc000 - item->pos.yRot);
						else			// Hit angle = 0x4000.
							item->pos.yRot = 0xc000 + (0x4000 - item->pos.yRot);
					}
					else		// Z crossed boundary.
					{
						item->pos.yRot = 0x8000 - item->pos.yRot;
					}

					if (item->objectNumber == ID_GRENADE)
						item->speed -= item->speed / 8;
					else
						item->speed /= 2;

					/* Put item back in its last position */
					item->pos.xPos = x;
					item->pos.yPos = y;
					item->pos.zPos = z;
				}
				else
					item->pos.yPos = collResult.CeilingHeight;

				if (item->fallspeed < 0)
					item->fallspeed = -item->fallspeed;
			}
		}
	}

	collResult = GetCollisionResult(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
	if (collResult.RoomNumber != item->roomNumber)
		ItemNewRoom(itemNumber, collResult.RoomNumber);
}

void DoObjectCollision(ITEM_INFO* l, COLL_INFO* coll) // previously LaraBaddieCollision
{
	ITEM_INFO* item;
	OBJECT_INFO* obj;

	l->hitStatus = false;
	coll->hitStatic = false;

	if (l == LaraItem)
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

			for (int j = 0; j < g_Level.Rooms[roomsToCheck[i]].mesh.size(); j++)
			{
				MESH_INFO* mesh = &g_Level.Rooms[roomsToCheck[i]].mesh[j];

				// Flag 0x01: collision
				// Flag 0x02: solid collision (processed in CollideSolidStatic)

				if ((mesh->flags & 1) && !(mesh->flags & 2))

				{
					int x = l->pos.xPos - mesh->x;
					int y = l->pos.yPos - mesh->y;
					int z = l->pos.zPos - mesh->z;

					if (x > -COLLISION_CHECK_DISTANCE && x < COLLISION_CHECK_DISTANCE && 
						y > -COLLISION_CHECK_DISTANCE && y < COLLISION_CHECK_DISTANCE && 
						z > -COLLISION_CHECK_DISTANCE && z < COLLISION_CHECK_DISTANCE)
					{
						if (TestBoundsCollideStatic(l, mesh, coll->radius))
						{
							coll->hitStatic = true;

							if (coll->enableBaddiePush)
								ItemPushStatic(l, mesh, coll);
							else
								break;
						}
					}
				}
			}
		}

		if (l == LaraItem && Lara.hitDirection == -1)
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

						if (ItemPushItem(item, l, coll, ((deadlyBits & 1) & coll->enableSpaz), 3) && (deadlyBits & 1))
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

// New function for rotating item along XZ slopes.
// (int radiusDivide) is for radiusZ, else the MaxZ is too high and cause rotation problem !
// Dont need to set a value in radiusDivide if you dont need it (radiusDivide is set to 1 by default).
// Warning: dont set it to 0 !!!!

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

static bool ItemCollide(int value, int radius)
{
	return value >= -radius && value <= radius;
}

static bool ItemInRange(int x, int z, int radius)
{
	return (SQUARE(x) + SQUARE(z)) <= SQUARE(radius);
}

bool ItemNearLara(PHD_3DPOS* pos, int radius)
{
	BOUNDING_BOX* bounds;
	GAME_VECTOR target;
	target.x = pos->xPos - LaraItem->pos.xPos;
	target.y = pos->yPos - LaraItem->pos.yPos;
	target.z = pos->zPos - LaraItem->pos.zPos;
	if (!ItemCollide(target.y, ITEM_RADIUS_YMAX))
		return false;
	if (!ItemCollide(target.x, radius) || !ItemCollide(target.z, radius))
		return false;
	if (!ItemInRange(target.x, target.z, radius))
		return false;

	bounds = GetBoundsAccurate(LaraItem);
	if (target.y >= bounds->Y1 && target.y <= (bounds->Y2 + LARA_RAD))
		return true;

	return false;
}

bool ItemNearTarget(PHD_3DPOS* src, ITEM_INFO* target, int radius)
{
	BOUNDING_BOX* bounds;
	PHD_VECTOR pos;
	pos.x = src->xPos - target->pos.xPos;
	pos.y = src->yPos - target->pos.yPos;
	pos.z = src->zPos - target->pos.zPos;
	if (!ItemCollide(pos.y, ITEM_RADIUS_YMAX))
		return false;
	if (!ItemCollide(pos.x, radius) || !ItemCollide(pos.z, radius))
		return false;
	if (!ItemInRange(pos.x, pos.z, radius))
		return false;

	bounds = GetBoundsAccurate(target);
	if (pos.y >= bounds->Y1 && pos.y <= bounds->Y2)
		return true;

	return false;
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