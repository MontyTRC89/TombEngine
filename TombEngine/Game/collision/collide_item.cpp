#include "framework.h"
#include "Game/collision/collide_item.h"

#include "Game/animation.h"
#include "Game/control/los.h"
#include "Game/collision/sphere.h"
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Game/effects/debris.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/simple_particle.h"
#include "Game/room.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Specific/trmath.h"
#include "Specific/prng.h"
#include "ScriptInterfaceGame.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;
using namespace TEN::Renderer;

BOUNDING_BOX GlobalCollisionBounds;
ItemInfo* CollidedItems[MAX_COLLIDED_OBJECTS];
MESH_INFO* CollidedMeshes[MAX_COLLIDED_OBJECTS];

void GenericSphereBoxCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->Status != ITEM_INVISIBLE)
	{
		if (TestBoundsCollide(item, laraItem, coll->Setup.Radius))
		{
			int collidedBits = TestCollision(item, laraItem);
			if (collidedBits)
			{
				short oldYOrient = item->Pose.Orientation.y;

				item->Pose.Orientation.y = 0;
				GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
				item->Pose.Orientation.y = oldYOrient;

				int deadlyBits = *((int*)&item->ItemFlags[0]);
				auto* sphere = &CreatureSpheres[0];

				if (item->ItemFlags[2] != 0)
					collidedBits &= ~1;

				while (collidedBits)
				{
					if (collidedBits & 1)
					{
						GlobalCollisionBounds.X1 = sphere->x - sphere->r - item->Pose.Position.x;
						GlobalCollisionBounds.X2 = sphere->x + sphere->r - item->Pose.Position.x;
						GlobalCollisionBounds.Y1 = sphere->y - sphere->r - item->Pose.Position.y;
						GlobalCollisionBounds.Y2 = sphere->y + sphere->r - item->Pose.Position.y;
						GlobalCollisionBounds.Z1 = sphere->z - sphere->r - item->Pose.Position.z;
						GlobalCollisionBounds.Z2 = sphere->z + sphere->r - item->Pose.Position.z;

						int x = laraItem->Pose.Position.x;
						int y = laraItem->Pose.Position.y;
						int z = laraItem->Pose.Position.z;

						if (ItemPushItem(item, laraItem, coll, deadlyBits & 1, 3) && (deadlyBits & 1))
						{
							DoDamage(laraItem, item->ItemFlags[3]);

							int dx = x - laraItem->Pose.Position.x;
							int dy = y - laraItem->Pose.Position.y;
							int dz = z - laraItem->Pose.Position.z;

							if (dx || dy || dz)
							{
								if (TriggerActive(item))
									TriggerLaraBlood();
							}

							if (!coll->Setup.EnableObjectPush)
							{
								laraItem->Pose.Position.x += dx;
								laraItem->Pose.Position.y += dy;
								laraItem->Pose.Position.z += dz;
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

bool GetCollidedObjects(ItemInfo* collidingItem, int radius, bool onlyVisible, ItemInfo** collidedItems, MESH_INFO** collidedMeshes, bool ignoreLara)
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

				if (collidingItem->Pose.Position.y + radius + CLICK(0.5f) < mesh->pos.Position.y + staticMesh->collisionBox.Y1)
					continue;

				if (collidingItem->Pose.Position.y > mesh->pos.Position.y + staticMesh->collisionBox.Y2)
					continue;

				float s = phd_sin(mesh->pos.Orientation.y);
				float c = phd_cos(mesh->pos.Orientation.y);
				float rx = (collidingItem->Pose.Position.x - mesh->pos.Position.x) * c - s * (collidingItem->Pose.Position.z - mesh->pos.Position.z);
				float rz = (collidingItem->Pose.Position.z - mesh->pos.Position.z) * c + s * (collidingItem->Pose.Position.x - mesh->pos.Position.x);

				if (radius + rx + CLICK(0.5f) < staticMesh->collisionBox.X1 || rx - radius - CLICK(0.5f) > staticMesh->collisionBox.X2)
					continue;

				if (radius + rz + CLICK(0.5f) < staticMesh->collisionBox.Z1 || rz - radius - CLICK(0.5f) > staticMesh->collisionBox.Z2)
					continue;

				collidedMeshes[numMeshes++] = mesh;

				if (!radius)
				{
					collidedItems[0] = nullptr;
					return true;
				}
			}

			collidedMeshes[numMeshes] = nullptr;
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
						item->MeshBits == NO_JOINT_BITS ||
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

					int dx = collidingItem->Pose.Position.x - item->Pose.Position.x;
					int dy = collidingItem->Pose.Position.y - item->Pose.Position.y;
					int dz = collidingItem->Pose.Position.z - item->Pose.Position.z;

					auto* framePtr = GetBestFrame(item);

					if (dx >= -SECTOR(2) && dx <= SECTOR(2) &&
						dy >= -SECTOR(2) && dy <= SECTOR(2) &&
						dz >= -SECTOR(2) && dz <= SECTOR(2) &&
						collidingItem->Pose.Position.y + radius + CLICK(0.5f) >= item->Pose.Position.y + framePtr->boundingBox.Y1 &&
						collidingItem->Pose.Position.y - radius - CLICK(0.5f) <= item->Pose.Position.y + framePtr->boundingBox.Y2)
					{
						float s = phd_sin(item->Pose.Orientation.y);
						float c = phd_cos(item->Pose.Orientation.y);

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
							}

						}
						else
						{
							if (collidingItem->Pose.Position.y + radius + 128 >= item->Pose.Position.y + framePtr->boundingBox.Y1 &&
								collidingItem->Pose.Position.y - radius - 128 <= item->Pose.Position.y + framePtr->boundingBox.Y2)
							{
								float s = phd_sin(item->Pose.Orientation.y);
								float c = phd_cos(item->Pose.Orientation.y);

								int rx = dx * c - s * dz;
								int rz = dz * c + s * dx;

								if (item->ObjectNumber == ID_TURN_SWITCH)
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

bool TestWithGlobalCollisionBounds(ItemInfo* item, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* framePtr = GetBestFrame(laraItem);

	if (item->Pose.Position.y + GlobalCollisionBounds.Y2 <= laraItem->Pose.Position.y + framePtr->boundingBox.Y1)
		return false;

	if (item->Pose.Position.y + GlobalCollisionBounds.Y1 >= framePtr->boundingBox.Y2)
		return false;

	float s = phd_sin(item->Pose.Orientation.y);
	float c = phd_cos(item->Pose.Orientation.y);

	int dx = laraItem->Pose.Position.x - item->Pose.Position.x;
	int dz = laraItem->Pose.Position.z - item->Pose.Position.z;

	int x = c * dx - s * dz;
	int z = c * dz + s * dx;

	if (x < GlobalCollisionBounds.X1 - coll->Setup.Radius ||
		x > GlobalCollisionBounds.X2 + coll->Setup.Radius ||
		z < GlobalCollisionBounds.Z1 - coll->Setup.Radius ||
		z > GlobalCollisionBounds.Z2 + coll->Setup.Radius)
		return false;

	return true;
}

void TestForObjectOnLedge(ItemInfo* item, CollisionInfo* coll)
{
	auto* bounds = GetBoundsAccurate(item);
	int height = abs(bounds->Y2 + bounds->Y1);

	for (int i = 0; i < 3; i++)
	{
		auto s = (i != 1) ? phd_sin(coll->Setup.ForwardAngle + ANGLE((i * 90) - 90)) : 0;
		auto c = (i != 1) ? phd_cos(coll->Setup.ForwardAngle + ANGLE((i * 90) - 90)) : 0;

		auto x = item->Pose.Position.x + (s * (coll->Setup.Radius));
		auto y = item->Pose.Position.y - height - STEP_SIZE;
		auto z = item->Pose.Position.z + (c * (coll->Setup.Radius));

		auto origin = Vector3(x, y, z);
		auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(coll->Setup.ForwardAngle), 0, 0);
		auto direction = (Matrix::CreateTranslation(Vector3::UnitZ) * mxR).Translation();

		// g_Renderer.AddDebugSphere(origin, 16, Vector4::One, RENDERER_DEBUG_PAGE::DIMENSION_STATS);

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

				if (phd_Distance(&item->Pose, &item2->Pose) < COLLISION_CHECK_DISTANCE)
				{
					auto box = TO_DX_BBOX(item2->Pose, GetBoundsAccurate(item2));
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

				if (phd_Distance(&item->Pose, &mesh->pos) < COLLISION_CHECK_DISTANCE)
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

bool TestLaraPosition(OBJECT_COLLISION_BOUNDS* bounds, ItemInfo* item, ItemInfo* laraItem)
{
	auto rotRel = laraItem->Pose.Orientation - item->Pose.Orientation;

	if (rotRel.x < bounds->rotX1)
		return false;

	if (rotRel.x > bounds->rotX2)
		return false;

	if (rotRel.y < bounds->rotY1)
		return false;

	if (rotRel.y > bounds->rotY2)
		return false;

	if (rotRel.z < bounds->rotZ1)
		return false;

	if (rotRel.z > bounds->rotZ2)
		return false;

	auto pos = (laraItem->Pose.Position - item->Pose.Position).ToVector3();

	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->Pose.Orientation.y),
		TO_RAD(item->Pose.Orientation.x),
		TO_RAD(item->Pose.Orientation.z)
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

bool AlignLaraPosition(Vector3Int* vec, ItemInfo* item, ItemInfo* laraItem)
{
	laraItem->Pose.Orientation = item->Pose.Orientation;

	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->Pose.Orientation.y),
		TO_RAD(item->Pose.Orientation.x),
		TO_RAD(item->Pose.Orientation.z)
	);

	Vector3 pos = Vector3::Transform(Vector3(vec->x, vec->y, vec->z), matrix);
	Vector3 newPos = item->Pose.Position.ToVector3() + pos;

	int height = GetCollision(newPos.x, newPos.y, newPos.z, laraItem->RoomNumber).Position.Floor;
	if (abs(height - laraItem->Pose.Position.y) <= CLICK(2))
	{
		laraItem->Pose.Position.x = newPos.x;
		laraItem->Pose.Position.y = newPos.y;
		laraItem->Pose.Position.z = newPos.z;
		return true;
	}

	auto* lara = GetLaraInfo(laraItem);
	if (lara->Control.IsMoving)
	{
		lara->Control.IsMoving = false;
		lara->Control.HandStatus = HandStatus::Free;
	}

	return false;
}

bool MoveLaraPosition(Vector3Int* vec, ItemInfo* item, ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	auto dest = PHD_3DPOS(item->Pose.Orientation);

	Vector3 pos = Vector3(vec->x, vec->y, vec->z);

	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->Pose.Orientation.y),
		TO_RAD(item->Pose.Orientation.x),
		TO_RAD(item->Pose.Orientation.z)
	);

	pos = Vector3::Transform(pos, matrix);

	dest.Position.x = item->Pose.Position.x + pos.x;
	dest.Position.y = item->Pose.Position.y + pos.y;
	dest.Position.z = item->Pose.Position.z + pos.z;

	if (item->ObjectNumber != ID_FLARE_ITEM && item->ObjectNumber != ID_BURNING_TORCH_ITEM)
		return Move3DPosTo3DPos(&laraItem->Pose, &dest, LARA_VELOCITY, ANGLE(2.0f));

	int height = GetCollision(dest.Position.x, dest.Position.y, dest.Position.z, laraItem->RoomNumber).Position.Floor;
	if (abs(height - laraItem->Pose.Position.y) <= CLICK(2))
	{
		auto direction = dest.Position - laraItem->Pose.Position;

		float distance = sqrt(pow(direction.x, 2) + pow(direction.y, 2) + pow(direction.z, 2));
		if (distance < CLICK(0.5f))
			return true;

		return Move3DPosTo3DPos(&laraItem->Pose, &dest, LARA_VELOCITY, ANGLE(2.0f));
	}

	if (lara->Control.IsMoving)
	{
		lara->Control.IsMoving = false;
		lara->Control.HandStatus = HandStatus::Free;
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
	GameVector target;
	target.x = pos->Position.x - LaraItem->Pose.Position.x;
	target.y = pos->Position.y - LaraItem->Pose.Position.y;
	target.z = pos->Position.z - LaraItem->Pose.Position.z;

	if (!ItemCollide(target.y, ITEM_RADIUS_YMAX))
		return false;

	if (!ItemCollide(target.x, radius) || !ItemCollide(target.z, radius))
		return false;

	if (!ItemInRange(target.x, target.z, radius))
		return false;

	auto* bounds = GetBoundsAccurate(LaraItem);
	if (target.y >= bounds->Y1 && target.y <= (bounds->Y2 + LARA_RADIUS))
		return true;

	return false;
}

bool ItemNearTarget(PHD_3DPOS* src, ItemInfo* target, int radius)
{
	auto pos = src->Position - target->Pose.Position;

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
	auto direction = dest->Position - src->Position;
	int distance = sqrt(pow(direction.x, 2) + pow(direction.y, 2) + pow(direction.z, 2));

	if (velocity < distance)
		src->Position += direction * velocity / distance;
	else
		src->Position = dest->Position;

	if (!Lara.Control.IsMoving)
	{
		if (Lara.Control.WaterStatus != WaterStatus::Underwater)
		{
			int angle = mGetAngle(dest->Position.x, dest->Position.z, src->Position.x, src->Position.z);
			int direction = (GetQuadrant(angle) - GetQuadrant(dest->Orientation.y)) & 3;

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

	short deltaAngle = dest->Orientation.x - src->Orientation.x;
	if (deltaAngle > angleAdd)
		src->Orientation.x += angleAdd;
	else if (deltaAngle < -angleAdd)
		src->Orientation.x -= angleAdd;
	else
		src->Orientation.x = dest->Orientation.x;

	deltaAngle = dest->Orientation.y - src->Orientation.y;
	if (deltaAngle > angleAdd)
		src->Orientation.y += angleAdd;
	else if (deltaAngle < -angleAdd)
		src->Orientation.y -= angleAdd;
	else
		src->Orientation.y = dest->Orientation.y;

	deltaAngle = dest->Orientation.z - src->Orientation.z;
	if (deltaAngle > angleAdd)
		src->Orientation.z += angleAdd;
	else if (deltaAngle < -angleAdd)
		src->Orientation.z -= angleAdd;
	else
		src->Orientation.z = dest->Orientation.z;

	return (src->Position == dest->Position && src->Orientation == dest->Orientation);
}

bool TestBoundsCollide(ItemInfo* item, ItemInfo* laraItem, int radius)
{
	auto bounds = (BOUNDING_BOX*)GetBestFrame(item);
	auto laraBounds = (BOUNDING_BOX*)GetBestFrame(laraItem);

	if (item->Pose.Position.y + bounds->Y2 <= laraItem->Pose.Position.y + laraBounds->Y1)
		return false;

	if (item->Pose.Position.y + bounds->Y1 >= laraItem->Pose.Position.y + laraBounds->Y2)
		return false;

	float sinY = phd_sin(item->Pose.Orientation.y);
	float cosY = phd_cos(item->Pose.Orientation.y);

	int x = laraItem->Pose.Position.x - item->Pose.Position.x;
	int z = laraItem->Pose.Position.z - item->Pose.Position.z;
	int dx = (cosY * x) - (sinY * z);
	int dz = (cosY * z) + (sinY * x);

	if (dx >= bounds->X1 - radius &&
		dx <= radius + bounds->X2 &&
		dz >= bounds->Z1 - radius &&
		dz <= radius + bounds->Z2)
	{
		return true;
	}

	return false;
}

bool TestBoundsCollideStatic(ItemInfo* item, MESH_INFO* mesh, int radius)
{
	auto bounds = StaticObjects[mesh->staticNumber].collisionBox;

	if (!(bounds.Z2 != 0 || bounds.Z1 != 0 || bounds.X1 != 0 || bounds.X2 != 0 || bounds.Y1 != 0 || bounds.Y2 != 0))
		return false;

	auto* frame = GetBestFrame(item);
	if (mesh->pos.Position.y + bounds.Y2 <= item->Pose.Position.y + frame->boundingBox.Y1)
		return false;

	if (mesh->pos.Position.y + bounds.Y1 >= item->Pose.Position.y + frame->boundingBox.Y2)
		return false;

	float sinY = phd_sin(mesh->pos.Orientation.y);
	float cosY = phd_cos(mesh->pos.Orientation.y);

	int x = item->Pose.Position.x - mesh->pos.Position.x;
	int z = item->Pose.Position.z - mesh->pos.Position.z;
	int dx = cosY * x - sinY * z;
	int dz = cosY * z + sinY * x;

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

bool ItemPushItem(ItemInfo* item, ItemInfo* item2, CollisionInfo* coll, bool spasmEnabled, char bigPush) // previously ItemPushLara
{
	// Get item's rotation
	float sinY = phd_sin(item->Pose.Orientation.y);
	float cosY = phd_cos(item->Pose.Orientation.y);

	// Get vector from item to Lara
	int dx = item2->Pose.Position.x - item->Pose.Position.x;
	int dz = item2->Pose.Position.z - item->Pose.Position.z;

	// Rotate Lara vector into item frame
	int rx = cosY * dx - sinY * dz;
	int rz = cosY * dz + sinY * dx;

	BOUNDING_BOX* bounds;
	if (bigPush & 2)
		bounds = &GlobalCollisionBounds;
	else
		bounds = (BOUNDING_BOX*)GetBestFrame(item);

	int minX = bounds->X1;
	int maxX = bounds->X2;
	int minZ = bounds->Z1;
	int maxZ = bounds->Z2;

	if (bigPush & 1)
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

	item2->Pose.Position.x = item->Pose.Position.x + cosY * rx + sinY * rz;
	item2->Pose.Position.z = item->Pose.Position.z + cosY * rz - sinY * rx;

	auto* lara = item2->IsLara() ? GetLaraInfo(item2) : nullptr;

	if (lara != nullptr && spasmEnabled && bounds->Y2 - bounds->Y1 > CLICK(1))
	{
		rx = (bounds->X1 + bounds->X2) / 2;
		rz = (bounds->Z1 + bounds->Z2) / 2;

		dx -= cosY * rx + sinY * rz;
		dz -= cosY * rz - sinY * rx;

		lara->HitDirection = (item2->Pose.Orientation.y - phd_atan(dz, dz) - ANGLE(135.0f)) / ANGLE(90.0f);
		DoDamage(item2, 0); // Dummy hurt call. Only for ooh sound!

		lara->HitFrame++;
		if (lara->HitFrame > 34)
			lara->HitFrame = 34;
	}

	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.UpperCeilingBound = MAX_HEIGHT;

	auto facing = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = phd_atan(item2->Pose.Position.z - coll->Setup.OldPosition.z, item2->Pose.Position.x - coll->Setup.OldPosition.x);

	GetCollisionInfo(coll, item2);

	coll->Setup.ForwardAngle = facing;

	if (coll->CollisionType == CT_NONE)
	{
		coll->Setup.OldPosition = item2->Pose.Position;

		// Commented because causes Lara to jump out of the water if she touches an object on the surface. re: "kayak bug"
		// UpdateItemRoom(item2, -10);
	}
	else
	{
		item2->Pose.Position.x = coll->Setup.OldPosition.x;
		item2->Pose.Position.z = coll->Setup.OldPosition.z;
	}

	// If Lara is in the process of aligning to an object, cancel it.
	if (lara != nullptr && lara->Control.Count.PositionAdjust > (LARA_POSITION_ADJUST_MAX_TIME / 6))
	{
		Lara.Control.IsMoving = false;
		Lara.Control.HandStatus = HandStatus::Free;
	}

	return true;
}

bool ItemPushStatic(ItemInfo* item, MESH_INFO* mesh, CollisionInfo* coll) // previously ItemPushLaraStatic
{
	auto bounds = StaticObjects[mesh->staticNumber].collisionBox;

	float sinY = phd_sin(mesh->pos.Orientation.y);
	float cosY = phd_cos(mesh->pos.Orientation.y);
	
	auto dx = item->Pose.Position.x - mesh->pos.Position.x;
	auto dz = item->Pose.Position.z - mesh->pos.Position.z;
	auto rx = cosY * dx - sinY * dz;
	auto rz = cosY * dz + sinY * dx;
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

	item->Pose.Position.x = mesh->pos.Position.x + cosY * rx + sinY * rz;
	item->Pose.Position.z = mesh->pos.Position.z + cosY * rz - sinY * rx;

	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;

	auto oldFacing = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = phd_atan(item->Pose.Position.z - coll->Setup.OldPosition.z, item->Pose.Position.x - coll->Setup.OldPosition.x);

	GetCollisionInfo(coll, item);

	coll->Setup.ForwardAngle = oldFacing;

	if (coll->CollisionType == CT_NONE)
	{
		coll->Setup.OldPosition = item->Pose.Position;
		if (item->IsLara())
			UpdateItemRoom(item, -10);
	}
	else
	{
		item->Pose.Position.x = coll->Setup.OldPosition.x;
		item->Pose.Position.z = coll->Setup.OldPosition.z;
	}

	// If Lara is in the process of aligning to an object, cancel it.
	if (item->IsLara() && Lara.Control.IsMoving && Lara.Control.Count.PositionAdjust > (LARA_POSITION_ADJUST_MAX_TIME / 6))
	{
		auto* lara = GetLaraInfo(item);
		lara->Control.IsMoving = false;
		lara->Control.HandStatus = HandStatus::Free;
	}

	return true;
}

void CollideSolidStatics(ItemInfo* item, CollisionInfo* coll)
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
				if (phd_Distance(&item->Pose, &mesh->pos) < COLLISION_CHECK_DISTANCE)
				{
					auto staticInfo = &StaticObjects[mesh->staticNumber];
					if (CollideSolidBounds(item, staticInfo->collisionBox, mesh->pos, coll))
						coll->HitStatic = true;
				}
			}
		}
	}
}

bool CollideSolidBounds(ItemInfo* item, BOUNDING_BOX box, PHD_3DPOS pos, CollisionInfo* coll)
{
	bool result = false;

	// Get DX static bounds in global coords
	auto staticBounds = TO_DX_BBOX(pos, &box);

	// Get local TR bounds and DX item bounds in global coords
	auto itemBBox = GetBoundsAccurate(item);
	auto itemBounds = TO_DX_BBOX(item->Pose, itemBBox);

	// Extend bounds a bit for visual testing
	itemBounds.Extents = itemBounds.Extents + Vector3(WALL_SIZE);

	// Filter out any further checks if static isn't nearby
	if (!staticBounds.Intersects(itemBounds))
		return false;

	// Bring back extents
	itemBounds.Extents = itemBounds.Extents - Vector3(WALL_SIZE);

	// Draw static bounds
	g_Renderer.AddDebugBox(staticBounds, Vector4(1, 0.3f, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

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
	auto collBounds = TO_DX_BBOX(PHD_3DPOS(item->Pose.Position), &collBox);
	bool intersects = staticBounds.Intersects(collBounds);

	// Check if previous item horizontal position intersects bounds
	auto oldCollBounds = TO_DX_BBOX(PHD_3DPOS(coll->Setup.OldPosition.x, item->Pose.Position.y, coll->Setup.OldPosition.z), &collBox);
	bool oldHorIntersects = staticBounds.Intersects(oldCollBounds);

	// Draw item coll bounds
	g_Renderer.AddDebugBox(collBounds, intersects ? Vector4(1, 0, 0, 1) : Vector4(0, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

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
	auto center = item->Pose.Position.y - height / 2;

	// Do a series of angular tests with 90 degree steps to determine top/bottom collision.

	int closestPlane = -1;
	Ray closestRay;
	auto minDistance = std::numeric_limits<float>::max();

	for (int i = 0; i < 4; i++)
	{
		// Calculate ray direction
		auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(item->Pose.Orientation.y), TO_RAD(item->Pose.Orientation.x + (ANGLE(90 * i))), 0);
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
		if (intersects && oldHorIntersects && minDistance < height)
		{
			if (bottom)
			{
				// HACK: additionally subtract 2 from bottom plane, or else false positives may occur.
				item->Pose.Position.y += distanceToVerticalPlane + 2;
				coll->CollisionType = CT_TOP;
			}
			else
			{
				// Set collision type only if dry room (in water rooms it causes stucking)
				item->Pose.Position.y -= distanceToVerticalPlane;
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
	if (!staticBounds.Intersects(TO_DX_BBOX(PHD_3DPOS(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z), &collBox)))
		return result;

	// Determine identity rotation/distance
	auto distance = Vector3(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z) - Vector3(pos.Position.x, pos.Position.y, pos.Position.z);
	auto c = phd_cos(pos.Orientation.y);
	auto s = phd_sin(pos.Orientation.y);

	// Rotate item to collision bounds identity
	auto x = round(distance.x * c - distance.z * s) + pos.Position.x;
	auto y = item->Pose.Position.y;
	auto z = round(distance.x * s + distance.z * c) + pos.Position.z;

	// Determine identity static collision bounds
	auto XMin = pos.Position.x + box.X1;
	auto XMax = pos.Position.x + box.X2;
	auto YMin = pos.Position.y + box.Y1;
	auto YMax = pos.Position.y + box.Y2;
	auto ZMin = pos.Position.z + box.Z1;
	auto ZMax = pos.Position.z + box.Z2;

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

	Vector3Int rawShift = {};

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
	distance = Vector3(coll->Setup.OldPosition.x, coll->Setup.OldPosition.y, coll->Setup.OldPosition.z) - Vector3(pos.Position.x, pos.Position.y, pos.Position.z);
	auto ox = round(distance.x * c - distance.z * s) + pos.Position.x;
	auto oz = round(distance.x * s + distance.z * c) + pos.Position.z;

	// Calculate collisison type based on identity rotation
	switch (GetQuadrant(coll->Setup.ForwardAngle - pos.Orientation.y))
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
	distance = Vector3(x + coll->Shift.x, y, z + coll->Shift.z) - Vector3(pos.Position.x, pos.Position.y, pos.Position.z);
	c = phd_cos(-pos.Orientation.y);
	s = phd_sin(-pos.Orientation.y);

	// Calculate final shifts rotation/distance
	coll->Shift.x = (round(distance.x * c - distance.z * s) + pos.Position.x) - item->Pose.Position.x;
	coll->Shift.z = (round(distance.x * s + distance.z * c) + pos.Position.z) - item->Pose.Position.z;

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

	auto* bounds = GetBoundsAccurate(item);
	int radius = abs(bounds->Y2 - bounds->Y1);

	item->Pose.Position.y += radius;

	if (item->Pose.Position.y >= collResult.Position.Floor)
	{
		bs = 0;

		if (collResult.Position.FloorSlope && oldCollResult.Position.Floor < collResult.Position.Floor)
		{
			yAngle = (long)((unsigned short)item->Pose.Orientation.y);
			if (collResult.FloorTilt.x < 0)
			{
				if (yAngle >= ANGLE(180.0f))
					bs = 1;
			}
			else if (collResult.FloorTilt.x > 0)
			{
				if (yAngle <= ANGLE(180.0f))
					bs = 1;
			}

			if (collResult.FloorTilt.y < 0)
			{
				if (yAngle >= ANGLE(90.0f) && yAngle <= ANGLE(270.0f))
					bs = 1;
			}
			else if (collResult.FloorTilt.y > 0)
			{
				if (yAngle <= ANGLE(90.0f) || yAngle >= ANGLE(270.0f))
					bs = 1;
			}
		}

		// If last position of item was also below this floor height, we've hit a wall, else we've hit a floor.

		if (y > (collResult.Position.Floor + 32) && bs == 0 &&
			(((x / SECTOR(1)) != (item->Pose.Position.x / SECTOR(1))) ||
				((z / SECTOR(1)) != (item->Pose.Position.z / SECTOR(1)))))
		{
			// Need to know which direction the wall is.

			long xs;

			if ((x & (~(WALL_SIZE - 1))) != (item->Pose.Position.x & (~(WALL_SIZE - 1))) &&	// X crossed boundary?
				(z & (~(WALL_SIZE - 1))) != (item->Pose.Position.z & (~(WALL_SIZE - 1))))	// Z crossed boundary as well?
			{
				if (abs(x - item->Pose.Position.x) < abs(z - item->Pose.Position.z))
					xs = 1;	// X has travelled the shortest, so (maybe) hit first. (Seems to work ok).
				else
					xs = 0;
			}
			else
				xs = 1;

			if ((x & (~(WALL_SIZE - 1))) != (item->Pose.Position.x & (~(WALL_SIZE - 1))) && xs)	// X crossed boundary?
			{
				// Hit angle = ANGLE(270.0f).
				if (xv <= 0)
					item->Pose.Orientation.y = ANGLE(90.0f) + (ANGLE(270.0f) - item->Pose.Orientation.y);
				// Hit angle = 0x4000.
				else
					item->Pose.Orientation.y = ANGLE(270.0f) + (ANGLE(90.0f) - item->Pose.Orientation.y);
			}
			// Z crossed boundary.
			else
				item->Pose.Orientation.y = ANGLE(180.0f) - item->Pose.Orientation.y;

			item->Animation.Velocity /= 2;

			// Put item back in its last position.
			item->Pose.Position.x = x;
			item->Pose.Position.y = y;
			item->Pose.Position.z = z;
		}
		// Hit a steep slope?
		else if (collResult.Position.FloorSlope)
		{
			// Need to know which direction the slope is.

			item->Animation.Velocity -= (item->Animation.Velocity / 4);

			// Hit angle = ANGLE(90.0f)
			if (collResult.FloorTilt.x < 0 && ((abs(collResult.FloorTilt.x)) - (abs(collResult.FloorTilt.y)) >= 2))
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(180.0f))
				{
					item->Pose.Orientation.y = ANGLE(90.0f) + (ANGLE(270.0f) - (unsigned short)item->Pose.Orientation.y - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity -= collResult.FloorTilt.x * 2;
						if ((unsigned short)item->Pose.Orientation.y > ANGLE(90.0f) && (unsigned short)item->Pose.Orientation.y < ANGLE(270.0f))
						{
							item->Pose.Orientation.y -= ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y < ANGLE(90.0f))
								item->Pose.Orientation.y = ANGLE(90.0f);
						}
						else if ((unsigned short)item->Pose.Orientation.y < ANGLE(90.0f))
						{
							item->Pose.Orientation.y += ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y > ANGLE(90.0f))
								item->Pose.Orientation.y = ANGLE(90.0f);
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			// Hit angle = ANGLE(270.0f)
			else if (collResult.FloorTilt.x > 0 && ((abs(collResult.FloorTilt.x)) - (abs(collResult.FloorTilt.y)) >= 2))
			{
				if (((unsigned short)item->Pose.Orientation.y) < ANGLE(180.0f))
				{
					item->Pose.Orientation.y = ANGLE(270.0f) + (ANGLE(90.0f) - (unsigned short)item->Pose.Orientation.y - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity += collResult.FloorTilt.x * 2;
						if ((unsigned short)item->Pose.Orientation.y > ANGLE(270.0f) || (unsigned short)item->Pose.Orientation.y < ANGLE(90.0f))
						{
							item->Pose.Orientation.y -= ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y < ANGLE(270.0f))
								item->Pose.Orientation.y = ANGLE(270.0f);
						}
						else if ((unsigned short)item->Pose.Orientation.y < ANGLE(270.0f))
						{
							item->Pose.Orientation.y += ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y > ANGLE(270.0f))
								item->Pose.Orientation.y = ANGLE(270.0f);
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			// Hit angle = 0
			else if (collResult.FloorTilt.y < 0 && ((abs(collResult.FloorTilt.y)) - (abs(collResult.FloorTilt.x)) >= 2))
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(90.0f) && ((unsigned short)item->Pose.Orientation.y) < ANGLE(270.0f))
				{
					item->Pose.Orientation.y = ANGLE(180.0f) - item->Pose.Orientation.y - 1;
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity -= collResult.FloorTilt.y * 2;

						if ((unsigned short)item->Pose.Orientation.y < ANGLE(180.0f))
						{
							item->Pose.Orientation.y -= ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y > ANGLE(337.5))
								item->Pose.Orientation.y = 0;
						}
						else if ((unsigned short)item->Pose.Orientation.y >= ANGLE(180.0f))
						{
							item->Pose.Orientation.y += ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y < ANGLE(22.5f))
								item->Pose.Orientation.y = 0;
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			// Hit angle = ANGLE(180.0f)
			else if (collResult.FloorTilt.y > 0 && ((abs(collResult.FloorTilt.y)) - (abs(collResult.FloorTilt.x)) >= 2))
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(270.0f) || ((unsigned short)item->Pose.Orientation.y) < ANGLE(90.0f))
				{
					item->Pose.Orientation.y = ANGLE(180.0f) - item->Pose.Orientation.y - 1;
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity += collResult.FloorTilt.y * 2;

						if ((unsigned short)item->Pose.Orientation.y > ANGLE(180.0f))
						{
							item->Pose.Orientation.y -= ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y < ANGLE(180.0f))
								item->Pose.Orientation.y = ANGLE(180.0f);
						}
						else if ((unsigned short)item->Pose.Orientation.y < ANGLE(180.0f))
						{
							item->Pose.Orientation.y += ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y > ANGLE(180.0f))
								item->Pose.Orientation.y = ANGLE(180.0f);
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			else if (collResult.FloorTilt.x < 0 && collResult.FloorTilt.y < 0)	// Hit angle = 0x2000
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(135.0f) && ((unsigned short)item->Pose.Orientation.y) < ANGLE(315.0f))
				{
					item->Pose.Orientation.y = ANGLE(45.0f) + (ANGLE(225.0f) - (unsigned short)item->Pose.Orientation.y - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity += -collResult.FloorTilt.x + -collResult.FloorTilt.y;
						if ((unsigned short)item->Pose.Orientation.y > ANGLE(45.0f) && (unsigned short)item->Pose.Orientation.y < ANGLE(225.0f))
						{
							item->Pose.Orientation.y -= ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y < ANGLE(45.0f))
								item->Pose.Orientation.y = ANGLE(45.0f);
						}
						else if ((unsigned short)item->Pose.Orientation.y != ANGLE(45.0f))
						{
							item->Pose.Orientation.y += ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y > ANGLE(45.0f))
								item->Pose.Orientation.y = ANGLE(45.0f);
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			// Hit angle = ANGLE(135.0f)
			else if (collResult.FloorTilt.x < 0 && collResult.FloorTilt.y > 0)
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(225.0f) || ((unsigned short)item->Pose.Orientation.y) < ANGLE(45.0f))
				{
					item->Pose.Orientation.y = ANGLE(135.0f) + (ANGLE(315.0f) - (unsigned short)item->Pose.Orientation.y - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity += (-collResult.FloorTilt.x) + collResult.FloorTilt.y;
						if ((unsigned short)item->Pose.Orientation.y < ANGLE(315.0f) && (unsigned short)item->Pose.Orientation.y > ANGLE(135.0f))
						{
							item->Pose.Orientation.y -= ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y < ANGLE(135.0f))
								item->Pose.Orientation.y = ANGLE(135.0f);
						}
						else if ((unsigned short)item->Pose.Orientation.y != ANGLE(135.0f))
						{
							item->Pose.Orientation.y += ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y > ANGLE(135.0f))
								item->Pose.Orientation.y = ANGLE(135.0f);
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			// Hit angle = ANGLE(225.5f)
			else if (collResult.FloorTilt.x > 0 && collResult.FloorTilt.y > 0)
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(315.0f) || ((unsigned short)item->Pose.Orientation.y) < ANGLE(135.0f))
				{
					item->Pose.Orientation.y = ANGLE(225.5f) + (ANGLE(45.0f) - (unsigned short)item->Pose.Orientation.y - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity += collResult.FloorTilt.x + collResult.FloorTilt.y;
						if ((unsigned short)item->Pose.Orientation.y < ANGLE(45.0f) || (unsigned short)item->Pose.Orientation.y > ANGLE(225.5f))
						{
							item->Pose.Orientation.y -= ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y < ANGLE(225.5f))
								item->Pose.Orientation.y = ANGLE(225.5f);
						}
						else if ((unsigned short)item->Pose.Orientation.y != ANGLE(225.5f))
						{
							item->Pose.Orientation.y += ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y > ANGLE(225.5f))
								item->Pose.Orientation.y = ANGLE(225.5f);
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
					else
						item->Animation.VerticalVelocity = 0;
				}
			}
			// Hit angle = ANGLE(315.0f)
			else if (collResult.FloorTilt.x > 0 && collResult.FloorTilt.y < 0)
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(45.0f) && ((unsigned short)item->Pose.Orientation.y) < ANGLE(225.5f))
				{
					item->Pose.Orientation.y = ANGLE(315.0f) + (ANGLE(135.0f)- (unsigned short)item->Pose.Orientation.y - 1);
					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity / 2;
				}
				else
				{
					if (item->Animation.Velocity < 32)
					{
						item->Animation.Velocity += collResult.FloorTilt.x + (-collResult.FloorTilt.y);
						if ((unsigned short)item->Pose.Orientation.y < ANGLE(135.0f) || (unsigned short)item->Pose.Orientation.y > ANGLE(315.0f))
						{
							item->Pose.Orientation.y -= ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y < ANGLE(315.0f))
								item->Pose.Orientation.y = ANGLE(315.0f);
						}
						else if ((unsigned short)item->Pose.Orientation.y != ANGLE(315.0f))
						{
							item->Pose.Orientation.y += ANGLE(22.5f);
							if ((unsigned short)item->Pose.Orientation.y > ANGLE(315.0f))
								item->Pose.Orientation.y = ANGLE(315.0f);
						}
					}

					if (item->Animation.VerticalVelocity > 0)
						item->Animation.VerticalVelocity = -(item->Animation.VerticalVelocity / 2);
					else
						item->Animation.VerticalVelocity = 0;
				}
			}

			// Put item back in its last position.
			item->Pose.Position.x = x;
			item->Pose.Position.y = y;
			item->Pose.Position.z = z;
		}
		else
		{
			// Hit the floor; bounce and slow down.
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
					// Roll on floor.
					item->Animation.VerticalVelocity = 0;
					if (item->ObjectNumber == ID_GRENADE)
					{
						item->Animation.RequiredState = 1;
						item->Pose.Orientation.x = 0;
						item->Animation.Velocity--;
					}
					else
						item->Animation.Velocity -= 3;

					if (item->Animation.Velocity < 0)
						item->Animation.Velocity = 0;
				}
			}

			item->Pose.Position.y = collResult.Position.Floor;
		}
	}
	// Check for on top of object.
	else
	{
		if (yv >= 0)
		{
			oldCollResult = GetCollision(item->Pose.Position.x, y, item->Pose.Position.z, item->RoomNumber);
			collResult = GetCollision(item);

			// Bounce off floor.

			// Removed weird OnObject global check from here which didnt make sense because OnObject
			// was always set to 0 by GetHeight() function which was called before the check.
			// Possibly a mistake or unfinished feature by Core? -- Lwmte, 27.08.21

			if (item->Pose.Position.y >= oldCollResult.Position.Floor)
			{
				// Hit the floor; bounce and slow down.
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
						// Roll on floor.
						item->Animation.VerticalVelocity = 0;
						if (item->ObjectNumber == ID_GRENADE)
						{
							item->Animation.RequiredState = 1;
							item->Pose.Orientation.x = 0;
							item->Animation.Velocity--;
						}
						else
							item->Animation.Velocity -= 3;

						if (item->Animation.Velocity < 0)
							item->Animation.Velocity = 0;
					}
				}

				item->Pose.Position.y = oldCollResult.Position.Floor;
			}
		}
		// else
		{
			// Bounce off ceiling.
			collResult = GetCollision(item);

			if (item->Pose.Position.y < collResult.Position.Ceiling)
			{
				if (y < collResult.Position.Ceiling &&
					(((x / SECTOR(1)) != (item->Pose.Position.x / SECTOR(1))) ||
						((z / SECTOR(1)) != (item->Pose.Position.z / SECTOR(1)))))
				{
					// Need to know which direction the wall is.

					// X crossed boundary?
					if ((x & (~(WALL_SIZE - 1))) != (item->Pose.Position.x & (~(WALL_SIZE - 1))))
					{
						if (xv <= 0)	// Hit angle = ANGLE(270.0f).
							item->Pose.Orientation.y = ANGLE(90.0f) + (ANGLE(270.0f) - item->Pose.Orientation.y);
						else			// Hit angle = ANGLE(90.0f).
							item->Pose.Orientation.y = ANGLE(270.0f) + (ANGLE(90.0f) - item->Pose.Orientation.y);
					}
					// Z crossed boundary.
					else
						item->Pose.Orientation.y = 0x8000 - item->Pose.Orientation.y;

					if (item->ObjectNumber == ID_GRENADE)
						item->Animation.Velocity -= item->Animation.Velocity / 8;
					else
						item->Animation.Velocity /= 2;

					// Put item back in its last position.
					item->Pose.Position.x = x;
					item->Pose.Position.y = y;
					item->Pose.Position.z = z;
				}
				else
					item->Pose.Position.y = collResult.Position.Ceiling;

				if (item->Animation.VerticalVelocity < 0)
					item->Animation.VerticalVelocity = -item->Animation.VerticalVelocity;
			}
		}
	}

	collResult = GetCollision(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);

	if (collResult.RoomNumber != item->RoomNumber)
	{
		if (item->ObjectNumber == ID_GRENADE && TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, collResult.RoomNumber))
			Splash(item);

		ItemNewRoom(itemNumber, collResult.RoomNumber);
	}

	item->Pose.Position.y -= radius;
}

void DoObjectCollision(ItemInfo* laraItem, CollisionInfo* coll)
{
	laraItem->HitStatus = false;
	coll->HitStatic     = false;

	bool playerCollision = laraItem->IsLara();
	bool harmless = !playerCollision && (laraItem->Data.is<KayakInfo>() || laraItem->Data.is<UPVInfo>());

	if (playerCollision)
	{
		GetLaraInfo(laraItem)->HitDirection = -1;

		if (laraItem->HitPoints <= 0)
			return;
	}

	if (Objects[laraItem->ObjectNumber].intelligent)
		return;

	for (auto i : GetRoomList(laraItem->RoomNumber))
	{
		int nextItem = g_Level.Rooms[i].itemNumber;
		while (nextItem != NO_ITEM)
		{
			auto* item = &g_Level.Items[nextItem];
			int itemNumber = nextItem;

			// HACK: For some reason, sometimes an infinite loop may happen here.
			if (nextItem == item->NextItem)
				break;

			nextItem = item->NextItem;

			if (item == laraItem)
				continue;

			if (!(item->Collidable && item->Status != ITEM_INVISIBLE))
				continue;

			auto* object = &Objects[item->ObjectNumber];

			if (object->collision == nullptr)
				continue;

			if (phd_Distance(&item->Pose, &laraItem->Pose) >= COLLISION_CHECK_DISTANCE)
				continue;

			if (playerCollision)
			{
				// Objects' own collision routines were almost universally written only for
				// managing collisions with Lara and nothing else. Until all of these routines
				// are refactored (which won't happen anytime soon), we need this differentiation.
				object->collision(itemNumber, laraItem, coll);
			}
			else
			{
				if (!TestBoundsCollide(item, laraItem, coll->Setup.Radius))
					continue;

				// Guess if object is a nullmesh or invisible object by existence of draw routine.
				if (object->drawRoutine == nullptr)
					continue;

				// Pickups are also not processed.
				if (object->isPickup)
					continue;

				// If colliding object is an enemy, kill it.
				if (object->intelligent)
				{
					// Don't try to kill already dead or non-targetable enemies.
					if (item->HitPoints <= 0 || item->HitPoints == NOT_TARGETABLE)
						continue;

					if (harmless || abs(laraItem->Animation.Velocity) < VEHICLE_COLLISION_TERMINAL_VELOCITY)
					{
						// If vehicle is harmless or speed is too low, just push the enemy.
						ItemPushItem(laraItem, item, coll, false, 0);
						continue;
					}
					else
					{
						DoDamage(item, INT_MAX);

						DoLotsOfBlood(item->Pose.Position.x,
							laraItem->Pose.Position.y - CLICK(1),
							item->Pose.Position.z,
							laraItem->Animation.Velocity,
							laraItem->Pose.Orientation.y,
							item->RoomNumber, 3);
					}
				}
				else
					ItemPushItem(item, laraItem, coll, false, 1);
			}
		}

		for (int j = 0; j < g_Level.Rooms[i].mesh.size(); j++)
		{
			auto* mesh = &g_Level.Rooms[i].mesh[j];

			if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
				continue;

			// For Lara, solid static mesh collisions are directly managed by GetCollisionInfo,
			// so we bypass them here to avoid interference.
			if (playerCollision && (mesh->flags & StaticMeshFlags::SM_SOLID))
				continue;

			if (phd_Distance(&mesh->pos, &laraItem->Pose) >= COLLISION_CHECK_DISTANCE)
				continue;

			if (!TestBoundsCollideStatic(laraItem, mesh, coll->Setup.Radius))
				continue;

			coll->HitStatic = true;

			// HACK: Shatter statics only by non-harmless vehicles.
			if (!playerCollision && 
				!harmless && abs(laraItem->Animation.Velocity) > VEHICLE_COLLISION_TERMINAL_VELOCITY &&
				StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
			{
				SoundEffect(GetShatterSound(mesh->staticNumber), (PHD_3DPOS*)mesh);
				ShatterObject(nullptr, mesh, -128, laraItem->RoomNumber, 0);
				SmashedMeshRoom[SmashedMeshCount] = laraItem->RoomNumber;
				SmashedMesh[SmashedMeshCount] = mesh;
				SmashedMeshCount++;
				mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
			}
			else if (coll->Setup.EnableObjectPush)
				ItemPushStatic(laraItem, mesh, coll);
			else
				continue;
		}
	}

	if (playerCollision)
	{
		auto* lara = GetLaraInfo(laraItem);
		if (lara->HitDirection == -1)
			lara->HitFrame = 0;
	}
}

void AIPickupCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->ObjectNumber == ID_SHOOT_SWITCH1 && !(item->MeshBits & 1))
		item->Status = ITEM_INVISIBLE;
}

