#include "framework.h"
#include "Game/control/los.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/debris.h"
#include "Game/items.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/Switches/switch.h"
#include "Scripting/Include/Objects/ScriptInterfaceObjectsHandler.h"
#include "Scripting/Include/ScriptInterfaceGame.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"

using namespace TEN::Math;

// Globals
int NumberLosRooms;
int LosRooms[20];
int ClosestItem;
int ClosestDist;
Vector3i ClosestCoord;

static int xLOS(const GameVector& origin, GameVector& target)
{
	int dx = target.x - origin.x;
	if (dx == 0)
		return 1;

	int dy = BLOCK(target.y - origin.y) / dx;
	int dz = BLOCK(target.z - origin.z) / dx;

	NumberLosRooms = 1;
	LosRooms[0] = origin.RoomNumber;

	short roomNumber0 = origin.RoomNumber;
	short roomNumber1 = origin.RoomNumber;

	bool isNegative = (dx < 0);
	int sign = (isNegative ? -1 : 1);

	int x = isNegative ? (origin.x & (UINT_MAX - WALL_MASK)) : (origin.x | WALL_MASK);
	int y = (((x - origin.x) * dy) / BLOCK(1)) + origin.y;
	int z = (((x - origin.x) * dz) / BLOCK(1)) + origin.z;

	int flag = 1;
	while (isNegative ? (x > target.x) : (x < target.x))
	{
		auto* sectorPtr = GetFloor(x, y, z, &roomNumber0);
		if (roomNumber0 != roomNumber1)
		{
			roomNumber1 = roomNumber0;
			LosRooms[NumberLosRooms] = roomNumber0;
			++NumberLosRooms;
		}

		if (y > GetFloorHeight(sectorPtr, x, y, z) ||
			y < GetCeiling(sectorPtr, x, y, z))
		{
			flag = -1;
			break;
		}

		sectorPtr = GetFloor(x + sign, y, z, &roomNumber0);
		if (roomNumber0 != roomNumber1)
		{
			roomNumber1 = roomNumber0;
			LosRooms[NumberLosRooms] = roomNumber0;
			++NumberLosRooms;
		}

		if (y > GetFloorHeight(sectorPtr, x + sign, y, z) ||
			y < GetCeiling(sectorPtr, x + sign, y, z))
		{
			flag = 0;
			break;
		}

		x += BLOCK(sign);
		y += dy * sign;
		z += dz * sign;
	}

	if (flag != 1)
		target = GameVector(x, y, z, target.RoomNumber);

	target.RoomNumber = flag ? roomNumber0 : roomNumber1;
	return flag;
}

static int zLOS(const GameVector& origin, GameVector& target)
{
	int dz = target.z - origin.z;
	if (dz == 0)
		return 1;

	int dx = BLOCK(target.x - origin.x) / dz;
	int dy = BLOCK(target.y - origin.y) / dz;

	NumberLosRooms = 1;
	LosRooms[0] = origin.RoomNumber;

	short roomNumber0 = origin.RoomNumber;
	short roomNumber1 = origin.RoomNumber;

	bool isNegative = (dz < 0);
	int sign = (isNegative ? -1 : 1);

	int z = isNegative ? (origin.z & (UINT_MAX - WALL_MASK)) : (origin.z | WALL_MASK);
	int x = (((z - origin.z) * dx) / BLOCK(1)) + origin.x;
	int y = (((z - origin.z) * dy) / BLOCK(1)) + origin.y;

	int flag = 1;
	while (isNegative ? (z > target.z) : (z < target.z))
	{
		auto* sectorPtr = GetFloor(x, y, z, &roomNumber0);
		if (roomNumber0 != roomNumber1)
		{
			roomNumber1 = roomNumber0;
			LosRooms[NumberLosRooms] = roomNumber0;
			++NumberLosRooms;
		}

		if (y > GetFloorHeight(sectorPtr, x, y, z) ||
			y < GetCeiling(sectorPtr, x, y, z))
		{
			flag = -1;
			break;
		}

		sectorPtr = GetFloor(x, y, z + sign, &roomNumber0);
		if (roomNumber0 != roomNumber1)
		{
			roomNumber1 = roomNumber0;
			LosRooms[NumberLosRooms] = roomNumber0;
			++NumberLosRooms;
		}

		if (y > GetFloorHeight(sectorPtr, x, y, z + sign) ||
			y < GetCeiling(sectorPtr, x, y, z + sign))
		{
			flag = 0;
			break;
		}

		x += dx * sign;
		y += dy * sign;
		z += BLOCK(sign);
	}

	if (flag != 1)
		target = GameVector(x, y, z, target.RoomNumber);

	target.RoomNumber = flag ? roomNumber0 : roomNumber1;
	return flag;
}

