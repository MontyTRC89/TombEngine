#include "framework.h"
#include "control/los.h"
#include "collide.h"
#include "animation.h"
#include "Lara.h"
#include "items.h"
#include "effects/effects.h"
#include "sphere.h"
#include "misc.h"
#include "setup.h"
#include "Sound/sound.h"
#include "Specific/trmath.h"
#include "Specific/prng.h"
#include "room.h"
#include "Renderer11.h"

using std::vector;
using namespace TEN::Math::Random;
using namespace TEN::Floordata;
using namespace TEN::Renderer;

BOUNDING_BOX GlobalCollisionBounds;
ITEM_INFO* CollidedItems[MAX_COLLIDED_OBJECTS];
MESH_INFO* CollidedMeshes[MAX_COLLIDED_OBJECTS];

bool GetCollidedObjects(ITEM_INFO* collidingItem, int radius, bool onlyVisible, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int ignoreLara)
{
	ROOM_INFO* room;
	short roomsArray[255];
	short numRooms;
	short numItems = 0, numMeshes = 0;

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

				if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
					continue;

				if (collidingItem->pos.yPos + radius + CLICK(0.5f) < mesh->pos.yPos + staticMesh->collisionBox.Y1)
					continue;

				if (collidingItem->pos.yPos > mesh->pos.yPos + staticMesh->collisionBox.Y2)
					continue;

				auto s = phd_sin(mesh->pos.yRot);
				auto c = phd_cos(mesh->pos.yRot);
				auto rx = (collidingItem->pos.xPos - mesh->pos.xPos) * c - s * (collidingItem->pos.zPos - mesh->pos.zPos);
				auto rz = (collidingItem->pos.zPos - mesh->pos.zPos) * c + s * (collidingItem->pos.xPos - mesh->pos.xPos);

				if (radius + rx + CLICK(0.5f) < staticMesh->collisionBox.X1 || rx - radius - CLICK(0.5f) > staticMesh->collisionBox.X2)
					continue;

				if (radius + rz + CLICK(0.5f) < staticMesh->collisionBox.Z1 || rz - radius - CLICK(0.5f) > staticMesh->collisionBox.Z2)
					continue;

				collidedMeshes[numMeshes++] = mesh;
				if (!radius)
				{
					collidedItems[0] = NULL;
					return true;
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
					if (item->objectNumber == ID_UPV && item->hitPoints == 1)
					{
						itemNumber = item->nextItem;
						continue;
					}
					if (item->objectNumber == ID_BIGGUN && item->hitPoints == 1)
					{
						itemNumber = item->nextItem;
						continue;
					}
					/*we need a better system*/

					int dx = collidingItem->pos.xPos - item->pos.xPos;
					int dy = collidingItem->pos.yPos - item->pos.yPos;
					int dz = collidingItem->pos.zPos - item->pos.zPos;

					ANIM_FRAME* framePtr = GetBestFrame(item);

					if (dx >= -2048	&& dx <= 2048 && 
						dy >= -2048	&& dy <= 2048 && 
						dz >= -2048	&& dz <= 2048 && 
						collidingItem->pos.yPos + radius + 128 >= item->pos.yPos + framePtr->boundingBox.Y1 && 
						collidingItem->pos.yPos - radius - 128 <= item->pos.yPos + framePtr->boundingBox.Y2)
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
									return true;
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
	coll->HitTallObject = false;

	for (auto i : CollectConnectedRooms(item->roomNumber))
	{
		for (int j = 0; j < g_Level.Rooms[i].mesh.size(); j++)
		{
			auto mesh = &g_Level.Rooms[i].mesh[j];

			// Only process meshes which are visible and solid
			if ((mesh->flags & StaticMeshFlags::SM_VISIBLE) && (mesh->flags & StaticMeshFlags::SM_SOLID))
			{
				if (phd_Distance(&item->pos, &mesh->pos) < COLLISION_CHECK_DISTANCE)
				{
					auto stInfo = StaticObjects[mesh->staticNumber];
					if (CollideSolidBounds(item, stInfo.collisionBox, mesh->pos, coll))
						coll->HitStatic = true;
				}
			}
		}
	}
}

bool CollideSolidBounds(ITEM_INFO* item, BOUNDING_BOX box, PHD_3DPOS pos, COLL_INFO* coll)
{
	bool result = false;

	// Get DX static bounds in global coords
	auto staticBounds = TO_DX_BBOX(pos, &box);

	// Get local TR bounds and DX item bounds in global coords
	auto itemBBox = GetBoundsAccurate(item);
	auto itemBounds = TO_DX_BBOX(item->pos, itemBBox);

	// Extend bounds a bit for visual testing
	itemBounds.Extents = itemBounds.Extents + Vector3(WALL_SIZE);

	// Filter out any further checks if static isn't nearby
	if (!staticBounds.Intersects(itemBounds))
		return false;

	// Bring back extents
	itemBounds.Extents = itemBounds.Extents - Vector3(WALL_SIZE);

	// Draw static bounds
	g_Renderer.addDebugBox(staticBounds, Vector4(1, 0.3, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	// Calculate horizontal item coll bounds according to radius
	BOUNDING_BOX collBox;
	collBox.X1 = -coll->Setup.Radius;
	collBox.X2 =  coll->Setup.Radius;
	collBox.Z1 = -coll->Setup.Radius;
	collBox.Z2 =  coll->Setup.Radius;

	// Calculate vertical item coll bounds according to either height (land mode) or precise bounds (water mode).
	// Water mode needs special processing because height calc in original engines is inconsistent in such cases.
	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		collBox.Y1 = itemBBox->Y1;
		collBox.Y2 = itemBBox->Y2;
	}
	else
	{
		collBox.Y1 = -coll->Setup.Height;
		collBox.Y2 = 0;
	}

	// Get and test DX item coll bounds
	auto collBounds = TO_DX_BBOX(PHD_3DPOS(item->pos.xPos, item->pos.yPos, item->pos.zPos), &collBox);
	bool intersects = staticBounds.Intersects(collBounds);

	// Draw item coll bounds
	g_Renderer.addDebugBox(collBounds, intersects ? Vector4(1, 0, 0, 1) : Vector4(0, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	// Decompose static bounds into top/bottom plane vertices
	Vector3 corners[8];
	staticBounds.GetCorners(corners);
	Vector3 planeVertices[4][3] = 
	{ 
		{ corners[0], corners[4], corners[1] }, 
		{ corners[5], corners[4], corners[1] }, 
		{ corners[3], corners[6], corners[7] }, 
		{ corners[3], corners[6], corners[2] } 
	};

	// Determine collision box vertical dimensions
	auto height = collBox.Y2 - collBox.Y1;
	auto center = item->pos.yPos - height / 2;

	// Do a series of angular tests with 90 degree steps to determine top/bottom collision.

	int closestPlane = -1;
	Ray closestRay;
	auto minDistance = std::numeric_limits<float>::max();

	for (int i = 0; i < 4; i++)
	{
		// Calculate ray direction
		auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(item->pos.yRot), TO_RAD(item->pos.xRot + (ANGLE(90 * i))), 0);
		auto mxT = Matrix::CreateTranslation(Vector3::UnitY);
		auto direction = (mxT * mxR).Translation();

		// Make a ray and do ray tests against all decomposed planes
		auto ray = Ray(collBounds.Center, direction);

		// Determine if top/bottom planes are closest ones or not
		for (int p = 0; p < 4; p++)
		{
			// No plane intersection, quickly discard
			float d = 0.0f;
			if (!ray.Intersects(planeVertices[p][0], planeVertices[p][1], planeVertices[p][2], d))
				continue;

			// Process plane intersection only if distance is smaller
			// than already found minimum
			if (d < minDistance)
			{
				closestRay = ray;
				closestPlane = p;
				minDistance  = d;
			}
		}
	}

	if (closestPlane != -1) // Top/bottom plane found
	{
		auto bottom = closestPlane >= 2;
		auto yPoint = abs((closestRay.direction * minDistance).y);
		auto distanceToVerticalPlane = height / 2 - yPoint;

		// Correct position according to top/bottom bounds, if collided and plane is nearby
		if (intersects && minDistance < height)
		{
			if (bottom)
			{
				// HACK: additionally subtract 2 from bottom plane, or else false positives may occur.
				item->pos.yPos += distanceToVerticalPlane + 2;
				coll->CollisionType = CT_TOP;
			}
			else
			{
				// Set collision type only if dry room (in water rooms it causes stucking)
				item->pos.yPos -= distanceToVerticalPlane;
				coll->CollisionType = (g_Level.Rooms[item->roomNumber].flags & 1) ? coll->CollisionType : CT_CLAMP;
			}

			result = true;
		}

		if (bottom && coll->Middle.Ceiling < distanceToVerticalPlane)
			coll->Middle.Ceiling = distanceToVerticalPlane;
	}

	// If no actual intersection occured, stop testing.
	if (!intersects)
		return false;

	// Check if bounds still collide after top/bottom position correction
	if (!staticBounds.Intersects(TO_DX_BBOX(PHD_3DPOS(item->pos.xPos, item->pos.yPos, item->pos.zPos), &collBox)))
		return result;

	// Determine identity rotation/distance
	auto distance = Vector3(item->pos.xPos, item->pos.yPos, item->pos.zPos) - Vector3(pos.xPos, pos.yPos, pos.zPos);
	auto c = phd_cos(pos.yRot);
	auto s = phd_sin(pos.yRot);

	// Rotate item to collision bounds identity
	auto x = round(distance.x * c - distance.z * s) + pos.xPos;
	auto y = item->pos.yPos;
	auto z = round(distance.x * s + distance.z * c) + pos.zPos;
	
	// Determine identity static collision bounds
	auto XMin = pos.xPos + box.X1;
	auto XMax = pos.xPos + box.X2;
	auto YMin = pos.yPos + box.Y1;
	auto YMax = pos.yPos + box.Y2;
	auto ZMin = pos.zPos + box.Z1;
	auto ZMax = pos.zPos + box.Z2;

	// Determine item collision bounds
	auto inXMin = x + collBox.X1;
	auto inXMax = x + collBox.X2;
	auto inYMin = y + collBox.Y1;
	auto inYMax = y + collBox.Y2;
	auto inZMin = z + collBox.Z1;
	auto inZMax = z + collBox.Z2;

	// Don't calculate shifts if not in bounds
	if (inXMax <= XMin || inXMin >= XMax ||
		inYMax <= YMin || inYMin >= YMax ||
		inZMax <= ZMin || inZMin >= ZMax)
		return result;
	
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
	distance = Vector3(coll->Setup.OldPosition.x, coll->Setup.OldPosition.y, coll->Setup.OldPosition.z) - Vector3(pos.xPos, pos.yPos, pos.zPos);
	auto ox = round(distance.x * c - distance.z * s) + pos.xPos;
	auto oz = round(distance.x * s + distance.z * c) + pos.zPos;

	// Calculate collisison type based on identity rotation
	switch (GetQuadrant(coll->Setup.ForwardAngle - pos.yRot))
	{
	case NORTH:
		if (rawShift.x > coll->Setup.Radius || rawShift.x < -coll->Setup.Radius)
		{
			coll->Shift.z = rawShift.z;
			coll->Shift.x = ox - x;
			coll->CollisionType = CT_FRONT;
		}
		else if (rawShift.x > 0 && rawShift.x <= coll->Setup.Radius)
		{
			coll->Shift.x = rawShift.x;
			coll->Shift.z = 0;
			coll->CollisionType = CT_LEFT;
		}
		else if (rawShift.x < 0 && rawShift.x >= -coll->Setup.Radius)
		{
			coll->Shift.x = rawShift.x;
			coll->Shift.z = 0;
			coll->CollisionType = CT_RIGHT;
		}
		break;

	case SOUTH:
		if (rawShift.x > coll->Setup.Radius || rawShift.x < -coll->Setup.Radius)
		{
			coll->Shift.z = rawShift.z;
			coll->Shift.x = ox - x;
			coll->CollisionType = CT_FRONT;
		}
		else if (rawShift.x > 0 && rawShift.x <= coll->Setup.Radius)
		{
			coll->Shift.x = rawShift.x;
			coll->Shift.z = 0;
			coll->CollisionType = CT_RIGHT;
		}
		else if (rawShift.x < 0 && rawShift.x >= -coll->Setup.Radius)
		{
			coll->Shift.x = rawShift.x;
			coll->Shift.z = 0;
			coll->CollisionType = CT_LEFT;
		}
		break;

	case EAST:
		if (rawShift.z > coll->Setup.Radius || rawShift.z < -coll->Setup.Radius)
		{
			coll->Shift.x = rawShift.x;
			coll->Shift.z = oz - z;
			coll->CollisionType = CT_FRONT;
		}
		else if (rawShift.z > 0 && rawShift.z <= coll->Setup.Radius)
		{
			coll->Shift.z = rawShift.z;
			coll->Shift.x = 0;
			coll->CollisionType = CT_RIGHT;
		}
		else if (rawShift.z < 0 && rawShift.z >= -coll->Setup.Radius)
		{
			coll->Shift.z = rawShift.z;
			coll->Shift.x = 0;
			coll->CollisionType = CT_LEFT;
		}
		break;

	case WEST:
		if (rawShift.z > coll->Setup.Radius || rawShift.z < -coll->Setup.Radius)
		{
			coll->Shift.x = rawShift.x;
			coll->Shift.z = oz - z;
			coll->CollisionType = CT_FRONT;
		}
		else if (rawShift.z > 0 && rawShift.z <= coll->Setup.Radius)
		{
			coll->Shift.z = rawShift.z;
			coll->Shift.x = 0;
			coll->CollisionType = CT_LEFT;
		}
		else if (rawShift.z < 0 && rawShift.z >= -coll->Setup.Radius)
		{
			coll->Shift.z = rawShift.z;
			coll->Shift.x = 0;
			coll->CollisionType = CT_RIGHT;
		}
		break;
	}

	// Determine final shifts rotation/distance
	distance = Vector3(x + coll->Shift.x, y, z + coll->Shift.z) - Vector3(pos.xPos, pos.yPos, pos.zPos);
	c = phd_cos(-pos.yRot);
	s = phd_sin(-pos.yRot);

	// Calculate final shifts rotation/distance
	coll->Shift.x = (round(distance.x * c - distance.z * s) + pos.xPos) - item->pos.xPos;
	coll->Shift.z = (round(distance.x * s + distance.z * c) + pos.zPos) - item->pos.zPos;

	if (coll->Shift.x == 0 && coll->Shift.z == 0)
		coll->CollisionType = CT_NONE; // Paranoid

	// Set splat state flag if item is Lara and bounds are taller than Lara's headroom
	if (item == LaraItem && coll->CollisionType == CT_FRONT)
		coll->HitTallObject = (YMin <= inYMin + LARA_HEADROOM);

	return true;
}

bool TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll)
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

	if (x < GlobalCollisionBounds.X1 - coll->Setup.Radius ||
		x > GlobalCollisionBounds.X2 + coll->Setup.Radius ||
		z < GlobalCollisionBounds.Z1 - coll->Setup.Radius ||
		z > GlobalCollisionBounds.Z2 + coll->Setup.Radius)
		return false;

	return true;
}

void TrapCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->status == ITEM_ACTIVE)
	{
		if (!TestBoundsCollide(item, l, coll->Setup.Radius))
			return;

		TestCollision(item, l);
	}
	else if (item->status != ITEM_INVISIBLE)
		ObjectCollision(itemNumber, l, coll);
}

