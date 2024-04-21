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
#include "Renderer/Renderer.h"
#include "Scripting/Include/Objects/ScriptInterfaceObjectsHandler.h"
#include "Scripting/Include/ScriptInterfaceGame.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/trutils.h"

using namespace TEN::Math;
using namespace TEN::Utils;
using TEN::Renderer::g_Renderer;

// Globals
int NumberLosRooms;
int LosRooms[20];
int ClosestItem;
int ClosestDist;
Vector3i ClosestCoord;

// TODO: Figure out what the flags mean in xLOS and zLOS.
enum class RoomLosClipType
{
	InnerWall,
	OuterWall,
	Surface
};

static int xRoomLos(const GameVector& origin, GameVector& target, std::vector<const FloorInfo*>& sectorPtrs, std::optional<std::set<int>*> roomNumbers)
{
	int flag = 1;

	int dx = target.x - origin.x;
	if (dx == 0)
		return flag;

	int dy = BLOCK(target.y - origin.y) / dx;
	int dz = BLOCK(target.z - origin.z) / dx;

	NumberLosRooms = 1;
	LosRooms[0] = origin.RoomNumber;

	// Collect room number.
	if (roomNumbers.has_value())
		roomNumbers.value()->insert(origin.RoomNumber);

	short roomNumber0 = origin.RoomNumber;
	short roomNumber1 = origin.RoomNumber;

	bool isNegative = (dx < 0);
	int sign = (isNegative ? -1 : 1);

	int x = isNegative ? (origin.x & (UINT_MAX - WALL_MASK)) : (origin.x | WALL_MASK);
	int y = (((x - origin.x) * dy) / BLOCK(1)) + origin.y;
	int z = (((x - origin.x) * dz) / BLOCK(1)) + origin.z;

	while (isNegative ? (x > target.x) : (x < target.x))
	{
		// Collect sector pointer.
		auto* sectorPtr = GetFloor(x, y, z, &roomNumber0);
		sectorPtrs.push_back(sectorPtr);

		if (roomNumber0 != roomNumber1)
		{
			roomNumber1 = roomNumber0;
			LosRooms[NumberLosRooms] = roomNumber0;
			++NumberLosRooms;

			// Collect room number.
			if (roomNumbers.has_value())
				roomNumbers.value()->insert(roomNumber0);
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

			// Collect room number.
			if (roomNumbers.has_value())
				roomNumbers.value()->insert(roomNumber0);
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

	target.RoomNumber = (flag != 0) ? roomNumber0 : roomNumber1;
	return flag;
}

static int zRoomLos(const GameVector& origin, GameVector& target, std::vector<const FloorInfo*>& sectorPtrs, std::optional<std::set<int>*> roomNumbers)
{
	int flag = 1;

	int dz = target.z - origin.z;
	if (dz == 0)
		return flag;

	int dx = BLOCK(target.x - origin.x) / dz;
	int dy = BLOCK(target.y - origin.y) / dz;

	NumberLosRooms = 1;
	LosRooms[0] = origin.RoomNumber;

	if (roomNumbers.has_value())
		roomNumbers.value()->insert(origin.RoomNumber);

	short roomNumber0 = origin.RoomNumber;
	short roomNumber1 = origin.RoomNumber;

	bool isNegative = (dz < 0);
	int sign = (isNegative ? -1 : 1);

	int z = isNegative ? (origin.z & (UINT_MAX - WALL_MASK)) : (origin.z | WALL_MASK);
	int x = (((z - origin.z) * dx) / BLOCK(1)) + origin.x;
	int y = (((z - origin.z) * dy) / BLOCK(1)) + origin.y;

	while (isNegative ? (z > target.z) : (z < target.z))
	{
		// Collect sector pointer.
		auto* sectorPtr = GetFloor(x, y, z, &roomNumber0);
		sectorPtrs.push_back(sectorPtr);

		if (roomNumber0 != roomNumber1)
		{
			roomNumber1 = roomNumber0;
			LosRooms[NumberLosRooms] = roomNumber0;
			++NumberLosRooms;

			// Collect room number.
			if (roomNumbers.has_value())
				roomNumbers.value()->insert(roomNumber0);
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

			// Collect room number.
			if (roomNumbers.has_value())
				roomNumbers.value()->insert(roomNumber0);
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

	target.RoomNumber = (flag != 0) ? roomNumber0 : roomNumber1;
	return flag;
}

static std::vector<TriangleMesh> GenerateSectorTriangleMeshes(const Vector3& pos, const FloorInfo& sector, bool isFloor)
{
	auto base = Vector3(FloorToStep(pos.x, BLOCK(1)), 0.0f, FloorToStep(pos.z, BLOCK(1)));
	auto corner0 = base;
	auto corner1 = base + Vector3(0.0f, 0.0f, BLOCK(1));
	auto corner2 = base + Vector3(BLOCK(1), 0.0f, BLOCK(1));
	auto corner3 = base + Vector3(BLOCK(1), 0.0f, 0.0f);

	auto tris = std::vector<TriangleMesh>{};
	if (sector.IsSurfaceSplit(isFloor))
	{
		const auto& surface = isFloor ? sector.FloorSurface : sector.CeilingSurface;

		if (surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0)
		{
			// Calculate triangle 0.
			auto tri0 = TriangleMesh(
				Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 0), corner0.z),
				Vector3(corner1.x, sector.GetSurfaceHeight(corner1.x, corner1.z - 1, isFloor, 0), corner1.z),
				Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 0), corner2.z));

			// Calculate triangle 1.
			auto tri1 = TriangleMesh(
				Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 1), corner0.z),
				Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 1), corner2.z),
				Vector3(corner3.x, sector.GetSurfaceHeight(corner3.x - 1, corner3.z, isFloor, 1), corner3.z));

			// Collect surface triangles.
			tris.push_back(tri0);
			tris.push_back(tri1);

			// Calculate and collect diagonal wall triangles.
			if (tri0.Vertices[0] != tri1.Vertices[0] && tri0.Vertices[2] != tri1.Vertices[1])
			{
				auto tri2 = TriangleMesh(tri0.Vertices[0], tri1.Vertices[0], tri0.Vertices[2]);
				auto tri3 = TriangleMesh(tri1.Vertices[0], tri0.Vertices[2], tri1.Vertices[1]);

				tris.push_back(tri2);
				tris.push_back(tri3);
			}
			else if (tri0.Vertices[0] != tri1.Vertices[0] && tri0.Vertices[2] == tri1.Vertices[1])
			{
				auto tri2 = TriangleMesh(tri0.Vertices[0], tri1.Vertices[0], tri0.Vertices[2]);
				tris.push_back(tri2);
			}
			else if (tri0.Vertices[2] == tri1.Vertices[1] && tri0.Vertices[2] != tri1.Vertices[1])
			{
				auto tri2 = TriangleMesh(tri1.Vertices[0], tri0.Vertices[2], tri1.Vertices[1]);
				tris.push_back(tri2);
			}
		}
		else if (surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_1)
		{
			// Calculate triangle 0.
			auto tri1 = TriangleMesh(
				Vector3(corner1.x, sector.GetSurfaceHeight(corner1.x, corner1.z - 1, isFloor, 0), corner1.z),
				Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 0), corner2.z),
				Vector3(corner3.x, sector.GetSurfaceHeight(corner3.x - 1, corner3.z, isFloor, 0), corner3.z));

			// Calculate triangle 1.
			auto tri0 = TriangleMesh(
				Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 1), corner0.z),
				Vector3(corner1.x, sector.GetSurfaceHeight(corner1.x, corner1.z - 1, isFloor, 1), corner1.z),
				Vector3(corner3.x, sector.GetSurfaceHeight(corner3.x - 1, corner3.z, isFloor, 1), corner3.z));

			// Collect surface triangles.
			tris.push_back(tri0);
			tris.push_back(tri1);

			// Calculate and collect diagonal wall triangles.
			if (tri0.Vertices[1] != tri1.Vertices[0] && tri0.Vertices[2] != tri1.Vertices[2])
			{
				auto tri2 = TriangleMesh(tri0.Vertices[1], tri1.Vertices[0], tri0.Vertices[2]);
				auto tri3 = TriangleMesh(tri1.Vertices[0], tri0.Vertices[2], tri1.Vertices[2]);

				tris.push_back(tri2);
				tris.push_back(tri3);
			}
			else if (tri0.Vertices[1] != tri1.Vertices[0] && tri0.Vertices[2] == tri1.Vertices[2])
			{
				auto tri2 = TriangleMesh(tri0.Vertices[1], tri1.Vertices[0], tri0.Vertices[2]);
				tris.push_back(tri2);
			}
			else if (tri0.Vertices[2] == tri1.Vertices[2] && tri0.Vertices[2] != tri1.Vertices[2])
			{
				auto tri2 = TriangleMesh(tri1.Vertices[0], tri0.Vertices[2], tri1.Vertices[2]);
				tris.push_back(tri2);
			}
		}
	}
	else
	{
		// Calculate triangle 0.
		auto tri0 = TriangleMesh(
			Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 0), corner0.z),
			Vector3(corner1.x, sector.GetSurfaceHeight(corner1.x, corner1.z - 1, isFloor, 0), corner1.z),
			Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 0), corner2.z));

		// Calculate triangle 1.
		auto tri1 = TriangleMesh(
			Vector3(corner0.x, sector.GetSurfaceHeight(corner0.x, corner0.z, isFloor, 1), corner0.z),
			Vector3(corner2.x, sector.GetSurfaceHeight(corner2.x - 1, corner2.z - 1, isFloor, 1), corner2.z),
			Vector3(corner3.x, sector.GetSurfaceHeight(corner3.x - 1, corner3.z, isFloor, 1), corner3.z));

		// Collect surface triangles.
		tris.push_back(tri0);
		tris.push_back(tri1);
	}

	return tris;
}