static bool ClipTarget(const GameVector& origin, GameVector& target)
{
	int x, y, z, wx, wy, wz;

	short roomNumber = target.RoomNumber;
	if (target.y > GetFloorHeight(GetFloor(target.x, target.y, target.z, &roomNumber), target.x, target.y, target.z))
	{
		x = (7 * (target.x - origin.x) >> 3) + origin.x;
		y = (7 * (target.y - origin.y) >> 3) + origin.y;
		z = (7 * (target.z - origin.z) >> 3) + origin.z;

		for (int i = 3; i > 0; --i)
		{
			wx = ((target.x - x) * i >> 2) + x;
			wy = ((target.y - y) * i >> 2) + y;
			wz = ((target.z - z) * i >> 2) + z;

			if (wy < GetFloorHeight(GetFloor(wx, wy, wz, &roomNumber), wx, wy, wz))
				break;
		}

		target.x = wx;
		target.y = wy;
		target.z = wz;
		target.RoomNumber = roomNumber;
		return false;
	}

	roomNumber = target.RoomNumber;
	if (target.y < GetCeiling(GetFloor(target.x, target.y, target.z, &roomNumber), target.x, target.y, target.z))
	{
		x = (7 * (target.x - origin.x) >> 3) + origin.x;
		y = (7 * (target.y - origin.y) >> 3) + origin.y;
		z = (7 * (target.z - origin.z) >> 3) + origin.z;

		for (int i = 3; i > 0; --i)
		{
			wx = ((target.x - x) * i >> 2) + x;
			wy = ((target.y - y) * i >> 2) + y;
			wz = ((target.z - z) * i >> 2) + z;

			if (wy > GetCeiling(GetFloor(wx, wy, wz, &roomNumber), wx, wy, wz))
				break;
		}

		target.x = wx;
		target.y = wy;
		target.z = wz;
		target.RoomNumber = roomNumber;
		return false;
	}

	return true;
}

bool LOS(const GameVector* origin, GameVector* target)
{
	int losAxis0 = 0;
	int losAxis1 = 0;

	target->RoomNumber = origin->RoomNumber;
	if (abs(target->z - origin->z) > abs(target->x - origin->x))
	{
		losAxis0 = xLOS(*origin, *target);
		losAxis1 = zLOS(*origin, *target);
	}
	else
	{
		losAxis0 = zLOS(*origin, *target);
		losAxis1 = xLOS(*origin, *target);
	}

	if (losAxis1)
	{
		GetFloor(target->x, target->y, target->z, &target->RoomNumber);

		if (ClipTarget(*origin, *target) && losAxis0 == 1 && losAxis1 == 1)
			return true;
	}

	return false;
}