void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll)
{
	auto bounds = GetBoundsAccurate(item);
	auto height = abs(bounds->Y2 + bounds->Y1);

	for (int i = 0; i < 3; i++)
	{
		auto s = (i != 1) ? phd_sin(coll->Setup.ForwardAngle + ANGLE((i * 90) - 90)) : 0;
		auto c = (i != 1) ? phd_cos(coll->Setup.ForwardAngle + ANGLE((i * 90) - 90)) : 0;

		auto x = item->pos.xPos + (s * (coll->Setup.Radius));
		auto y = item->pos.yPos - height - STEP_SIZE;
		auto z = item->pos.zPos + (c * (coll->Setup.Radius));

		auto origin = Vector3(x, y, z);
		auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(coll->Setup.ForwardAngle), 0, 0);
		auto direction = (Matrix::CreateTranslation(Vector3::UnitZ) * mxR).Translation();

		// g_Renderer.addDebugSphere(origin, 16, Vector4::One, RENDERER_DEBUG_PAGE::DIMENSION_STATS);

		for (auto i : CollectConnectedRooms(item->roomNumber))
		{
			short itemNumber = g_Level.Rooms[i].itemNumber;
			while (itemNumber != NO_ITEM)
			{
				auto item2 = &g_Level.Items[itemNumber];
				auto obj = &Objects[item2->objectNumber];

				if (obj->isPickup || obj->collision == nullptr || !item2->collidable || item2->status == ITEM_INVISIBLE)
				{
					itemNumber = item2->nextItem;
					continue;
				}

				if (phd_Distance(&item->pos, &item2->pos) < COLLISION_CHECK_DISTANCE)
				{
					auto box = TO_DX_BBOX(item2->pos, GetBoundsAccurate(item2));
					float dist;

					if (box.Intersects(origin, direction, dist) && dist < coll->Setup.Radius * 2)
					{
						coll->HitStatic = true;
						return;
					}
				}

				itemNumber = item2->nextItem;
			}

			for (int j = 0; j < g_Level.Rooms[i].mesh.size(); j++)
			{
				auto mesh = &g_Level.Rooms[i].mesh[j];

				if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
					continue;

				if (phd_Distance(&item->pos, &mesh->pos) < COLLISION_CHECK_DISTANCE)
				{
					auto box = TO_DX_BBOX(mesh->pos, &StaticObjects[mesh->staticNumber].collisionBox);
					float dist;

					if (box.Intersects(origin, direction, dist) && dist < coll->Setup.Radius * 2)
					{
						coll->HitStatic = true;
						return;
					}
				}
			}
		}
	}
}