static std::vector<TriangleMesh> GenerateTiltBridgeTriangleMeshes(const BoundingOrientedBox& box, const Vector3& offset)
{
	// Get box corners.
	auto corners = std::array<Vector3, 8>{};
	box.GetCorners(corners.data());

	// Offset key corners.
	corners[1] += offset;
	corners[3] -= offset;
	corners[5] += offset;
	corners[7] -= offset;

	// Calculate collision mesh.
	auto tris = std::vector<TriangleMesh>{};
	tris.push_back(TriangleMesh(corners[0], corners[1], corners[4]));
	tris.push_back(TriangleMesh(corners[1], corners[4], corners[5]));
	tris.push_back(TriangleMesh(corners[2], corners[3], corners[6]));
	tris.push_back(TriangleMesh(corners[3], corners[6], corners[7]));
	tris.push_back(TriangleMesh(corners[0], corners[1], corners[2]));
	tris.push_back(TriangleMesh(corners[0], corners[2], corners[3]));
	tris.push_back(TriangleMesh(corners[0], corners[3], corners[4]));
	tris.push_back(TriangleMesh(corners[3], corners[4], corners[7]));
	tris.push_back(TriangleMesh(corners[1], corners[2], corners[5]));
	tris.push_back(TriangleMesh(corners[2], corners[5], corners[6]));
	tris.push_back(TriangleMesh(corners[4], corners[5], corners[6]));
	tris.push_back(TriangleMesh(corners[4], corners[6], corners[7]));

	return tris;
}