bool GetTargetOnLOS(GameVector* origin, GameVector* target, bool drawTarget, bool isFiring)
{
	auto dir = target->ToVector3() - origin->ToVector3();
	dir.Normalize();

	auto target2 = *target;
	int result = LOS(origin, &target2);

	GetFloor(target2.x, target2.y, target2.z, &target2.RoomNumber);

	if (isFiring && Lara.Control.Look.IsUsingLasersight)
	{
		Lara.Control.Weapon.HasFired = true;
		Lara.Control.Weapon.Fired = true;

		if (Lara.Control.Weapon.GunType == LaraWeaponType::Revolver)
			SoundEffect(SFX_TR4_REVOLVER_FIRE, nullptr);
	}

	bool hasHit = false;

	MESH_INFO* mesh = nullptr;
	auto vector = Vector3i::Zero;
	int itemNumber = ObjectOnLOS2(origin, target, &vector, &mesh);

	if (itemNumber != NO_LOS_ITEM)
	{
		target2.x = vector.x - ((vector.x - origin->x) >> 5);
		target2.y = vector.y - ((vector.y - origin->y) >> 5);
		target2.z = vector.z - ((vector.z - origin->z) >> 5);

		GetFloor(target2.x, target2.y, target2.z, &target2.RoomNumber);

		if (isFiring)
		{
			if (Lara.Control.Weapon.GunType != LaraWeaponType::Crossbow)
			{
				if (itemNumber < 0)
				{
					if (StaticObjects[mesh->staticNumber].shatterType != ShatterType::None)
					{
						ShatterImpactData.impactDirection = dir;
						ShatterImpactData.impactLocation = Vector3(mesh->pos.Position.x, mesh->pos.Position.y, mesh->pos.Position.z);
						ShatterObject(nullptr, mesh, 128, target2.RoomNumber, 0);
						SoundEffect(GetShatterSound(mesh->staticNumber), (Pose*)mesh);
					}

					TriggerRicochetSpark(target2, LaraItem->Pose.Orientation.y, 3, 0);
					TriggerRicochetSpark(target2, LaraItem->Pose.Orientation.y, 3, 0);
				}
				else
				{
					auto* item = &g_Level.Items[itemNumber];

					if (item->ObjectNumber < ID_SHOOT_SWITCH1 || item->ObjectNumber > ID_SHOOT_SWITCH4)
					{
						if ((Objects[item->ObjectNumber].explodableMeshbits & ShatterItem.bit) &&
							Lara.Control.Look.IsUsingLasersight)
						{
								item->MeshBits &= ~ShatterItem.bit;
								ShatterImpactData.impactDirection = dir;
								ShatterImpactData.impactLocation = Vector3(ShatterItem.sphere.x, ShatterItem.sphere.y, ShatterItem.sphere.z);
								ShatterObject(&ShatterItem, 0, 128, target2.RoomNumber, 0);
								TriggerRicochetSpark(target2, LaraItem->Pose.Orientation.y, 3, 0);							
						}
						else
						{
							auto* object = &Objects[item->ObjectNumber];

							if (drawTarget && (Lara.Control.Weapon.GunType == LaraWeaponType::Revolver ||
								Lara.Control.Weapon.GunType == LaraWeaponType::HK))
							{
								if (object->intelligent || object->HitRoutine)
								{
									const auto& weapon = Weapons[(int)Lara.Control.Weapon.GunType];

									int num = GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
									auto ray = Ray(origin->ToVector3(), dir);
									float bestDistance = INFINITY;
									int bestJointIndex = NO_JOINT;

									for (int i = 0; i < num; i++)
									{
										auto sphere = BoundingSphere(Vector3(CreatureSpheres[i].x, CreatureSpheres[i].y, CreatureSpheres[i].z), CreatureSpheres[i].r);
										float distance = 0.0f;
										if (ray.Intersects(sphere, distance))
										{
											if (distance < bestDistance)
											{
												bestDistance = distance;
												bestJointIndex = i;
											}
										}
									}
									HitTarget(LaraItem, item, &target2, Weapons[(int)Lara.Control.Weapon.GunType].Damage, false, bestJointIndex);
								}
								else
								{
									// TR5
									if (object->hitEffect == HitEffect::Richochet)
										TriggerRicochetSpark(target2, LaraItem->Pose.Orientation.y, 3, 0);
								}
							}
							else
							{
								if (item->ObjectNumber >= ID_SMASH_OBJECT1 && item->ObjectNumber <= ID_SMASH_OBJECT8)
								{
									SmashObject(itemNumber);
								}
								else
								{
									const auto& weapon = Weapons[(int)Lara.Control.Weapon.GunType];
									if (object->HitRoutine != nullptr)
									{
										object->HitRoutine(*item, *LaraItem, target2, weapon.Damage, false, NO_JOINT);
									}
									else
									{
										DefaultItemHit(*item, *LaraItem, target2, weapon.Damage, false, NO_JOINT);
									}
								}
							}
						}
					}
					else
					{
						if (ShatterItem.bit == 1 << (Objects[item->ObjectNumber].nmeshes - 1))
						{
							if (!(item->Flags & 0x40))
							{
								if (item->ObjectNumber == ID_SHOOT_SWITCH1)
									ExplodeItemNode(item, Objects[item->ObjectNumber].nmeshes - 1, 0, 64);

								if (item->TriggerFlags == 444 && item->ObjectNumber == ID_SHOOT_SWITCH2)
								{
									// TR5 ID_SWITCH_TYPE_8/ID_SHOOT_SWITCH2
									ProcessExplodingSwitchType8(item);
								}
								else
								{
									/*if (item->objectNumber == ID_SHOOT_SWITCH3)
									{
										// TR4 ID_SWITCH_TYPE7
										ExplodeItemNode(item, Objects[item->objectNumber].nmeshes - 1, 0, 64);
									}*/

									if (item->Flags & IFLAG_ACTIVATION_MASK &&
										(item->Flags & IFLAG_ACTIVATION_MASK) != IFLAG_ACTIVATION_MASK)
									{
										TestTriggers(item->Pose.Position.x, item->Pose.Position.y - 256, item->Pose.Position.z, item->RoomNumber, true, item->Flags & IFLAG_ACTIVATION_MASK);
									}
									else
									{
										short triggerItems[8];
										for (int count = GetSwitchTrigger(item, triggerItems, 1); count > 0; --count)
										{
											AddActiveItem(triggerItems[count - 1]);
											g_Level.Items[triggerItems[count - 1]].Status = ITEM_ACTIVE;
											g_Level.Items[triggerItems[count - 1]].Flags |= IFLAG_ACTIVATION_MASK;
										}
									}
								}
							}

							if (item->Status != ITEM_DEACTIVATED)
							{
								AddActiveItem(itemNumber);
								item->Status = ITEM_ACTIVE;
								item->Flags |= IFLAG_ACTIVATION_MASK | 0x40;
							}
						}

						TriggerRicochetSpark(target2, LaraItem->Pose.Orientation.y, 3, 0);
					}
				}
			}
			else
			{
				if (Lara.Control.Look.IsUsingLasersight && isFiring)
					FireCrossBowFromLaserSight(*LaraItem, origin, &target2);
			}
		}

		hasHit = true;
	}
	else
	{
		if (Lara.Control.Weapon.GunType == LaraWeaponType::Crossbow)
		{
			if (isFiring && Lara.Control.Look.IsUsingLasersight)
				FireCrossBowFromLaserSight(*LaraItem, origin, &target2);
		}
		else
		{
			target2.x -= (target2.x - origin->x) >> 5;
			target2.y -= (target2.y - origin->y) >> 5;
			target2.z -= (target2.z - origin->z) >> 5;

			if (isFiring && !result)
				TriggerRicochetSpark(target2, LaraItem->Pose.Orientation.y, 8, 0);
		}
	}

	if (drawTarget && (hasHit || !result))
	{
		TriggerDynamicLight(target2.x, target2.y, target2.z, 64, 255, 0, 0);
		LaserSightActive = 1;
		LaserSightX = target2.x;
		LaserSightY = target2.y;
		LaserSightZ = target2.z;
	}

	return hasHit;
}

