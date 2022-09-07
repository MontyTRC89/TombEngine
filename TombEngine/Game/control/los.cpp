#include "framework.h"
#include "Game/control/los.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/debris.h"
#include "Game/items.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_one_gun.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/Switches/switch.h"
#include "Objects/ScriptInterfaceObjectsHandler.h"
#include "ScriptInterfaceGame.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/setup.h"

int NumberLosRooms;
short LosRooms[20];
int ClosestItem;
int ClosestDist;
Vector3Int ClosestCoord;

bool ClipTarget(GameVector* origin, GameVector* target)
{
	int x, y, z, wx, wy, wz;

	short roomNumber = target->roomNumber;
	if (target->y > GetFloorHeight(GetFloor(target->x, target->y, target->z, &roomNumber), target->x, target->y, target->z))
	{
		x = (7 * (target->x - origin->x) >> 3) + origin->x;
		y = (7 * (target->y - origin->y) >> 3) + origin->y;
		z = (7 * (target->z - origin->z) >> 3) + origin->z;

		for (int i = 3; i > 0; --i)
		{
			wx = ((target->x - x) * i >> 2) + x;
			wy = ((target->y - y) * i >> 2) + y;
			wz = ((target->z - z) * i >> 2) + z;

			if (wy < GetFloorHeight(GetFloor(wx, wy, wz, &roomNumber), wx, wy, wz))
				break;
		}

		target->x = wx;
		target->y = wy;
		target->z = wz;
		target->roomNumber = roomNumber;
		return false;
	}

	roomNumber = target->roomNumber;
	if (target->y < GetCeiling(GetFloor(target->x, target->y, target->z, &roomNumber), target->x, target->y, target->z))
	{
		x = (7 * (target->x - origin->x) >> 3) + origin->x;
		y = (7 * (target->y - origin->y) >> 3) + origin->y;
		z = (7 * (target->z - origin->z) >> 3) + origin->z;

		for (int i = 3; i > 0; --i)
		{
			wx = ((target->x - x) * i >> 2) + x;
			wy = ((target->y - y) * i >> 2) + y;
			wz = ((target->z - z) * i >> 2) + z;

			if (wy > GetCeiling(GetFloor(wx, wy, wz, &roomNumber), wx, wy, wz))
				break;
		}

		target->x = wx;
		target->y = wy;
		target->z = wz;
		target->roomNumber = roomNumber;
		return false;
	}

	return true;
}

