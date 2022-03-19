#include "framework.h"
#include "Game/collision/collide_item.h"

#include "Game/control/los.h"
#include "Game/collision/collide_room.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Game/collision/sphere.h"
#include "Game/room.h"
#include "Specific/setup.h"
#include "Sound/sound.h"
#include "Specific/trmath.h"
#include "Specific/prng.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Math::Random;
using namespace TEN::Renderer;

BOUNDING_BOX GlobalCollisionBounds;
ITEM_INFO* CollidedItems[MAX_COLLIDED_OBJECTS];
MESH_INFO* CollidedMeshes[MAX_COLLIDED_OBJECTS];

void GenericSphereBoxCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->Status != ITEM_INVISIBLE)
	{
		if (TestBoundsCollide(item, laraItem, coll->Setup.Radius))
		{
			int collidedBits = TestCollision(item, laraItem);
			if (collidedBits)
			{
				short oldRot = item->Position.yRot;

				item->Position.yRot = 0;
				GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
				item->Position.yRot = oldRot;

				int deadlyBits = *((int*)&item->ItemFlags[0]);
				auto* sphere = &CreatureSpheres[0];

				if (item->ItemFlags[2] != 0)
					collidedBits &= ~1;

				while (collidedBits)
				{
					if (collidedBits & 1)
					{
						GlobalCollisionBounds.X1 = sphere->x - sphere->r - item->Position.xPos;
						GlobalCollisionBounds.X2 = sphere->x + sphere->r - item->Position.xPos;
						GlobalCollisionBounds.Y1 = sphere->y - sphere->r - item->Position.yPos;
						GlobalCollisionBounds.Y2 = sphere->y + sphere->r - item->Position.yPos;
						GlobalCollisionBounds.Z1 = sphere->z - sphere->r - item->Position.zPos;
						GlobalCollisionBounds.Z2 = sphere->z + sphere->r - item->Position.zPos;

						int x = laraItem->Position.xPos;
						int y = laraItem->Position.yPos;
						int z = laraItem->Position.zPos;

						if (ItemPushItem(item, laraItem, coll, deadlyBits & 1, 3) && (deadlyBits & 1))
						{
							laraItem->HitPoints -= item->ItemFlags[3];

							int dx = x - laraItem->Position.xPos;
							int dy = y - laraItem->Position.yPos;
							int dz = z - laraItem->Position.zPos;

							if (dx || dy || dz)
							{
								if (TriggerActive(item))
									TriggerLaraBlood();
							}

							if (!coll->Setup.EnableObjectPush)
							{
								laraItem->Position.xPos += dx;
								laraItem->Position.yPos += dy;
								laraItem->Position.zPos += dz;
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

bool GetCollidedObjects(ITEM_INFO* collidingItem, int radius, bool onlyVisible, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int ignoreLara)
{
	short numItems = 0;
	short numMeshes = 0;

	// Collect all the rooms where to check
	auto roomsArray = GetRoomList(collidingItem->RoomNumber);

	for (auto i : roomsArray)
	{
		auto* room = &g_Level.Rooms[i];

		if (collidedMeshes)
		{
			for (int j = 0; j < room->mesh.size(); j++)
			{
				auto* mesh = &room->mesh[j];
				auto* staticMesh = &StaticObjects[mesh->staticNumber];

				if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
					continue;

				if (collidingItem->Position.yPos + radius + CLICK(0.5f) < mesh->pos.yPos + staticMesh->collisionBox.Y1)
					continue;

				if (collidingItem->Position.yPos > mesh->pos.yPos + staticMesh->collisionBox.Y2)
					continue;

				float s = phd_sin(mesh->pos.yRot);
				float c = phd_cos(mesh->pos.yRot);
				float rx = (collidingItem->Position.xPos - mesh->pos.xPos) * c - s * (collidingItem->Position.zPos - mesh->pos.zPos);
				float rz = (collidingItem->Position.zPos - mesh->pos.zPos) * c + s * (collidingItem->Position.xPos - mesh->pos.xPos);

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

			collidedMeshes[numMeshes] = NULL;
		}

		if (collidedItems)
		{
			int itemNumber = room->itemNumber;
			if (itemNumber != NO_ITEM)
			{
				do
				{
					auto* item = &g_Level.Items[itemNumber];

					if (item == collidingItem ||
						item->ObjectNumber == ID_LARA && ignoreLara ||
						item->Flags & 0x8000 ||
						item->MeshBits == 0 ||
						(Objects[item->ObjectNumber].drawRoutine == NULL && item->ObjectNumber != ID_LARA) ||
						(Objects[item->ObjectNumber].collision == NULL && item->ObjectNumber != ID_LARA) ||
						onlyVisible && item->Status == ITEM_INVISIBLE ||
						item->ObjectNumber == ID_BURNING_FLOOR)
					{
						itemNumber = item->NextItem;
						continue;
					}

					/*this is awful*/
					if (item->ObjectNumber == ID_UPV && item->HitPoints == 1)
					{
						itemNumber = item->NextItem;
						continue;
					}
					if (item->ObjectNumber == ID_BIGGUN && item->HitPoints == 1)
					{
						itemNumber = item->NextItem;
						continue;
					}
					/*we need a better system*/

					int dx = collidingItem->Position.xPos - item->Position.xPos;
					int dy = collidingItem->Position.yPos - item->Position.yPos;
					int dz = collidingItem->Position.zPos - item->Position.zPos;

					auto* framePtr = GetBestFrame(item);

					if (dx >= -SECTOR(2) && dx <= SECTOR(2) &&
						dy >= -SECTOR(2) && dy <= SECTOR(2) &&
						dz >= -SECTOR(2) && dz <= SECTOR(2) &&
						collidingItem->Position.yPos + radius + CLICK(0.5f) >= item->Position.yPos + framePtr->boundingBox.Y1 &&
						collidingItem->Position.yPos - radius - CLICK(0.5f) <= item->Position.yPos + framePtr->boundingBox.Y2)
					{
						float s = phd_sin(item->Position.yRot);
						float c = phd_cos(item->Position.yRot);

						int rx = dx * c - s * dz;
						int rz = dz * c + s * dx;

						if (item->ObjectNumber == ID_TURN_SWITCH)
						{
							framePtr->boundingBox.X1 = -CLICK(1);
							framePtr->boundingBox.X2 = CLICK(1);
							framePtr->boundingBox.Z1 = -CLICK(1);
							framePtr->boundingBox.Z1 = CLICK(1);
						}

						if (radius + rx + CLICK(0.5f) >= framePtr->boundingBox.X1 && rx - radius - CLICK(0.5f) <= framePtr->boundingBox.X2)
						{
							if (radius + rz + CLICK(0.5f) >= framePtr->boundingBox.Z1 && rz - radius - CLICK(0.5f) <= framePtr->boundingBox.Z2)
							{
								collidedItems[numItems++] = item;

								if (!radius)
									return true;
							}
						}
					}

					itemNumber = item->NextItem;
				} while (itemNumber != NO_ITEM);
			}

			collidedItems[numItems] = NULL;
		}
	}

	return (numItems || numMeshes);
}

bool TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* laraItem, CollisionInfo* coll)
{
	auto* framePtr = GetBestFrame(laraItem);

	if (item->Position.yPos + GlobalCollisionBounds.Y2 <= laraItem->Position.yPos + framePtr->boundingBox.Y1)
		return false;

	if (item->Position.yPos + GlobalCollisionBounds.Y1 >= framePtr->boundingBox.Y2)
		return false;

	float s = phd_sin(item->Position.yRot);
	float c = phd_cos(item->Position.yRot);

	int dx = laraItem->Position.xPos - item->Position.xPos;
	int dz = laraItem->Position.zPos - item->Position.zPos;

	int x = c * dx - s * dz;
	int z = c * dz + s * dx;

	if (x < GlobalCollisionBounds.X1 - coll->Setup.Radius ||
		x > GlobalCollisionBounds.X2 + coll->Setup.Radius ||
		z < GlobalCollisionBounds.Z1 - coll->Setup.Radius ||
		z > GlobalCollisionBounds.Z2 + coll->Setup.Radius)
		return false;

	return true;
}

void TestForObjectOnLedge(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* bounds = GetBoundsAccurate(item);
	int height = abs(bounds->Y2 + bounds->Y1);

	for (int i = 0; i < 3; i++)
	{
		auto s = (i != 1) ? phd_sin(coll->Setup.ForwardAngle + ANGLE((i * 90) - 90)) : 0;
		auto c = (i != 1) ? phd_cos(coll->Setup.ForwardAngle + ANGLE((i * 90) - 90)) : 0;

		auto x = item->Position.xPos + (s * (coll->Setup.Radius));
		auto y = item->Position.yPos - height - STEP_SIZE;
		auto z = item->Position.zPos + (c * (coll->Setup.Radius));

		auto origin = Vector3(x, y, z);
		auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(coll->Setup.ForwardAngle), 0, 0);
		auto direction = (Matrix::CreateTranslation(Vector3::UnitZ) * mxR).Translation();

		// g_Renderer.addDebugSphere(origin, 16, Vector4::One, RENDERER_DEBUG_PAGE::DIMENSION_STATS);

		for (auto i : GetRoomList(item->RoomNumber))
		{
			short itemNumber = g_Level.Rooms[i].itemNumber;
			while (itemNumber != NO_ITEM)
			{
				auto item2 = &g_Level.Items[itemNumber];
				auto obj = &Objects[item2->ObjectNumber];

				if (obj->isPickup || obj->collision == nullptr || !item2->Collidable || item2->Status == ITEM_INVISIBLE)
				{
					itemNumber = item2->NextItem;
					continue;
				}

				if (phd_Distance(&item->Position, &item2->Position) < COLLISION_CHECK_DISTANCE)
				{
					auto box = TO_DX_BBOX(item2->Position, GetBoundsAccurate(item2));
					float distance;

					if (box.Intersects(origin, direction, distance) && distance < coll->Setup.Radius * 2)
					{
						coll->HitStatic = true;
						return;
					}
				}

				itemNumber = item2->NextItem;
			}

			for (int j = 0; j < g_Level.Rooms[i].mesh.size(); j++)
			{
				auto mesh = &g_Level.Rooms[i].mesh[j];

				if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
					continue;

				if (phd_Distance(&item->Position, &mesh->pos) < COLLISION_CHECK_DISTANCE)
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

bool TestLaraPosition(OBJECT_COLLISION_BOUNDS* bounds, ITEM_INFO* item, ITEM_INFO* laraItem)
{
	short xRotRel = laraItem->Position.xRot - item->Position.xRot;
	short yRotRel = laraItem->Position.yRot - item->Position.yRot;
	short zRotRel = laraItem->Position.zRot - item->Position.zRot;

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

	int x = laraItem->Position.xPos - item->Position.xPos;
	int y = laraItem->Position.yPos - item->Position.yPos;
	int z = laraItem->Position.zPos - item->Position.zPos;

	Vector3 pos = Vector3(x, y, z);

	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->Position.yRot),
		TO_RAD(item->Position.xRot),
		TO_RAD(item->Position.zRot)
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

void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* laraItem)
{
	laraItem->Position.xRot = item->Position.xRot;
	laraItem->Position.yRot = item->Position.yRot;
	laraItem->Position.zRot = item->Position.zRot;

	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->Position.yRot),
		TO_RAD(item->Position.xRot),
		TO_RAD(item->Position.zRot)
	);

	Vector3 pos = Vector3::Transform(Vector3(vec->x, vec->y, vec->z), matrix);

	laraItem->Position.xPos = item->Position.xPos + pos.x;
	laraItem->Position.yPos = item->Position.yPos + pos.y;
	laraItem->Position.zPos = item->Position.zPos + pos.z;
}

bool MoveLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* laraItem)
{
	PHD_3DPOS dest;
	dest.xRot = item->Position.xRot;
	dest.yRot = item->Position.yRot;
	dest.zRot = item->Position.zRot;

	Vector3 pos = Vector3(vec->x, vec->y, vec->z);

	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->Position.yRot),
		TO_RAD(item->Position.xRot),
		TO_RAD(item->Position.zRot)
	);

	pos = Vector3::Transform(pos, matrix);

	dest.xPos = item->Position.xPos + pos.x;
	dest.yPos = item->Position.yPos + pos.y;
	dest.zPos = item->Position.zPos + pos.z;

	if (item->ObjectNumber != ID_FLARE_ITEM && item->ObjectNumber != ID_BURNING_TORCH_ITEM)
		return Move3DPosTo3DPos(&laraItem->Position, &dest, LARA_VELOCITY, ANGLE(2.0f));

	int height = GetCollision(dest.xPos, dest.yPos, dest.zPos, laraItem->RoomNumber).Position.Floor;
	if (abs(height - laraItem->Position.yPos) <= CLICK(2))
	{
		if (sqrt(pow(dest.xPos - laraItem->Position.xPos, 2) + pow(dest.yPos - laraItem->Position.yPos, 2) + pow(dest.zPos - laraItem->Position.zPos, 2)) < CLICK(0.5f))
			return true;

		return Move3DPosTo3DPos(&laraItem->Position, &dest, LARA_VELOCITY, ANGLE(2.0f));
	}

	if (Lara.Control.IsMoving)
	{
		Lara.Control.IsMoving = false;
		Lara.Control.HandStatus = HandStatus::Free;
	}

	return false;
}

static bool ItemCollide(int value, int radius)
{
	return value >= -radius && value <= radius;
}

static bool ItemInRange(int x, int z, int radius)
{
	return (pow(x, 2) + pow(z, 2)) <= pow(radius, 2);
}

bool ItemNearLara(PHD_3DPOS* pos, int radius)
{
	GAME_VECTOR target;
	target.x = pos->xPos - LaraItem->Position.xPos;
	target.y = pos->yPos - LaraItem->Position.yPos;
	target.z = pos->zPos - LaraItem->Position.zPos;

	if (!ItemCollide(target.y, ITEM_RADIUS_YMAX))
		return false;

	if (!ItemCollide(target.x, radius) || !ItemCollide(target.z, radius))
		return false;

	if (!ItemInRange(target.x, target.z, radius))
		return false;

	auto* bounds = GetBoundsAccurate(LaraItem);
	if (target.y >= bounds->Y1 && target.y <= (bounds->Y2 + LARA_RAD))
		return true;

	return false;
}

bool ItemNearTarget(PHD_3DPOS* src, ITEM_INFO* target, int radius)
{
	PHD_VECTOR pos;
	pos.x = src->xPos - target->Position.xPos;
	pos.y = src->yPos - target->Position.yPos;
	pos.z = src->zPos - target->Position.zPos;

	if (!ItemCollide(pos.y, ITEM_RADIUS_YMAX))
		return false;

	if (!ItemCollide(pos.x, radius) || !ItemCollide(pos.z, radius))
		return false;

	if (!ItemInRange(pos.x, pos.z, radius))
		return false;

	auto* bounds = GetBoundsAccurate(target);
	if (pos.y >= bounds->Y1 && pos.y <= bounds->Y2)
		return true;

	return false;
}

bool Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angleAdd)
{
	int x = dest->xPos - src->xPos;
	int y = dest->yPos - src->yPos;
	int z = dest->zPos - src->zPos;
	int distance = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

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

	if (!Lara.Control.IsMoving)
	{
		if (Lara.Control.WaterStatus != WaterStatus::Underwater)
		{
			int angle = mGetAngle(dest->xPos, dest->zPos, src->xPos, src->zPos);
			int direction = (GetQuadrant(angle) - GetQuadrant(dest->yRot)) & 3;

			switch (direction)
			{
			case 0:
				SetAnimation(LaraItem, LA_SIDESTEP_LEFT);
				Lara.Control.HandStatus = HandStatus::Busy;
				break;

			case 1:
				SetAnimation(LaraItem, LA_WALK);
				Lara.Control.HandStatus = HandStatus::Busy;
				break;

			case 2:
				SetAnimation(LaraItem, LA_SIDESTEP_RIGHT);
				Lara.Control.HandStatus = HandStatus::Busy;
				break;

			case 3:
			default:
				SetAnimation(LaraItem, LA_WALK_BACK);
				Lara.Control.HandStatus = HandStatus::Busy;
				break;
			}
		}

		Lara.Control.IsMoving = true;
		Lara.Control.Count.PositionAdjust = 0;
	}

	short deltaAngle = dest->xRot - src->xRot;
	if (deltaAngle > angleAdd)
		src->xRot += angleAdd;
	else if (deltaAngle < -angleAdd)
		src->xRot -= angleAdd;
	else
		src->xRot = dest->xRot;

	deltaAngle = dest->yRot - src->yRot;
	if (deltaAngle > angleAdd)
		src->yRot += angleAdd;
	else if (deltaAngle < -angleAdd)
		src->yRot -= angleAdd;
	else
		src->yRot = dest->yRot;

	deltaAngle = dest->zRot - src->zRot;
	if (deltaAngle > angleAdd)
		src->zRot += angleAdd;
	else if (deltaAngle < -angleAdd)
		src->zRot -= angleAdd;
	else
		src->zRot = dest->zRot;

	return (src->xPos == dest->xPos &&
		src->yPos == dest->yPos &&
		src->zPos == dest->zPos &&
		src->xRot == dest->xRot &&
		src->yRot == dest->yRot &&
		src->zRot == dest->zRot);
}

bool TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* laraItem, int radius)
{
	auto bounds = (BOUNDING_BOX*)GetBestFrame(item);
	auto laraBounds = (BOUNDING_BOX*)GetBestFrame(laraItem);

	if (item->Position.yPos + bounds->Y2 <= laraItem->Position.yPos + laraBounds->Y1)
		return false;

	if (item->Position.yPos + bounds->Y1 >= laraItem->Position.yPos + laraBounds->Y2)
		return false;

	auto c = phd_cos(item->Position.yRot);
	auto s = phd_sin(item->Position.yRot);
	int x = laraItem->Position.xPos - item->Position.xPos;
	int z = laraItem->Position.zPos - item->Position.zPos;
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

bool TestBoundsCollideStatic(ITEM_INFO* item, MESH_INFO* mesh, int radius)
{
	auto bounds = StaticObjects[mesh->staticNumber].collisionBox;

	if (!(bounds.Z2 != 0 || bounds.Z1 != 0 || bounds.X1 != 0 || bounds.X2 != 0 || bounds.Y1 != 0 || bounds.Y2 != 0))
		return false;

	auto* frame = GetBestFrame(item);
	if (mesh->pos.yPos + bounds.Y2 <= item->Position.yPos + frame->boundingBox.Y1)
		return false;

	if (mesh->pos.yPos + bounds.Y1 >= item->Position.yPos + frame->boundingBox.Y2)
		return false;

	float c, s;
	int x, z, dx, dz;

	c = phd_cos(mesh->pos.yRot);
	s = phd_sin(mesh->pos.yRot);
	x = item->Position.xPos - mesh->pos.xPos;
	z = item->Position.zPos - mesh->pos.zPos;
	dx = c * x - s * z;
	dz = c * z + s * x;

	if (dx <= radius + bounds.X2 &&
		dx >= bounds.X1 - radius &&
		dz <= radius + bounds.Z2 &&
		dz >= bounds.Z1 - radius)
	{
		return true;
	}
	else
		return false;
}

bool ItemPushItem(ITEM_INFO* item, ITEM_INFO* item2, CollisionInfo* coll, bool spazon, char bigpush) // previously ItemPushLara
{
	// Get item's rotation
	auto c = phd_cos(item->Position.yRot);
	auto s = phd_sin(item->Position.yRot);

	// Get vector from item to Lara
	int dx = item2->Position.xPos - item->Position.xPos;
	int dz = item2->Position.zPos - item->Position.zPos;

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
	if (abs(dx) > 4608 || abs(dz) > 4608 ||
		rx <= minX || rx >= maxX ||
		rz <= minZ || rz >= maxZ)
	{
		return false;
	}

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

	item2->Position.xPos = item->Position.xPos + c * rx + s * rz;
	item2->Position.zPos = item->Position.zPos + c * rz - s * rx;

	auto* lara = item2->Data.is<LaraInfo*>() ? (LaraInfo*&)item2->Data : nullptr;

	if (lara != nullptr && spazon && bounds->Y2 - bounds->Y1 > CLICK(1))
	{
		rx = (bounds->X1 + bounds->X2) / 2;
		rz = (bounds->Z1 + bounds->Z2) / 2;

		dx -= c * rx + s * rz;
		dz -= c * rz - s * rx;

		lara->HitDirection = (item2->Position.yRot - phd_atan(dz, dz) - ANGLE(135.0f)) / 16384;

		if (!lara->HitFrame && !lara->SpasmEffectCount)
		{
			SoundEffect(SFX_TR4_LARA_INJURY, &item2->Position, 0);
			lara->SpasmEffectCount = GenerateInt(15, 35);
		}

		if (lara->SpasmEffectCount)
			lara->SpasmEffectCount--;

		lara->HitFrame++;
		if (lara->HitFrame > 34)
			lara->HitFrame = 34;
	}

	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.UpperCeilingBound = MAX_HEIGHT;

	auto facing = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = phd_atan(item2->Position.zPos - coll->Setup.OldPosition.z, item2->Position.xPos - coll->Setup.OldPosition.x);

	GetCollisionInfo(coll, item2);

	coll->Setup.ForwardAngle = facing;

	if (coll->CollisionType == CT_NONE)
	{
		coll->Setup.OldPosition.x = item2->Position.xPos;
		coll->Setup.OldPosition.y = item2->Position.yPos;
		coll->Setup.OldPosition.z = item2->Position.zPos;

		// Commented because causes Lara to jump out of the water if she touches an object on the surface. re: "kayak bug"
		// UpdateItemRoom(item2, -10);
	}
	else
	{
		item2->Position.xPos = coll->Setup.OldPosition.x;
		item2->Position.zPos = coll->Setup.OldPosition.z;
	}

	if (lara != nullptr && lara->Control.Count.PositionAdjust > 15)
	{
		Lara.Control.IsMoving = false;
		Lara.Control.HandStatus = HandStatus::Free;
	}

	return true;
}

bool ItemPushStatic(ITEM_INFO* item, MESH_INFO* mesh, CollisionInfo* coll) // previously ItemPushLaraStatic
{
	auto bounds = StaticObjects[mesh->staticNumber].collisionBox;

	auto c = phd_cos(mesh->pos.yRot);
	auto s = phd_sin(mesh->pos.yRot);
	
	auto dx = item->Position.xPos - mesh->pos.xPos;
	auto dz = item->Position.zPos - mesh->pos.zPos;
	auto rx = c * dx - s * dz;
	auto rz = c * dz + s * dx;
	auto minX = bounds.X1 - coll->Setup.Radius;
	auto maxX = bounds.X2 + coll->Setup.Radius;
	auto minZ = bounds.Z1 - coll->Setup.Radius;
	auto maxZ = bounds.Z2 + coll->Setup.Radius;

	if (abs(dx) > SECTOR(4.5f) || abs(dz) > SECTOR(4.5f) ||
		rx <= minX || rx >= maxX ||
		rz <= minZ || rz >= maxZ)
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

	item->Position.xPos = mesh->pos.xPos + c * rx + s * rz;
	item->Position.zPos = mesh->pos.zPos + c * rz - s * rx;

	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;

	auto oldFacing = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = phd_atan(item->Position.zPos - coll->Setup.OldPosition.z, item->Position.xPos - coll->Setup.OldPosition.x);

	GetCollisionInfo(coll, item);

	coll->Setup.ForwardAngle = oldFacing;

	if (coll->CollisionType == CT_NONE)
	{
		coll->Setup.OldPosition.x = item->Position.xPos;
		coll->Setup.OldPosition.y = item->Position.yPos;
		coll->Setup.OldPosition.z = item->Position.zPos;

		UpdateItemRoom(item, -10);
	}
	else
	{
		item->Position.xPos = coll->Setup.OldPosition.x;
		item->Position.zPos = coll->Setup.OldPosition.z;
	}

	if (item == LaraItem && Lara.Control.IsMoving && Lara.Control.Count.PositionAdjust > 15)
	{
		Lara.Control.IsMoving = false;
		Lara.Control.HandStatus = HandStatus::Free;
	}

	return true;
}

void CollideSolidStatics(ITEM_INFO* item, CollisionInfo* coll)
{
	coll->HitTallObject = false;

	for (auto i : GetRoomList(item->RoomNumber))
	{
		for (int j = 0; j < g_Level.Rooms[i].mesh.size(); j++)
		{
			auto mesh = &g_Level.Rooms[i].mesh[j];

			// Only process meshes which are visible and solid
			if ((mesh->flags & StaticMeshFlags::SM_VISIBLE) && (mesh->flags & StaticMeshFlags::SM_SOLID))
			{
				if (phd_Distance(&item->Position, &mesh->pos) < COLLISION_CHECK_DISTANCE)
				{
					auto staticInfo = StaticObjects[mesh->staticNumber];
					if (CollideSolidBounds(item, staticInfo.collisionBox, mesh->pos, coll))
						coll->HitStatic = true;
				}
			}
		}
	}
}

bool CollideSolidBounds(ITEM_INFO* item, BOUNDING_BOX box, PHD_3DPOS pos, CollisionInfo* coll)
{
	bool result = false;

	// Get DX static bounds in global coords
	auto staticBounds = TO_DX_BBOX(pos, &box);

	// Get local TR bounds and DX item bounds in global coords
	auto itemBBox = GetBoundsAccurate(item);
	auto itemBounds = TO_DX_BBOX(item->Position, itemBBox);

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
	collBox.X2 = coll->Setup.Radius;
	collBox.Z1 = -coll->Setup.Radius;
	collBox.Z2 = coll->Setup.Radius;

	// Calculate vertical item coll bounds according to either height (land mode) or precise bounds (water mode).
	// Water mode needs special processing because height calc in original engines is inconsistent in such cases.
	if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
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
	auto collBounds = TO_DX_BBOX(PHD_3DPOS(item->Position.xPos, item->Position.yPos, item->Position.zPos), &collBox);
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
	auto center = item->Position.yPos - height / 2;

	// Do a series of angular tests with 90 degree steps to determine top/bottom collision.

	int closestPlane = -1;
	Ray closestRay;
	auto minDistance = std::numeric_limits<float>::max();

	for (int i = 0; i < 4; i++)
	{
		// Calculate ray direction
		auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(item->Position.yRot), TO_RAD(item->Position.xRot + (ANGLE(90 * i))), 0);
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
				minDistance = d;
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
				item->Position.yPos += distanceToVerticalPlane + 2;
				coll->CollisionType = CT_TOP;
			}
			else
			{
				// Set collision type only if dry room (in water rooms it causes stucking)
				item->Position.yPos -= distanceToVerticalPlane;
				coll->CollisionType = (g_Level.Rooms[item->RoomNumber].flags & 1) ? coll->CollisionType : CT_CLAMP;
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
	if (!staticBounds.Intersects(TO_DX_BBOX(PHD_3DPOS(item->Position.xPos, item->Position.yPos, item->Position.zPos), &collBox)))
		return result;

	// Determine identity rotation/distance
	auto distance = Vector3(item->Position.xPos, item->Position.yPos, item->Position.zPos) - Vector3(pos.xPos, pos.yPos, pos.zPos);
	auto c = phd_cos(pos.yRot);
	auto s = phd_sin(pos.yRot);

	// Rotate item to collision bounds identity
	auto x = round(distance.x * c - distance.z * s) + pos.xPos;
	auto y = item->Position.yPos;
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
	coll->Shift.x = (round(distance.x * c - distance.z * s) + pos.xPos) - item->Position.xPos;
	coll->Shift.z = (round(distance.x * s + distance.z * c) + pos.zPos) - item->Position.zPos;

	if (coll->Shift.x == 0 && coll->Shift.z == 0)
		coll->CollisionType = CT_NONE; // Paranoid

	// Set splat state flag if item is Lara and bounds are taller than Lara's headroom
	if (item == LaraItem && coll->CollisionType == CT_FRONT)
		coll->HitTallObject = (YMin <= inYMin + LARA_HEADROOM);

	return true;
}

void DoProjectileDynamics(short itemNumber, int x, int y, int z, int xv, int yv, int zv) // previously DoProperDetection
{
	int bs, yAngle;

	auto* item = &g_Level.Items[itemNumber];

	auto oldCollResult = GetCollision(x, y, z, item->RoomNumber);
	auto collResult = GetCollision(item);

	if (item->Position.yPos >= collResult.Position.Floor)
	{
		bs = 0;

		if (collResult.Position.FloorSlope && oldCollResult.Position.Floor < collResult.Position.Floor)
		{
			yAngle = (long)((unsigned short)item->Position.yRot);
			if (collResult.FloorTilt.x < 0)
			{
				if (yAngle >= 0x8000)
					bs = 1;
			}
			else if (collResult.FloorTilt.x > 0)
			{
				if (yAngle <= 0x8000)
					bs = 1;
			}

			if (collResult.FloorTilt.y < 0)
			{
				if (yAngle >= 0x4000 && yAngle <= 0xc000)
					bs = 1;
			}
			else if (collResult.FloorTilt.y > 0)
			{
				if (yAngle <= 0x4000 || yAngle >= 0xc000)
					bs = 1;
			}
		}

		/* If last position of item was also below this floor height, we've hit a wall, else we've hit a floor */

		if (y > (collResult.Position.Floor + 32) && bs == 0 &&
			(((x / SECTOR(1)) != (item->Position.xPos / SECTOR(1))) ||
				((z / SECTOR(1)) != (item->Position.zPos / SECTOR(1)))))
		{
			// Need to know which direction the wall is.

			long	xs;

			if ((x & (~(WALL_SIZE - 1))) != (item->Position.xPos & (~(WALL_SIZE - 1))) &&	// X crossed boundary?
				(z & (~(WALL_SIZE - 1))) != (item->Position.zPos & (~(WALL_SIZE - 1))))	// Z crossed boundary as well?
			{
				if (abs(x - item->Position.xPos) < abs(z - item->Position.zPos))
					xs = 1;	// X has travelled the shortest, so (maybe) hit first. (Seems to work ok).
				else
					xs = 0;
			}
			else
				xs = 1;

			if ((x & (~(WALL_SIZE - 1))) != (item->Position.xPos & (~(WALL_SIZE - 1))) && xs)	// X crossed boundary?
			{
				if (xv <= 0)	// Hit angle = 0xc000.
					item->Position.yRot = 0x4000 + (0xc000 - item->Position.yRot);
				else			// Hit angle = 0x4000.
					item->Position.yRot = 0xc000 + (0x4000 - item->Position.yRot);
			}
			else		// Z crossed boundary.
				item->Position.yRot = 0x8000 - item->Position.yRot;

			item->Animation.Velocity /= 2;

			/* Put item back in its last position */
			item->Position.xPos = x;
			item->Position.yPos = y;
			item->Position.zPos = z;
		}
		else if (collResult.Position.FloorSlope) 	// Hit a steep slope?
		{
			// Need to know which direction the slope is.

			item->Animation.Velocity -= (item->Animation.Velocity / 4);

			if (collResult.FloorTilt.x < 0 && ((abs(collResult.FloorTilt.x)) - (abs(collResult.FloorTilt.y)) >= 2))	// Hit angle = 0x4000
			{
				if (((unsigned short)item->Position.yRot) > 0x8000)
				{
					item->Position.yRot = 0x4000 + (0xc000 - (unsigned short)item->Position.yRot - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity -= collResult.FloorTilt.x * 2;
						if ((unsigned short)item->Position.yRot > 0x4000 && (unsigned short)item->Position.yRot < 0xc000)
						{
							item->Position.yRot -= 4096;
							if ((unsigned short)item->Position.yRot < 0x4000)
								item->Position.yRot = 0x4000;
						}
						else if ((unsigned short)item->Position.yRot < 0x4000)
						{
							item->Position.yRot += 4096;
							if ((unsigned short)item->Position.yRot > 0x4000)
								item->Position.yRot = 0x4000;
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			else if (collResult.FloorTilt.x > 0 && ((abs(collResult.FloorTilt.x)) - (abs(collResult.FloorTilt.y)) >= 2))	// Hit angle = 0xc000
			{
				if (((unsigned short)item->Position.yRot) < 0x8000)
				{
					item->Position.yRot = 0xc000 + (0x4000 - (unsigned short)item->Position.yRot - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity += collResult.FloorTilt.x * 2;
						if ((unsigned short)item->Position.yRot > 0xc000 || (unsigned short)item->Position.yRot < 0x4000)
						{
							item->Position.yRot -= 4096;
							if ((unsigned short)item->Position.yRot < 0xc000)
								item->Position.yRot = 0xc000;
						}
						else if ((unsigned short)item->Position.yRot < 0xc000)
						{
							item->Position.yRot += 4096;
							if ((unsigned short)item->Position.yRot > 0xc000)
								item->Position.yRot = 0xc000;
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			else if (collResult.FloorTilt.y < 0 && ((abs(collResult.FloorTilt.y)) - (abs(collResult.FloorTilt.x)) >= 2))	// Hit angle = 0
			{
				if (((unsigned short)item->Position.yRot) > 0x4000 && ((unsigned short)item->Position.yRot) < 0xc000)
				{
					item->Position.yRot = (0x8000 - item->Position.yRot - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity -= collResult.FloorTilt.y * 2;

						if ((unsigned short)item->Position.yRot < 0x8000)
						{
							item->Position.yRot -= 4096;
							if ((unsigned short)item->Position.yRot > 0xf000)
								item->Position.yRot = 0;
						}
						else if ((unsigned short)item->Position.yRot >= 0x8000)
						{
							item->Position.yRot += 4096;
							if ((unsigned short)item->Position.yRot < 0x1000)
								item->Position.yRot = 0;
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			else if (collResult.FloorTilt.y > 0 && ((abs(collResult.FloorTilt.y)) - (abs(collResult.FloorTilt.x)) >= 2))	// Hit angle = 0x8000
			{
				if (((unsigned short)item->Position.yRot) > 0xc000 || ((unsigned short)item->Position.yRot) < 0x4000)
				{
					item->Position.yRot = (0x8000 - item->Position.yRot - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity += collResult.FloorTilt.y * 2;

						if ((unsigned short)item->Position.yRot > 0x8000)
						{
							item->Position.yRot -= 4096;
							if ((unsigned short)item->Position.yRot < 0x8000)
								item->Position.yRot = 0x8000;
						}
						else if ((unsigned short)item->Position.yRot < 0x8000)
						{
							item->Position.yRot += 4096;
							if ((unsigned short)item->Position.yRot > 0x8000)
								item->Position.yRot = 0x8000;
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			else if (collResult.FloorTilt.x < 0 && collResult.FloorTilt.y < 0)	// Hit angle = 0x2000
			{
				if (((unsigned short)item->Position.yRot) > 0x6000 && ((unsigned short)item->Position.yRot) < 0xe000)
				{
					item->Position.yRot = 0x2000 + (0xa000 - (unsigned short)item->Position.yRot - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity += (-collResult.FloorTilt.x) + (-collResult.FloorTilt.y);
						if ((unsigned short)item->Position.yRot > 0x2000 && (unsigned short)item->Position.yRot < 0xa000)
						{
							item->Position.yRot -= 4096;
							if ((unsigned short)item->Position.yRot < 0x2000)
								item->Position.yRot = 0x2000;
						}
						else if ((unsigned short)item->Position.yRot != 0x2000)
						{
							item->Position.yRot += 4096;
							if ((unsigned short)item->Position.yRot > 0x2000)
								item->Position.yRot = 0x2000;
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			else if (collResult.FloorTilt.x < 0 && collResult.FloorTilt.y > 0)	// Hit angle = 0x6000
			{
				if (((unsigned short)item->Position.yRot) > 0xa000 || ((unsigned short)item->Position.yRot) < 0x2000)
				{
					item->Position.yRot = 0x6000 + (0xe000 - (unsigned short)item->Position.yRot - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity += (-collResult.FloorTilt.x) + collResult.FloorTilt.y;
						if ((unsigned short)item->Position.yRot < 0xe000 && (unsigned short)item->Position.yRot > 0x6000)
						{
							item->Position.yRot -= 4096;
							if ((unsigned short)item->Position.yRot < 0x6000)
								item->Position.yRot = 0x6000;
						}
						else if ((unsigned short)item->Position.yRot != 0x6000)
						{
							item->Position.yRot += 4096;
							if ((unsigned short)item->Position.yRot > 0x6000)
								item->Position.yRot = 0x6000;
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			else if (collResult.FloorTilt.x > 0 && collResult.FloorTilt.y > 0)	// Hit angle = 0xa000
			{
				if (((unsigned short)item->Position.yRot) > 0xe000 || ((unsigned short)item->Position.yRot) < 0x6000)
				{
					item->Position.yRot = 0xa000 + (0x2000 - (unsigned short)item->Position.yRot - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity += collResult.FloorTilt.x + collResult.FloorTilt.y;
						if ((unsigned short)item->Position.yRot < 0x2000 || (unsigned short)item->Position.yRot > 0xa000)
						{
							item->Position.yRot -= 4096;
							if ((unsigned short)item->Position.yRot < 0xa000)
								item->Position.yRot = 0xa000;
						}
						else if ((unsigned short)item->Position.yRot != 0xa000)
						{
							item->Position.yRot += 4096;
							if ((unsigned short)item->Position.yRot > 0xa000)
								item->Position.yRot = 0xa000;
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			else if (collResult.FloorTilt.x > 0 && collResult.FloorTilt.y < 0)	// Hit angle = 0xe000
			{
				if (((unsigned short)item->Position.yRot) > 0x2000 && ((unsigned short)item->Position.yRot) < 0xa000)
				{
					item->Position.yRot = 0xe000 + (0x6000 - (unsigned short)item->Position.yRot - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity += collResult.FloorTilt.x + (-collResult.FloorTilt.y);
						if ((unsigned short)item->Position.yRot < 0x6000 || (unsigned short)item->Position.yRot > 0xe000)
						{
							item->Position.yRot -= 4096;
							if ((unsigned short)item->Position.yRot < 0xe000)
								item->Position.yRot = 0xe000;
						}
						else if ((unsigned short)item->Position.yRot != 0xe000)
						{
							item->Position.yRot += 4096;
							if ((unsigned short)item->Position.yRot > 0xe000)
								item->Position.yRot = 0xe000;
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
					else
						item->Animation.VerticalVelocity = 0;
				}
			}

			/* Put item back in its last position */
			item->Position.xPos = x;
			item->Position.yPos = y;
			item->Position.zPos = z;
		}
		else
		{
			/* Hit the floor; bounce and slow down */
			if (item->Animation.VerticalVelocity > 0)
			{
				if (item->Animation.VerticalVelocity > 16)
				{
					if (item->ObjectNumber == ID_GRENADE)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity - (item->Animation.VerticalVelocity / 2));
					else
					{
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
						if (item->Animation.VerticalVelocity < -100)
							item->Animation.VerticalVelocity = -100;
					}
				}
				else
				{
					/* Roll on floor */
					item->Animation.VerticalVelocity = 0;
					if (item->ObjectNumber == ID_GRENADE)
					{
						item->Animation.RequiredState = 1;
						item->Position.xRot = 0;
						item->Animation.Velocity--;
					}
					else
						item->Animation.Velocity -= 3;

					if (item->Animation.Velocity < 0)
						item->Animation.Velocity = 0;
				}
			}
			item->Position.yPos = collResult.Position.Floor;
		}
	}
	else	// Check for on top of object.
	{
		if (yv >= 0)
		{
			oldCollResult = GetCollision(item->Position.xPos, y, item->Position.zPos, item->RoomNumber);
			collResult = GetCollision(item);

			// Bounce off floor.

			// Removed weird OnObject global check from here which didnt make sense because OnObject
			// was always set to 0 by GetHeight() function which was called before the check.
			// Possibly a mistake or unfinished feature by Core? -- Lwmte, 27.08.21

			if (item->Position.yPos >= oldCollResult.Position.Floor)
			{
				/* Hit the floor; bounce and slow down */
				if (item->Animation.VerticalVelocity > 0)
				{
					if (item->Animation.VerticalVelocity > 16)
					{
						if (item->ObjectNumber == ID_GRENADE)
							item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity - (item->Animation.VerticalVelocity / 2));
						else
						{
							item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 4);
							if (item->Animation.VerticalVelocity < -100)
								item->Animation.VerticalVelocity = -100;
						}
					}
					else
					{
						/* Roll on floor */
						item->Animation.VerticalVelocity = 0;
						if (item->ObjectNumber == ID_GRENADE)
						{
							item->Animation.RequiredState = 1;
							item->Position.xRot = 0;
							item->Animation.Velocity--;
						}
						else
							item->Animation.Velocity -= 3;

						if (item->Animation.Velocity < 0)
							item->Animation.Velocity = 0;
					}
				}
				item->Position.yPos = oldCollResult.Position.Floor;
			}
		}
		//		else
		{
			/* Bounce off ceiling */
			collResult = GetCollision(item);

			if (item->Position.yPos < collResult.Position.Ceiling)
			{
				if (y < collResult.Position.Ceiling &&
					(((x / SECTOR(1)) != (item->Position.xPos / SECTOR(1))) ||
						((z / SECTOR(1)) != (item->Position.zPos / SECTOR(1)))))
				{
					// Need to know which direction the wall is.

					if ((x & (~(WALL_SIZE - 1))) != (item->Position.xPos & (~(WALL_SIZE - 1))))	// X crossed boundary?
					{
						if (xv <= 0)	// Hit angle = 0xc000.
							item->Position.yRot = 0x4000 + (0xc000 - item->Position.yRot);
						else			// Hit angle = 0x4000.
							item->Position.yRot = 0xc000 + (0x4000 - item->Position.yRot);
					}
					else		// Z crossed boundary.
					{
						item->Position.yRot = 0x8000 - item->Position.yRot;
					}

					if (item->ObjectNumber == ID_GRENADE)
						item->Animation.Velocity -= item->Animation.Velocity / 8;
					else
						item->Animation.Velocity /= 2;

					/* Put item back in its last position */
					item->Position.xPos = x;
					item->Position.yPos = y;
					item->Position.zPos = z;
				}
				else
					item->Position.yPos = collResult.Position.Ceiling;

				if (item->Animation.VerticalVelocity < 0)
					item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity;
			}
		}
	}

	collResult = GetCollision(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber);
	if (collResult.RoomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, collResult.RoomNumber);
}

void DoObjectCollision(ITEM_INFO* laraItem, CollisionInfo* coll) // previously LaraBaddieCollision
{
	laraItem->HitStatus = false;
	coll->HitStatic = false;

	if (laraItem == LaraItem)
		Lara.HitDirection = -1;

	if (laraItem->HitPoints > 0)
	{
		short* door, numDoors;
		for (auto i : GetRoomList(laraItem->RoomNumber))
		{
			short itemNumber = g_Level.Rooms[i].itemNumber;
			while (itemNumber != NO_ITEM)
			{
				auto* item = &g_Level.Items[itemNumber];
				if (item->Collidable && item->Status != ITEM_INVISIBLE)
				{
					auto* object = &Objects[item->ObjectNumber];
					if (object->collision != nullptr)
					{
						if (phd_Distance(&item->Position, &laraItem->Position) < COLLISION_CHECK_DISTANCE)
							object->collision(itemNumber, laraItem, coll);
					}
				}

				itemNumber = item->NextItem;
			}

			for (int j = 0; j < g_Level.Rooms[i].mesh.size(); j++)
			{
				auto* mesh = &g_Level.Rooms[i].mesh[j];

				// Only process meshes which are visible and non-solid
				if ((mesh->flags & StaticMeshFlags::SM_VISIBLE) && !(mesh->flags & StaticMeshFlags::SM_SOLID))
				{
					if (phd_Distance(&mesh->pos, &laraItem->Position) < COLLISION_CHECK_DISTANCE)
					{
						if (TestBoundsCollideStatic(laraItem, mesh, coll->Setup.Radius))
						{
							coll->HitStatic = true;

							if (coll->Setup.EnableObjectPush)
								ItemPushStatic(laraItem, mesh, coll);
							else
								break;
						}
					}
				}
			}
		}

		if (laraItem == LaraItem && Lara.HitDirection == -1)
			Lara.HitFrame = 0;
	}
}

void AIPickupCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->ObjectNumber == ID_SHOOT_SWITCH1 && !(item->MeshBits & 1))
		item->Status = ITEM_INVISIBLE;
}

void ObjectCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TestBoundsCollide(item, laraItem, coll->Setup.Radius))
	{
		if (TestCollision(item, laraItem))
		{
			if (coll->Setup.EnableObjectPush)
				ItemPushItem(item, laraItem, coll, false, true);
		}
	}
}

void CreatureCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->ObjectNumber != ID_HITMAN || item->Animation.ActiveState != LS_INSERT_PUZZLE)
	{
		if (TestBoundsCollide(item, laraItem, coll->Setup.Radius))
		{
			if (TestCollision(item, laraItem))
			{
				if (coll->Setup.EnableObjectPush ||
					Lara.Control.WaterStatus == WaterStatus::Underwater ||
					Lara.Control.WaterStatus == WaterStatus::TreadWater)
				{
					ItemPushItem(item, laraItem, coll, coll->Setup.EnableSpasm, 0);
				}
				else if (coll->Setup.EnableSpasm)
				{
					int x = laraItem->Position.xPos - item->Position.xPos;
					int z = laraItem->Position.zPos - item->Position.zPos;
					float s = phd_sin(item->Position.yRot);
					float c = phd_cos(item->Position.yRot);

					auto* frame = GetBestFrame(item);
					int rx = (frame->boundingBox.X1 + frame->boundingBox.X2) / 2;
					int rz = (frame->boundingBox.X2 + frame->boundingBox.Z2) / 2;

					if (frame->boundingBox.Y2 - frame->boundingBox.Y1 > STEP_SIZE)
					{
						int angle = (laraItem->Position.yRot - phd_atan(z - c * rx - s * rz, x - c * rx + s * rz) - ANGLE(135.0f)) / ANGLE(90.0f);
						Lara.HitDirection = (short)angle;
						// TODO: check if a second Lara.hitFrame++; is required there !
						
						Lara.HitFrame++;
						if (Lara.HitFrame > 30)
							Lara.HitFrame = 30;
					}
				}
			}
		}
	}
}

void TrapCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->Status == ITEM_ACTIVE)
	{
		if (!TestBoundsCollide(item, laraItem, coll->Setup.Radius))
			return;

		TestCollision(item, laraItem);
	}
	else if (item->Status != ITEM_INVISIBLE)
		ObjectCollision(itemNumber, laraItem, coll);
}