static bool DoRayBox(const GameVector& origin, const GameVector& target, const GameBoundingBox& bounds,
					 const Pose& objectPose, Vector3i& hitPos, int closestItemNumber)
{
	auto box = bounds.ToBoundingOrientedBox(objectPose);

	auto rayOrigin = origin.ToVector3();
	auto rayDir = (target - origin).ToVector3();
	rayDir.Normalize();

	// Don't test spheres if no intersection.
	float dist = 0.0f;
	if (rayDir == Vector3::Zero || !box.Intersects(rayOrigin, rayDir, dist))
		return false;

	// Get raw collision point.
	auto collidedPoint = Geometry::TranslatePoint(rayOrigin, rayDir, dist);
	hitPos.x = collidedPoint.x - objectPose.Position.x;
	hitPos.y = collidedPoint.y - objectPose.Position.y;
	hitPos.z = collidedPoint.z - objectPose.Position.z;

	// Test single spheres in case of items.
	int meshIndex = 0;
	int bit = 0;
	int sp = -2;
	float minDist = INFINITY;

	if (closestItemNumber < 0)
	{
		// Static meshes don't require further tests.
		sp = -1;
		minDist = dist;
	}
	else
	{
		// Test spheres instead for items.
		auto* item = &g_Level.Items[closestItemNumber];
		auto* object = &Objects[item->ObjectNumber];

		// Get transformed mesh sphere.
		GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
		SPHERE spheres[34];
		memcpy(spheres, CreatureSpheres, sizeof(SPHERE) * 34);

		if (object->nmeshes <= 0)
			return false;

		meshIndex = object->meshIndex;

		for (int i = 0; i < object->nmeshes; i++)
		{
			// If mesh is visible.
			if (item->MeshBits & (1 << i))
			{
				auto* sphere = &CreatureSpheres[i];

				// TODO: this approach is the correct one but, again, Core's math is a mystery and this test was meant
				// to fail deliberately in some way. I've so added again Core's legacy test for allowing the current game logic
				// but after more testing we should trash it in the future and restore the new way.

#if 0
				// Create the bounding sphere and test it against the ray
				BoundingSphere sph = BoundingSphere(Vector3(sphere->x, sphere->y, sphere->z), sphere->r);
				float newDist;
				if (sph.Intersects(rayStart, rayDirNormalized, newDist))
				{
					// HACK: Core seems to take in account for distance not the real hit point but the centre of the sphere.
					// This can work well for example for GUARDIAN because the head sphere is so big that would always be hit
					// and eyes would not be destroyed.
					newDist = sqrt(SQUARE(sphere->x - start->x) + SQUARE(sphere->y - start->y) + SQUARE(sphere->z - start->z));

					// Test for min distance
					if (newDist < minDistance)
					{
						minDistance = newDist;
						meshPtr = &g_Level.Meshes[obj->meshIndex + i];
						bit = 1 << i;
						sp = i;
					}
				}
#endif

				Vector3i p[4];

				p[1].x = origin.x;
				p[1].y = origin.y;
				p[1].z = origin.z;
				p[2].x = target.x;
				p[2].y = target.y;
				p[2].z = target.z;
				p[3].x = sphere->x;
				p[3].y = sphere->y;
				p[3].z = sphere->z;

				int r0 = (p[3].x - p[1].x) * (p[2].x - p[1].x) +
					(p[3].y - p[1].y) * (p[2].y - p[1].y) +
					(p[3].z - p[1].z) * (p[2].z - p[1].z);

				int r1 = SQUARE(p[2].x - p[1].x) +
					SQUARE(p[2].y - p[1].y) +
					SQUARE(p[2].z - p[1].z);

				if (((r0 < 0 && r1 < 0) ||
					(r1 > 0 && r0 > 0)) &&
					(abs(r0) <= abs(r1)))
				{
					r1 >>= 16;
					if (r1)
						r0 /= r1;
					else
						r0 = 0;

					p[0].x = p[1].x + ((r0 * (p[2].x - p[1].x)) >> 16);
					p[0].y = p[1].y + ((r0 * (p[2].y - p[1].y)) >> 16);
					p[0].z = p[1].z + ((r0 * (p[2].z - p[1].z)) >> 16);

					int dx = SQUARE(p[0].x - p[3].x);
					int dy = SQUARE(p[0].y - p[3].y);
					int dz = SQUARE(p[0].z - p[3].z);

					int distance = dx + dy + dz;

					if (distance < SQUARE(sphere->r))
					{
						dx = SQUARE(sphere->x - origin.x);
						dy = SQUARE(sphere->y - origin.y);
						dz = SQUARE(sphere->z - origin.z);

						distance = dx + dy + dz;

						if (distance < minDist)
						{
							minDist = distance;
							meshIndex = object->meshIndex + i;
							bit = 1 << i;
							sp = i;
						}
					}
				}
			}
		}

		if (sp < -1)
			return false;
	}

	if (dist >= ClosestDist)
		return false;

	// Set up test result.
	ClosestCoord.x = hitPos.x + objectPose.Position.x;
	ClosestCoord.y = hitPos.y + objectPose.Position.y;
	ClosestCoord.z = hitPos.z + objectPose.Position.z;
	ClosestDist = dist;
	ClosestItem = closestItemNumber;

	// If collided object is item, set up shatter item data struct.
	if (sp >= 0)
	{
		auto* item = &g_Level.Items[closestItemNumber];

		GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD | SPHERES_SPACE_BONE_ORIGIN, Matrix::Identity);

		ShatterItem.yRot = item->Pose.Orientation.y;
		ShatterItem.meshIndex = meshIndex;
		ShatterItem.color = item->Model.Color;
		ShatterItem.sphere.x = CreatureSpheres[sp].x;
		ShatterItem.sphere.y = CreatureSpheres[sp].y;
		ShatterItem.sphere.z = CreatureSpheres[sp].z;
		ShatterItem.bit = bit;
		ShatterItem.flags = 0;
	}

	return true;
}