void ShiftItem(ITEM_INFO* item, COLL_INFO* coll)
{
	item->pos.xPos += coll->Shift.x;
	item->pos.yPos += coll->Shift.y;
	item->pos.zPos += coll->Shift.z;
	coll->Shift.z = 0;
	coll->Shift.y = 0;
	coll->Shift.x = 0;
}

void MoveItem(ITEM_INFO* item, short angle, int x, int y)
{
	if (!x && !y)
		return;

	if (x != 0)
	{
		auto s = phd_sin(angle);
		auto c = phd_cos(angle);
		item->pos.xPos += round(x * s);
		item->pos.zPos += round(x * c);
	}

	if (y != 0)
	{

		auto s = phd_sin(angle + ANGLE(90));
		auto c = phd_cos(angle + ANGLE(90));
		item->pos.xPos += round(y * s);
		item->pos.zPos += round(y * c);
	}
}

void SnapItemToLedge(ITEM_INFO* item, COLL_INFO* coll, float offsetMultiplier)
{
	item->pos.xRot = 0;
	item->pos.yRot = coll->NearestLedgeAngle;
	item->pos.zRot = 0;
	item->pos.xPos += round(phd_sin(coll->NearestLedgeAngle) * (coll->NearestLedgeDistance + (coll->Setup.Radius * offsetMultiplier)));
	item->pos.zPos += round(phd_cos(coll->NearestLedgeAngle) * (coll->NearestLedgeDistance + (coll->Setup.Radius * offsetMultiplier)));
}

void SnapItemToLedge(ITEM_INFO* item, COLL_INFO* coll, short angle, float offsetMultiplier)
{
	auto backup = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = angle;

	float dist;
	auto ang = GetNearestLedgeAngle(item, coll, dist);

	coll->Setup.ForwardAngle = backup;

	item->pos.xRot = 0;
	item->pos.yRot = ang;
	item->pos.zRot = 0;
	item->pos.xPos += round(phd_sin(ang) * (dist + (coll->Setup.Radius * offsetMultiplier)));
	item->pos.zPos += round(phd_cos(ang) * (dist + (coll->Setup.Radius * offsetMultiplier)));
}

void SnapItemToGrid(ITEM_INFO* item, COLL_INFO* coll)
{
	SnapItemToLedge(item, coll);

	int dir = (unsigned short)(item->pos.yRot + ANGLE(45)) / ANGLE(90);

	switch (dir)
	{
	case NORTH:
		item->pos.zPos = (item->pos.zPos | (WALL_SIZE - 1)) - coll->Setup.Radius;
		break;
	case EAST:
		item->pos.xPos = (item->pos.xPos | (WALL_SIZE - 1)) - coll->Setup.Radius;
		break;
	case SOUTH:
		item->pos.zPos = (item->pos.zPos & ~(WALL_SIZE - 1)) + coll->Setup.Radius;
		break;
	case WEST:
		item->pos.xPos = (item->pos.xPos & ~(WALL_SIZE - 1)) + coll->Setup.Radius;
		break;
	}
}