void ObjectCollision(short const itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
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

void CreatureCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!TestBoundsCollide(item, laraItem, coll->Setup.Radius))
		return;

	if (!TestCollision(item, laraItem))
		return;

	bool playerCollision = laraItem->IsLara();
	bool waterPlayerCollision = playerCollision && GetLaraInfo(laraItem)->Control.WaterStatus >= WaterStatus::TreadWater;

	if (waterPlayerCollision || coll->Setup.EnableObjectPush)
	{
		ItemPushItem(item, laraItem, coll, coll->Setup.EnableSpasm, 0);
	}
	else if (playerCollision && coll->Setup.EnableSpasm)
	{
		int x = laraItem->Pose.Position.x - item->Pose.Position.x;
		int z = laraItem->Pose.Position.z - item->Pose.Position.z;

		float sinY = phd_sin(item->Pose.Orientation.y);
		float cosY = phd_cos(item->Pose.Orientation.y);

		auto* frame = GetBestFrame(item);
		int rx = (frame->boundingBox.X1 + frame->boundingBox.X2) / 2;
		int rz = (frame->boundingBox.X2 + frame->boundingBox.Z2) / 2;

		if (frame->boundingBox.Y2 - frame->boundingBox.Y1 > STEP_SIZE)
		{
			int angle = (laraItem->Pose.Orientation.y - phd_atan(z - cosY * rx - sinY * rz, x - cosY * rx + sinY * rz) - ANGLE(135.0f)) / ANGLE(90.0f);

			auto* lara = GetLaraInfo(laraItem);

			lara->HitDirection = (short)angle;

			// TODO: check if a second Lara.hitFrame++; is required there !						
			lara->HitFrame++;
			if (lara->HitFrame > 30)
				lara->HitFrame = 30;
		}
	}
}

void TrapCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
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