static bool ClipRoomLosIntersect(const GameVector& origin, GameVector& target, std::vector<const FloorInfo*>& sectorPtrs)
{
	static const auto TILT_BRIDGE_MOV_ASSET_IDS = std::vector
	{
		ID_BRIDGE_TILT1,
		ID_BRIDGE_TILT2,
		ID_BRIDGE_TILT3,
		ID_BRIDGE_TILT4
	};

	auto dir = target.ToVector3() - origin.ToVector3();
	dir.Normalize();
	auto ray = Ray(origin.ToVector3(), dir);

	bool isClipped = false;
	float closestDist = INFINITY;

	// 1) Clip bridge.
	for (const auto* sectorPtr : sectorPtrs)
	{
		// Run through all bridges in sector.
		for (int itemNumber : sectorPtr->BridgeItemNumbers)
		{
			const auto& bridgeMov = g_Level.Items[itemNumber];

			if (bridgeMov.Status == ItemStatus::ITEM_DEACTIVATED)
				continue;

			auto box = GameBoundingBox(&bridgeMov).ToBoundingOrientedBox(bridgeMov.Pose);
			if (!Contains(TILT_BRIDGE_MOV_ASSET_IDS, bridgeMov.ObjectNumber))
			{
				// Collide bridge box.
				float dist = 0.0f;
				if (box.Intersects(ray.position, ray.direction, dist) && dist < closestDist)
				{
					isClipped = true;
					closestDist = dist;
					target = GameVector(Geometry::TranslatePoint(ray.position, ray.direction, dist), sectorPtr->RoomNumber);
				}
			}
			else
			{
				// Determine relative tilt offset.
				auto offset = Vector3::Zero;
				switch (bridgeMov.ObjectNumber)
				{
				default:
				case ID_BRIDGE_TILT1:
					offset = Vector3(0.0f, CLICK(1), 0.0f);
					break;

				case ID_BRIDGE_TILT2:
					offset = Vector3(0.0f, CLICK(2), 0.0f);
					break;

				case ID_BRIDGE_TILT3:
					offset = Vector3(0.0f, CLICK(3), 0.0f);
					break;

				case ID_BRIDGE_TILT4:
					offset = Vector3(0.0f, CLICK(4), 0.0f);
					break;
				}

				// Calculate absolute offset.
				auto rotMatrix = bridgeMov.Pose.Orientation.ToRotationMatrix();
				offset = Vector3::Transform(offset, rotMatrix);

				// Collide bridge mesh.
				auto meshes = GenerateTiltBridgeTriangleMeshes(box, offset);
				for (const auto& mesh : meshes)
				{
					float dist = 0.0f;
					if (mesh.Intersects(ray, dist) && dist < closestDist)
					{
						isClipped = true;
						closestDist = dist;
						target = GameVector(Geometry::TranslatePoint(ray.position, ray.direction, dist), sectorPtr->RoomNumber);
					}
				}
			}
		}

		// Found bridge; exit loop early.
		if (closestDist != INFINITY)
			break;
	}

	short roomNumber = target.RoomNumber;
	auto* sectorPtr = GetFloor(target.x, target.y, target.z, &roomNumber);

	// 2) Clip floor.
	if (target.y > GetFloorHeight(sectorPtr, target.x, target.y, target.z))
	{
		isClipped = true;

		// Collide floor collision mesh.
		auto tris = GenerateSectorTriangleMeshes(target.ToVector3(), *sectorPtr, true);
		for (const auto& tri : tris)
		{
			float dist = 0.0f;
			if (tri.Intersects(ray, dist) && dist < closestDist)
			{
				closestDist = dist;
				target = GameVector(Geometry::TranslatePoint(ray.position, ray.direction, dist), roomNumber);
			}
		}
	}

	// 3) Clip ceiling.
	if (target.y < GetCeiling(sectorPtr, target.x, target.y, target.z))
	{
		isClipped = true;

		// Collide ceiling collision mesh.
		auto tris = GenerateSectorTriangleMeshes(target.ToVector3(), *sectorPtr, false);
		for (const auto& tri : tris)
		{
			float dist = 0.0f;
			if (tri.Intersects(ray, dist) && dist < closestDist)
			{
				closestDist = dist;
				target = GameVector(Geometry::TranslatePoint(ray.position, ray.direction, dist), roomNumber);
			}
		}
	}

	return !isClipped;
}