int ObjectOnLOS2(GameVector* origin, GameVector* target, Vector3i* vec, MESH_INFO** mesh, GAME_OBJECT_ID priorityObject)
{
	ClosestItem = NO_LOS_ITEM;
	ClosestDist = SQUARE(target->x - origin->x) + SQUARE(target->y - origin->y) + SQUARE(target->z - origin->z);

	for (int r = 0; r < NumberLosRooms; ++r)
	{
		auto* room = &g_Level.Rooms[LosRooms[r]];

		auto pose = Pose::Zero;

		for (int m = 0; m < room->mesh.size(); m++)
		{
			auto* meshp = &room->mesh[m];

			if (meshp->flags & StaticMeshFlags::SM_VISIBLE)
			{
				auto bounds = GetBoundsAccurate(*meshp, false);
				pose = Pose(meshp->pos.Position, EulerAngles(0, meshp->pos.Orientation.y, 0));

				if (DoRayBox(*origin, *target, bounds, pose, *vec, -1 - meshp->staticNumber))
				{
					*mesh = meshp;
					target->RoomNumber = LosRooms[r];
				}
			}
		}

		for (short linkNumber = room->itemNumber; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
		{
			auto* item = &g_Level.Items[linkNumber];

			if ((item->Status == ITEM_DEACTIVATED) || (item->Status == ITEM_INVISIBLE))
				continue;

			if ((priorityObject != GAME_OBJECT_ID::ID_NO_OBJECT) && (item->ObjectNumber != priorityObject))
				continue;

			if ((item->ObjectNumber != ID_LARA) && (Objects[item->ObjectNumber].collision == nullptr))
				continue;

			if ((item->ObjectNumber == ID_LARA) && (priorityObject != ID_LARA))
				continue;

			auto bounds = GameBoundingBox(item);
			pose = Pose(item->Pose.Position, EulerAngles(0, item->Pose.Orientation.y, 0));

			if (DoRayBox(*origin, *target, bounds, pose, *vec, linkNumber))
				target->RoomNumber = LosRooms[r];
		}
	}

	vec->x = ClosestCoord.x;
	vec->y = ClosestCoord.y;
	vec->z = ClosestCoord.z;

	return ClosestItem;
}

bool LOSAndReturnTarget(GameVector* origin, GameVector* target, int push)
{
	int x = origin->x;
	int y = origin->y;
	int z = origin->z;
	short roomNumber = origin->RoomNumber;
	short roomNumber2 = roomNumber;
	int dx = (target->x - x) >> 3;
	int dy = (target->y - y) >> 3;
	int dz = (target->z - z) >> 3;
	bool flag = false;
	bool result = false;

	int i;
	for (i = 0; i < 8; ++i)
	{
		roomNumber2 = roomNumber;
		auto* floor = GetFloor(x, y, z, &roomNumber);

		if (g_Level.Rooms[roomNumber2].flags & ENV_FLAG_SWAMP)
		{
			flag = true;
			break;
		}

		int floorHeight = GetFloorHeight(floor, x, y, z);
		int ceilingHeight = GetCeiling(floor, x, y, z);
		if (floorHeight != NO_HEIGHT && ceilingHeight != NO_HEIGHT && ceilingHeight < floorHeight)
		{
			if (y > floorHeight)
			{
				if (y - floorHeight >= push)
				{
					flag = true;
					break;
				}

				y = floorHeight;
			}

			if (y < ceilingHeight)
			{
				if (ceilingHeight - y >= push)
				{
					flag = true;
					break;
				}

				y = ceilingHeight;
			}

			result = true;
		}
		else if (result)
		{
			flag = true;
			break;
		}

		x += dx;
		y += dy;
		z += dz;
	}

	if (i)
	{
		x -= dx;
		y -= dy;
		z -= dz;
	}

	GetFloor(x, y, z, &roomNumber2);
	target->x = x;
	target->y = y;
	target->z = z;
	target->RoomNumber = roomNumber2;

	return !flag;
}

// TODO: Extend to be a more general, simple, all-in-one LOS function with a variety of flags for what to detect.
std::optional<Vector3> GetStaticObjectLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool onlySolid)
{
	// Run through neighbor rooms.
	const auto& room = g_Level.Rooms[roomNumber];
	for (int neighborRoomNumber : room.neighbors)
	{
		// Get neighbor room.
		const auto& neighborRoom = g_Level.Rooms[neighborRoomNumber];
		if (!neighborRoom.Active())
			continue;

		// Run through statics.
		for (const auto& staticObject : g_Level.Rooms[neighborRoomNumber].mesh)
		{
			// Check if static is visible.
			if (!(staticObject.flags & StaticMeshFlags::SM_VISIBLE))
				continue;

			// Check if static is solid (if applicable).
			if (onlySolid && !(staticObject.flags & StaticMeshFlags::SM_SOLID))
				continue;

			// Test ray-box intersection.
			auto box = GetBoundsAccurate(staticObject, false).ToBoundingOrientedBox(staticObject.pos);
			if (box.Intersects(origin, dir, dist))
				return Geometry::TranslatePoint(origin, dir, dist);
		}
	}

	return std::nullopt;
}