bool SnapAndTestItemAtNextCornerPosition(ITEM_INFO* item, COLL_INFO* coll, float angle, bool outer)
{
	// Determine real turning angle
	auto turnAngle = outer ? angle : -angle;

	// Determine collision box anchor point and rotate collision box around this anchor point.
	// Then determine new test position from centerpoint of new collision box position.

	// Push back item a bit to compensate for possible edge ledge cases
	item->pos.xPos -= round((coll->Setup.Radius * (outer ? -0.2f : 0.2f)) * phd_sin(item->pos.yRot));
	item->pos.zPos -= round((coll->Setup.Radius * (outer ? -0.2f : 0.2f)) * phd_cos(item->pos.yRot));

	// Move item at the distance of full collision diameter to movement direction 
	item->pos.xPos += round((coll->Setup.Radius * 2) * phd_sin(Lara.moveAngle));
	item->pos.zPos += round((coll->Setup.Radius * 2) * phd_cos(Lara.moveAngle));

	// Determine anchor point
	auto s = phd_sin(item->pos.yRot);
	auto c = phd_cos(item->pos.yRot);
	auto cX = item->pos.xPos + round(coll->Setup.Radius * s);
	auto cZ = item->pos.zPos + round(coll->Setup.Radius * c);
	cX += (coll->Setup.Radius * phd_sin(item->pos.yRot + ANGLE(90.0F * -std::copysign(1.0f, angle))));
	cZ += (coll->Setup.Radius * phd_cos(item->pos.yRot + ANGLE(90.0F * -std::copysign(1.0f, angle))));

	// Determine distance from anchor point to new item position
	auto dist = Vector2(item->pos.xPos, item->pos.zPos) - Vector2(cX, cZ);
	s = phd_sin(ANGLE(turnAngle));
	c = phd_cos(ANGLE(turnAngle));

	// Shift item to a new anchor point
	item->pos.xPos = dist.x * c - dist.y * s + cX;
	item->pos.zPos = dist.x * s + dist.y * c + cZ;

	// Virtually rotate item to new angle and snap to nearest ledge, if any.
	short newAngle = item->pos.yRot - ANGLE(turnAngle);
	item->pos.yRot = newAngle;
	SnapItemToLedge(item, coll, item->pos.yRot);

	return newAngle == item->pos.yRot;
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

bool TestBoundsCollideStatic(ITEM_INFO* item, MESH_INFO* mesh, int radius)
{
	auto bounds = StaticObjects[mesh->staticNumber].collisionBox;

	if (!(bounds.Z2 != 0 || bounds.Z1 != 0 || bounds.X1 != 0 || bounds.X2 != 0 || bounds.Y1 != 0 || bounds.Y2 != 0))
		return false;

	ANIM_FRAME* frame = GetBestFrame(item);
	if (mesh->pos.yPos + bounds.Y2 <= item->pos.yPos + frame->boundingBox.Y1)
		return false;

	if (mesh->pos.yPos + bounds.Y1 >= item->pos.yPos + frame->boundingBox.Y2)
		return false;

	float c, s;
	int x, z, dx, dz;

	c = phd_cos(mesh->pos.yRot);
	s = phd_sin(mesh->pos.yRot);
	x = item->pos.xPos - mesh->pos.xPos;
	z = item->pos.zPos - mesh->pos.zPos;
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

bool ItemPushStatic(ITEM_INFO* item, MESH_INFO* mesh, COLL_INFO* coll) // previously ItemPushLaraStatic
{
	auto bounds = StaticObjects[mesh->staticNumber].collisionBox;

	auto c = phd_cos(mesh->pos.yRot);
	auto s = phd_sin(mesh->pos.yRot);

	auto dx = item->pos.xPos - mesh->pos.xPos;
	auto dz = item->pos.zPos - mesh->pos.zPos;
	auto rx = c * dx - s * dz;
	auto rz = c * dz + s * dx;
	auto minX = bounds.X1 - coll->Setup.Radius;
	auto maxX = bounds.X2 + coll->Setup.Radius;
	auto minZ = bounds.Z1 - coll->Setup.Radius;
	auto maxZ = bounds.Z2 + coll->Setup.Radius;
	
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

	item->pos.xPos = mesh->pos.xPos + c * rx + s * rz;
	item->pos.zPos = mesh->pos.zPos + c * rz - s * rx;
	
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;

	auto oldFacing = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = phd_atan(item->pos.zPos - coll->Setup.OldPosition.z, item->pos.xPos - coll->Setup.OldPosition.x);

	GetCollisionInfo(coll, item);

	coll->Setup.ForwardAngle = oldFacing;

	if (coll->CollisionType == CT_NONE)
	{
		coll->Setup.OldPosition.x = item->pos.xPos;
		coll->Setup.OldPosition.y = item->pos.yPos;
		coll->Setup.OldPosition.z = item->pos.zPos;

		UpdateItemRoom(item, -10);
	}
	else
	{
		item->pos.xPos = coll->Setup.OldPosition.x;
		item->pos.zPos = coll->Setup.OldPosition.z;
	}

	if (item == LaraItem && Lara.isMoving && Lara.moveCount > 15)
	{
		Lara.isMoving = false;
		Lara.gunStatus = LG_NO_ARMS;
	}

	return true;
}

bool ItemPushItem(ITEM_INFO* item, ITEM_INFO* item2, COLL_INFO* coll, bool spazon, char bigpush) // previously ItemPushLara
{
	// Get item's rotation
	auto c = phd_cos(item->pos.yRot); 
	auto s = phd_sin(item->pos.yRot);

	// Get vector from item to Lara
	int dx = item2->pos.xPos - item->pos.xPos;
	int dz = item2->pos.zPos - item->pos.zPos;

	// Rotate Lara vector into item frame
	int rx = c * dx - s * dz; 
	int rz = c * dz + s * dx;

	BOUNDING_BOX* bounds;
	if (bigpush & 2)
		bounds = &GlobalCollisionBounds;
	else
		bounds = (BOUNDING_BOX*)GetBestFrame(item);

	int minX = bounds->X1;
	int maxX = bounds->X2;
	int minZ = bounds->Z1;
	int maxZ = bounds->Z2;

	if (bigpush & 1)
	{
		minX -= coll->Setup.Radius;
		maxX += coll->Setup.Radius;
		minZ -= coll->Setup.Radius;
		maxZ += coll->Setup.Radius;
	}

	// Big enemies
	if (abs(dx) > 4608
	||  abs(dz) > 4608
	||  rx <= minX
	||  rx >= maxX
	||  rz <= minZ
	||  rz >= maxZ)
		return false;

	int left = rx - minX;
	int top = maxZ - rz;
	int bottom = rz - minZ;
	int right = maxX - rx;

	if (right <= left && right <= top && right <= bottom)
		rx += right;
	else if (left <= right && left <= top && left <= bottom)
		rx -= left;
	else if (top <= left && top <= right && top <= bottom)
		rz += top;
	else
		rz -= bottom;

	item2->pos.xPos = item->pos.xPos + c * rx + s * rz;
	item2->pos.zPos = item->pos.zPos + c * rz - s * rx;

	auto lara = item2->data.is<LaraInfo*>() ? (LaraInfo*&)item2->data : nullptr;

	if (lara != nullptr && spazon && bounds->Y2 - bounds->Y1 > CLICK(1))
	{
		rx = (bounds->X1 + bounds->X2) / 2;	 
		rz = (bounds->Z1 + bounds->Z2) / 2;

		dx -= c * rx + s * rz;
		dz -= c * rz - s * rx;

		lara->hitDirection = (item2->pos.yRot - phd_atan(dz, dz) - ANGLE(135)) / 16384;

		if ((!lara->hitFrame) && (!lara->spazEffectCount))
		{
				SoundEffect(SFX_TR4_LARA_INJURY, &item2->pos, 0);
				lara->spazEffectCount = GenerateInt(15, 35);
		}

		if (lara->spazEffectCount)
			lara->spazEffectCount--;

		lara->hitFrame++;
		if (lara->hitFrame > 34) 
			lara->hitFrame = 34;
	}

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;

	auto facing = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = phd_atan(item2->pos.zPos - coll->Setup.OldPosition.z, item2->pos.xPos - coll->Setup.OldPosition.x);

	GetCollisionInfo(coll, item2);

	coll->Setup.ForwardAngle = facing;

	if (coll->CollisionType == CT_NONE)
	{
		coll->Setup.OldPosition.x = item2->pos.xPos;
		coll->Setup.OldPosition.y = item2->pos.yPos;
		coll->Setup.OldPosition.z = item2->pos.zPos;

		// Commented because causes Lara to jump out of the water if she touches an object on the surface. re: "kayak bug"
		// UpdateItemRoom(item2, -10);
	}
	else
	{
		item2->pos.xPos = coll->Setup.OldPosition.x;
		item2->pos.zPos = coll->Setup.OldPosition.z;
	}

	if (lara != nullptr && lara->moveCount > 15)
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

void ObjectCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TestBoundsCollide(item, l, coll->Setup.Radius))
	{
		if (TestCollision(item, l))
		{
			if (coll->Setup.EnableObjectPush)
				ItemPushItem(item, l, coll, false, true);
		}
	}
}

void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l)
{
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

bool TestLaraPosition(OBJECT_COLLISION_BOUNDS* bounds, ITEM_INFO* item, ITEM_INFO* l)
{
	short xRotRel = l->pos.xRot - item->pos.xRot;
	short yRotRel = l->pos.yRot - item->pos.yRot;
	short zRotRel = l->pos.zRot - item->pos.zRot;

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

	int x = l->pos.xPos - item->pos.xPos; 
	int y = l->pos.yPos - item->pos.yPos;
	int z = l->pos.zPos - item->pos.zPos;

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

	if (pos.x < bounds->boundingBox.X1 || pos.x > bounds->boundingBox.X2 ||
		pos.y < bounds->boundingBox.Y1 || pos.y > bounds->boundingBox.Y2 ||
		pos.z < bounds->boundingBox.Z1 || pos.z > bounds->boundingBox.Z2)
		return false;

	return true;
}

bool Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angAdd)
{
	int x = dest->xPos - src->xPos;
	int y = dest->yPos - src->yPos;
	int z = dest->zPos - src->zPos;
	int distance = sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));

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
			int direction = (GetQuadrant(angle) - GetQuadrant(dest->yRot)) & 3;
			
			switch (direction)
			{
				case 0:
					SetAnimation(LaraItem, LA_SIDESTEP_LEFT);
					Lara.gunStatus = LG_HANDS_BUSY;
					break;

				case 1:
					SetAnimation(LaraItem, LA_WALK);
					Lara.gunStatus = LG_HANDS_BUSY;
					break;

				case 2:
					SetAnimation(LaraItem, LA_SIDESTEP_RIGHT);
					Lara.gunStatus = LG_HANDS_BUSY;
					break;

				case 3:
				default:
					SetAnimation(LaraItem, LA_WALK_BACK);
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

bool MoveLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l)
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