// NOTE: Accurate walls, floors/ceilings, and flat bridges. Ignores tilted bridges and objects.
bool LOS(const GameVector* origin, GameVector* target, std::optional<std::set<int>*> roomNumbers)
{
	int losAxis0 = 0;
	int losAxis1 = 0;

	auto sectorPtrs = std::vector<const FloorInfo*>{};

	target->RoomNumber = origin->RoomNumber;
	if (abs(target->z - origin->z) > abs(target->x - origin->x))
	{
		losAxis0 = xRoomLos(*origin, *target, sectorPtrs, roomNumbers);
		losAxis1 = zRoomLos(*origin, *target, sectorPtrs, roomNumbers);
	}
	else
	{
		losAxis0 = zRoomLos(*origin, *target, sectorPtrs, roomNumbers);
		losAxis1 = xRoomLos(*origin, *target, sectorPtrs, roomNumbers);
	}

	if (losAxis1 != 0)
	{
		GetFloor(target->x, target->y, target->z, &target->RoomNumber);

		if (ClipRoomLosIntersect(*origin, *target, sectorPtrs) && losAxis0 == 1 && losAxis1 == 1)
			return true;
	}

	return false;
}

// Deprecated. Legacy version was coarse, new version simply wraps more accurate LOS().
bool LOSAndReturnTarget(GameVector* origin, GameVector* target, int push)
{
	if (!LOS(origin, target))
		return true;

	auto dir = target->ToVector3() - origin->ToVector3();
	dir.Normalize();

	*target = GameVector(Geometry::TranslatePoint(target->ToVector3(), -dir, push), target->RoomNumber);
	GetFloor(target->x, target->y, target->z, &target->RoomNumber);

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

	if (itemNumber != NO_VALUE)
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
						const auto& weapon = Weapons[(int)Lara.Control.Weapon.GunType];
						mesh->HitPoints -= weapon.Damage;
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
									int bestJointIndex = NO_VALUE;

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
										object->HitRoutine(*item, *LaraItem, target2, weapon.Damage, false, NO_VALUE);
									}
									else
									{
										DefaultItemHit(*item, *LaraItem, target2, weapon.Damage, false, NO_VALUE);
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

int ObjectOnLOS2(GameVector* origin, GameVector* target, Vector3i* vec, MESH_INFO** staticPtrPtr, GAME_OBJECT_ID priorityObjectID)
{
	ClosestItem = NO_VALUE;
	ClosestDist = SQUARE(target->x - origin->x) + SQUARE(target->y - origin->y) + SQUARE(target->z - origin->z);

	for (int r = 0; r < NumberLosRooms; ++r)
	{
		auto& room = g_Level.Rooms[LosRooms[r]];

		auto pose = Pose::Zero;

		if (staticPtrPtr != nullptr)
		{
			for (int m = 0; m < room.mesh.size(); m++)
			{
				auto& meshp = room.mesh[m];

				if (meshp.flags & StaticMeshFlags::SM_VISIBLE)
				{
					auto bounds = GetBoundsAccurate(meshp, false);
					pose = Pose(meshp.pos.Position, EulerAngles(0, meshp.pos.Orientation.y, 0));

					if (DoRayBox(*origin, *target, bounds, pose, *vec, -1 - meshp.staticNumber))
					{
						*staticPtrPtr = &meshp;
						target->RoomNumber = LosRooms[r];
					}
				}
			}
		}

		for (int linkNumber = room.itemNumber; linkNumber != NO_VALUE; linkNumber = g_Level.Items[linkNumber].NextItem)
		{
			const auto& item = g_Level.Items[linkNumber];

			if (item.Status == ITEM_DEACTIVATED || item.Status == ITEM_INVISIBLE)
				continue;

			if (priorityObjectID != GAME_OBJECT_ID::ID_NO_OBJECT && item.ObjectNumber != priorityObjectID)
				continue;

			if (item.ObjectNumber != ID_LARA && Objects[item.ObjectNumber].collision == nullptr)
				continue;

			if (item.ObjectNumber == ID_LARA && priorityObjectID != ID_LARA)
				continue;

			auto bounds = GameBoundingBox(&item);
			pose = Pose(item.Pose.Position, EulerAngles(0, item.Pose.Orientation.y, 0));

			if (DoRayBox(*origin, *target, bounds, pose, *vec, linkNumber))
				target->RoomNumber = LosRooms[r];
		}
	}

	vec->x = ClosestCoord.x;
	vec->y = ClosestCoord.y;
	vec->z = ClosestCoord.z;

	return ClosestItem;
}
