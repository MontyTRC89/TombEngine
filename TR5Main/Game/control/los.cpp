#include "framework.h"
#include "Game/control/los.h"

#include "Game/animation.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/debris.h"
#include "Game/items.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_one_gun.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/Switches/switch.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/setup.h"

int NumberLosRooms;
short LosRooms[20];
int ClosestItem;
int ClosestDist;
PHD_VECTOR ClosestCoord;

bool ClipTarget(GAME_VECTOR* start, GAME_VECTOR* target)
{
	int x, y, z, wx, wy, wz;

	short roomNumber = target->roomNumber;
	if (target->y > GetFloorHeight(GetFloor(target->x, target->y, target->z, &roomNumber), target->x, target->y, target->z))
	{
		x = (7 * (target->x - start->x) >> 3) + start->x;
		y = (7 * (target->y - start->y) >> 3) + start->y;
		z = (7 * (target->z - start->z) >> 3) + start->z;

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
		x = (7 * (target->x - start->x) >> 3) + start->x;
		y = (7 * (target->y - start->y) >> 3) + start->y;
		z = (7 * (target->z - start->z) >> 3) + start->z;

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

bool GetTargetOnLOS(GAME_VECTOR* src, GAME_VECTOR* dest, int drawTarget, int firing)
{
	Vector3 direction = Vector3(dest->x, dest->y, dest->z) - Vector3(src->x, src->y, src->z);
	direction.Normalize();

	GAME_VECTOR target = { dest->x, dest->y, dest->z };
	int result = LOS(src, &target);

	GetFloor(target.x, target.y, target.z, &target.roomNumber);

	if (firing && LaserSight)
	{
		Lara.Control.Weapon.HasFired = true;
		Lara.Control.Weapon.Fired = true;

		if (Lara.Control.Weapon.GunType == WEAPON_REVOLVER)
			SoundEffect(SFX_TR4_DESSERT_EAGLE_FIRE, NULL, 0);
	}

	bool hit = false;

	MESH_INFO* mesh;
	PHD_VECTOR vector;
	int itemNumber = ObjectOnLOS2(src, dest, &vector, &mesh);
	if (itemNumber != NO_LOS_ITEM)
	{
		target.x = vector.x - (vector.x - src->x >> 5);
		target.y = vector.y - (vector.y - src->y >> 5);
		target.z = vector.z - (vector.z - src->z >> 5);

		GetFloor(target.x, target.y, target.z, &target.roomNumber);

		// TODO: for covering scientist
//		if ((itemNumber >= 0) && (BaddieSlots[itemNumber].itemNum != NO_ITEM))  // BUGFIX: ensure target has AI. No more pistol desync and camera wobble when shooting non-AI movable objects.
//			Lara.target = &g_Level.Items[itemNumber];
		// this is crashing and it's not really doing anything..

		if (firing)
		{
			if (Lara.Control.Weapon.GunType != WEAPON_CROSSBOW)
			{
				if (itemNumber < 0)
				{
					if (StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
					{
						ShatterImpactData.impactDirection = direction;
						ShatterImpactData.impactLocation = Vector3(mesh->pos.xPos, mesh->pos.yPos, mesh->pos.zPos);
						ShatterObject(NULL, mesh, 128, target.roomNumber, 0);
						SmashedMeshRoom[SmashedMeshCount] = target.roomNumber;
						SmashedMesh[SmashedMeshCount] = mesh;
						++SmashedMeshCount;
						mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
						SoundEffect(GetShatterSound(mesh->staticNumber), (PHD_3DPOS*)mesh, 0);
					}

					TriggerRicochetSpark(&target, LaraItem->Position.yRot, 3, 0);
					TriggerRicochetSpark(&target, LaraItem->Position.yRot, 3, 0);
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
							ShatterObject(&ShatterItem, 0, 128, target.roomNumber, 0);
							TriggerRicochetSpark(&target, LaraItem->Position.yRot, 3, 0);
							/*}
							else
							{
								if (item->objectNumber != ID_GUARD_LASER)
								{
									item->HitPoints -= 30;
									if (item->HitPoints < 0)
										item->HitPoints = 0;
									HitTarget(item, &target, Weapons[Lara.gunType].damage, 0);
								}
								else
								{
									angle = phd_atan(LaraItem->pos.zPos - item->pos.zPos, LaraItem->pos.xPos - item->pos.xPos) - item->pos.yRot;
									if (angle > -ANGLE(90) && angle < ANGLE(90))
									{
										item->HitPoints = 0;
										HitTarget(item, &target, Weapons[Lara.gunType].damage, 0);
									}
								}
							}*/
						}
						else
						{
							if (drawTarget && (Lara.Control.Weapon.GunType == WEAPON_REVOLVER ||
								Lara.Control.Weapon.GunType == WEAPON_HK))
							{
								if (Objects[item->ObjectNumber].intelligent)
									HitTarget(LaraItem, item, &target, Weapons[Lara.Control.Weapon.GunType].Damage, 0);
								else
								{
									// TR5
									if (Objects[item->ObjectNumber].hitEffect == HIT_RICOCHET)
										TriggerRicochetSpark(&target, LaraItem->Position.yRot, 3, 0);
								}
							}
							else
							{
								if (item->ObjectNumber >= ID_SMASH_OBJECT1 && item->ObjectNumber <= ID_SMASH_OBJECT8)
									SmashObject(itemNumber);
								else
								{
									if (Objects[item->ObjectNumber].hitEffect == HIT_BLOOD)
										DoBloodSplat(target.x, target.y, target.z, (GetRandomControl() & 3) + 3, item->Position.yRot, item->RoomNumber);
									else if (Objects[item->ObjectNumber].hitEffect == HIT_SMOKE)
										TriggerRicochetSpark(&target, LaraItem->Position.yRot, 3, -5);
									else if (Objects[item->ObjectNumber].hitEffect == HIT_RICOCHET)
										TriggerRicochetSpark(&target, LaraItem->Position.yRot, 3, 0);
									
									item->HitStatus = true;
									if (!Objects[item->ObjectNumber].undead)
										item->HitPoints -= Weapons[Lara.Control.Weapon.GunType].Damage;
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
										TestTriggers(item->Position.xPos, item->Position.yPos - 256, item->Position.zPos, item->RoomNumber, true, item->Flags & IFLAG_ACTIVATION_MASK);
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

						TriggerRicochetSpark(&target, LaraItem->Position.yRot, 3, 0);
					}
				}
			}
			else
			{
				if (LaserSight && firing)
					FireCrossBowFromLaserSight(LaraItem, src, &target);
			}
		}

		hit = true;
	}
	else
	{
		if (Lara.Control.Weapon.GunType == WEAPON_CROSSBOW)
		{
			if (firing && LaserSight)
				FireCrossBowFromLaserSight(LaraItem, src, &target);
		}
		else
		{
			target.x -= target.x - src->x >> 5;
			target.y -= target.y - src->y >> 5;
			target.z -= target.z - src->z >> 5;

			if (firing && !result)
				TriggerRicochetSpark(&target, LaraItem->Position.yRot, 8, 0);
		}
	}

	if (drawTarget && (hit || !result))
	{
		TriggerDynamicLight(target.x, target.y, target.z, 64, 255, 0, 0);
		LaserSightActive = 1;
		LaserSightX = target.x;
		LaserSightY = target.y;
		LaserSightZ = target.z;
	}

	return hit;
}

int ObjectOnLOS2(GAME_VECTOR* start, GAME_VECTOR* end, PHD_VECTOR* vec, MESH_INFO** mesh, GAME_OBJECT_ID priorityObject)
{
	ClosestItem = NO_LOS_ITEM;
	ClosestDist = SQUARE(end->x - start->x) + SQUARE(end->y - start->y) + SQUARE(end->z - start->z);

	for (int r = 0; r < NumberLosRooms; ++r)
	{
		PHD_3DPOS pos;
		auto* room = &g_Level.Rooms[LosRooms[r]];

		for (int m = 0; m < room->mesh.size(); m++)
		{
			auto* meshp = &room->mesh[m];

			if (meshp->flags & StaticMeshFlags::SM_VISIBLE)
			{
				pos.xPos = meshp->pos.xPos;
				pos.yPos = meshp->pos.yPos;
				pos.zPos = meshp->pos.zPos;
				pos.yRot = meshp->pos.yRot;

				if (DoRayBox(start, end, &StaticObjects[meshp->staticNumber].collisionBox, &pos, vec, -1 - meshp->staticNumber))
				{
					*mesh = meshp;
					end->roomNumber = LosRooms[r];
				}
			}
		}

		for (short linknum = room->itemNumber; linknum != NO_ITEM; linknum = g_Level.Items[linknum].NextItem)
		{
			auto* item = &g_Level.Items[linknum];

			if ((item->Status == ITEM_DEACTIVATED) || (item->Status == ITEM_INVISIBLE))
				continue;

			if ((priorityObject != GAME_OBJECT_ID::ID_NO_OBJECT) && (item->ObjectNumber != priorityObject))
				continue;

			if ((item->ObjectNumber != ID_LARA) && (Objects[item->ObjectNumber].collision == NULL))
				continue;

			if ((item->ObjectNumber == ID_LARA) && (priorityObject != ID_LARA))
				continue;

			auto* box = GetBoundsAccurate(item);

			pos.xPos = item->Position.xPos;
			pos.yPos = item->Position.yPos;
			pos.zPos = item->Position.zPos;
			pos.yRot = item->Position.yRot;

			if (DoRayBox(start, end, box, &pos, vec, linknum))
				end->roomNumber = LosRooms[r];
		}
	}

	vec->x = ClosestCoord.x;
	vec->y = ClosestCoord.y;
	vec->z = ClosestCoord.z;

	return ClosestItem;
}

bool DoRayBox(GAME_VECTOR* start, GAME_VECTOR* end, BOUNDING_BOX* box, PHD_3DPOS* itemOrStaticPos, PHD_VECTOR* hitPos, short closesItemNumber)
{
	// Ray
	FXMVECTOR rayStart = { (float)start->x, (float)start->y, (float)start->z };
	FXMVECTOR rayEnd = { (float)end->x, (float)end->y, (float)end->z };
	FXMVECTOR rayDir = { (float)(end->x - start->x), (float)(end->y - start->y), (float)(end->z - start->z) };
	XMVECTOR rayDirNormalized = XMVector3Normalize(rayDir);

	// Create the bounding box for raw collision detection
	auto obox = TO_DX_BBOX(*itemOrStaticPos, box);

	// Get the collision with the bounding box
	float distance;
	bool collided = obox.Intersects(rayStart, rayDirNormalized, distance);

	// If no collision happened, then don't test spheres
	if (!collided)
		return false;

	// Get the raw collision point
	Vector3 collidedPoint = rayStart + distance * rayDirNormalized;
	hitPos->x = collidedPoint.x - itemOrStaticPos->xPos;
	hitPos->y = collidedPoint.y - itemOrStaticPos->yPos;
	hitPos->z = collidedPoint.z - itemOrStaticPos->zPos;

	// Now in the case of items we need to test single spheres
	int meshIndex = 0;
	int bit = 0;
	int sp = -2;
	float minDistance = std::numeric_limits<float>::max();

	int action = TrInput & IN_ACTION;

	if (closesItemNumber < 0)
	{
		// Static meshes don't require further tests
		sp = -1;
		minDistance = distance;
	}
	else
	{
		// For items instead we need to test spheres
		ITEM_INFO* item = &g_Level.Items[closesItemNumber];
		OBJECT_INFO* obj = &Objects[item->ObjectNumber];

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

				PHD_VECTOR p[4];

				p[1].x = start->x;
				p[1].y = start->y;
				p[1].z = start->z;
				p[2].x = end->x;
				p[2].y = end->y;
				p[2].z = end->z;
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
						dx = SQUARE(sphere->x - start->x);
						dy = SQUARE(sphere->y - start->y);
						dz = SQUARE(sphere->z - start->z);

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
	ClosestCoord.x = hitPos->x + itemOrStaticPos->xPos;
	ClosestCoord.y = hitPos->y + itemOrStaticPos->yPos;
	ClosestCoord.z = hitPos->z + itemOrStaticPos->zPos;
	ClosestDist = distance;
	ClosestItem = closesItemNumber;

	// If collided object is an item, then setup the shatter item data struct
	if (sp >= 0)
	{
		ITEM_INFO* item = &g_Level.Items[closesItemNumber];

		GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD | SPHERES_SPACE_BONE_ORIGIN, Matrix::Identity);

		ShatterItem.yRot = item->Position.yRot;
		ShatterItem.meshIndex = meshIndex;
		ShatterItem.sphere.x = CreatureSpheres[sp].x;
		ShatterItem.sphere.y = CreatureSpheres[sp].y;
		ShatterItem.sphere.z = CreatureSpheres[sp].z;
		ShatterItem.bit = bit;
		ShatterItem.flags = 0;
	}

	return true;
}

bool LOS(GAME_VECTOR* start, GAME_VECTOR* end)
{
	int result1, result2;

	end->roomNumber = start->roomNumber;
	if (abs(end->z - start->z) > abs(end->x - start->x))
	{
		result1 = xLOS(start, end);
		result2 = zLOS(start, end);
	}
	else
	{
		result1 = zLOS(start, end);
		result2 = xLOS(start, end);
	}

	if (result2)
	{
		GetFloor(end->x, end->y, end->z, &end->roomNumber);
		if (ClipTarget(start, end) && result1 == 1 && result2 == 1)
			return true;
	}

	return false;
}

int xLOS(GAME_VECTOR* start, GAME_VECTOR* end)
{
	int x, y, z;
	FLOOR_INFO* floor;

	int dx = end->x - start->x;
	if (!dx)
		return 1;

	int dy = (end->y - start->y << 10) / dx;
	int dz = (end->z - start->z << 10) / dx;

	NumberLosRooms = 1;
	LosRooms[0] = start->roomNumber;

	short room = start->roomNumber;
	short room2 = start->roomNumber;

	int flag = 1;
	if (dx < 0)
	{
		x = start->x & 0xFFFFFC00;
		y = ((x - start->x) * dy >> 10) + start->y;
		z = ((x - start->x) * dz >> 10) + start->z;
		while (x > end->x)
		{
			floor = GetFloor(x, y, z, &room);
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

			x -= 1024;
			y -= dy;
			z -= dz;
		}

		if (flag != 1)
		{
			end->x = x;
			end->y = y;
			end->z = z;
		}

		end->roomNumber = flag ? room : room2;
	}
	else
	{
		x = start->x | 0x3FF;
		y = ((x - start->x) * dy >> 10) + start->y;
		z = ((x - start->x) * dz >> 10) + start->z;
		while (x < end->x)
		{
			floor = GetFloor(x, y, z, &room);
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

			x += 1024;
			y += dy;
			z += dz;
		}

		if (flag != 1)
		{
			end->x = x;
			end->y = y;
			end->z = z;
		}

		end->roomNumber = flag ? room : room2;
	}

	return flag;
}

int zLOS(GAME_VECTOR* start, GAME_VECTOR* end)
{
	int  x, y, z;
	FLOOR_INFO* floor;

	int dz = end->z - start->z;
	if (!dz)
		return 1;

	int dx = (end->x - start->x << 10) / dz;
	int dy = (end->y - start->y << 10) / dz;

	NumberLosRooms = 1;
	LosRooms[0] = start->roomNumber;

	short room = start->roomNumber;
	short room2 = start->roomNumber;

	int flag = 1;
	if (dz < 0)
	{
		z = start->z & 0xFFFFFC00;
		x = ((z - start->z) * dx >> 10) + start->x;
		y = ((z - start->z) * dy >> 10) + start->y;
		while (z > end->z)
		{
			floor = GetFloor(x, y, z, &room);
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

			z -= 1024;
			x -= dx;
			y -= dy;
		}

		if (flag != 1)
		{
			end->x = x;
			end->y = y;
			end->z = z;
		}

		end->roomNumber = flag ? room : room2;
	}
	else
	{
		z = start->z | 0x3FF;
		x = ((z - start->z) * dx >> 10) + start->x;
		y = ((z - start->z) * dy >> 10) + start->y;
		while (z < end->z)
		{
			floor = GetFloor(x, y, z, &room);
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

			z += 1024;
			x += dx;
			y += dy;
		}

		if (flag != 1)
		{
			end->x = x;
			end->y = y;
			end->z = z;
		}

		end->roomNumber = flag ? room : room2;
	}

	return flag;
}

bool LOSAndReturnTarget(GAME_VECTOR* start, GAME_VECTOR* target, int push)
{
	int floorHeight, ceilingHeight;
	FLOOR_INFO* floor;

	int x = start->x;
	int y = start->y;
	int z = start->z;
	short roomNum = start->roomNumber;
	short roomNum2 = roomNum;
	int dx = target->x - x >> 3;
	int dy = target->y - y >> 3;
	int dz = target->z - z >> 3;
	bool flag = false;
	bool result = false;

	int i;
	for (i = 0; i < 8; ++i)
	{
		roomNum2 = roomNum;
		floor = GetFloor(x, y, z, &roomNum);

		if (g_Level.Rooms[roomNum2].flags & ENV_FLAG_SWAMP)
		{
			flag = true;
			break;
		}

		floorHeight = GetFloorHeight(floor, x, y, z);
		ceilingHeight = GetCeiling(floor, x, y, z);
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

	GetFloor(x, y, z, &roomNum2);
	target->x = x;
	target->y = y;
	target->z = z;
	target->roomNumber = roomNum2;

	return !flag;
}