bool TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int radius)
{
	auto bounds = (BOUNDING_BOX*)GetBestFrame(item);
	auto laraBounds = (BOUNDING_BOX*)GetBestFrame(l);

	if (item->pos.yPos + bounds->Y2 <= l->pos.yPos + laraBounds->Y1)
		return false;

	if (item->pos.yPos + bounds->Y1 >= l->pos.yPos + laraBounds->Y2)
		return false;

	auto c = phd_cos(item->pos.yRot);
	auto s = phd_sin(item->pos.yRot);
	int x = l->pos.xPos - item->pos.xPos;
	int z = l->pos.zPos - item->pos.zPos;
	int dx = c * x - s * z;
	int dz = c * z + s * x;

	if (dx >= bounds->X1 - radius &&
		dx <= radius + bounds->X2 &&
		dz >= bounds->Z1 - radius && 
		dz <= radius + bounds->Z2)
	{
		return true;
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
		if (TestBoundsCollide(item, l, coll->Setup.Radius))
		{
			if (TestCollision(item, l))
			{
				if (coll->Setup.EnableObjectPush || Lara.waterStatus == LW_UNDERWATER || Lara.waterStatus == LW_SURFACE)
				{
					ItemPushItem(item, l, coll, coll->Setup.EnableSpaz, 0);
				}
				else if (coll->Setup.EnableSpaz)
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

// Overload of GetCollisionResult which can be used to probe collision parameters
// from a given item.

COLL_RESULT GetCollisionResult(ITEM_INFO* item, short angle, int dist, int height)
{
	auto xProbe = item->pos.xPos + phd_sin(angle) * dist;
	auto yProbe = item->pos.yPos + height;
	auto zProbe = item->pos.zPos + phd_cos(angle) * dist;

	return GetCollisionResult(xProbe, yProbe, zProbe, GetRoom(item->location, xProbe, yProbe, zProbe).roomNumber);
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
	result.Position.Floor = GetFloorHeight(ROOM_VECTOR{ floor->Room, y }, x, z).value_or(NO_HEIGHT);
	result.Position.Ceiling = GetCeilingHeight(ROOM_VECTOR{ floor->Room, y }, x, z).value_or(NO_HEIGHT);

	// Probe bottom block through portals.
	while (floor->RoomBelow(x, y, z).value_or(NO_ROOM) != NO_ROOM)
	{
		auto r = &g_Level.Rooms[floor->RoomBelow(x, y, z).value_or(floor->Room)];
		floor = GetSector(r, x - r->x, z - r->z);
	}

	// Return probed bottom block into result.
	result.BottomBlock = floor;

	// Get tilts from new floordata.
	auto tilts = floor->TiltXZ(x, z);
	result.TiltX = tilts.first;
	result.TiltZ = tilts.second;

	// Split, bridge and slope data
	result.Position.DiagonalStep = floor->FloorIsDiagonalStep();
	result.Position.SplitAngle = floor->FloorCollision.SplitAngle;
	result.Position.Bridge = result.BottomBlock->InsideBridge(x, result.Position.Floor, z, true, false);
	result.Position.Slope = (result.Position.Bridge < 0) && ((abs(tilts.first)) > 2 || (abs(tilts.second)) > 2);

	// TODO: check if we need to keep here this slope vs. bridge check from legacy GetTiltType
	if ((y + CLICK(2)) < (floor->FloorHeight(x, z)))
		result.TiltZ = result.TiltX = 0;

	return result;
}

void GetCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, bool resetRoom)
{
	GetCollisionInfo(coll, item, PHD_VECTOR(), resetRoom);
}

void GetCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, PHD_VECTOR offset, bool resetRoom)
{
	// Player collision has several more precise checks for bridge collisions.
	// Therefore, we should differentiate these code paths.
	bool playerCollision = item->data.is<LaraInfo*>();

	// Reset out collision parameters.
	coll->CollisionType = CT_NONE;
	coll->Shift.x = 0;
	coll->Shift.y = 0;
	coll->Shift.z = 0;

	// Offset base probe position by provided offset, if any.
	int xPos = item->pos.xPos + offset.x;
	int yPos = item->pos.yPos + offset.y;
	int zPos = item->pos.zPos + offset.z;

	// Specify base probe position, Y position being bounds top side.
	int x = xPos;
	int y = yPos - coll->Setup.Height;
	int z = zPos;

	// Define side probe offsets.
	int xfront, xright, xleft, zfront, zright, zleft;

	// Get nearest 90-degree snapped angle (quadrant).
	auto quadrant = GetQuadrant(coll->Setup.ForwardAngle);

	// Get side probe offsets depending on quadrant.
	// If unconstrained mode is specified, don't use quadrant.
	switch (coll->Setup.Mode == COLL_PROBE_MODE::QUADRANTS ? quadrant : -1)
	{
	case 0:
		xfront =  phd_sin(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		zfront =  coll->Setup.Radius;
		xleft  = -coll->Setup.Radius;
		zleft  =  coll->Setup.Radius;
		xright =  coll->Setup.Radius;
		zright =  coll->Setup.Radius;
		break;

	case 1:
		xfront =  coll->Setup.Radius;
		zfront =  phd_cos(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		xleft  =  coll->Setup.Radius;
		zleft  =  coll->Setup.Radius;
		xright =  coll->Setup.Radius;
		zright = -coll->Setup.Radius;
		break;

	case 2:
		xfront =  phd_sin(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		zfront = -coll->Setup.Radius;
		xleft  =  coll->Setup.Radius;
		zleft  = -coll->Setup.Radius;
		xright = -coll->Setup.Radius;
		zright = -coll->Setup.Radius;
		break;

	case 3:
		xfront = -coll->Setup.Radius;
		zfront =  phd_cos(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		xleft  = -coll->Setup.Radius;
		zleft  = -coll->Setup.Radius;
		xright = -coll->Setup.Radius;
		zright =  coll->Setup.Radius;
		break;

	default: 
		// No valid quadrant, return true probe offsets from object rotation.
		xfront = phd_sin(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		zfront = phd_cos(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		xleft  = (xfront * (coll->Setup.Mode == COLL_PROBE_MODE::FREE_FORWARD ? 0.5f : 1.0f)) + phd_sin(coll->Setup.ForwardAngle - ANGLE(90)) * coll->Setup.Radius;
		zleft  = (zfront * (coll->Setup.Mode == COLL_PROBE_MODE::FREE_FORWARD ? 0.5f : 1.0f)) + phd_cos(coll->Setup.ForwardAngle - ANGLE(90)) * coll->Setup.Radius;
		xright = (xfront * (coll->Setup.Mode == COLL_PROBE_MODE::FREE_FORWARD ? 0.5f : 1.0f)) + phd_sin(coll->Setup.ForwardAngle + ANGLE(90)) * coll->Setup.Radius;
		zright = (zfront * (coll->Setup.Mode == COLL_PROBE_MODE::FREE_FORWARD ? 0.5f : 1.0f)) + phd_cos(coll->Setup.ForwardAngle + ANGLE(90)) * coll->Setup.Radius;
		break;
	}

	// Define generic variables used for later object-specific position test shifts.
	ROOM_VECTOR tfLocation{}, tcLocation{}, lrfLocation{}, lrcLocation{};
	int height, ceiling;

	// Parameter definition ends here, now process to actual collision tests...
	
	// TEST 1: TILT AND NEAREST LEDGE CALCULATION

	auto collResult = GetCollisionResult(x, item->pos.yPos, z, item->roomNumber);
	coll->TiltX = collResult.TiltX;
	coll->TiltZ = collResult.TiltZ;
	coll->NearestLedgeAngle = GetNearestLedgeAngle(item, coll, coll->NearestLedgeDistance);

	// Debug angle and distance
	// g_Renderer.printDebugMessage("Nearest angle: %d", coll->NearestLedgeAngle);
	// g_Renderer.printDebugMessage("Nearest dist:  %f", coll->NearestLedgeDistance);
	
	// TEST 2: CENTERPOINT PROBE

	collResult = GetCollisionResult(x, y, z, item->roomNumber);
	auto topRoomNumber = collResult.RoomNumber; // Keep top room number as we need it to re-probe from origin room

	if (playerCollision)
	{
		tfLocation = GetRoom(item->location, x, y, z);
		height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(item->location, x, y - item->fallspeed, z);
		ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->fallspeed, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->Middle = collResult.Position;
	coll->Middle.Floor = height;
	coll->Middle.Ceiling = ceiling;

	// TEST 3: FRONTAL PROBE

	x = xfront + xPos;
	z = zfront + zPos;

	g_Renderer.addDebugSphere(Vector3(x, y, z), 32, Vector4(1, 0, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	collResult = GetCollisionResult(x, y, z, topRoomNumber);

	if (playerCollision)
	{
		if (resetRoom)
		{
			tfLocation = item->location;
			tcLocation = item->location;
			topRoomNumber = item->roomNumber;
		}

		tfLocation = GetRoom(tfLocation, x, y, z);
		height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(tcLocation, x, y - item->fallspeed, z);
		ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->fallspeed, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->Front = collResult.Position;
	coll->Front.Floor = height;
	coll->Front.Ceiling = ceiling;

	collResult = GetCollisionResult(x + xfront, y, z + zfront, topRoomNumber);

	if (playerCollision)
	{
		tfLocation = GetRoom(tfLocation, x + xfront, y, z + zfront);
		height = GetFloorHeight(tfLocation, x + xfront, z + zfront).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
	}
	if (height != NO_HEIGHT) height -= (playerCollision ? yPos : y);

	if (coll->Setup.SlopesAreWalls && 
		coll->Front.Slope && 
		coll->Front.Floor < coll->Middle.Floor && 
		coll->Front.Floor < 0 &&
		height < coll->Front.Floor)
	{
		coll->Front.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.SlopesArePits && 
			 coll->Front.Slope && 
			 coll->Front.Floor > coll->Middle.Floor)
	{
		coll->Front.Floor = STOP_SIZE;
	}
	else if (coll->Setup.DeathFlagIsPit && 
			 coll->Front.Floor > 0 && 
			 collResult.BottomBlock->Flags.Death)
	{
		coll->Front.Floor = STOP_SIZE;
	}

	// TEST 4: MIDDLE-LEFT PROBE

	x = xPos + xleft;
	z = zPos + zleft;

	g_Renderer.addDebugSphere(Vector3(x, y, z), 32, Vector4(0, 0, 1, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	collResult = GetCollisionResult(x, y, z, item->roomNumber);

	if (playerCollision)
	{
		lrfLocation = GetRoom(item->location, x, y, z);
		height = GetFloorHeight(lrfLocation, x, z).value_or(NO_HEIGHT);

		lrcLocation = GetRoom(item->location, x, y - item->fallspeed, z);
		ceiling = GetCeilingHeight(lrcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->fallspeed, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->MiddleLeft = collResult.Position;
	coll->MiddleLeft.Floor = height;
	coll->MiddleLeft.Ceiling = ceiling;

	if (coll->Setup.SlopesAreWalls && 
		coll->MiddleLeft.Slope && 
		coll->MiddleLeft.Floor < 0)
	{
		coll->MiddleLeft.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.SlopesArePits && 
			 coll->MiddleLeft.Slope && 
			 coll->MiddleLeft.Floor > 0)
	{
		coll->MiddleLeft.Floor = STOP_SIZE;
	}
	else if (coll->Setup.DeathFlagIsPit && 
			 coll->MiddleLeft.Floor > 0 && 
			 collResult.BottomBlock->Flags.Death)
	{
		coll->MiddleLeft.Floor = STOP_SIZE;
	}

	// TEST 5: FRONT-LEFT PROBE

	collResult = GetCollisionResult(x, y, z, topRoomNumber); // We use plain x/z values here, proposed by Choco

	if (playerCollision)
	{
		tfLocation = GetRoom(tfLocation, x, y, z);
		height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(tcLocation, x, y - item->fallspeed, z);
		ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->fallspeed, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->FrontLeft = collResult.Position;
	coll->FrontLeft.Floor = height;
	coll->FrontLeft.Ceiling = ceiling;

	if (coll->Setup.SlopesAreWalls && 
		coll->FrontLeft.Slope && 
		coll->FrontLeft.Floor < 0)
	{
		coll->FrontLeft.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.SlopesArePits && 
			 coll->FrontLeft.Slope && 
			 coll->FrontLeft.Floor > 0)
	{
		coll->FrontLeft.Floor = STOP_SIZE;
	}
	else if (coll->Setup.DeathFlagIsPit && 
			 coll->FrontLeft.Floor > 0 && 
			 collResult.BottomBlock->Flags.Death)
	{
		coll->FrontLeft.Floor = STOP_SIZE;
	}

	// TEST 6: MIDDLE-RIGHT PROBE

	x = xPos + xright;
	z = zPos + zright;

	g_Renderer.addDebugSphere(Vector3(x, y, z), 32, Vector4(0, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	collResult = GetCollisionResult(x, y, z, item->roomNumber);

	if (playerCollision)
	{
		lrfLocation = GetRoom(item->location, x, y, z);
		height = GetFloorHeight(lrfLocation, x, z).value_or(NO_HEIGHT);

		lrcLocation = GetRoom(item->location, x, y - item->fallspeed, z);
		ceiling = GetCeilingHeight(lrcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->fallspeed, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->MiddleRight = collResult.Position;
	coll->MiddleRight.Floor = height;
	coll->MiddleRight.Ceiling = ceiling;

	if (coll->Setup.SlopesAreWalls && 
		coll->MiddleRight.Slope && 
		coll->MiddleRight.Floor < 0)
	{
		coll->MiddleRight.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.SlopesArePits && 
			 coll->MiddleRight.Slope && 
			 coll->MiddleRight.Floor > 0)
	{
		coll->MiddleRight.Floor = STOP_SIZE;
	}
	else if (coll->Setup.DeathFlagIsPit && 
			 coll->MiddleRight.Floor > 0 && 
			 collResult.BottomBlock->Flags.Death)
	{
		coll->MiddleRight.Floor = STOP_SIZE;
	}

	// TEST 7: FRONT-RIGHT PROBE

	collResult = GetCollisionResult(x, y, z, topRoomNumber);

	if (playerCollision)
	{
		tfLocation = GetRoom(tfLocation, x, y, z);
		height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(tcLocation, x, y - item->fallspeed, z);
		ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->fallspeed, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->FrontRight = collResult.Position;
	coll->FrontRight.Floor = height;
	coll->FrontRight.Ceiling = ceiling;

	if (coll->Setup.SlopesAreWalls && 
		coll->FrontRight.Slope && 
		coll->FrontRight.Floor < 0)
	{
		coll->FrontRight.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.SlopesArePits && 
			 coll->FrontRight.Slope && 
			 coll->FrontRight.Floor > 0)
	{
		coll->FrontRight.Floor = STOP_SIZE;
	}
	else if (coll->Setup.DeathFlagIsPit && 
			 coll->FrontRight.Floor > 0 && 
			 collResult.BottomBlock->Flags.Death)
	{
		coll->FrontRight.Floor = STOP_SIZE;
	}

	// TEST 8: SOLID STATIC MESHES

	CollideSolidStatics(item, coll);

	// Collision tests now end.
	// Get to calculation of collision side and shifts.

	if (coll->Middle.Floor == NO_HEIGHT)
	{
		coll->Shift.x = coll->Setup.OldPosition.x - xPos;
		coll->Shift.y = coll->Setup.OldPosition.y - yPos;
		coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		coll->CollisionType = CT_FRONT;
		return;
	}

	if (coll->Middle.Floor - coll->Middle.Ceiling <= 0)
	{
		coll->Shift.x = coll->Setup.OldPosition.x - xPos;
		coll->Shift.y = coll->Setup.OldPosition.y - yPos;
		coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		coll->CollisionType = CT_CLAMP;
		return;
	}

	if (coll->Middle.Ceiling >= 0)
	{
		coll->Shift.y = coll->Middle.Ceiling;
		coll->CollisionType = CT_TOP;
	}

	if (coll->Front.Floor > coll->Setup.BadHeightDown || 
		coll->Front.Floor < coll->Setup.BadHeightUp ||
		coll->Front.Ceiling > coll->Setup.BadCeilingHeight)
	{
		if (coll->Front.HasDiagonalSplit())
		{
			coll->Shift.x = coll->Setup.OldPosition.x - xPos;
			coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		}
		else
		{
			switch (quadrant)
			{
			case 0:
			case 2:
				coll->Shift.x = coll->Setup.OldPosition.x - xPos;
				coll->Shift.z = FindGridShift(zPos + zfront, zPos);
				break;

			case 1:
			case 3:
				coll->Shift.x = FindGridShift(xPos + xfront, xPos);
				coll->Shift.z = coll->Setup.OldPosition.z - zPos;
				break;

			}
		}
		coll->CollisionType = (coll->CollisionType == CT_TOP ? CT_TOP_FRONT : CT_FRONT);
		return;
	}

	if (coll->Front.Ceiling >= coll->Setup.BadCeilingHeight)
	{
		coll->Shift.x = coll->Setup.OldPosition.x - xPos;
		coll->Shift.y = coll->Setup.OldPosition.y - yPos;
		coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		coll->CollisionType = CT_TOP_FRONT;
		return;
	}

	if (coll->MiddleLeft.Floor > coll->Setup.BadHeightDown ||
		coll->MiddleLeft.Floor < coll->Setup.BadHeightUp ||
		coll->MiddleLeft.Ceiling > coll->Setup.BadCeilingHeight)
	{
		if (coll->TriangleAtLeft() && !coll->MiddleLeft.Slope)
		{
			// HACK: Force slight push-out to the left side to avoid stucking
			MoveItem(item, coll->Setup.ForwardAngle + ANGLE(8), item->speed);

			coll->Shift.x = coll->Setup.OldPosition.x - xPos;
			coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		}
		else
		{
			switch (quadrant)
			{
			case 0:
			case 2:
				coll->Shift.x = FindGridShift(xPos + xleft, xPos + xfront);
				break;

			case 1:
			case 3:
				coll->Shift.z = FindGridShift(zPos + zleft, zPos + zfront);
				break;
			}
		}

		if (coll->DiagonalStepAtLeft())
		{
			int quarter = (unsigned short)(coll->Setup.ForwardAngle) / ANGLE(90); // different from quadrant!
			quarter %= 2;

			if (coll->MiddleLeft.HasFlippedDiagonalSplit())
			{
				if (quarter) coll->CollisionType = CT_LEFT;
			}
			else
			{
				if (!quarter) coll->CollisionType = CT_LEFT;
			}
		}
		else
		{
			coll->CollisionType = CT_LEFT;
		}

		return;
	}

	if (coll->MiddleRight.Floor > coll->Setup.BadHeightDown ||
		coll->MiddleRight.Floor < coll->Setup.BadHeightUp ||
		coll->MiddleRight.Ceiling > coll->Setup.BadCeilingHeight)
	{
		if (coll->TriangleAtRight() && !coll->MiddleRight.Slope)
		{
			// HACK: Force slight push-out to the right side to avoid stucking
			MoveItem(item, coll->Setup.ForwardAngle - ANGLE(8), item->speed);

			coll->Shift.x = coll->Setup.OldPosition.x - xPos;
			coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		}
		else
		{
			switch (quadrant)
			{
			case 0:
			case 2:
				coll->Shift.x = FindGridShift(xPos + xright, xPos + xfront);
				break;

			case 1:
			case 3:
				coll->Shift.z = FindGridShift(zPos + zright, zPos + zfront);
				break;
			}
		}

		if (coll->DiagonalStepAtRight())
		{
			int quarter = (unsigned short)(coll->Setup.ForwardAngle) / ANGLE(90); // different from quadrant!
			quarter %= 2;

			if (coll->MiddleRight.HasFlippedDiagonalSplit())
			{
				if (quarter) coll->CollisionType = CT_RIGHT;
			}
			else
			{
				if (!quarter) coll->CollisionType = CT_RIGHT;
			}
		}
		else
		{
			coll->CollisionType = CT_RIGHT;
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

	if (item->pos.yPos >= collResult.Position.Floor)
	{
		bs = 0;

		if (collResult.Position.Slope && oldCollResult.Position.Floor < collResult.Position.Floor)
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

		if (y > (collResult.Position.Floor + 32) && bs == 0 &&
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
		else if (collResult.Position.Slope) 	// Hit a steep slope?
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
			item->pos.yPos = collResult.Position.Floor;
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

			if (item->pos.yPos >= oldCollResult.Position.Floor) 
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
				item->pos.yPos = oldCollResult.Position.Floor;
			}
		}
		//		else
		{
			/* Bounce off ceiling */
			collResult = GetCollisionResult(item);

			if (item->pos.yPos < collResult.Position.Ceiling)
			{
				if (y < collResult.Position.Ceiling &&
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
					item->pos.yPos = collResult.Position.Ceiling;

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
	coll->HitStatic = false;

	if (l == LaraItem)
		Lara.hitDirection = -1;

	if (l->hitPoints > 0)
	{
		short* door, numDoors;
		for (auto i : CollectConnectedRooms(l->roomNumber))
		{
			short itemNumber = g_Level.Rooms[i].itemNumber;
			while (itemNumber != NO_ITEM)
			{
				item = &g_Level.Items[itemNumber];
				if (item->collidable && item->status != ITEM_INVISIBLE)		 
				{
					obj = &Objects[item->objectNumber];
					if (obj->collision != nullptr)
					{
						if (phd_Distance(&item->pos, &l->pos) < COLLISION_CHECK_DISTANCE)
							obj->collision(itemNumber, l, coll);
					}
				}
				itemNumber = item->nextItem;
			}

			for (int j = 0; j < g_Level.Rooms[i].mesh.size(); j++)
			{
				MESH_INFO* mesh = &g_Level.Rooms[i].mesh[j];

				// Only process meshes which are visible and non-solid
				if ((mesh->flags & StaticMeshFlags::SM_VISIBLE) && !(mesh->flags & StaticMeshFlags::SM_SOLID))
				{
					if (phd_Distance(&mesh->pos, &l->pos) < COLLISION_CHECK_DISTANCE)
					{
						if (TestBoundsCollideStatic(l, mesh, coll->Setup.Radius))
						{
							coll->HitStatic = true;

							if (coll->Setup.EnableObjectPush)
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
		if (TestBoundsCollide(item, l, coll->Setup.Radius))
		{
			int collidedBits = TestCollision(item, l);
			if (collidedBits)
			{
				short oldRot = item->pos.yRot;

				item->pos.yRot = 0;
				GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
				item->pos.yRot = oldRot;

				int deadlyBits = *((int*)&item->itemFlags[0]);
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

						if (ItemPushItem(item, l, coll, deadlyBits & 1, 3) && (deadlyBits & 1))
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

							if (!coll->Setup.EnableObjectPush)
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
	if (!radiusDivide)
		return;

	GAME_VECTOR pos = {};
	pos.x = item->pos.xPos;
	pos.y = item->pos.yPos;
	pos.z = item->pos.zPos;
	pos.roomNumber = item->roomNumber;

	auto bounds = GetBoundsAccurate(item);
	auto radiusX = bounds->X2;
	auto radiusZ = bounds->Z2 / radiusDivide; // Need divide in any case else it's too much !

	auto ratioXZ = radiusZ / radiusX;
	auto frontX = phd_sin(item->pos.yRot) * radiusZ;
	auto frontZ = phd_cos(item->pos.yRot) * radiusZ;
	auto leftX  = -frontZ * ratioXZ;
	auto leftZ  =  frontX * ratioXZ;
	auto rightX =  frontZ * ratioXZ;
	auto rightZ = -frontX * ratioXZ;

	auto frontHeight = GetCollisionResult(pos.x + frontX, pos.y, pos.z + frontZ, pos.roomNumber).Position.Floor;
	auto backHeight  = GetCollisionResult(pos.x - frontX, pos.y, pos.z - frontZ, pos.roomNumber).Position.Floor;
	auto leftHeight  = GetCollisionResult(pos.x + leftX,  pos.y, pos.z + leftZ,  pos.roomNumber).Position.Floor;
	auto rightHeight = GetCollisionResult(pos.x + rightX, pos.y, pos.z + rightZ, pos.roomNumber).Position.Floor;

	auto frontHDif = backHeight  - frontHeight;
	auto sideHDif  = rightHeight - leftHeight;

	// Don't align if height differences are too large
	if ((abs(frontHDif) > STEPUP_HEIGHT) || (abs(sideHDif) > STEPUP_HEIGHT))
		return;

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

int GetQuadrant(short angle)
{
	return (unsigned short) (angle + ANGLE(45)) / ANGLE(90);
}

// Determines vertical surfaces and gets nearest ledge angle.
// Allows to eventually use unconstrained vaults and shimmying.

short GetNearestLedgeAngle(ITEM_INFO* item, COLL_INFO* coll, float& dist)
{
	// Get item bounds and current rotation
	auto bounds = GetBoundsAccurate(item);
	auto c = phd_cos(coll->Setup.ForwardAngle);
	auto s = phd_sin(coll->Setup.ForwardAngle);

	// Origin test position should be slightly in front of origin, because otherwise
	// misfire may occur near block corners for split angles.
	auto frontalOffset = coll->Setup.Radius * 0.3f;
	auto x = item->pos.xPos + frontalOffset * s;
	auto z = item->pos.zPos + frontalOffset * c;

	// Determine two Y points to test (lower and higher).
	// 1/10 headroom crop is needed to avoid possible issues with tight diagonal headrooms.
	int headroom = abs(bounds->Y2 - bounds->Y1) / 20.0f;
	int yPoints[2] = { item->pos.yPos + bounds->Y1 + headroom,
					   item->pos.yPos + bounds->Y2 - headroom };

	// Prepare test data
	float finalDistance[2] = { FLT_MAX, FLT_MAX };
	short finalResult[2] = { 0 };
	bool  hitBridge = false;

	// Do a two-pass surface test for all possible planes in a block.
	// Two-pass test is needed to resolve different scissor cases with diagonal geometry.

	for (int h = 0; h < 2; h++)
	{
		// Use either bottom or top Y point to test
		auto y = yPoints[h];

		// Prepare test data
		Ray   originRay;
		Plane closestPlane[3] = { };
		float closestDistance[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
		short result[3] = { };

		// If bridge was hit on the first pass, stop checking
		if (h == 1 && hitBridge)
			break;

		for (int p = 0; p < 3; p++)
		{
			// Prepare test data
			float distance = 0.0f;

			// Determine horizontal probe coordinates
			auto eX = x;
			auto eZ = z;

			// Determine if probe must be shifted (if left or right probe)
			if (p > 0)
			{
				auto s2 = phd_sin(coll->Setup.ForwardAngle + (p == 1 ? ANGLE(90) : ANGLE(-90)));
				auto c2 = phd_cos(coll->Setup.ForwardAngle + (p == 1 ? ANGLE(90) : ANGLE(-90)));

				// Slightly extend width beyond coll radius to hit adjacent blocks for sure
				eX += s2 * (coll->Setup.Radius * 2);
				eZ += c2 * (coll->Setup.Radius * 2);
			}

			// Debug probe point
			// g_Renderer.addDebugSphere(Vector3(eX, y, eZ), 16, Vector4(1, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

			// Determine front floor probe offset.
			// It is needed to identify if there is bridge or ceiling split in front.
			auto frontFloorProbeOffset = coll->Setup.Radius * 1.5f;
			auto ffpX = eX + frontFloorProbeOffset * s;
			auto ffpZ = eZ + frontFloorProbeOffset * c;

			// Get front floor block
			auto room = GetRoom(item->location, ffpX, y, ffpZ).roomNumber;
			auto block = GetCollisionResult(ffpX, y, ffpZ, room).Block;

			// Get front floor surface heights
			auto floorHeight = block->BridgeFloorHeight(ffpX, y, ffpZ); // HACK? FloorHeight never returns real bridge height!
			auto ceilingHeight = block->CeilingHeight(ffpX, y, ffpZ);

			// If ceiling height tests lower than Y value, it means ceiling
			// ledge is in front and we should use it instead of floor.
			bool useCeilingLedge = ceilingHeight > y;
			int height = useCeilingLedge ? ceilingHeight : floorHeight;

			// Determine if there is a bridge in front
			auto bridge = block->InsideBridge(ffpX, height, ffpZ, true, y == height);

			// Determine floor probe offset.
			// This must be slightly in front of own coll radius so no bridge misfires occur.
			auto floorProbeOffset = coll->Setup.Radius * 0.3f;
			auto fpX = eX + floorProbeOffset * s;
			auto fpZ = eZ + floorProbeOffset * c;

			// Debug probe point
			// g_Renderer.addDebugSphere(Vector3(fpX, y, fpZ), 16, Vector4(0, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

			// Get true room number and block, based on derived height
			room = GetRoom(item->location, fpX, height, fpZ).roomNumber;
			block = GetCollisionResult(fpX, height, fpZ, room).Block;

			// We don't need actual corner heights to build planes, so just use normalized value here
			auto fY = height - 1;
			auto cY = height + 1;

			// Calculate ray
			auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(coll->Setup.ForwardAngle), 0, 0);
			auto direction = (Matrix::CreateTranslation(Vector3::UnitZ) * mxR).Translation();
			auto ray = Ray(Vector3(eX, cY, eZ), direction);

			// Debug ray direction
			// g_Renderer.addLine3D(Vector3(eX, y, eZ), Vector3(eX, y, eZ) + direction * 256, Vector4(1, 0, 0, 1));

			// Keep origin ray to calculate true centerpoint distance to ledge later
			if (p == 0)
				originRay = Ray(Vector3(eX, cY, eZ), direction);

			if (bridge >= 0) // Surface is inside bridge
			{
				// Get and test DX item coll bounds
				auto bounds = GetBoundsAccurate(&g_Level.Items[bridge]);
				auto dxBounds = TO_DX_BBOX(g_Level.Items[bridge].pos, bounds);

				// Decompose bounds into planes
				Vector3 corners[8];
				dxBounds.GetCorners(corners);
				Plane plane[4] =
				{
					Plane(corners[2], corners[1], corners[0]),
					Plane(corners[0], corners[4], corners[3]),
					Plane(corners[5], corners[6], corners[7]),
					Plane(corners[6], corners[5], corners[1])
				};

				// Find closest bridge edge plane
				for (int i = 0; i < 4; i++)
				{
					// No plane intersection, quickly discard
					if (!ray.Intersects(plane[i], distance))
						continue;

					// Process plane intersection only if distance is smaller
					// than already found minimum
					if (distance < closestDistance[p])
					{
						closestPlane[p] = plane[i];
						closestDistance[p] = distance;
						auto normal = closestPlane[p].Normal();
						result[p] = FROM_RAD(atan2(normal.x, normal.z));
						hitBridge = true;
					}
				}
			}
			else // Surface is inside block
			{
				// Determine if we should use floor or ceiling split angle based on early tests.
				auto splitAngle = (useCeilingLedge ? block->CeilingCollision.SplitAngle : block->FloorCollision.SplitAngle);

				// Get horizontal block corner coordinates
				auto fX = floor(eX / WALL_SIZE) * WALL_SIZE - 1;
				auto fZ = floor(eZ / WALL_SIZE) * WALL_SIZE - 1;
				auto cX = fX + WALL_SIZE + 1;
				auto cZ = fZ + WALL_SIZE + 1;

				// Debug used block
				// g_Renderer.addDebugSphere(Vector3(round(eX / WALL_SIZE) * WALL_SIZE + 512, y, round(eZ / WALL_SIZE) * WALL_SIZE + 512), 16, Vector4(1, 1, 1, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

				// Get split angle coordinates
				auto sX = fX + 1 + WALL_SIZE / 2;
				auto sZ = fZ + 1 + WALL_SIZE / 2;
				auto sShiftX = coll->Setup.Radius * sin(splitAngle);
				auto sShiftZ = coll->Setup.Radius * cos(splitAngle);

				// Get block edge planes + split angle plane
				Plane plane[5] =
				{
					Plane(Vector3(fX, cY, cZ), Vector3(cX, cY, cZ), Vector3(cX, fY, cZ)), // North 
					Plane(Vector3(fX, cY, fZ), Vector3(fX, cY, cZ), Vector3(fX, fY, cZ)), // West
					Plane(Vector3(cX, fY, fZ), Vector3(cX, cY, fZ), Vector3(fX, cY, fZ)), // South
					Plane(Vector3(cX, fY, cZ), Vector3(cX, cY, cZ), Vector3(cX, cY, fZ)), // East
					Plane(Vector3(sX, cY, sZ), Vector3(sX, fY, sZ), Vector3(sX + sShiftX, cY, sZ + sShiftZ)) // Split
				};

				// If split angle exists, take split plane into account too.
				auto useSplitAngle = (useCeilingLedge ? block->CeilingIsSplit() : block->FloorIsSplit());

				// Find closest block edge plane
				for (int i = 0; i < (useSplitAngle ? 5 : 4); i++)
				{
					// No plane intersection, quickly discard
					if (!ray.Intersects(plane[i], distance))
						continue;

					// Process plane intersection only if distance is smaller
					// than already found minimum
					if (distance < closestDistance[p])
					{
						closestDistance[p] = distance;
						closestPlane[p] = plane[i];

						// Store according rotation.
						// For block edges (cases 0-3), return ordinary normal values.
						// For split angle (case 4), return axis perpendicular to split angle (hence + ANGLE(90)) and dependent on
						// origin sector plane, which determines the direction of edge normal.

						if (i == 4)
						{
							auto usedSectorPlane = useCeilingLedge ? block->SectorPlaneCeiling(eX, eZ) : block->SectorPlane(eX, eZ);
							result[p] = FROM_RAD(splitAngle) + ANGLE(usedSectorPlane * 180.0f) + ANGLE(90);
						}
						else
						{
							auto normal = closestPlane[p].Normal();
							result[p] = FROM_RAD(atan2(normal.x, normal.z)) + ANGLE(180.0f);
						}
					}
				}
			}
		}

		// Compare all 3 probe results and prioritize resulting angle based on
		// angle occurence. This approach is needed to filter out false detections
		// on the near-zero thickness edges of diagonal geometry which probes tend to tunnel through.

		std::set<short> angles;
		for (int p = 0; p < 3; p++)
		{
			// Prioritize ledge angle which was twice recognized
			if (!angles.insert(result[p]).second)
			{
				// Find existing angle in results
				int firstEqualAngle;
				for (firstEqualAngle = 0; firstEqualAngle < 3; firstEqualAngle++)
				{
					if (result[firstEqualAngle] == result[p])
						break;
					else if (firstEqualAngle == 2)
						firstEqualAngle = 0; // No equal angles, use center one
				}
				
				// Remember distance to the closest plane with same angle (it happens sometimes with bridges)
				float dist1 = FLT_MAX;
				float dist2 = FLT_MAX;
				auto r1 = originRay.Intersects(closestPlane[p], dist1);
				auto r2 = originRay.Intersects(closestPlane[firstEqualAngle], dist2);

				finalDistance[h] = (dist1 > dist2 && r2) ? dist2 : (r1 ? dist1 : dist2);
				finalResult[h] = result[p];
				break;
			}
		}

		// Store first result in case all 3 results are different (no priority) or prioritized result if long-distance misfire occured
		if (finalDistance[h] == FLT_MAX || finalDistance[h] > WALL_SIZE / 2)
		{
			finalDistance[h] = closestDistance[0];
			finalResult[h] = result[0];
		}
	}

	// Return upper probe result in case it returned lower distance or has hit a bridge.
	auto usedProbe = ((finalDistance[0] < finalDistance[1]) || hitBridge) ? 0 : 1;
	dist = finalDistance[usedProbe] - (coll->Setup.Radius - frontalOffset);
	return finalResult[usedProbe];
}