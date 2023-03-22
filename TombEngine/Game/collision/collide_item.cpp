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
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "ScriptInterfaceGame.h"
#include "Sound/sound.h"
#include "Specific/setup.h"

using namespace TEN::Math;
using namespace TEN::Renderer;

GameBoundingBox GlobalCollisionBounds;
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
				const auto& bBox = GetBoundsAccurate(*mesh, false);

				if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
					continue;

				if ((collidingItem->Pose.Position.y + radius + CLICK(0.5f)) < (mesh->pos.Position.y + bBox.Y1))
					continue;

				if (collidingItem->Pose.Position.y > (mesh->pos.Position.y + bBox.Y2))
					continue;

				float sinY = phd_sin(mesh->pos.Orientation.y);
				float cosY = phd_cos(mesh->pos.Orientation.y);

				float rx = ((collidingItem->Pose.Position.x - mesh->pos.Position.x) * cosY) - ((collidingItem->Pose.Position.z - mesh->pos.Position.z) * sinY);
				float rz = ((collidingItem->Pose.Position.z - mesh->pos.Position.z) * cosY) + ((collidingItem->Pose.Position.x - mesh->pos.Position.x) * sinY);

				if ((radius + rx + CLICK(0.5f) < bBox.X1) || (rx - radius - CLICK(0.5f) > bBox.X2))
					continue;

				if ((radius + rz + CLICK(0.5f) < bBox.Z1) || (rz - radius - CLICK(0.5f) > bBox.Z2))
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
						(ignoreLara && item->ObjectNumber == ID_LARA) ||
						(onlyVisible && item->Status == ITEM_INVISIBLE) ||
						item->Flags & IFLAG_KILLED ||
						item->MeshBits == NO_JOINT_BITS ||
						(Objects[item->ObjectNumber].drawRoutine == nullptr && item->ObjectNumber != ID_LARA) ||
						(Objects[item->ObjectNumber].collision == nullptr && item->ObjectNumber != ID_LARA))
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

					// TODO: Don't modify object animation data!!!
					auto& bounds = GetBestFrame(*item).BoundingBox;

					if (dx >= -BLOCK(2) && dx <= BLOCK(2) &&
						dy >= -BLOCK(2) && dy <= BLOCK(2) &&
						dz >= -BLOCK(2) && dz <= BLOCK(2) &&
						(collidingItem->Pose.Position.y + radius + CLICK(0.5f)) >= (item->Pose.Position.y + bounds.Y1) &&
						(collidingItem->Pose.Position.y - radius - CLICK(0.5f)) <= (item->Pose.Position.y + bounds.Y2))
					{
						float sinY = phd_sin(item->Pose.Orientation.y);
						float cosY = phd_cos(item->Pose.Orientation.y);

						int rx = (dx * cosY) - (dz * sinY);
						int rz = (dz * cosY) + (dx * sinY);

						if (item->ObjectNumber == ID_TURN_SWITCH)
						{
							bounds.X1 = -CLICK(1);
							bounds.X2 = CLICK(1);
							bounds.Z1 = -CLICK(1);
							bounds.Z1 = CLICK(1);
						}

						if ((radius + rx + CLICK(0.5f)) >= bounds.X1 &&
							(rx - radius - CLICK(0.5f)) <= bounds.X2)
						{
							if ((radius + rz + CLICK(0.5f)) >= bounds.Z1 &&
								(rz - radius - CLICK(0.5f)) <= bounds.Z2)
							{
								collidedItems[numItems++] = item;
							}
						}
						else
						{
							if ((collidingItem->Pose.Position.y + radius + CLICK(0.5f)) >= (item->Pose.Position.y + bounds.Y1) &&
								(collidingItem->Pose.Position.y - radius - CLICK(0.5f)) <= (item->Pose.Position.y + bounds.Y2))
							{
								float sinY = phd_sin(item->Pose.Orientation.y);
								float cosY = phd_cos(item->Pose.Orientation.y);

								int rx = (dx * cosY) - (dz * sinY);
								int rz = (dz * cosY) + (dx * sinY);

								if (item->ObjectNumber == ID_TURN_SWITCH)
								{
									bounds.X1 = -CLICK(1);
									bounds.X2 = CLICK(1);
									bounds.Z1 = -CLICK(1);
									bounds.Z1 = CLICK(1);
								}

								if ((radius + rx + CLICK(0.5f)) >= bounds.X1 &&
									(rx - radius - CLICK(0.5f)) <= bounds.X2)
								{
									if ((radius + rz + CLICK(0.5f)) >= bounds.Z1 &&
										(rz - radius - CLICK(0.5f)) <= bounds.Z2)
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
	const auto& bounds = GetBestFrame(*laraItem).BoundingBox;

	if ((item->Pose.Position.y + GlobalCollisionBounds.Y2) <= (laraItem->Pose.Position.y + bounds.Y1))
		return false;

	if ((item->Pose.Position.y + GlobalCollisionBounds.Y1) >= bounds.Y2)
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
	int height = GameBoundingBox(item).GetHeight();

	for (int i = 0; i < 3; i++)
	{
		auto sinHeading = (i != 1) ? (phd_sin(coll->Setup.ForwardAngle + ANGLE((i * 90.0f) - 90.0f))) : 0;
		auto cosHeading = (i != 1) ? (phd_cos(coll->Setup.ForwardAngle + ANGLE((i * 90.0f) - 90.0f))) : 0;

		auto origin = Vector3(
			item->Pose.Position.x + (sinHeading * (coll->Setup.Radius)),
			item->Pose.Position.y - (height + CLICK(1)),
			item->Pose.Position.z + (cosHeading * (coll->Setup.Radius))
		);
		auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(coll->Setup.ForwardAngle), 0.0f, 0.0f);
		auto direction = (Matrix::CreateTranslation(Vector3::UnitZ) * mxR).Translation();

		// g_Renderer.AddDebugSphere(origin, 16, Vector4::One, RENDERER_DEBUG_PAGE::DIMENSION_STATS);

		for (auto i : g_Level.Rooms[item->RoomNumber].neighbors)
		{
			if (!g_Level.Rooms[i].Active())
				continue;

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

				if (Vector3i::Distance(item->Pose.Position, item2->Pose.Position) < COLLISION_CHECK_DISTANCE)
				{
					auto box = GameBoundingBox(item2).ToBoundingOrientedBox(item2->Pose);
					float distance;

					if (box.Intersects(origin, direction, distance) && distance < (coll->Setup.Radius * 2))
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

				if (Vector3i::Distance(item->Pose.Position, mesh.pos.Position) < COLLISION_CHECK_DISTANCE)
				{
					const auto& bBox = GetBoundsAccurate(mesh, false).ToBoundingOrientedBox(mesh.pos);
					float distance;

					if (bBox.Intersects(origin, direction, distance) && distance < (coll->Setup.Radius * 2))
					{
						coll->HitStatic = true;
						return;
					}
				}
			}
		}
	}
}

bool TestLaraPosition(const ObjectCollisionBounds& bounds, ItemInfo* item, ItemInfo* laraItem)
{
	auto deltaOrient = laraItem->Pose.Orientation - item->Pose.Orientation;
	if (deltaOrient.x < bounds.OrientConstraint.first.x || deltaOrient.x > bounds.OrientConstraint.second.x ||
		deltaOrient.y < bounds.OrientConstraint.first.y || deltaOrient.y > bounds.OrientConstraint.second.y ||
		deltaOrient.z < bounds.OrientConstraint.first.z || deltaOrient.z > bounds.OrientConstraint.second.z)
	{
		return false;
	}

	auto pos = (laraItem->Pose.Position - item->Pose.Position).ToVector3();
	auto rotMatrix = item->Pose.Orientation.ToRotationMatrix();

	// This solves once for all the minus sign hack of CreateFromYawPitchRoll.
	// In reality it should be the inverse, but the inverse of a rotation matrix is equal to the transpose
	// and transposing a matrix is faster.
	// It's the only piece of code that does it, because we want Lara's location relative to the identity frame
	// of the object we are test against.
	rotMatrix = rotMatrix.Transpose();

	pos = Vector3::Transform(pos, rotMatrix);

	if (pos.x < bounds.BoundingBox.X1 || pos.x > bounds.BoundingBox.X2 ||
		pos.y < bounds.BoundingBox.Y1 || pos.y > bounds.BoundingBox.Y2 ||
		pos.z < bounds.BoundingBox.Z1 || pos.z > bounds.BoundingBox.Z2)
	{
		return false;
	}

	return true;
}

bool AlignLaraPosition(const Vector3i& offset, ItemInfo* item, ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	laraItem->Pose.Orientation = item->Pose.Orientation;

	auto rotMatrix = item->Pose.Orientation.ToRotationMatrix();
	auto pos = Vector3::Transform(offset.ToVector3(), rotMatrix);
	auto target = item->Pose.Position.ToVector3() + pos;

	int height = GetCollision(target.x, target.y, target.z, laraItem->RoomNumber).Position.Floor;
	if ((laraItem->Pose.Position.y - height) <= CLICK(2))
	{
		laraItem->Pose.Position = Vector3i(target);
		return true;
	}

	if (lara->Control.IsMoving)
	{
		lara->Control.IsMoving = false;
		lara->Control.HandStatus = HandStatus::Free;
	}

	return false;
}

bool MoveLaraPosition(const Vector3i& offset, ItemInfo* item, ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	auto rotMatrix = item->Pose.Orientation.ToRotationMatrix();
	auto pos = Vector3::Transform(offset.ToVector3(), rotMatrix);
	auto target = Pose(item->Pose.Position + Vector3i(pos), item->Pose.Orientation);

	if (!Objects[item->ObjectNumber].isPickup)
		return Move3DPosTo3DPos(laraItem, laraItem->Pose, target, LARA_ALIGN_VELOCITY, ANGLE(2.0f));
	else
	{
		// Prevent picking up items which can result in so called "flare pickup bug"
		int height = GetCollision(target.Position.x, target.Position.y, target.Position.z, laraItem->RoomNumber).Position.Floor;
		if (abs(height - laraItem->Pose.Position.y) <= CLICK(2))
			return Move3DPosTo3DPos(laraItem, laraItem->Pose, target, LARA_ALIGN_VELOCITY, ANGLE(2.0f));
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
	return ((value >= -radius) && (value <= radius));
}

static bool ItemInRange(int x, int z, int radius)
{
	return ((SQUARE(x) + SQUARE(z)) <= SQUARE(radius));
}

bool ItemNearLara(const Vector3i& origin, int radius)
{
	auto target = GameVector(origin - LaraItem->Pose.Position);

	if (!ItemCollide(target.y, ITEM_RADIUS_YMAX))
		return false;

	if (!ItemCollide(target.x, radius) || !ItemCollide(target.z, radius))
		return false;

	if (!ItemInRange(target.x, target.z, radius))
		return false;

	auto bounds = GameBoundingBox(LaraItem);
	if (target.y >= bounds.Y1 && target.y <= (bounds.Y2 + LARA_RADIUS))
		return true;

	return false;
}

bool ItemNearTarget(const Vector3i& origin, ItemInfo* targetEntity, int radius)
{
	auto pos = origin - targetEntity->Pose.Position;

	if (!ItemCollide(pos.y, ITEM_RADIUS_YMAX))
		return false;

	if (!ItemCollide(pos.x, radius) || !ItemCollide(pos.z, radius))
		return false;

	if (!ItemInRange(pos.x, pos.z, radius))
		return false;

	auto bounds = GameBoundingBox(targetEntity);
	if (pos.y >= bounds.Y1 && pos.y <= bounds.Y2)
		return true;

	return false;
}

bool Move3DPosTo3DPos(ItemInfo* item, Pose& fromPose, const Pose& toPose, int velocity, short turnRate)
{
	auto* lara = GetLaraInfo(item);

	auto direction = toPose.Position - fromPose.Position;
	float distance = Vector3i::Distance(fromPose.Position, toPose.Position);

	if (velocity < distance)
		fromPose.Position += direction * (velocity / distance);
	else
		fromPose.Position = toPose.Position;

	if (!lara->Control.IsMoving)
	{
		bool shouldAnimate = ((distance - velocity) > (velocity * ANIMATED_ALIGNMENT_FRAME_COUNT_THRESHOLD));

		if (shouldAnimate && lara->Control.WaterStatus != WaterStatus::Underwater)
		{
			short angle = Geometry::GetOrientToPoint(fromPose.Position.ToVector3(), toPose.Position.ToVector3()).y;
			int direction = (GetQuadrant(angle) - GetQuadrant(fromPose.Orientation.y)) & 3;

			switch (direction)
			{
			default:
			case NORTH:
				SetAnimation(item, LA_WALK);
				break;

			case SOUTH:
				SetAnimation(item, LA_WALK_BACK);
				break;

			case EAST:
				SetAnimation(item, LA_SIDESTEP_RIGHT);
				break;

			case WEST:
				SetAnimation(item, LA_SIDESTEP_LEFT);
				break;
			}

			lara->Control.HandStatus = HandStatus::Busy;
		}

		lara->Control.IsMoving = true;
		lara->Control.Count.PositionAdjust = 0;
	}

	auto deltaOrient = toPose.Orientation - fromPose.Orientation;

	if (deltaOrient.x > turnRate)
		fromPose.Orientation.x += turnRate;
	else if (deltaOrient.x < -turnRate)
		fromPose.Orientation.x -= turnRate;
	else
		fromPose.Orientation.x = toPose.Orientation.x;

	if (deltaOrient.y > turnRate)
		fromPose.Orientation.y += turnRate;
	else if (deltaOrient.y < -turnRate)
		fromPose.Orientation.y -= turnRate;
	else
		fromPose.Orientation.y = toPose.Orientation.y;

	if (deltaOrient.z > turnRate)
		fromPose.Orientation.z += turnRate;
	else if (deltaOrient.z < -turnRate)
		fromPose.Orientation.z -= turnRate;
	else
		fromPose.Orientation.z = toPose.Orientation.z;

	return (fromPose == toPose);
}

bool TestBoundsCollide(ItemInfo* item, ItemInfo* laraItem, int radius)
{
	const auto& bounds = GetBestFrame(*item).BoundingBox;
	const auto& playerBounds = GetBestFrame(*laraItem).BoundingBox;

	if ((item->Pose.Position.y + bounds.Y2) <= (laraItem->Pose.Position.y + playerBounds.Y1))
		return false;

	if ((item->Pose.Position.y + bounds.Y1) >= (laraItem->Pose.Position.y + playerBounds.Y2))
		return false;

	float sinY = phd_sin(item->Pose.Orientation.y);
	float cosY = phd_cos(item->Pose.Orientation.y);

	int x = laraItem->Pose.Position.x - item->Pose.Position.x;
	int z = laraItem->Pose.Position.z - item->Pose.Position.z;
	int dx = (x * cosY) - (z * sinY);
	int dz = (z * cosY) + (x * sinY);

	if (dx >= (bounds.X1 - radius) &&
		dx <= (bounds.X2 + radius) &&
		dz >= (bounds.Z1 - radius) &&
		dz <= (bounds.Z2 + radius))
	{
		return true;
	}

	return false;
}

bool TestBoundsCollideStatic(ItemInfo* item, const MESH_INFO& mesh, int radius)
{
	const auto& bounds = GetBoundsAccurate(mesh, false);

	if (!(bounds.Z2 != 0 || bounds.Z1 != 0 || bounds.X1 != 0 || bounds.X2 != 0 || bounds.Y1 != 0 || bounds.Y2 != 0))
		return false;

	const auto& itemBounds = GetBestFrame(*item).BoundingBox;
	if (mesh.pos.Position.y + bounds.Y2 <= item->Pose.Position.y + itemBounds.Y1)
		return false;

	if (mesh.pos.Position.y + bounds.Y1 >= item->Pose.Position.y + itemBounds.Y2)
		return false;

	float sinY = phd_sin(mesh.pos.Orientation.y);
	float cosY = phd_cos(mesh.pos.Orientation.y);

	int x = item->Pose.Position.x - mesh.pos.Position.x;
	int z = item->Pose.Position.z - mesh.pos.Position.z;
	int dx = (x * cosY) - (z * sinY);
	int dz = (z * cosY) + (x * sinY);

	if (dx <= (radius + bounds.X2) &&
		dx >= (bounds.X1 - radius) &&
		dz <= (radius + bounds.Z2) &&
		dz >= (bounds.Z1 - radius))
	{
		return true;
	}
	else
	{
		return false;
	}
}

// NOTE: Previously ItemPushLara().
bool ItemPushItem(ItemInfo* item, ItemInfo* item2, CollisionInfo* coll, bool enableSpasm, char bigPush)
{
	float sinY = phd_sin(item->Pose.Orientation.y);
	float cosY = phd_cos(item->Pose.Orientation.y);

	// Get direction vector from item to player.
	auto direction = item2->Pose.Position - item->Pose.Position;

	// Rotate Lara vector into item frame.
	int rx = (direction.x * cosY) - (direction.z * sinY);
	int rz = (direction.z * cosY) + (direction.x * sinY);

	const auto& bounds = (bigPush & 2) ? GlobalCollisionBounds : GetBestFrame(*item).BoundingBox;

	int minX = bounds.X1;
	int maxX = bounds.X2;
	int minZ = bounds.Z1;
	int maxZ = bounds.Z2;

	if (bigPush & 1)
	{
		minX -= coll->Setup.Radius;
		maxX += coll->Setup.Radius;
		minZ -= coll->Setup.Radius;
		maxZ += coll->Setup.Radius;
	}

	// Big enemies
	if (abs(direction.x) > BLOCK(4.5f) || abs(direction.z) > BLOCK(4.5f) ||
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

	if (lara != nullptr && enableSpasm && (bounds.Y2 - bounds.Y1) > CLICK(1))
	{
		rx = (bounds.X1 + bounds.X2) / 2;
		rz = (bounds.Z1 + bounds.Z2) / 2;

		direction.x -= (rx * cosY) + (rz * sinY);
		direction.z -= (rz * cosY) - (rx * sinY);

		// TODO: Is this phd_atan() call a mistake?
		lara->HitDirection = (item2->Pose.Orientation.y - phd_atan(direction.z, direction.z) - ANGLE(135.0f)) / ANGLE(90.0f);
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
		// UpdateLaraRoom(item2, -10);
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
bool ItemPushStatic(ItemInfo* item, const MESH_INFO& mesh, CollisionInfo* coll)
{
	const auto& bounds = GetBoundsAccurate(mesh, false);

	float sinY = phd_sin(mesh.pos.Orientation.y);
	float cosY = phd_cos(mesh.pos.Orientation.y);
	
	auto direction = item->Pose.Position - mesh.pos.Position;
	auto dz = item->Pose.Position.z - mesh.pos.Position.z;
	auto rx = (direction.x * cosY) - (direction.z * sinY);
	auto rz = (direction.z * cosY) + (direction.x * sinY);
	auto minX = bounds.X1 - coll->Setup.Radius;
	auto maxX = bounds.X2 + coll->Setup.Radius;
	auto minZ = bounds.Z1 - coll->Setup.Radius;
	auto maxZ = bounds.Z2 + coll->Setup.Radius;

	if (abs(direction.x) > BLOCK(4.5f) || abs(direction.z) > BLOCK(4.5f) ||
		rx <= minX || rx >= maxX ||
		rz <= minZ || rz >= maxZ)
	{
		return false;
	}

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

	item->Pose.Position.x = mesh.pos.Position.x + cosY * rx + sinY * rz;
	item->Pose.Position.z = mesh.pos.Position.z + cosY * rz - sinY * rx;

	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;

	auto prevHeadingAngle = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = phd_atan(item->Pose.Position.z - coll->Setup.OldPosition.z, item->Pose.Position.x - coll->Setup.OldPosition.x);

	GetCollisionInfo(coll, item);

	coll->Setup.ForwardAngle = prevHeadingAngle;

	if (coll->CollisionType == CT_NONE)
	{
		coll->Setup.OldPosition = item->Pose.Position;
		if (item->IsLara())
			UpdateLaraRoom(item, -10);
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
		if (!g_Level.Rooms[i].Active())
			continue;

		for (auto& mesh : g_Level.Rooms[i].mesh)
		{
			// Only process meshes which are visible and solid.
			if (!(mesh.flags & StaticMeshFlags::SM_VISIBLE) || !(mesh.flags & StaticMeshFlags::SM_SOLID))
				continue;

			float distance = Vector3i::Distance(item->Pose.Position, mesh.pos.Position);
			if (distance < COLLISION_CHECK_DISTANCE)
			{
				if (CollideSolidBounds(item, GetBoundsAccurate(mesh, false), mesh.pos, coll))
					coll->HitStatic = true;
			}
		}
	}
}

bool CollideSolidBounds(ItemInfo* item, const GameBoundingBox& box, const Pose& pose, CollisionInfo* coll)
{
	bool result = false;

	// Get DX static bounds in global coordinates.
	auto staticBounds = box.ToBoundingOrientedBox(pose);

	// Get local TR bounds and DX item bounds in global coordinates.
	auto itemBBox = GameBoundingBox(item);
	auto itemBounds = itemBBox.ToBoundingOrientedBox(item->Pose);

	// Extend bounds a bit for visual testing.
	itemBounds.Extents = itemBounds.Extents + Vector3(BLOCK(1));

	// Filter out any further checks if static isn't nearby.
	if (!staticBounds.Intersects(itemBounds))
		return false;

	// Bring back extents.
	itemBounds.Extents = itemBounds.Extents - Vector3(BLOCK(1));

	// Draw static bounds.
	g_Renderer.AddDebugBox(staticBounds, Vector4(1, 0.3f, 0, 1), RENDERER_DEBUG_PAGE::DIMENSION_STATS);

	// Calculate horizontal item collision bounds according to radius.
	GameBoundingBox collBox;
	collBox.X1 = -coll->Setup.Radius;
	collBox.X2 = coll->Setup.Radius;
	collBox.Z1 = -coll->Setup.Radius;
	collBox.Z2 = coll->Setup.Radius;

	// Calculate vertical item collision bounds according to either height (land mode) or precise bounds (water mode).
	// Water mode needs special processing because height calculation in original engines is inconsistent in such cases.
	if (TestEnvironment(ENV_FLAG_WATER, item))
	{
		collBox.Y1 = itemBBox.Y1;
		collBox.Y2 = itemBBox.Y2;
	}
	else
	{
		collBox.Y1 = -coll->Setup.Height;
		collBox.Y2 = 0;
	}

	// Get and test DX item collision bounds.
	auto collBounds = collBox.ToBoundingOrientedBox(Pose(item->Pose.Position));
	bool intersects = staticBounds.Intersects(collBounds);

	// Check if previous item horizontal position intersects bounds.
	auto prevCollBounds = collBox.ToBoundingOrientedBox(Pose(coll->Setup.OldPosition));
	bool prevHorIntersects = staticBounds.Intersects(prevCollBounds);

	// Draw item coll bounds.
	g_Renderer.AddDebugBox(collBounds, intersects ? Vector4(1, 0, 0, 1) : Vector4(0, 1, 0, 1), RENDERER_DEBUG_PAGE::DIMENSION_STATS);

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
	auto height = collBox.GetHeight();
	auto center = item->Pose.Position.y - (height / 2);

	// Do a series of angular tests with 90 degree steps to determine top/bottom collision.
	int closestPlane = -1;
	auto closestRay = Ray();
	auto minDistance = FLT_MAX;
	for (int i = 0; i < 4; i++)
	{
		// Calculate ray direction.
		auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(item->Pose.Orientation.y), TO_RAD(item->Pose.Orientation.x + (ANGLE(90 * i))), 0.0f);
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
		if (intersects && prevHorIntersects && minDistance < height)
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
	if (!staticBounds.Intersects(collBox.ToBoundingOrientedBox(Pose(item->Pose.Position))))
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
	auto xMin = pose.Position.x + box.X1;
	auto xMax = pose.Position.x + box.X2;
	auto yMin = pose.Position.y + box.Y1;
	auto yMax = pose.Position.y + box.Y2;
	auto zMin = pose.Position.z + box.Z1;
	auto zMax = pose.Position.z + box.Z2;

	// Determine item collision bounds.
	auto inXMin = x + collBox.X1;
	auto inXMax = x + collBox.X2;
	auto inYMin = y + collBox.Y1;
	auto inYMax = y + collBox.Y2;
	auto inZMin = z + collBox.Z1;
	auto inZMax = z + collBox.Z2;

	// Don't calculate shifts if not in bounds.
	if (inXMax <= xMin || inXMin >= xMax ||
		inYMax <= yMin || inYMin >= yMax ||
		inZMax <= zMin || inZMin >= zMax)
	{
		return result;
	}

	// Calculate shifts.

	auto rawShift = Vector3i::Zero;
	auto shiftLeft = inXMax - xMin;
	auto shiftRight = xMax - inXMin;

	if (shiftLeft < shiftRight)
		rawShift.x = -shiftLeft;
	else
		rawShift.x = shiftRight;

	shiftLeft = inZMax - zMin;
	shiftRight = zMax - inZMin;

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
		coll->HitTallObject = (yMin <= inYMin + LARA_HEADROOM);

	return true;
}

// NOTE: Previously DoProperDetection().
void DoProjectileDynamics(short itemNumber, int x, int y, int z, int xv, int yv, int zv)
{
	auto* item = &g_Level.Items[itemNumber];

	auto prevPointProbe = GetCollision(x, y, z, item->RoomNumber);
	auto pointProbe = GetCollision(item);

	auto bounds = GameBoundingBox(item);
	int radius = bounds.GetHeight();

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
			(((x / BLOCK(1)) != (item->Pose.Position.x / BLOCK(1))) ||
				((z / BLOCK(1)) != (item->Pose.Position.z / BLOCK(1)))))
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
					(((x / BLOCK(1)) != (item->Pose.Position.x / BLOCK(1))) ||
						((z / BLOCK(1)) != (item->Pose.Position.z / BLOCK(1)))))
				{
					// Need to know which direction the wall is.

					// X crossed boundary?
					if ((x & ~WALL_MASK) != (item->Pose.Position.x & ~WALL_MASK))
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
		if (!g_Level.Rooms[i].Active())
			continue;

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

			if (Vector3i::Distance(item->Pose.Position, laraItem->Pose.Position) >= COLLISION_CHECK_DISTANCE)
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
			auto& mesh = g_Level.Rooms[i].mesh[j];

			if (!(mesh.flags & StaticMeshFlags::SM_VISIBLE))
				continue;

			// For Lara, solid static mesh collisions are directly managed by GetCollisionInfo,
			// so we bypass them here to avoid interference.
			if (doPlayerCollision && (mesh.flags & StaticMeshFlags::SM_SOLID))
				continue;

			if (Vector3i::Distance(mesh.pos.Position, laraItem->Pose.Position) >= COLLISION_CHECK_DISTANCE)
				continue;

			if (!TestBoundsCollideStatic(laraItem, mesh, coll->Setup.Radius))
				continue;

			coll->HitStatic = true;

			// HACK: Shatter statics only by non-harmless vehicles.
			if (!doPlayerCollision && 
				!harmless && abs(laraItem->Animation.Velocity.z) > VEHICLE_COLLISION_TERMINAL_VELOCITY &&
				StaticObjects[mesh.staticNumber].shatterType != SHT_NONE)
			{
				SoundEffect(GetShatterSound(mesh.staticNumber), &mesh.pos);
				ShatterObject(nullptr, &mesh, -128, laraItem->RoomNumber, 0);
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

		const auto& bounds = GetBestFrame(*item).BoundingBox;
		int rx = (bounds.X1 + bounds.X2) / 2;
		int rz = (bounds.X2 + bounds.Z2) / 2;

		if (bounds.GetHeight() > CLICK(1))
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
