#include "framework.h"
#include "Game/collision/collide_item.h"

#include "Game/animation.h"
#include "Game/control/los.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/simple_particle.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/pickup/pickup.h"
#include "Game/room.h"
#include "Renderer/Renderer11.h"
#include "ScriptInterfaceGame.h"
#include "Sound/sound.h"
#include "Specific/prng.h"
#include "Specific/setup.h"
#include "Specific/trmath.h"

using namespace TEN::Math::Random;
using namespace TEN::Renderer;

BOUNDING_BOX GlobalCollisionBounds;
ItemInfo* CollidedItems[MAX_COLLIDED_OBJECTS];
MESH_INFO* CollidedMeshes[MAX_COLLIDED_OBJECTS];

constexpr auto ANIMATED_ALIGNMENT_FRAME_COUNT_THRESHOLD = 6;

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
				short prevYOrient = item->Pose.Orientation.y;

				item->Pose.Orientation.y = 0;
				GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
				item->Pose.Orientation.y = prevYOrient;

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
	for (auto i : g_Level.Rooms[collidingItem->RoomNumber].neighbors)
	{
		auto* room = &g_Level.Rooms[i];

		if (collidedMeshes)
		{
			for (int j = 0; j < room->mesh.size(); j++)
			{
				auto* mesh = &room->mesh[j];
				auto* bbox = GetBoundsAccurate(mesh, false);

				if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
					continue;

				if ((collidingItem->Pose.Position.y + radius + CLICK(0.5f)) < (mesh->pos.Position.y + bbox->Y1))
					continue;

				if (collidingItem->Pose.Position.y > (mesh->pos.Position.y + bbox->Y2))
					continue;

				float sinY = phd_sin(mesh->pos.Orientation.y);
				float cosY = phd_cos(mesh->pos.Orientation.y);

				float rx = ((collidingItem->Pose.Position.x - mesh->pos.Position.x) * cosY) - ((collidingItem->Pose.Position.z - mesh->pos.Position.z) * sinY);
				float rz = ((collidingItem->Pose.Position.z - mesh->pos.Position.z) * cosY) + ((collidingItem->Pose.Position.x - mesh->pos.Position.x) * sinY);

				if ((radius + rx + CLICK(0.5f) < bbox->X1) || (rx - radius - CLICK(0.5f) > bbox->X2))
					continue;

				if ((radius + rz + CLICK(0.5f) < bbox->Z1) || (rz - radius - CLICK(0.5f) > bbox->Z2))
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
						(Objects[item->ObjectNumber].drawRoutine == nullptr && item->ObjectNumber != ID_LARA) ||
						(Objects[item->ObjectNumber].collision == nullptr && item->ObjectNumber != ID_LARA) ||
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

					auto* FramePtr = GetBestFrame(item);

					if (dx >= -SECTOR(2) && dx <= SECTOR(2) &&
						dy >= -SECTOR(2) && dy <= SECTOR(2) &&
						dz >= -SECTOR(2) && dz <= SECTOR(2) &&
						(collidingItem->Pose.Position.y + radius + CLICK(0.5f)) >= (item->Pose.Position.y + FramePtr->boundingBox.Y1) &&
						(collidingItem->Pose.Position.y - radius - CLICK(0.5f)) <= (item->Pose.Position.y + FramePtr->boundingBox.Y2))
					{
						float sinY = phd_sin(item->Pose.Orientation.y);
						float cosY = phd_cos(item->Pose.Orientation.y);

						int rx = (dx * cosY) - (dz * sinY);
						int rz = (dz * cosY) + (dx * sinY);

						if (item->ObjectNumber == ID_TURN_SWITCH)
						{
							FramePtr->boundingBox.X1 = -CLICK(1);
							FramePtr->boundingBox.X2 = CLICK(1);
							FramePtr->boundingBox.Z1 = -CLICK(1);
							FramePtr->boundingBox.Z1 = CLICK(1);
						}

						if ((radius + rx + CLICK(0.5f)) >= FramePtr->boundingBox.X1 &&
							(rx - radius - CLICK(0.5f)) <= FramePtr->boundingBox.X2)
						{
							if ((radius + rz + CLICK(0.5f)) >= FramePtr->boundingBox.Z1 &&
								(rz - radius - CLICK(0.5f)) <= FramePtr->boundingBox.Z2)
							{
								collidedItems[numItems++] = item;
							}
						}
						else
						{
							if ((collidingItem->Pose.Position.y + radius + CLICK(0.5f)) >= (item->Pose.Position.y + FramePtr->boundingBox.Y1) &&
								(collidingItem->Pose.Position.y - radius - CLICK(0.5f)) <= (item->Pose.Position.y + FramePtr->boundingBox.Y2))
							{
								float sinY = phd_sin(item->Pose.Orientation.y);
								float cosY = phd_cos(item->Pose.Orientation.y);

								int rx = (dx * cosY) - (dz * sinY);
								int rz = (dz * cosY) + (dx * sinY);

								if (item->ObjectNumber == ID_TURN_SWITCH)
								{
									FramePtr->boundingBox.X1 = -CLICK(1);
									FramePtr->boundingBox.X2 = CLICK(1);
									FramePtr->boundingBox.Z1 = -CLICK(1);
									FramePtr->boundingBox.Z1 = CLICK(1);
								}

								if ((radius + rx + CLICK(0.5f)) >= FramePtr->boundingBox.X1 &&
									(rx - radius - CLICK(0.5f)) <= FramePtr->boundingBox.X2)
								{
									if ((radius + rz + CLICK(0.5f)) >= FramePtr->boundingBox.Z1 &&
										(rz - radius - CLICK(0.5f)) <= FramePtr->boundingBox.Z2)
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
				}
				while (itemNumber != NO_ITEM);
			}

			collidedItems[numItems] = nullptr;
		}
	}

	return (numItems || numMeshes);
}

bool TestWithGlobalCollisionBounds(ItemInfo* item, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* FramePtr = GetBestFrame(laraItem);

	if ((item->Pose.Position.y + GlobalCollisionBounds.Y2) <= (laraItem->Pose.Position.y + FramePtr->boundingBox.Y1))
		return false;

	if ((item->Pose.Position.y + GlobalCollisionBounds.Y1) >= FramePtr->boundingBox.Y2)
		return false;

	float sinY = phd_sin(item->Pose.Orientation.y);
	float cosY = phd_cos(item->Pose.Orientation.y);

	int dx = laraItem->Pose.Position.x - item->Pose.Position.x;
	int dz = laraItem->Pose.Position.z - item->Pose.Position.z;

	int x = (dx * cosY) - (dz * sinY);
	int z = (dz * cosY) + (dx * sinY);

	if (x < (GlobalCollisionBounds.X1 - coll->Setup.Radius) ||
		x > (GlobalCollisionBounds.X2 + coll->Setup.Radius) ||
		z < (GlobalCollisionBounds.Z1 - coll->Setup.Radius) ||
		z > (GlobalCollisionBounds.Z2 + coll->Setup.Radius))
	{
		return false;
	}

	return true;
}

void TestForObjectOnLedge(ItemInfo* item, CollisionInfo* coll)
{
	auto* bounds = GetBoundsAccurate(item);
	int height = abs(bounds->Y2 + bounds->Y1);

	for (int i = 0; i < 3; i++)
	{
		auto sinForwardAngle = (i != 1) ? (phd_sin(coll->Setup.ForwardAngle + ANGLE((i * 90.0f) - 90.0f))) : 0;
		auto cosForwardAngle = (i != 1) ? (phd_cos(coll->Setup.ForwardAngle + ANGLE((i * 90.0f) - 90.0f))) : 0;

		auto x = item->Pose.Position.x + (sinForwardAngle * (coll->Setup.Radius));
		auto y = item->Pose.Position.y - height - CLICK(1);
		auto z = item->Pose.Position.z + (cosForwardAngle * (coll->Setup.Radius));

		auto origin = Vector3(x, y, z);
		auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(coll->Setup.ForwardAngle), 0.0f, 0.0f);
		auto direction = (Matrix::CreateTranslation(Vector3::UnitZ) * mxR).Translation();

		// g_Renderer.AddDebugSphere(origin, 16, Vector4::One, RENDERER_DEBUG_PAGE::DIMENSION_STATS);

		for (auto i : g_Level.Rooms[item->RoomNumber].neighbors)
		{
			short itemNumber = g_Level.Rooms[i].itemNumber;
			while (itemNumber != NO_ITEM)
			{
				auto* item2 = &g_Level.Items[itemNumber];
				auto* object = &Objects[item2->ObjectNumber];

				if (object->isPickup || object->collision == nullptr || !item2->Collidable || item2->Status == ITEM_INVISIBLE)
				{
					itemNumber = item2->NextItem;
					continue;
				}

				if (Vector3Int::Distance(item->Pose.Position, item2->Pose.Position) < COLLISION_CHECK_DISTANCE)
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

			for (auto& mesh : g_Level.Rooms[i].mesh)
			{
				if (!(mesh.flags & StaticMeshFlags::SM_VISIBLE))
					continue;

				if (phd_Distance(&item->Pose, &mesh.pos) < COLLISION_CHECK_DISTANCE)
				{
					auto box = TO_DX_BBOX(mesh.pos, GetBoundsAccurate(&mesh, false));
					float distance;

					if (box.Intersects(origin, direction, distance) && distance < coll->Setup.Radius * 2)
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
	auto deltaOrient = laraItem->Pose.Orientation - item->Pose.Orientation;

	if (deltaOrient.x < bounds->rotX1)
		return false;

	if (deltaOrient.x > bounds->rotX2)
		return false;

	if (deltaOrient.y < bounds->rotY1)
		return false;

	if (deltaOrient.y > bounds->rotY2)
		return false;

	if (deltaOrient.z < bounds->rotZ1)
		return false;

	if (deltaOrient.z > bounds->rotZ2)
		return false;

	auto pos = (laraItem->Pose.Position - item->Pose.Position).ToVector3();

	auto matrix = Matrix::CreateFromYawPitchRoll(
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
	{
		return false;
	}

	return true;
}

bool AlignLaraPosition(Vector3Int* offset, ItemInfo* item, ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	laraItem->Pose.Orientation = item->Pose.Orientation;

	auto matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->Pose.Orientation.y),
		TO_RAD(item->Pose.Orientation.x),
		TO_RAD(item->Pose.Orientation.z)
	);

	auto pos = Vector3::Transform(offset->ToVector3(), matrix);
	auto target = item->Pose.Position.ToVector3() + pos;

	int height = GetCollision(target.x, target.y, target.z, laraItem->RoomNumber).Position.Floor;
	if ((laraItem->Pose.Position.y - height) <= CLICK(2))
	{
		laraItem->Pose.Position = Vector3Int(target);
		return true;
	}

	if (lara->Control.IsMoving)
	{
		lara->Control.IsMoving = false;
		lara->Control.HandStatus = HandStatus::Free;
	}

	return false;
}

bool MoveLaraPosition(Vector3Int* offset, ItemInfo* item, ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	auto matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->Pose.Orientation.y),
		TO_RAD(item->Pose.Orientation.x),
		TO_RAD(item->Pose.Orientation.z)
	);

	auto pos = Vector3::Transform(offset->ToVector3(), matrix);
	auto target = PHD_3DPOS(item->Pose.Position + Vector3Int(pos), item->Pose.Orientation);

	if (!Objects[item->ObjectNumber].isPickup)
		return Move3DPosTo3DPos(&laraItem->Pose, &target, LARA_ALIGN_VELOCITY, ANGLE(2.0f));
	else
	{
		// Prevent picking up items which can result in so called "flare pickup bug"
		int height = GetCollision(target.Position.x, target.Position.y, target.Position.z, laraItem->RoomNumber).Position.Floor;
		if (abs(height - laraItem->Pose.Position.y) <= CLICK(2))
			return Move3DPosTo3DPos(&laraItem->Pose, &target, LARA_ALIGN_VELOCITY, ANGLE(2.0f));
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
	return (value >= -radius && value <= radius);
}

static bool ItemInRange(int x, int z, int radius)
{
	return ((SQUARE(x) + SQUARE(z)) <= SQUARE(radius));
}

bool ItemNearLara(Vector3Int* origin, int radius)
{
	auto target = GameVector(
		origin->x - LaraItem->Pose.Position.x,
		origin->y - LaraItem->Pose.Position.y,
		origin->z - LaraItem->Pose.Position.z
	);

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

bool ItemNearTarget(Vector3Int* origin, ItemInfo* targetEntity, int radius)
{
	auto pos = *origin - targetEntity->Pose.Position;

	if (!ItemCollide(pos.y, ITEM_RADIUS_YMAX))
		return false;

	if (!ItemCollide(pos.x, radius) || !ItemCollide(pos.z, radius))
		return false;

	if (!ItemInRange(pos.x, pos.z, radius))
		return false;

	auto* bounds = GetBoundsAccurate(targetEntity);
	if (pos.y >= bounds->Y1 && pos.y <= bounds->Y2)
		return true;

	return false;
}

bool Move3DPosTo3DPos(PHD_3DPOS* fromPose, PHD_3DPOS* toPose, int velocity, short angleAdd)
{
	auto direction = toPose->Position - fromPose->Position;
	float distance = Vector3Int::Distance(fromPose->Position, toPose->Position);

	if (velocity < distance)
		fromPose->Position += direction * (velocity / distance);
	else
		fromPose->Position = toPose->Position;

	if (!Lara.Control.IsMoving)
	{
		bool shouldAnimate = ((distance - velocity) > (velocity * ANIMATED_ALIGNMENT_FRAME_COUNT_THRESHOLD));

		if (shouldAnimate && Lara.Control.WaterStatus != WaterStatus::Underwater)
		{
			int angle = mGetAngle(toPose->Position.x, toPose->Position.z, fromPose->Position.x, fromPose->Position.z);
			int direction = (GetQuadrant(angle) - GetQuadrant(toPose->Orientation.y)) & 3;

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

	short deltaAngle = toPose->Orientation.x - fromPose->Orientation.x;
	if (deltaAngle > angleAdd)
		fromPose->Orientation.x += angleAdd;
	else if (deltaAngle < -angleAdd)
		fromPose->Orientation.x -= angleAdd;
	else
		fromPose->Orientation.x = toPose->Orientation.x;

	deltaAngle = toPose->Orientation.y - fromPose->Orientation.y;
	if (deltaAngle > angleAdd)
		fromPose->Orientation.y += angleAdd;
	else if (deltaAngle < -angleAdd)
		fromPose->Orientation.y -= angleAdd;
	else
		fromPose->Orientation.y = toPose->Orientation.y;

	deltaAngle = toPose->Orientation.z - fromPose->Orientation.z;
	if (deltaAngle > angleAdd)
		fromPose->Orientation.z += angleAdd;
	else if (deltaAngle < -angleAdd)
		fromPose->Orientation.z -= angleAdd;
	else
		fromPose->Orientation.z = toPose->Orientation.z;

	return (fromPose->Position == toPose->Position && fromPose->Orientation == toPose->Orientation);
}

bool TestBoundsCollide(ItemInfo* item, ItemInfo* laraItem, int radius)
{
	auto* bounds = (BOUNDING_BOX*)GetBestFrame(item);
	auto* laraBounds = (BOUNDING_BOX*)GetBestFrame(laraItem);

	if ((item->Pose.Position.y + bounds->Y2) <= (laraItem->Pose.Position.y + laraBounds->Y1))
		return false;

	if ((item->Pose.Position.y + bounds->Y1) >= (laraItem->Pose.Position.y + laraBounds->Y2))
		return false;

	float sinY = phd_sin(item->Pose.Orientation.y);
	float cosY = phd_cos(item->Pose.Orientation.y);

	int x = laraItem->Pose.Position.x - item->Pose.Position.x;
	int z = laraItem->Pose.Position.z - item->Pose.Position.z;
	int dx = (x * cosY) - (z * sinY);
	int dz = (z * cosY) + (x * sinY);

	if (dx >= (bounds->X1 - radius) &&
		dx <= (radius + bounds->X2) &&
		dz >= (bounds->Z1 - radius) &&
		dz <= (radius + bounds->Z2))
	{
		return true;
	}

	return false;
}

bool TestBoundsCollideStatic(ItemInfo* item, MESH_INFO* mesh, int radius)
{
	auto bounds = GetBoundsAccurate(mesh, false);

	if (!(bounds->Z2 != 0 || bounds->Z1 != 0 || bounds->X1 != 0 || bounds->X2 != 0 || bounds->Y1 != 0 || bounds->Y2 != 0))
		return false;

	auto* frame = GetBestFrame(item);
	if (mesh->pos.Position.y + bounds->Y2 <= item->Pose.Position.y + frame->boundingBox.Y1)
		return false;

	if (mesh->pos.Position.y + bounds->Y1 >= item->Pose.Position.y + frame->boundingBox.Y2)
		return false;

	float sinY = phd_sin(mesh->pos.Orientation.y);
	float cosY = phd_cos(mesh->pos.Orientation.y);

	int x = item->Pose.Position.x - mesh->pos.Position.x;
	int z = item->Pose.Position.z - mesh->pos.Position.z;
	int dx = cosY * x - sinY * z;
	int dz = cosY * z + sinY * x;

	if (dx <= radius + bounds->X2 &&
		dx >= bounds->X1 - radius &&
		dz <= radius + bounds->Z2 &&
		dz >= bounds->Z1 - radius)
	{
		return true;
	}
	else
		return false;
}

// NOTE: Previously ItemPushLara().
bool ItemPushItem(ItemInfo* item, ItemInfo* item2, CollisionInfo* coll, bool spasmEnabled, char bigPush)
{
	// Get item's rotation.
	float sinY = phd_sin(item->Pose.Orientation.y);
	float cosY = phd_cos(item->Pose.Orientation.y);

	// Get vector from item to Lara.
	int dx = item2->Pose.Position.x - item->Pose.Position.x;
	int dz = item2->Pose.Position.z - item->Pose.Position.z;

	// Rotate Lara vector into item frame.
	int rx = (dx * cosY) - (dz * sinY);
	int rz = (dz * cosY) + (dx * sinY);

	auto* bounds = (bigPush & 2) ? &GlobalCollisionBounds : (BOUNDING_BOX*)GetBestFrame(item);

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

	item2->Pose.Position.x = item->Pose.Position.x + (rx * cosY) + (rz * sinY);
	item2->Pose.Position.z = item->Pose.Position.z + (rz * cosY) - (rx * sinY);

	auto* lara = item2->IsLara() ? GetLaraInfo(item2) : nullptr;

	if (lara != nullptr && spasmEnabled && (bounds->Y2 - bounds->Y1) > CLICK(1))
	{
		rx = (bounds->X1 + bounds->X2) / 2;
		rz = (bounds->Z1 + bounds->Z2) / 2;

		dx -= (rx * cosY) + (rz * sinY);
		dz -= (rz * cosY) - (rx * sinY);

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

	auto headingAngle = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = phd_atan(item2->Pose.Position.z - coll->Setup.OldPosition.z, item2->Pose.Position.x - coll->Setup.OldPosition.x);

	GetCollisionInfo(coll, item2);

	coll->Setup.ForwardAngle = headingAngle;

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
		lara->Control.IsMoving = false;
		lara->Control.HandStatus = HandStatus::Free;
	}

	return true;
}

// NOTE: Previously ItemPushLaraStatic().
bool ItemPushStatic(ItemInfo* item, MESH_INFO* mesh, CollisionInfo* coll)
{
	auto bounds = GetBoundsAccurate(mesh, false);

	float sinY = phd_sin(mesh->pos.Orientation.y);
	float cosY = phd_cos(mesh->pos.Orientation.y);
	
	auto dx = item->Pose.Position.x - mesh->pos.Position.x;
	auto dz = item->Pose.Position.z - mesh->pos.Position.z;
	auto rx = cosY * dx - sinY * dz;
	auto rz = cosY * dz + sinY * dx;
	auto minX = bounds->X1 - coll->Setup.Radius;
	auto maxX = bounds->X2 + coll->Setup.Radius;
	auto minZ = bounds->Z1 - coll->Setup.Radius;
	auto maxZ = bounds->Z2 + coll->Setup.Radius;

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

	for (auto i : g_Level.Rooms[item->RoomNumber].neighbors)
	{
		for (int j = 0; j < g_Level.Rooms[i].mesh.size(); j++)
		{
			auto* mesh = &g_Level.Rooms[i].mesh[j];

			// Only process meshes which are visible and solid.
			if ((mesh->flags & StaticMeshFlags::SM_VISIBLE) && (mesh->flags & StaticMeshFlags::SM_SOLID))
			{
				if (phd_Distance(&item->Pose, &mesh->pos) < COLLISION_CHECK_DISTANCE)
				{
					auto staticInfo = &StaticObjects[mesh->staticNumber];
					if (CollideSolidBounds(item, GetBoundsAccurate(mesh, false), mesh->pos, coll))
						coll->HitStatic = true;
				}
			}
		}
	}
}

bool CollideSolidBounds(ItemInfo* item, BOUNDING_BOX* box, PHD_3DPOS pose, CollisionInfo* coll)
{
	bool result = false;

	// Get DX static bounds in global coordinates.
	auto staticBounds = TO_DX_BBOX(pose, box);

	// Get local TR bounds and DX item bounds in global coordinates.
	auto itemBBox = GetBoundsAccurate(item);
	auto itemBounds = TO_DX_BBOX(item->Pose, itemBBox);

	// Extend bounds a bit for visual testing.
	itemBounds.Extents = itemBounds.Extents + Vector3(SECTOR(1));

	// Filter out any further checks if static isn't nearby.
	if (!staticBounds.Intersects(itemBounds))
		return false;

	// Bring back extents.
	itemBounds.Extents = itemBounds.Extents - Vector3(SECTOR(1));

	// Draw static bounds.
	g_Renderer.AddDebugBox(staticBounds, Vector4(1, 0.3f, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	// Calculate horizontal item collision bounds according to radius.
	BOUNDING_BOX collBox;
	collBox.X1 = -coll->Setup.Radius;
	collBox.X2 = coll->Setup.Radius;
	collBox.Z1 = -coll->Setup.Radius;
	collBox.Z2 = coll->Setup.Radius;

	// Calculate vertical item collision bounds according to either height (land mode) or precise bounds (water mode).
	// Water mode needs special processing because height calculation in original engines is inconsistent in such cases.
	if (TestEnvironment(ENV_FLAG_WATER, item))
	{
		collBox.Y1 = itemBBox->Y1;
		collBox.Y2 = itemBBox->Y2;
	}
	else
	{
		collBox.Y1 = -coll->Setup.Height;
		collBox.Y2 = 0;
	}

	// Get and test DX item collision bounds.
	auto collBounds = TO_DX_BBOX(PHD_3DPOS(item->Pose.Position), &collBox);
	bool intersects = staticBounds.Intersects(collBounds);

	// Check if previous item horizontal position intersects bounds.
	auto oldCollBounds = TO_DX_BBOX(PHD_3DPOS(coll->Setup.OldPosition.x, item->Pose.Position.y, coll->Setup.OldPosition.z), &collBox);
	bool oldHorIntersects = staticBounds.Intersects(oldCollBounds);

	// Draw item coll bounds.
	g_Renderer.AddDebugBox(collBounds, intersects ? Vector4(1, 0, 0, 1) : Vector4(0, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	// Decompose static bounds into top/bottom plane vertices.
	Vector3 corners[8];
	staticBounds.GetCorners(corners);
	Vector3 planeVertices[4][3] =
	{
		{ corners[0], corners[4], corners[1] },
		{ corners[5], corners[4], corners[1] },
		{ corners[3], corners[6], corners[7] },
		{ corners[3], corners[6], corners[2] }
	};

	// Determine collision box vertical dimensions.
	auto height = collBox.Height();
	auto center = item->Pose.Position.y - height / 2;

	// Do a series of angular tests with 90 degree steps to determine top/bottom collision.

	int closestPlane = -1;
	Ray closestRay;
	auto minDistance = std::numeric_limits<float>::max();

	for (int i = 0; i < 4; i++)
	{
		// Calculate ray direction.
		auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(item->Pose.Orientation.y), TO_RAD(item->Pose.Orientation.x + (ANGLE(90 * i))), 0);
		auto mxT = Matrix::CreateTranslation(Vector3::UnitY);
		auto direction = (mxT * mxR).Translation();

		// Make a ray and do ray tests against all decomposed planes.
		auto ray = Ray(collBounds.Center, direction);

		// Determine if top/bottom planes are closest ones or not.
		for (int p = 0; p < 4; p++)
		{
			// No plane intersection, quickly discard.
			float d = 0.0f;
			if (!ray.Intersects(planeVertices[p][0], planeVertices[p][1], planeVertices[p][2], d))
				continue;

			// Process plane intersection only if distance is smaller than already found minimum.
			if (d < minDistance)
			{
				closestRay = ray;
				closestPlane = p;
				minDistance = d;
			}
		}
	}

	// Top/bottom plane found.
	if (closestPlane != -1)
	{
		auto bottom = closestPlane >= 2;
		auto yPoint = abs((closestRay.direction * minDistance).y);
		auto distanceToVerticalPlane = height / 2 - yPoint;

		// Correct position according to top/bottom bounds, if collided and plane is nearby.
		if (intersects && oldHorIntersects && minDistance < height)
		{
			if (bottom)
			{
				// HACK: Additionally subtract 2 from bottom plane, otherwise false positives may occur.
				item->Pose.Position.y += distanceToVerticalPlane + 2;
				coll->CollisionType = CT_TOP;
			}
			else
			{
				// Set collision type only if dry room (in water rooms the player can get stuck).
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

	// Check if bounds still collide after top/bottom position correction.
	if (!staticBounds.Intersects(TO_DX_BBOX(PHD_3DPOS(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z), &collBox)))
		return result;

	// Determine identity orientation/distance.
	auto distance = (item->Pose.Position - pose.Position).ToVector3();
	auto sinY = phd_sin(pose.Orientation.y);
	auto cosY = phd_cos(pose.Orientation.y);

	// Rotate item to collision bounds identity.
	auto x = round((distance.x * cosY) - (distance.z * sinY)) + pose.Position.x;
	auto y = item->Pose.Position.y;
	auto z = round((distance.x * sinY) + (distance.z * cosY)) + pose.Position.z;

	// Determine identity static collision bounds.
	auto XMin = pose.Position.x + box->X1;
	auto XMax = pose.Position.x + box->X2;
	auto YMin = pose.Position.y + box->Y1;
	auto YMax = pose.Position.y + box->Y2;
	auto ZMin = pose.Position.z + box->Z1;
	auto ZMax = pose.Position.z + box->Z2;

	// Determine item collision bounds.
	auto inXMin = x + collBox.X1;
	auto inXMax = x + collBox.X2;
	auto inYMin = y + collBox.Y1;
	auto inYMax = y + collBox.Y2;
	auto inZMin = z + collBox.Z1;
	auto inZMax = z + collBox.Z2;

	// Don't calculate shifts if not in bounds.
	if (inXMax <= XMin || inXMin >= XMax ||
		inYMax <= YMin || inYMin >= YMax ||
		inZMax <= ZMin || inZMin >= ZMax)
		return result;

	// Calculate shifts.

	auto rawShift = Vector3Int::Zero;

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

	// Rotate previous collision position to identity.
	distance = (coll->Setup.OldPosition - pose.Position).ToVector3();
	auto ox = round((distance.x * cosY) - (distance.z * sinY)) + pose.Position.x;
	auto oz = round((distance.x * sinY) + (distance.z * cosY)) + pose.Position.z;

	// Calculate collisison type based on identity orientation.
	switch (GetQuadrant(coll->Setup.ForwardAngle - pose.Orientation.y))
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

	// Determine final shifts orientation/distance.
	distance = Vector3(x + coll->Shift.x, y, z + coll->Shift.z) - pose.Position.ToVector3();
	sinY = phd_sin(-pose.Orientation.y);
	cosY = phd_cos(-pose.Orientation.y);

	// Calculate final shifts orientation/distance.
	coll->Shift.x = (round((distance.x * cosY) - (distance.z * sinY)) + pose.Position.x) - item->Pose.Position.x;
	coll->Shift.z = (round((distance.x * sinY) + (distance.z * cosY)) + pose.Position.z) - item->Pose.Position.z;

	if (coll->Shift.x == 0 && coll->Shift.z == 0)
		coll->CollisionType = CT_NONE; // Paranoid.

	// Set splat state flag if item is Lara and bounds are taller than Lara's headroom.
	if (item == LaraItem && coll->CollisionType == CT_FRONT)
		coll->HitTallObject = (YMin <= inYMin + LARA_HEADROOM);

	return true;
}

// NOTE: Previously DoProperDetection().
void DoProjectileDynamics(short itemNumber, int x, int y, int z, int xv, int yv, int zv)
{
	auto* item = &g_Level.Items[itemNumber];

	auto prevPointProbe = GetCollision(x, y, z, item->RoomNumber);
	auto pointProbe = GetCollision(item);

	auto* bounds = GetBoundsAccurate(item);
	int radius = bounds->Height();

	item->Pose.Position.y += radius;

	if (item->Pose.Position.y >= pointProbe.Position.Floor)
	{
		int bs = 0;

		if (pointProbe.Position.FloorSlope && prevPointProbe.Position.Floor < pointProbe.Position.Floor)
		{
			int yAngle = (long)((unsigned short)item->Pose.Orientation.y);

			if (pointProbe.FloorTilt.x < 0)
			{
				if (yAngle >= ANGLE(180.0f))
					bs = 1;
			}
			else if (pointProbe.FloorTilt.x > 0)
			{
				if (yAngle <= ANGLE(180.0f))
					bs = 1;
			}

			if (pointProbe.FloorTilt.y < 0)
			{
				if (yAngle >= ANGLE(90.0f) && yAngle <= ANGLE(270.0f))
					bs = 1;
			}
			else if (pointProbe.FloorTilt.y > 0)
			{
				if (yAngle <= ANGLE(90.0f) || yAngle >= ANGLE(270.0f))
					bs = 1;
			}
		}

		// If last position of item was also below this floor height, we've hit a wall, else we've hit a floor.

		if (y > (pointProbe.Position.Floor + 32) && bs == 0 &&
			(((x / SECTOR(1)) != (item->Pose.Position.x / SECTOR(1))) ||
				((z / SECTOR(1)) != (item->Pose.Position.z / SECTOR(1)))))
		{
			// Need to know which direction the wall is.

			long xs;

			if ((x & (~WALL_MASK)) != (item->Pose.Position.x & (~WALL_MASK)) &&	// X crossed boundary?
				(z & (~WALL_MASK)) != (item->Pose.Position.z & (~WALL_MASK)))	// Z crossed boundary as well?
			{
				if (abs(x - item->Pose.Position.x) < abs(z - item->Pose.Position.z))
					xs = 1;	// X has travelled the shortest, so (maybe) hit first. (Seems to work ok).
				else
					xs = 0;
			}
			else
				xs = 1;

			if ((x & (~WALL_MASK)) != (item->Pose.Position.x & (~WALL_MASK)) && xs)	// X crossed boundary?
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

			item->Animation.Velocity.z /= 2;

			// Put item back in its last position.
			item->Pose.Position.x = x;
			item->Pose.Position.y = y;
			item->Pose.Position.z = z;
		}
		// Hit a steep slope?
		else if (pointProbe.Position.FloorSlope)
		{
			// Need to know which direction the slope is.

			item->Animation.Velocity.z -= (item->Animation.Velocity.z / 4);

			// Hit angle = ANGLE(90.0f)
			if (pointProbe.FloorTilt.x < 0 && ((abs(pointProbe.FloorTilt.x)) - (abs(pointProbe.FloorTilt.y)) >= 2))
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(180.0f))
				{
					item->Pose.Orientation.y = ANGLE(90.0f) + (ANGLE(270.0f) - (unsigned short)item->Pose.Orientation.y - 1);
					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
				}
				else
				{
					if (item->Animation.Velocity.z < 32)
					{
						item->Animation.Velocity.z -= pointProbe.FloorTilt.x * 2;
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

					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
					else
						item->Animation.Velocity.y = 0;
				}
			}
			// Hit angle = ANGLE(270.0f)
			else if (pointProbe.FloorTilt.x > 0 && ((abs(pointProbe.FloorTilt.x)) - (abs(pointProbe.FloorTilt.y)) >= 2))
			{
				if (((unsigned short)item->Pose.Orientation.y) < ANGLE(180.0f))
				{
					item->Pose.Orientation.y = ANGLE(270.0f) + (ANGLE(90.0f) - (unsigned short)item->Pose.Orientation.y - 1);
					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
				}
				else
				{
					if (item->Animation.Velocity.z < 32)
					{
						item->Animation.Velocity.z += pointProbe.FloorTilt.x * 2;
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

					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
					else
						item->Animation.Velocity.y = 0;
				}
			}
			// Hit angle = 0
			else if (pointProbe.FloorTilt.y < 0 && ((abs(pointProbe.FloorTilt.y)) - (abs(pointProbe.FloorTilt.x)) >= 2))
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(90.0f) && ((unsigned short)item->Pose.Orientation.y) < ANGLE(270.0f))
				{
					item->Pose.Orientation.y = ANGLE(180.0f) - item->Pose.Orientation.y - 1;
					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
				}
				else
				{
					if (item->Animation.Velocity.z < 32)
					{
						item->Animation.Velocity.z -= pointProbe.FloorTilt.y * 2;

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

					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
					else
						item->Animation.Velocity.y = 0;
				}
			}
			// Hit angle = ANGLE(180.0f)
			else if (pointProbe.FloorTilt.y > 0 && ((abs(pointProbe.FloorTilt.y)) - (abs(pointProbe.FloorTilt.x)) >= 2))
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(270.0f) || ((unsigned short)item->Pose.Orientation.y) < ANGLE(90.0f))
				{
					item->Pose.Orientation.y = ANGLE(180.0f) - item->Pose.Orientation.y - 1;
					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
				}
				else
				{
					if (item->Animation.Velocity.z < 32)
					{
						item->Animation.Velocity.z += pointProbe.FloorTilt.y * 2;

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

					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
					else
						item->Animation.Velocity.y = 0;
				}
			}
			else if (pointProbe.FloorTilt.x < 0 && pointProbe.FloorTilt.y < 0)	// Hit angle = 0x2000
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(135.0f) && ((unsigned short)item->Pose.Orientation.y) < ANGLE(315.0f))
				{
					item->Pose.Orientation.y = ANGLE(45.0f) + (ANGLE(225.0f) - (unsigned short)item->Pose.Orientation.y - 1);
					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
				}
				else
				{
					if (item->Animation.Velocity.z < 32)
					{
						item->Animation.Velocity.z += -pointProbe.FloorTilt.x + -pointProbe.FloorTilt.y;
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

					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
					else
						item->Animation.Velocity.y = 0;
				}
			}
			// Hit angle = ANGLE(135.0f)
			else if (pointProbe.FloorTilt.x < 0 && pointProbe.FloorTilt.y > 0)
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(225.0f) || ((unsigned short)item->Pose.Orientation.y) < ANGLE(45.0f))
				{
					item->Pose.Orientation.y = ANGLE(135.0f) + (ANGLE(315.0f) - (unsigned short)item->Pose.Orientation.y - 1);
					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
				}
				else
				{
					if (item->Animation.Velocity.z < 32)
					{
						item->Animation.Velocity.z += (-pointProbe.FloorTilt.x) + pointProbe.FloorTilt.y;
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

					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
					else
						item->Animation.Velocity.y = 0;
				}
			}
			// Hit angle = ANGLE(225.5f)
			else if (pointProbe.FloorTilt.x > 0 && pointProbe.FloorTilt.y > 0)
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(315.0f) || ((unsigned short)item->Pose.Orientation.y) < ANGLE(135.0f))
				{
					item->Pose.Orientation.y = ANGLE(225.5f) + (ANGLE(45.0f) - (unsigned short)item->Pose.Orientation.y - 1);
					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
				}
				else
				{
					if (item->Animation.Velocity.z < 32)
					{
						item->Animation.Velocity.z += pointProbe.FloorTilt.x + pointProbe.FloorTilt.y;
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

					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
					else
						item->Animation.Velocity.y = 0;
				}
			}
			// Hit angle = ANGLE(315.0f)
			else if (pointProbe.FloorTilt.x > 0 && pointProbe.FloorTilt.y < 0)
			{
				if (((unsigned short)item->Pose.Orientation.y) > ANGLE(45.0f) && ((unsigned short)item->Pose.Orientation.y) < ANGLE(225.5f))
				{
					item->Pose.Orientation.y = ANGLE(315.0f) + (ANGLE(135.0f)- (unsigned short)item->Pose.Orientation.y - 1);
					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -item->Animation.Velocity.y / 2;
				}
				else
				{
					if (item->Animation.Velocity.z < 32)
					{
						item->Animation.Velocity.z += pointProbe.FloorTilt.x + (-pointProbe.FloorTilt.y);
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

					if (item->Animation.Velocity.y > 0)
						item->Animation.Velocity.y = -(item->Animation.Velocity.y / 2);
					else
						item->Animation.Velocity.y = 0;
				}
			}

			// Move item back to its previous position.
			item->Pose.Position.x = x;
			item->Pose.Position.y = y;
			item->Pose.Position.z = z;
		}
		else
		{
			// Hit the floor; bounce and slow down.
			if (item->Animation.Velocity.y > 0)
			{
				if (item->Animation.Velocity.y > 16)
				{
					if (item->ObjectNumber == ID_GRENADE)
						item->Animation.Velocity.y = -(item->Animation.Velocity.y - (item->Animation.Velocity.y / 2));
					else
					{
						item->Animation.Velocity.y = -(item->Animation.Velocity.y / 2);
						if (item->Animation.Velocity.y < -100)
							item->Animation.Velocity.y = -100;
					}
				}
				else
				{
					// Roll on floor.
					item->Animation.Velocity.y = 0;
					if (item->ObjectNumber == ID_GRENADE)
					{
						item->Animation.RequiredState = 1;
						item->Pose.Orientation.x = 0;
						item->Animation.Velocity.z--;
					}
					else
						item->Animation.Velocity.z -= 3;

					if (item->Animation.Velocity.z < 0)
						item->Animation.Velocity.z = 0;
				}
			}

			item->Pose.Position.y = pointProbe.Position.Floor;
		}
	}
	// Check for on top of object.
	else
	{
		if (yv >= 0)
		{
			prevPointProbe = GetCollision(item->Pose.Position.x, y, item->Pose.Position.z, item->RoomNumber);
			pointProbe = GetCollision(item);

			// Bounce off floor.

			// Removed weird OnObject global check from here which didnt make sense because OnObject
			// was always set to 0 by GetHeight() function which was called before the check.
			// Possibly a mistake or unfinished feature by Core? -- Lwmte, 27.08.21

			if (item->Pose.Position.y >= prevPointProbe.Position.Floor)
			{
				// Hit the floor; bounce and slow down.
				if (item->Animation.Velocity.y > 0)
				{
					if (item->Animation.Velocity.y > 16)
					{
						if (item->ObjectNumber == ID_GRENADE)
							item->Animation.Velocity.y = -(item->Animation.Velocity.y - (item->Animation.Velocity.y / 2));
						else
						{
							item->Animation.Velocity.y = -(item->Animation.Velocity.y / 4);
							if (item->Animation.Velocity.y < -100)
								item->Animation.Velocity.y = -100;
						}
					}
					else
					{
						// Roll on floor.
						item->Animation.Velocity.y = 0;
						if (item->ObjectNumber == ID_GRENADE)
						{
							item->Animation.RequiredState = 1;
							item->Pose.Orientation.x = 0;
							item->Animation.Velocity.z--;
						}
						else
							item->Animation.Velocity.z -= 3;

						if (item->Animation.Velocity.z < 0)
							item->Animation.Velocity.z = 0;
					}
				}

				item->Pose.Position.y = prevPointProbe.Position.Floor;
			}
		}
		// else
		{
			// Bounce off ceiling.
			pointProbe = GetCollision(item);

			if (item->Pose.Position.y < pointProbe.Position.Ceiling)
			{
				if (y < pointProbe.Position.Ceiling &&
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
						item->Animation.Velocity.z -= item->Animation.Velocity.z / 8;
					else
						item->Animation.Velocity.z /= 2;

					// Move item back to its previous position.
					item->Pose.Position.x = x;
					item->Pose.Position.y = y;
					item->Pose.Position.z = z;
				}
				else
					item->Pose.Position.y = pointProbe.Position.Ceiling;

				if (item->Animation.Velocity.y < 0)
					item->Animation.Velocity.y = -item->Animation.Velocity.y;
			}
		}
	}

	pointProbe = GetCollision(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);

	if (pointProbe.RoomNumber != item->RoomNumber)
	{
		if (item->ObjectNumber == ID_GRENADE && TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, pointProbe.RoomNumber))
			Splash(item);

		ItemNewRoom(itemNumber, pointProbe.RoomNumber);
	}

	item->Pose.Position.y -= radius;
}

void DoObjectCollision(ItemInfo* laraItem, CollisionInfo* coll)
{
	laraItem->HitStatus = false;
	coll->HitStatic     = false;

	bool doPlayerCollision = laraItem->IsLara();
	bool harmless = !doPlayerCollision && (laraItem->Data.is<KayakInfo>() || laraItem->Data.is<UPVInfo>());

	if (doPlayerCollision)
	{
		GetLaraInfo(laraItem)->HitDirection = -1;

		if (laraItem->HitPoints <= 0)
			return;
	}

	if (Objects[laraItem->ObjectNumber].intelligent)
		return;

	for (auto i : g_Level.Rooms[laraItem->RoomNumber].neighbors)
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

			if (doPlayerCollision)
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

					if (harmless || abs(laraItem->Animation.Velocity.z) < VEHICLE_COLLISION_TERMINAL_VELOCITY)
					{
						// If vehicle is harmless or speed is too low, just push the enemy.
						ItemPushItem(laraItem, item, coll, false, 0);
						continue;
					}
					else
					{
						DoDamage(item, INT_MAX);
						DoLotsOfBlood(
							item->Pose.Position.x,
							laraItem->Pose.Position.y - CLICK(1),
							item->Pose.Position.z,
							laraItem->Animation.Velocity.z,
							laraItem->Pose.Orientation.y,
							item->RoomNumber, 3);
					}
				}
				else if (coll->Setup.EnableObjectPush)
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
			if (doPlayerCollision && (mesh->flags & StaticMeshFlags::SM_SOLID))
				continue;

			if (phd_Distance(&mesh->pos, &laraItem->Pose) >= COLLISION_CHECK_DISTANCE)
				continue;

			if (!TestBoundsCollideStatic(laraItem, mesh, coll->Setup.Radius))
				continue;

			coll->HitStatic = true;

			// HACK: Shatter statics only by non-harmless vehicles.
			if (!doPlayerCollision && 
				!harmless && abs(laraItem->Animation.Velocity.z) > VEHICLE_COLLISION_TERMINAL_VELOCITY &&
				StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
			{
				SoundEffect(GetShatterSound(mesh->staticNumber), (PHD_3DPOS*)mesh);
				ShatterObject(nullptr, mesh, -128, laraItem->RoomNumber, 0);
			}
			else if (coll->Setup.EnableObjectPush)
				ItemPushStatic(laraItem, mesh, coll);
			else
				continue;
		}
	}

	if (doPlayerCollision)
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

void ObjectCollision(const short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
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

	bool doPlayerCollision = laraItem->IsLara();
	bool waterPlayerCollision = doPlayerCollision && GetLaraInfo(laraItem)->Control.WaterStatus >= WaterStatus::TreadWater;

	if (waterPlayerCollision || coll->Setup.EnableObjectPush)
		ItemPushItem(item, laraItem, coll, coll->Setup.EnableSpasm, 0);
	else if (doPlayerCollision && coll->Setup.EnableSpasm)
	{
		int x = laraItem->Pose.Position.x - item->Pose.Position.x;
		int z = laraItem->Pose.Position.z - item->Pose.Position.z;

		float sinY = phd_sin(item->Pose.Orientation.y);
		float cosY = phd_cos(item->Pose.Orientation.y);

		auto* frame = GetBestFrame(item);
		int rx = (frame->boundingBox.X1 + frame->boundingBox.X2) / 2;
		int rz = (frame->boundingBox.X2 + frame->boundingBox.Z2) / 2;

		if (frame->boundingBox.Height() > CLICK(1))
		{
			auto* lara = GetLaraInfo(laraItem);

			int angle = (laraItem->Pose.Orientation.y - phd_atan(z - cosY * rx - sinY * rz, x - cosY * rx + sinY * rz) - ANGLE(135.0f)) / ANGLE(90.0f);

			lara->HitDirection = (short)angle;

			// TODO: Check if a second Lara.hitFrame++ is required. -- TokyoSU					
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