bool GetTargetOnLOS(GameVector* origin, GameVector* target, bool drawTarget, bool isFiring)
{
	auto direction = Vector3(target->x, target->y, target->z) - Vector3(origin->x, origin->y, origin->z);
	direction.Normalize();

	auto target2 = GameVector(target->x, target->y, target->z);
	int result = LOS(origin, &target2);

	GetFloor(target2.x, target2.y, target2.z, &target2.roomNumber);

	if (isFiring && LaserSight)
	{
		Lara.Control.Weapon.HasFired = true;
		Lara.Control.Weapon.Fired = true;

		if (Lara.Control.Weapon.GunType == LaraWeaponType::Revolver)
			SoundEffect(SFX_TR4_REVOLVER_FIRE, nullptr);
	}

	bool hasHit = false;

	MESH_INFO* mesh;
	Vector3Int vector;
	int itemNumber = ObjectOnLOS2(origin, target, &vector, &mesh);

	if (itemNumber != NO_LOS_ITEM)
	{
		target2.x = vector.x - (vector.x - origin->x >> 5);
		target2.y = vector.y - (vector.y - origin->y >> 5);
		target2.z = vector.z - (vector.z - origin->z >> 5);

		GetFloor(target2.x, target2.y, target2.z, &target2.roomNumber);

		if (isFiring)
		{
			if (Lara.Control.Weapon.GunType != LaraWeaponType::Crossbow)
			{
				if (itemNumber < 0)
				{
					if (StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
					{
						ShatterImpactData.impactDirection = direction;
						ShatterImpactData.impactLocation = Vector3(mesh->pos.Position.x, mesh->pos.Position.y, mesh->pos.Position.z);
						ShatterObject(nullptr, mesh, 128, target2.roomNumber, 0);
						SmashedMeshRoom[SmashedMeshCount] = target2.roomNumber;
						SmashedMesh[SmashedMeshCount] = mesh;
						++SmashedMeshCount;
						mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
						SoundEffect(GetShatterSound(mesh->staticNumber), (PHD_3DPOS*)mesh);
					}

					TriggerRicochetSpark(&target2, LaraItem->Pose.Orientation.y, 3, 0);
					TriggerRicochetSpark(&target2, LaraItem->Pose.Orientation.y, 3, 0);
				}
				else
				{
					auto* item = &g_Level.Items[itemNumber];

					if (item->ObjectNumber < ID_SHOOT_SWITCH1 || item->ObjectNumber > ID_SHOOT_SWITCH4)
					{
						if ((Objects[item->ObjectNumber].explodableMeshbits & ShatterItem.bit) &&
							LaserSight)
						{
							//if (!Objects[item->objectNumber].intelligent)
							//{
							item->MeshBits &= ~ShatterItem.bit;
							ShatterImpactData.impactDirection = direction;
							ShatterImpactData.impactLocation = Vector3(ShatterItem.sphere.x, ShatterItem.sphere.y, ShatterItem.sphere.z);
							ShatterObject(&ShatterItem, 0, 128, target2.roomNumber, 0);
							TriggerRicochetSpark(&target2, LaraItem->Pose.Orientation.y, 3, 0);
							/*}
							else
							{
								if (item->objectNumber != ID_GUARD_LASER)
								{
									DoDamage(item, 30);
									HitTarget(item, &target, Weapons[Lara.gunType].damage, 0);
								}
								else
								{
									angle = atan2(LaraItem->pos.Position.z - item->pos.Position.z, LaraItem->pos.Position.x - item->pos.Position.x) - item->pos.Orientation.y;
									if (angle > Angle::DegToRad(-90) && angle < Angle::DegToRad(90))
									{
										DoDamage(item, INT_MAX);
										HitTarget(item, &target, Weapons[Lara.gunType].damage, 0);
									}
								}
							}*/
						}
						else
						{
							if (drawTarget && (Lara.Control.Weapon.GunType == LaraWeaponType::Revolver ||
								Lara.Control.Weapon.GunType == LaraWeaponType::HK))
							{
								if (Objects[item->ObjectNumber].intelligent)
									HitTarget(LaraItem, item, &target2, Weapons[(int)Lara.Control.Weapon.GunType].Damage, 0);
								else
								{
									// TR5
									if (Objects[item->ObjectNumber].hitEffect == HIT_RICOCHET)
										TriggerRicochetSpark(&target2, LaraItem->Pose.Orientation.y, 3, 0);
								}
							}
							else
							{
								if (item->ObjectNumber >= ID_SMASH_OBJECT1 && item->ObjectNumber <= ID_SMASH_OBJECT8)
									SmashObject(itemNumber);
								else
								{
									if (Objects[item->ObjectNumber].hitEffect == HIT_BLOOD)
										DoBloodSplat(target2.x, target2.y, target2.z, (GetRandomControl() & 3) + 3, item->Pose.Orientation.y, item->RoomNumber);
									else if (Objects[item->ObjectNumber].hitEffect == HIT_SMOKE)
										TriggerRicochetSpark(&target2, LaraItem->Pose.Orientation.y, 3, -5);
									else if (Objects[item->ObjectNumber].hitEffect == HIT_RICOCHET)
										TriggerRicochetSpark(&target2, LaraItem->Pose.Orientation.y, 3, 0);

									DoDamage(item, Weapons[(int)Lara.Control.Weapon.GunType].Damage);

									if (!item->LuaCallbackOnHitName.empty())
									{
										short index = g_GameScriptEntities->GetIndexByName(item->LuaName);
										g_GameScript->ExecuteFunction(item->LuaCallbackOnHitName, index);
									}
								}
							}
						}
					}
					else
					{
						if (ShatterItem.bit == 1 << Objects[item->ObjectNumber].nmeshes - 1)
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

						TriggerRicochetSpark(&target2, LaraItem->Pose.Orientation.y, 3, 0);
					}
				}
			}
			else
			{
				if (LaserSight && isFiring)
					FireCrossBowFromLaserSight(LaraItem, origin, &target2);
			}
		}

		hasHit = true;
	}
	else
	{
		if (Lara.Control.Weapon.GunType == LaraWeaponType::Crossbow)
		{
			if (isFiring && LaserSight)
				FireCrossBowFromLaserSight(LaraItem, origin, &target2);
		}
		else
		{
			target2.x -= target2.x - origin->x >> 5;
			target2.y -= target2.y - origin->y >> 5;
			target2.z -= target2.z - origin->z >> 5;

			if (isFiring && !result)
				TriggerRicochetSpark(&target2, LaraItem->Pose.Orientation.y, 8, 0);
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

int ObjectOnLOS2(GameVector* origin, GameVector* target, Vector3Int* vec, MESH_INFO** mesh, GAME_OBJECT_ID priorityObject)
{
	ClosestItem = NO_LOS_ITEM;
	ClosestDist = SQUARE(target->x - origin->x) + SQUARE(target->y - origin->y) + SQUARE(target->z - origin->z);

	for (int r = 0; r < NumberLosRooms; ++r)
	{
		PHD_3DPOS pos;
		auto* room = &g_Level.Rooms[LosRooms[r]];

		for (int m = 0; m < room->mesh.size(); m++)
		{
			auto* meshp = &room->mesh[m];

			if (meshp->flags & StaticMeshFlags::SM_VISIBLE)
			{
				pos.Position = meshp->pos.Position;
				pos.Orientation.y = meshp->pos.Orientation.y;

				if (DoRayBox(origin, target, GetBoundsAccurate(meshp, false), &pos, vec, -1 - meshp->staticNumber))
				{
					*mesh = meshp;
					target->roomNumber = LosRooms[r];
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

			auto* box = GetBoundsAccurate(item);

			pos.Position = item->Pose.Position;
			pos.Orientation.y = item->Pose.Orientation.y;

			if (DoRayBox(origin, target, box, &pos, vec, linkNumber))
				target->roomNumber = LosRooms[r];
		}
	}

	vec->x = ClosestCoord.x;
	vec->y = ClosestCoord.y;
	vec->z = ClosestCoord.z;

	return ClosestItem;
}

bool DoRayBox(GameVector* origin, GameVector* target, BOUNDING_BOX* box, PHD_3DPOS* itemOrStaticPos, Vector3Int* hitPos, short closesItemNumber)
{
	// Ray
	FXMVECTOR rayOrigin = { (float)origin->x, (float)origin->y, (float)origin->z };
	FXMVECTOR rayTarget = { (float)target->x, (float)target->y, (float)target->z };
	FXMVECTOR rayDirection = { (float)(target->x - origin->x), (float)(target->y - origin->y), (float)(target->z - origin->z) };
	XMVECTOR rayDirectionNorm = XMVector3Normalize(rayDirection);

	// Create the bounding box for raw collision detection
	auto obox = TO_DX_BBOX(*itemOrStaticPos, box);

	// Get the collision with the bounding box
	float distance;
	bool collided = obox.Intersects(rayOrigin, rayDirectionNorm, distance);

	// If no collision happened, then don't test spheres
	if (!collided)
		return false;

	// Get the raw collision point
	Vector3 collidedPoint = rayOrigin + distance * rayDirectionNorm;
	hitPos->x = collidedPoint.x - itemOrStaticPos->Position.x;
	hitPos->y = collidedPoint.y - itemOrStaticPos->Position.y;
	hitPos->z = collidedPoint.z - itemOrStaticPos->Position.z;

	// Now in the case of items we need to test single spheres
	int meshIndex = 0;
	int bit = 0;
	int sp = -2;
	float minDistance = std::numeric_limits<float>::max();

	if (closesItemNumber < 0)
	{
		// Static meshes don't require further tests
		sp = -1;
		minDistance = distance;
	}
	else
	{
		// For items instead we need to test spheres
		ItemInfo* item = &g_Level.Items[closesItemNumber];
		ObjectInfo* obj = &Objects[item->ObjectNumber];

		// Get the transformed sphere of meshes
		GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
		SPHERE spheres[34];
		memcpy(spheres, CreatureSpheres, sizeof(SPHERE) * 34);

		if (obj->nmeshes <= 0)
			return false;

		meshIndex = obj->meshIndex;

		for (int i = 0; i < obj->nmeshes; i++)
		{
			// If mesh is visible...
			if (item->MeshBits & (1 << i))
			{
				SPHERE* sphere = &CreatureSpheres[i];

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

				Vector3Int p[4];

				p[1].x = origin->x;
				p[1].y = origin->y;
				p[1].z = origin->z;
				p[2].x = target->x;
				p[2].y = target->y;
				p[2].z = target->z;
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

					int dx = p[0].x - p[3].x;
					int dy = p[0].y - p[3].y;
					int dz = p[0].z - p[3].z;

					int distance = dx + dy + dz;

					if (distance < SQUARE(sphere->r))
					{
						dx = SQUARE(sphere->x - origin->x);
						dy = SQUARE(sphere->y - origin->y);
						dz = SQUARE(sphere->z - origin->z);

						distance = dx + dy + dz;

						if (distance < minDistance)
						{
							minDistance = distance;
							meshIndex = obj->meshIndex + i;
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

	if (distance >= ClosestDist)
		return false;

	// Setup test result
	ClosestCoord.x = hitPos->x + itemOrStaticPos->Position.x;
	ClosestCoord.y = hitPos->y + itemOrStaticPos->Position.y;
	ClosestCoord.z = hitPos->z + itemOrStaticPos->Position.z;
	ClosestDist = distance;
	ClosestItem = closesItemNumber;

	// If collided object is an item, then setup the shatter item data struct
	if (sp >= 0)
	{
		ItemInfo* item = &g_Level.Items[closesItemNumber];

		GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD | SPHERES_SPACE_BONE_ORIGIN, Matrix::Identity);

		ShatterItem.yRot = item->Pose.Orientation.y;
		ShatterItem.meshIndex = meshIndex;
		ShatterItem.color = item->Color;
		ShatterItem.sphere.x = CreatureSpheres[sp].x;
		ShatterItem.sphere.y = CreatureSpheres[sp].y;
		ShatterItem.sphere.z = CreatureSpheres[sp].z;
		ShatterItem.bit = bit;
		ShatterItem.flags = 0;
	}

	return true;
}

bool LOS(GameVector* origin, GameVector* target)
{
	int result1, result2;

	target->roomNumber = origin->roomNumber;
	if (abs(target->z - origin->z) > abs(target->x - origin->x))
	{
		result1 = xLOS(origin, target);
		result2 = zLOS(origin, target);
	}
	else
	{
		result1 = zLOS(origin, target);
		result2 = xLOS(origin, target);
	}

	if (result2)
	{
		GetFloor(target->x, target->y, target->z, &target->roomNumber);
		if (ClipTarget(origin, target) && result1 == 1 && result2 == 1)
			return true;
	}

	return false;
}

int xLOS(GameVector* origin, GameVector* target)
{
	int dx = target->x - origin->x;
	if (!dx)
		return 1;

	int dy = (target->y - origin->y << 10) / dx;
	int dz = (target->z - origin->z << 10) / dx;

	NumberLosRooms = 1;
	LosRooms[0] = origin->roomNumber;

	short room = origin->roomNumber;
	short room2 = origin->roomNumber;

	int flag = 1;
	if (dx < 0)
	{
		int x = origin->x & 0xFFFFFC00;
		int y = ((x - origin->x) * dy >> 10) + origin->y;
		int z = ((x - origin->x) * dz >> 10) + origin->z;

		while (x > target->x)
		{
			auto* floor = GetFloor(x, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				LosRooms[NumberLosRooms] = room;
				++NumberLosRooms;
			}

			if (y > GetFloorHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
			{
				flag = -1;
				break;
			}

			floor = GetFloor(x - 1, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				LosRooms[NumberLosRooms] = room;
				++NumberLosRooms;
			}

			if (y > GetFloorHeight(floor, x - 1, y, z) || y < GetCeiling(floor, x - 1, y, z))
			{
				flag = 0;
				break;
			}

			x -= SECTOR(1);
			y -= dy;
			z -= dz;
		}

		if (flag != 1)
		{
			target->x = x;
			target->y = y;
			target->z = z;
		}

		target->roomNumber = flag ? room : room2;
	}
	else
	{
		int x = origin->x | 0x3FF;
		int y = ((x - origin->x) * dy >> 10) + origin->y;
		int z = ((x - origin->x) * dz >> 10) + origin->z;

		while (x < target->x)
		{
			auto* floor = GetFloor(x, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				LosRooms[NumberLosRooms] = room;
				++NumberLosRooms;
			}

			if (y > GetFloorHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
			{
				flag = -1;
				break;
			}

			floor = GetFloor(x + 1, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				LosRooms[NumberLosRooms] = room;
				++NumberLosRooms;
			}

			if (y > GetFloorHeight(floor, x + 1, y, z) || y < GetCeiling(floor, x + 1, y, z))
			{
				flag = 0;
				break;
			}

			x += SECTOR(1);
			y += dy;
			z += dz;
		}

		if (flag != 1)
		{
			target->x = x;
			target->y = y;
			target->z = z;
		}

		target->roomNumber = flag ? room : room2;
	}

	return flag;
}

int zLOS(GameVector* origin, GameVector* target)
{
	int dz = target->z - origin->z;
	if (!dz)
		return 1;

	int dx = (target->x - origin->x << 10) / dz;
	int dy = (target->y - origin->y << 10) / dz;

	NumberLosRooms = 1;
	LosRooms[0] = origin->roomNumber;

	short room = origin->roomNumber;
	short room2 = origin->roomNumber;

	int flag = 1;
	if (dz < 0)
	{
		int z = origin->z & 0xFFFFFC00;
		int x = ((z - origin->z) * dx >> 10) + origin->x;
		int y = ((z - origin->z) * dy >> 10) + origin->y;

		while (z > target->z)
		{
			auto* floor = GetFloor(x, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				LosRooms[NumberLosRooms] = room;
				++NumberLosRooms;
			}

			if (y > GetFloorHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
			{
				flag = -1;
				break;
			}

			floor = GetFloor(x, y, z - 1, &room);
			if (room != room2)
			{
				room2 = room;
				LosRooms[NumberLosRooms] = room;
				++NumberLosRooms;
			}

			if (y > GetFloorHeight(floor, x, y, z - 1) || y < GetCeiling(floor, x, y, z - 1))
			{
				flag = 0;
				break;
			}

			z -= SECTOR(1);
			x -= dx;
			y -= dy;
		}

		if (flag != 1)
		{
			target->x = x;
			target->y = y;
			target->z = z;
		}

		target->roomNumber = flag ? room : room2;
	}
	else
	{
		int z = origin->z | 0x3FF;
		int x = ((z - origin->z) * dx >> 10) + origin->x;
		int y = ((z - origin->z) * dy >> 10) + origin->y;

		while (z < target->z)
		{
			auto* floor = GetFloor(x, y, z, &room);
			if (room != room2)
			{
				room2 = room;
				LosRooms[NumberLosRooms] = room;
				++NumberLosRooms;
			}

			if (y > GetFloorHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z))
			{
				flag = -1;
				break;
			}

			floor = GetFloor(x, y, z + 1, &room);
			if (room != room2)
			{
				room2 = room;
				LosRooms[NumberLosRooms] = room;
				++NumberLosRooms;
			}

			if (y > GetFloorHeight(floor, x, y, z + 1) || y < GetCeiling(floor, x, y, z + 1))
			{
				flag = 0;
				break;
			}

			z += SECTOR(1);
			x += dx;
			y += dy;
		}

		if (flag != 1)
		{
			target->x = x;
			target->y = y;
			target->z = z;
		}

		target->roomNumber = flag ? room : room2;
	}

	return flag;
}

bool LOSAndReturnTarget(GameVector* origin, GameVector* target, int push)
{
	int x = origin->x;
	int y = origin->y;
	int z = origin->z;
	short roomNumber = origin->roomNumber;
	short roomNumber2 = roomNumber;
	int dx = target->x - x >> 3;
	int dy = target->y - y >> 3;
	int dz = target->z - z >> 3;
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
	target->roomNumber = roomNumber2;

	return !flag;
}
