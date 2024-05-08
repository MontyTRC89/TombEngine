#include "framework.h"
#include "Game/Lara/lara.h"

#include "Game/Lara/lara_basic.h"
#include "Game/Lara/lara_cheat.h"
#include "Game/Lara/lara_climb.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_crawl.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_hang.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_initialise.h"
#include "Game/Lara/lara_jump.h"
#include "Game/Lara/lara_monkey.h"
#include "Game/Lara/lara_objects.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_overhang.h"
#include "Game/Lara/lara_slide.h"
#include "Game/Lara/lara_surface.h"
#include "Game/Lara/lara_swim.h"
#include "Game/Lara/lara_tests.h"
#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/floordata.h"
#include "Game/collision/Point.h"
#include "Game/control/flipeffect.h"
#include "Game/control/volume.h"
#include "Game/effects/Hair.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Gui.h"
#include "Game/items.h"
#include "Game/Lara/lara_cheat.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_initialise.h"
#include "Game/Lara/PlayerStateMachine.h"
#include "Game/misc.h"
#include "Game/savegame.h"
#include "Renderer/Renderer.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Scripting/Include/ScriptInterfaceLevel.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/winmain.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Collision::Point;
using namespace TEN::Control::Volumes;
using namespace TEN::Effects::Hair;
using namespace TEN::Effects::Items;
using namespace TEN::Entities::Player;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Gui;

using TEN::Renderer::g_Renderer;

LaraInfo Lara = {};
ItemInfo* LaraItem;
CollisionInfo LaraCollision = {};

//debug
#include <Game/control/los.h>
#include "Specific/Input/Input.h"
#include <OISKeyboard.h>
#include <Game/collision/Los.h>
using namespace TEN::Collision::Los;

static void HandleLosDebug(const ItemInfo& item)
{
	static auto rot = EulerAngles::Identity;
	if (KeyMap[OIS::KC_T])
	{
		rot.x += ANGLE(2);
	}
	else if (KeyMap[OIS::KC_G])
	{
		rot.x -= ANGLE(2);
	}

	auto dir = (item.Pose.Orientation + rot).ToDirection();

	float dist = BLOCK(1.25f);

	short roomNumber = item.RoomNumber;
	GetFloor(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, &roomNumber);

	auto origin = (item.Pose.Position + Vector3i(0, -BLOCK(0.9f), 0)).ToVector3();
	auto target = Geometry::TranslatePoint(origin, dir, dist);
	auto los = GetLos(origin, roomNumber, dir, dist, true, true, true);

	float closestDist = los.Room.Distance;
	target = los.Room.Position.first;

	for (const auto& movLos : los.Moveables)
	{
		if (movLos.Moveable->ObjectNumber == ID_LARA)
			continue;

		if (movLos.Distance < closestDist)
		{
			closestDist = movLos.Distance;
			target = movLos.Position.first;
			break;
		}
	}

	for (const auto& staticLos : los.Statics)
	{
		if (staticLos.Distance < closestDist)
		{
			closestDist = staticLos.Distance;
			target = staticLos.Position.first;
			break;
		}
	}

	g_Renderer.AddDebugLine(origin, target, Vector4::One);
	g_Renderer.AddDebugTarget(target, Quaternion::Identity, 100, Color(1, 1, 1));
}

static int GetSurfaceTriangleVertexHeight(const FloorInfo& sector, int relX, int relZ, int triID, bool isFloor)
{
	constexpr auto AXIS_OFFSET = -BLOCK(0.5f);

	const auto& tri = isFloor ? sector.FloorSurface.Triangles[triID] : sector.CeilingSurface.Triangles[triID];

	relX += AXIS_OFFSET;
	relZ += AXIS_OFFSET;

	auto normal = tri.Plane.Normal();
	float relPlaneHeight = -((normal.x * relX) + (normal.z * relZ)) / normal.y;
	return (tri.Plane.D() + relPlaneHeight);
}

static Vector3 GetTriangleNormal(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2)
{
	auto edge0 = vertex1 - vertex0;
	auto edge1 = vertex2 - vertex0;
	auto normal = edge0.Cross(edge1);
	normal.Normalize();
	return normal;
}

static CollisionMesh GenerateSectorCollisionMesh(const FloorInfo& sector,
												 const FloorInfo* prevSectorX, const FloorInfo* prevSectorZ, bool isXEnd, bool isZEnd)
{
	constexpr auto REL_CORNER_0 = Vector2i(0, 0);
	constexpr auto REL_CORNER_1 = Vector2i(0, BLOCK(1));
	constexpr auto REL_CORNER_2 = Vector2i(BLOCK(1), BLOCK(1));
	constexpr auto REL_CORNER_3 = Vector2i(BLOCK(1), 0);

	auto corner0 = sector.Position + REL_CORNER_0;
	auto corner1 = sector.Position + REL_CORNER_1;
	auto corner2 = sector.Position + REL_CORNER_2;
	auto corner3 = sector.Position + REL_CORNER_3;

	auto tris = std::vector<CollisionTriangle>{};

	// Collect triangles.
	bool isFloor = true;
	for (int i = 0; i < 2; i++)
	{
		const auto& surface = isFloor ? sector.FloorSurface : sector.CeilingSurface;

		bool isSurfTri0Portal = (surface.Triangles[0].PortalRoomNumber != NO_VALUE);
		bool isSurfTri1Portal = (surface.Triangles[1].PortalRoomNumber != NO_VALUE);

		bool isSplit = sector.IsSurfaceSplit(isFloor);
		bool isSplitAngle0 = (surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);

		// Define surface triangle 0.
		auto vertex00 = Vector3::Zero;
		auto vertex01 = Vector3::Zero;
		auto vertex02 = Vector3::Zero;
		auto surfTri0 = CollisionTriangle();
		if (!isSplit || isSplitAngle0)
		{
			vertex00 = Vector3(corner0.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_0.x, REL_CORNER_0.y, 0, isFloor), corner0.y);
			vertex01 = Vector3(corner1.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner1.y);
			vertex02 = Vector3(corner2.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_2.x, REL_CORNER_2.y, 0, isFloor), corner2.y);
		}
		else
		{
			vertex00 = Vector3(corner1.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner1.y);
			vertex01 = Vector3(corner2.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_2.x, REL_CORNER_2.y, 0, isFloor), corner2.y);
			vertex02 = Vector3(corner3.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, 0, isFloor), corner3.y);
		}
		surfTri0 = CollisionTriangle(vertex00, vertex01, vertex02, GetTriangleNormal(vertex00, vertex01, vertex02) * (isFloor ? -1 : 1));
		
		// Define surface triangle 1.
		auto vertex10 = Vector3::Zero;
		auto vertex11 = Vector3::Zero;
		auto vertex12 = Vector3::Zero;
		auto surfTri1 = CollisionTriangle();
		if (!isSplit || isSplitAngle0)
		{
			vertex10 = Vector3(corner0.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_0.x, REL_CORNER_0.y, 1, isFloor), corner0.y);
			vertex11 = Vector3(corner2.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_2.x, REL_CORNER_2.y, 1, isFloor), corner2.y);
			vertex12 = Vector3(corner3.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, 1, isFloor), corner3.y);
		}
		else
		{
			vertex10 = Vector3(corner0.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_0.x, REL_CORNER_0.y, 1, isFloor), corner0.y);
			vertex11 = Vector3(corner1.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_1.x, REL_CORNER_1.y, 1, isFloor), corner1.y);
			vertex12 = Vector3(corner3.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, 1, isFloor), corner3.y);
		}
		surfTri1 = CollisionTriangle(vertex10, vertex11, vertex12, GetTriangleNormal(vertex10, vertex11, vertex12) * (isFloor ? -1 : 1));

		// todo: wall check.
		if (sector.IsSurfaceSplit(isFloor))
		{
			if (!isSurfTri0Portal || !isSurfTri1Portal)
			{
				if (surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0)
				{
					// Surface triangles
					if (!isSurfTri0Portal)
						tris.push_back(surfTri0);
					if (!isSurfTri1Portal)
						tris.push_back(surfTri1);

					// TODO: Criss-cross case?
					// Diagonal wall triangles.
					if (surfTri0.GetVertices()[0] != surfTri1.GetVertices()[0] && surfTri0.GetVertices()[2] != surfTri1.GetVertices()[1])
					{
						auto tri2 = CollisionTriangle(surfTri0.GetVertices()[0], surfTri1.GetVertices()[0], surfTri0.GetVertices()[2]);
						tris.push_back(tri2);

						auto tri3 = CollisionTriangle(surfTri1.GetVertices()[0], surfTri0.GetVertices()[2], surfTri1.GetVertices()[1]);
						tris.push_back(tri3);
					}
					else if (surfTri0.GetVertices()[0] != surfTri1.GetVertices()[0] && surfTri0.GetVertices()[2] == surfTri1.GetVertices()[1])
					{
						auto tri2 = CollisionTriangle(surfTri0.GetVertices()[0], surfTri1.GetVertices()[0], surfTri0.GetVertices()[2]);
						tris.push_back(tri2);
					}
					else if (surfTri0.GetVertices()[2] == surfTri1.GetVertices()[1] && surfTri0.GetVertices()[2] != surfTri1.GetVertices()[1])
					{
						auto tri2 = CollisionTriangle(surfTri1.GetVertices()[0], surfTri0.GetVertices()[2], surfTri1.GetVertices()[1]);
						tris.push_back(tri2);
					}
				}
				else if (surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_1)
				{
					// Surface triangles.
					if (!isSurfTri0Portal)
						tris.push_back(surfTri0);
					if (!isSurfTri1Portal)
						tris.push_back(surfTri1);

					// Diagonal wall triangles.
					if (surfTri1.GetVertices()[1] != surfTri0.GetVertices()[0] && surfTri1.GetVertices()[2] != surfTri0.GetVertices()[2])
					{
						auto tri2 = CollisionTriangle(surfTri1.GetVertices()[1], surfTri0.GetVertices()[0], surfTri1.GetVertices()[2]);
						tris.push_back(tri2);

						auto tri3 = CollisionTriangle(surfTri0.GetVertices()[0], surfTri1.GetVertices()[2], surfTri0.GetVertices()[2]);
						tris.push_back(tri3);
					}
					else if (surfTri1.GetVertices()[1] != surfTri0.GetVertices()[0] && surfTri1.GetVertices()[2] == surfTri0.GetVertices()[2])
					{
						auto tri2 = CollisionTriangle(surfTri1.GetVertices()[1], surfTri0.GetVertices()[0], surfTri1.GetVertices()[2]);
						tris.push_back(tri2);
					}
					else if (surfTri1.GetVertices()[2] == surfTri0.GetVertices()[2] && surfTri1.GetVertices()[2] != surfTri0.GetVertices()[2])
					{
						auto tri2 = CollisionTriangle(surfTri0.GetVertices()[0], surfTri1.GetVertices()[2], surfTri0.GetVertices()[2]);
						tris.push_back(tri2);
					}
				}
			}
		}
		else
		{
			// Surface triangles.
			if (!isSurfTri0Portal)
				tris.push_back(surfTri0);
			if (!isSurfTri1Portal)
				tris.push_back(surfTri1);

			// Cardinal wall triangles on X axis.
			/*if (prevSectorX != nullptr)
			{
				// TODO: Wall portals.
				const auto& prevSurfaceX = isFloor ? prevSectorX->FloorSurface : prevSectorX->CeilingSurface;

				// TODO: Full wall needs to reference floor and ceiling.
				if (surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0)
				{
					if (sector.IsWall(1) && isFloor)
					{
						const auto& ceilSurface = sector.CeilingSurface;
					}
				}
				else
				{
					if (sector.IsWall(0))
					{

					}
				}

				bool prevSectorTriID = (prevSurfaceX.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_1) ? 1 : 0;

				auto vertex0 = Vector3(corner0.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_0.x, REL_CORNER_0.y, 0, isFloor), corner0.y);
				auto vertex1 = Vector3(corner1.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner1.y);

				if (vertex0.x < 19000 && vertex0.x > 18200 && vertex0.z < 16600 && vertex0.z > 14900)
				{
					g_Renderer.PrintDebugMessage("%d", GetSurfaceTriangleVertexHeight(sector, REL_CORNER_0.x, REL_CORNER_0.y, 0, isFloor));
					g_Renderer.PrintDebugMessage("%d", GetSurfaceTriangleVertexHeight(*prevSectorX, REL_CORNER_2.x, REL_CORNER_2.y, prevSectorTriID, isFloor));
				}

				auto vertex2 = Vector3(corner0.x, GetSurfaceTriangleVertexHeight(*prevSectorX, REL_CORNER_2.x, REL_CORNER_2.y, prevSectorTriID, isFloor), corner0.y);
				auto vertex3 = Vector3(corner1.x, GetSurfaceTriangleVertexHeight(*prevSectorX, REL_CORNER_3.x, REL_CORNER_3.y, prevSectorTriID, isFloor), corner1.y);

				auto tPos0 = g_Renderer.Get2DPosition(vertex0);
				auto tPos1 = g_Renderer.Get2DPosition(vertex1);
				auto tPos2 = g_Renderer.Get2DPosition(vertex2);
				auto tPos3 = g_Renderer.Get2DPosition(vertex3);
				if (tPos0.has_value())
					g_Renderer.AddDebugString("0", *tPos0, Color(1, 1, 1), 1, 0, RendererDebugPage::None);
				if (tPos1.has_value())
					g_Renderer.AddDebugString("1", *tPos1, Color(1, 1, 1), 1, 0, RendererDebugPage::None);
				if (tPos2.has_value())
					g_Renderer.AddDebugString("2", *tPos2, Color(1, 1, 1), 1, 0, RendererDebugPage::None);
				if (tPos3.has_value())
					g_Renderer.AddDebugString("3", *tPos3, Color(1, 1, 1), 1, 0, RendererDebugPage::None);

				// Wall triangles.
				if (vertex0.y != vertex2.y && vertex1.y != vertex3.y) // TODO: Always true?
				{
					auto surfTri0 = CollisionTriangle(vertex0, vertex1, vertex2);
					auto surfTri1 = CollisionTriangle(vertex1, vertex2, vertex3);

					tris.push_back(surfTri0);
					tris.push_back(surfTri1);
				}
				else if (vertex0.y == vertex2.y && vertex1.y != vertex3.y)
				{
					auto surfTri0 = CollisionTriangle(vertex0, vertex1, vertex3);
					tris.push_back(surfTri0);
				}
				else if (vertex0.y != vertex2.y && vertex1.y == vertex3.y)
				{
					auto surfTri0 = CollisionTriangle(vertex1, vertex0, vertex1);
					tris.push_back(surfTri0);
				}

				// TODO: Walls at ends where no further sectors exist.
				if (isXEnd)
				{

				}
			}

			// TODO
			// Cardinal wall triangles on Z axis.
			if (prevSectorZ != nullptr)
			{

			}*/
		}

		isFloor = false;
	}

	return CollisionMesh(tris);
}

void HandleRoomCollisionMesh()
{
	auto& room = g_Level.Rooms[LaraItem->RoomNumber];

	for (const auto& sector : room.floor)
	{
		for (const auto& tri : sector.Mesh.GetTriangles())
		{
			g_Renderer.AddDebugTriangle(tri.GetVertices()[0], tri.GetVertices()[1], tri.GetVertices()[2], Color(1, 1, 0, 0.2f));

			auto origin = (tri.GetVertices()[0] + tri.GetVertices()[1] + tri.GetVertices()[2]) / 3;
			auto target = Geometry::TranslatePoint(origin, tri.GetNormal(), BLOCK(0.25f));
			g_Renderer.AddDebugLine(origin, target, Color(1, 1, 0));
		}
	}

	if (!IsClicked(In::Walk))
		return;

	// Run through sectors (ignoring border).
	for (int x = 1; x < (room.xSize - 1); x++)
	{
		for (int z = 1; z < (room.zSize - 1); z++)
		{
			auto& sector = room.floor[(x * room.zSize) + z];

			// Get previous X sector.
			const auto& tempPrevSectorX = room.floor[((x - 1) * room.zSize) + z];
			const FloorInfo* prevSectorX = nullptr;
			if (x != 1)
			{
				prevSectorX = &tempPrevSectorX;
			}
			else
			{
				if (tempPrevSectorX.SidePortalRoomNumber != NO_VALUE)
				{
					const auto& prevRoomX = g_Level.Rooms[tempPrevSectorX.SidePortalRoomNumber];
					auto prevRoomGridCoordX = GetRoomGridCoord(prevRoomX.index, prevRoomX.x, prevRoomX.z + BLOCK(z)); // TODO

					prevSectorX = &prevRoomX.floor[(prevRoomGridCoordX.x * prevRoomX.zSize) + prevRoomGridCoordX.y];
				}
			}

			// Get previous Z sector.
			const auto& tempPrevSectorZ = room.floor[(x * room.zSize) + (z - 1)];
			const FloorInfo* prevSectorZ = nullptr;
			if (z != 1)
			{
				prevSectorZ = &tempPrevSectorZ;
			}
			else
			{
				if (tempPrevSectorZ.SidePortalRoomNumber != NO_VALUE)
				{
					const auto& prevRoomZ = g_Level.Rooms[tempPrevSectorZ.SidePortalRoomNumber];
					auto prevRoomGridCoordZ = GetRoomGridCoord(prevRoomZ.index, prevRoomZ.x + BLOCK(x), prevRoomZ.z); // TODO

					prevSectorX = &prevRoomZ.floor[(prevRoomGridCoordZ.x * prevRoomZ.zSize) + prevRoomGridCoordZ.y];
				}
			}

			// Test if at room edge on X axis.
			bool isXEnd = (x == (room.xSize - 2));
			if (isXEnd)
			{
				const auto& nextSectorX = room.floor[((x + 1) * room.zSize) + z];
				isXEnd = (nextSectorX.SidePortalRoomNumber == NO_VALUE);
			}

			// Test if at room edge on Z axis.
			bool isZEnd = (z == (room.zSize - 2));
			if (isZEnd)
			{
				const auto& nextSectorZ = room.floor[(x * room.zSize) + (z + 1)];
				isZEnd = (nextSectorZ.SidePortalRoomNumber == NO_VALUE);
			}

			sector.Mesh = GenerateSectorCollisionMesh(sector, prevSectorX, prevSectorZ, isXEnd, isZEnd);
		}
	}
}

void LaraControl(ItemInfo* item, CollisionInfo* coll)
{
	auto& player = GetLaraInfo(*item);

	//--------

	HandleLosDebug(*item);
	HandleRoomCollisionMesh();

	const auto& room = g_Level.Rooms[item->RoomNumber];

	short deltaAngle = Geometry::GetShortestAngle(GetPlayerHeadingAngleY(*item), Camera.actualAngle);
	//g_Renderer.PrintDebugMessage("%d", abs(deltaAngle));

	//--------

	// Update reference move axis.
	if (GetMoveAxis() != Vector2::Zero)
		player.Control.RefMoveAxis = GetMoveAxis();

	// Alert nearby creatures.
	if (player.Control.Weapon.HasFired)
	{
		AlertNearbyGuards(item);
		player.Control.Weapon.HasFired = false;
	}

	// Handle object interation adjustment parameters.
	if (player.Control.IsMoving)
	{
		if (player.Control.Count.PositionAdjust > PLAYER_POSITION_ADJUST_MAX_TIME)
		{
			player.Control.IsMoving = false;
			player.Control.HandStatus = HandStatus::Free;
		}

		++player.Control.Count.PositionAdjust;
	}
	else
	{
		player.Control.Count.PositionAdjust = 0;
	}

	if (!player.Control.IsLocked)
		player.LocationPad = NO_VALUE;

	// FAILSAFE: Force hand status reset.
	if (item->Animation.AnimNumber == LA_STAND_IDLE &&
		item->Animation.ActiveState == LS_IDLE &&
		item->Animation.TargetState == LS_IDLE &&
		!item->Animation.IsAirborne &&
		player.Control.HandStatus == HandStatus::Busy)
	{
		player.Control.HandStatus = HandStatus::Free;
	}

	HandlePlayerQuickActions(*item);
	RumbleLaraHealthCondition(item);

	auto water = GetPlayerWaterData(*item);
	player.Context.WaterSurfaceDist = -water.HeightFromWater;

	if (player.Context.Vehicle == NO_VALUE)
		SpawnPlayerWaterSurfaceEffects(*item, water.WaterHeight, water.WaterDepth);

	bool isWaterOnHeadspace = false;

	// TODO: Move unrelated handling elsewhere.
	// Handle environment state transition.
	if (player.Context.Vehicle == NO_VALUE && player.ExtraAnim == NO_VALUE)
	{
		switch (player.Control.WaterStatus)
		{
		case WaterStatus::Dry:
			for (int i = 0; i < NUM_LARA_MESHES; i++)
				player.Effect.BubbleNodes[i] = 0.0f;

			if (water.HeightFromWater == NO_HEIGHT || water.HeightFromWater < WADE_WATER_DEPTH)
				break;

			Camera.targetElevation = ANGLE(-22.0f);

			// Water is at swim depth; dispatch dive.
			if (water.WaterDepth >= SWIM_WATER_DEPTH && !water.IsSwamp)
			{
				if (water.IsWater)
				{
					item->Pose.Position.y += CLICK(0.5f) - 28; // TODO: Demagic.
					item->Animation.IsAirborne = false;
					player.Control.WaterStatus = WaterStatus::Underwater;
					player.Status.Air = LARA_AIR_MAX;

					for (int i = 0; i < NUM_LARA_MESHES; i++)
						player.Effect.BubbleNodes[i] = PLAYER_BUBBLE_NODE_MAX;

					UpdateLaraRoom(item, 0);
					StopSoundEffect(SFX_TR4_LARA_FALL);

					if (item->Animation.ActiveState == LS_SWAN_DIVE)
					{
						SetAnimation(item, LA_SWANDIVE_DIVE);
						item->Animation.Velocity.y /= 2;
						item->Pose.Orientation.x = ANGLE(-45.0f);
						player.Control.HandStatus = HandStatus::Free;
					}
					else if (item->Animation.ActiveState == LS_FREEFALL_DIVE)
					{
						SetAnimation(item, LA_SWANDIVE_FREEFALL_DIVE);
						item->Animation.Velocity.y /= 2;
						item->Pose.Orientation.x = ANGLE(-85.0f);
						player.Control.HandStatus = HandStatus::Free;
					}
					else
					{
						SetAnimation(item, LA_FREEFALL_DIVE);
						item->Animation.Velocity.y = item->Animation.Velocity.y * (3 / 8.0f);
						item->Pose.Orientation.x = ANGLE(-45.0f);
					}

					ResetPlayerFlex(item);
				}
			}
			// Water is at wade depth; update water status and do special handling.
			else if (water.HeightFromWater >= WADE_WATER_DEPTH)
			{
				player.Control.WaterStatus = WaterStatus::Wade;

				// Make splash ONLY within this particular threshold before swim depth while airborne (SpawnPlayerSplash() above interferes otherwise).
				if (water.WaterDepth > (SWIM_WATER_DEPTH - CLICK(1)) &&
					item->Animation.IsAirborne && !water.IsSwamp)
				{
					item->Animation.TargetState = LS_IDLE;
				}
				// Player is grounded; don't splash again.
				else if (!item->Animation.IsAirborne)
				{
					item->Animation.TargetState = LS_IDLE;
				}
				else if (water.IsSwamp)
				{
					if (item->Animation.ActiveState == LS_SWAN_DIVE ||
						item->Animation.ActiveState == LS_FREEFALL_DIVE)
					{
						item->Pose.Position.y = water.WaterHeight + (BLOCK(1) - 24); // TODO: Demagic.
					}

					SetAnimation(item, LA_WADE);
				}
			}

			break;

		case WaterStatus::Underwater:
			// Disable potential player resurfacing if health is <= 0.
			// Originals worked without this condition, but TEN does not. -- Lwmte, 11.08.22
			if (item->HitPoints <= 0)
				break;

			// Determine if player's head is above water surface. Needed to prevent
			// pre-TR5 bug where player would keep submerged until root mesh was above water level.
			isWaterOnHeadspace = TestEnvironment(
				ENV_FLAG_WATER, item->Pose.Position.x, item->Pose.Position.y - CLICK(1), item->Pose.Position.z,
				GetPointCollision(*item, 0, 0, -CLICK(1)).GetRoomNumber());

			if (water.WaterDepth == NO_HEIGHT || abs(water.HeightFromWater) >= CLICK(1) || isWaterOnHeadspace ||
				item->Animation.AnimNumber == LA_UNDERWATER_RESURFACE || item->Animation.AnimNumber == LA_ONWATER_DIVE)
			{
				if (!water.IsWater)
				{
					if (water.WaterDepth == NO_HEIGHT || abs(water.HeightFromWater) >= CLICK(1))
					{
						SetAnimation(item, LA_FALL_START);
						ResetPlayerLean(item);
						ResetPlayerFlex(item);
						item->Animation.IsAirborne = true;
						item->Animation.Velocity.z = item->Animation.Velocity.y;
						item->Animation.Velocity.y = 0.0f;
						player.Control.WaterStatus = WaterStatus::Dry;
					}
					else
					{
						SetAnimation(item, LA_UNDERWATER_RESURFACE);
						ResetPlayerLean(item);
						ResetPlayerFlex(item);
						item->Animation.Velocity.y = 0.0f;
						item->Pose.Position.y = water.WaterHeight;
						player.Control.WaterStatus = WaterStatus::TreadWater;

						UpdateLaraRoom(item, -(STEPUP_HEIGHT - 3));
					}
				}
			}
			else
			{
				SetAnimation(item, LA_UNDERWATER_RESURFACE);
				ResetPlayerLean(item);
				ResetPlayerFlex(item);
				item->Animation.Velocity.y = 0.0f;
				item->Pose.Position.y = water.WaterHeight + 1;
				player.Control.WaterStatus = WaterStatus::TreadWater;

				UpdateLaraRoom(item, 0);
			}

			break;

		case WaterStatus::TreadWater:
			if (!water.IsWater)
			{
				if (water.HeightFromWater <= WADE_WATER_DEPTH)
				{
					SetAnimation(item, LA_FALL_START);
					item->Animation.IsAirborne = true;
					item->Animation.Velocity.z = item->Animation.Velocity.y;
					player.Control.WaterStatus = WaterStatus::Dry;
				}
				else
				{
					SetAnimation(item, LA_STAND_IDLE);
					player.Control.WaterStatus = WaterStatus::Wade;
				}

				ResetPlayerLean(item);
				ResetPlayerFlex(item);
				item->Animation.Velocity.y = 0.0f;
			}

			break;

		case WaterStatus::Wade:
			Camera.targetElevation = ANGLE(-22.0f);

			if (water.HeightFromWater >= WADE_WATER_DEPTH)
			{
				if (water.HeightFromWater > SWIM_WATER_DEPTH && !water.IsSwamp)
				{
					SetAnimation(item, LA_ONWATER_IDLE);
					ResetPlayerLean(item);
					ResetPlayerFlex(item);
					item->Animation.IsAirborne = false;
					item->Animation.Velocity.y = 0.0f;
					item->Pose.Position.y += 1 - water.HeightFromWater;
					player.Control.WaterStatus = WaterStatus::TreadWater;

					UpdateLaraRoom(item, 0);
				}
			}
			else
			{
				player.Control.WaterStatus = WaterStatus::Dry;

				if (item->Animation.ActiveState == LS_WADE_FORWARD)
					item->Animation.TargetState = LS_RUN_FORWARD;
			}

			break;
		}
	}

	HandlePlayerStatusEffects(*item, player.Control.WaterStatus, water);

	auto prevPos = item->Pose.Position;

	// Handle environment state.
	switch (player.Control.WaterStatus)
	{
	case WaterStatus::Dry:
	case WaterStatus::Wade:
		LaraAboveWater(item, coll);
		break;

	case WaterStatus::Underwater:
		LaraUnderwater(item, coll);
		break;

	case WaterStatus::TreadWater:
		LaraWaterSurface(item, coll);
		break;

	case WaterStatus::FlyCheat:
		LaraCheat(item, coll);
		break;
	}

	SaveGame::Statistics.Game.Distance += (int)round(Vector3i::Distance(prevPos, item->Pose.Position));

	if (DebugMode)
	{
		DrawNearbyPathfinding(GetPointCollision(*item).GetBottomSector().Box);
		DrawNearbySectorFlags(*item);
	}
}

void LaraAboveWater(ItemInfo* item, CollisionInfo* coll)
{
	auto& player = GetLaraInfo(*item);

	// Reset collision setup.
	coll->Setup.Mode = CollisionProbeMode::Quadrants;
	coll->Setup.Radius = LARA_RADIUS;
	coll->Setup.Height = LARA_HEIGHT;
	coll->Setup.UpperCeilingBound = NO_UPPER_BOUND;
	coll->Setup.BlockFloorSlopeUp = false;
	coll->Setup.BlockFloorSlopeDown = false;
	coll->Setup.BlockCeilingSlope = false;
	coll->Setup.BlockDeathFloorDown = false;
	coll->Setup.BlockMonkeySwingEdge = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = true;
	coll->Setup.PrevPosition = item->Pose.Position;
	coll->Setup.PrevAnimObjectID = item->Animation.AnimObjectID;
	coll->Setup.PrevAnimNumber = item->Animation.AnimNumber;
	coll->Setup.PrevFrameNumber = item->Animation.FrameNumber;
	coll->Setup.PrevState = item->Animation.ActiveState;

	UpdateLaraRoom(item, -LARA_HEIGHT / 2);

	// Handle look-around.
	if (((IsHeld(In::Look) && CanPlayerLookAround(*item)) ||
			(player.Control.Look.IsUsingBinoculars || player.Control.Look.IsUsingLasersight)) &&
		player.ExtraAnim == NO_VALUE)
	{
		HandlePlayerLookAround(*item);
	}
	else
	{
		ResetPlayerLookAround(*item);
	}
	player.Control.Look.Mode = LookMode::None;

	// Process vehicles.
	if (HandleLaraVehicle(item, coll))
		return;

	HandlePlayerBehaviorState(*item, *coll, PlayerBehaviorStateRoutineType::Control);
	HandleLaraMovementParameters(item, coll);
	AnimateItem(item);

	if (player.ExtraAnim == NO_VALUE)
	{
		// Check for collision with items.
		DoObjectCollision(item, coll);

		// Handle player behavior state collision.
		if (player.Context.Vehicle == NO_VALUE)
			HandlePlayerBehaviorState(*item, *coll, PlayerBehaviorStateRoutineType::Collision);
	}

	HandleWeapon(*item);
	LaraBreath(item);

	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(item->Index, &coll->Setup);
}

void LaraWaterSurface(ItemInfo* item, CollisionInfo* coll)
{
	auto& player = GetLaraInfo(*item);

	// Reset collision setup.
	coll->Setup.Mode = CollisionProbeMode::FreeForward;
	coll->Setup.Radius = LARA_RADIUS;
	coll->Setup.Height = LARA_HEIGHT_SURFACE;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -CLICK(0.5f);
	coll->Setup.LowerCeilingBound = LARA_RADIUS;
	coll->Setup.UpperCeilingBound = NO_UPPER_BOUND;
	coll->Setup.BlockFloorSlopeUp = false;
	coll->Setup.BlockFloorSlopeDown = false;
	coll->Setup.BlockCeilingSlope = false;
	coll->Setup.BlockDeathFloorDown = false;
	coll->Setup.BlockMonkeySwingEdge = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = false;
	coll->Setup.PrevPosition = item->Pose.Position;

	player.Control.IsLow = false;
	Camera.targetElevation = ANGLE(-22.0f);

	// Handle look-around.
	if (IsHeld(In::Look) && CanPlayerLookAround(*item))
	{
		HandlePlayerLookAround(*item);
	}
	else
	{
		ResetPlayerLookAround(*item);
	}

	player.Control.Count.Pose = 0;

	HandlePlayerBehaviorState(*item, *coll, PlayerBehaviorStateRoutineType::Control);

	const auto& level = *g_GameFlow->GetLevel(CurrentLevel);
	bool hasDivesuit = (level.GetLaraType() == LaraType::Divesuit);

	// TODO: Subsuit gradually slows down at rate of 0.5 degrees. @Sezz 2022.06.23
	// Apply and reset turn rate.
	item->Pose.Orientation.y += player.Control.TurnRate.y;
	if (!(IsHeld(In::Left) || IsHeld(In::Right)))
		player.Control.TurnRate.y = 0;

	if (hasDivesuit)
		UpdateLaraSubsuitAngles(item);

	// Reset lean.
	if (!player.Control.IsMoving && !(IsHeld(In::Left) || IsHeld(In::Right)))
		ResetPlayerLean(item, 1 / 8.0f);

	if (player.Context.WaterCurrentActive && player.Control.WaterStatus != WaterStatus::FlyCheat)
		LaraWaterCurrent(item, coll);

	AnimateItem(item);
	TranslateItem(item, player.Control.HeadingOrient.y, item->Animation.Velocity.y);

	DoObjectCollision(item, coll);

	if (player.Context.Vehicle == NO_VALUE)
		HandlePlayerBehaviorState(*item, *coll, PlayerBehaviorStateRoutineType::Collision);

	UpdateLaraRoom(item, LARA_RADIUS);
	HandleWeapon(*item);

	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(item->Index);
}

void LaraUnderwater(ItemInfo* item, CollisionInfo* coll)
{
	auto& player = GetLaraInfo(*item);

	// Reset collision setup.
	coll->Setup.Mode = CollisionProbeMode::Quadrants;
	coll->Setup.Radius = LARA_RADIUS_UNDERWATER;
	coll->Setup.Height = LARA_HEIGHT;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -(LARA_RADIUS_UNDERWATER + (LARA_RADIUS_UNDERWATER / 3));
	coll->Setup.LowerCeilingBound = LARA_RADIUS_UNDERWATER + (LARA_RADIUS_UNDERWATER / 3);
	coll->Setup.UpperCeilingBound = NO_UPPER_BOUND;
	coll->Setup.BlockFloorSlopeUp = false;
	coll->Setup.BlockFloorSlopeDown = false;
	coll->Setup.BlockCeilingSlope = false;
	coll->Setup.BlockDeathFloorDown = false;
	coll->Setup.BlockMonkeySwingEdge = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = false;
	coll->Setup.PrevPosition = item->Pose.Position;

	player.Control.IsLow = false;

	// Handle look-around.
	if (IsHeld(In::Look) && CanPlayerLookAround(*item))
	{
		HandlePlayerLookAround(*item);
	}
	else
	{
		ResetPlayerLookAround(*item);
	}

	player.Control.Count.Pose = 0;

	HandlePlayerBehaviorState(*item, *coll, PlayerBehaviorStateRoutineType::Control);

	const auto& level = *g_GameFlow->GetLevel(CurrentLevel);
	bool hasDivesuit = (level.GetLaraType() == LaraType::Divesuit);

	// TODO: Subsuit gradually slowed down at rate of 0.5 degrees. @Sezz 2022.06.23
	// Apply and reset turn rate.
	item->Pose.Orientation.y += player.Control.TurnRate.y;
	if (!(IsHeld(In::Left) || IsHeld(In::Right)))
		player.Control.TurnRate.y = 0;

	if (hasDivesuit)
		UpdateLaraSubsuitAngles(item);

	if (!player.Control.IsMoving && !(IsHeld(In::Left) || IsHeld(In::Right)))
		ResetPlayerLean(item, 1 / 8.0f, true, false);

	if (item->Pose.Orientation.x < -ANGLE(85.0f))
	{
		item->Pose.Orientation.x = -ANGLE(85.0f);
	}
	else if (item->Pose.Orientation.x > ANGLE(85.0f))
	{
		item->Pose.Orientation.x = ANGLE(85.0f);
	}

	if (hasDivesuit)
	{
		if (item->Pose.Orientation.z > ANGLE(44.0f))
		{
			item->Pose.Orientation.z = ANGLE(44.0f);
		}
		else if (item->Pose.Orientation.z < -ANGLE(44.0f))
		{
			item->Pose.Orientation.z = -ANGLE(44.0f);
		}
	}
	else
	{
		if (item->Pose.Orientation.z > ANGLE(22.0f))
		{
			item->Pose.Orientation.z = ANGLE(22.0f);
		}
		else if (item->Pose.Orientation.z < -ANGLE(22.0f))
		{
			item->Pose.Orientation.z = -ANGLE(22.0f);
		}
	}

	if (player.Context.WaterCurrentActive && player.Control.WaterStatus != WaterStatus::FlyCheat)
		LaraWaterCurrent(item, coll);

	AnimateItem(item);
	TranslateItem(item, item->Pose.Orientation, item->Animation.Velocity.y);

	DoObjectCollision(item, coll);

	if (player.Context.Vehicle == NO_VALUE)
		HandlePlayerBehaviorState(*item, *coll, PlayerBehaviorStateRoutineType::Collision);

	UpdateLaraRoom(item, 0);
	HandleWeapon(*item);

	ProcessSectorFlags(item);
	TestTriggers(item, false);
	TestVolumes(item->Index);
}

void LaraCheat(ItemInfo* item, CollisionInfo* coll)
{
	auto& player = GetLaraInfo(*item);

	item->HitPoints = LARA_HEALTH_MAX;
	player.Status.Air = LARA_AIR_MAX;
	player.Status.Exposure = LARA_EXPOSURE_MAX;
	player.Status.Poison = 0;
	player.Status.Stamina = LARA_STAMINA_MAX;
	
	LaraUnderwater(item, coll);

	if (IsHeld(In::Walk) && !IsHeld(In::Look))
	{
		if (TestEnvironment(ENV_FLAG_WATER, item) ||
			(player.Context.WaterSurfaceDist > 0 && player.Context.WaterSurfaceDist != NO_HEIGHT))
		{
			SetAnimation(item, LA_UNDERWATER_IDLE);
			player.Control.WaterStatus = WaterStatus::Underwater;
		}
		else
		{
			SetAnimation(item, LA_STAND_IDLE);
			item->Pose.Orientation.x = 0;
			item->Pose.Orientation.z = 0;
			player.Control.WaterStatus = WaterStatus::Dry;
		}

		ResetPlayerFlex(item);
		InitializeLaraMeshes(item);
		item->HitPoints = LARA_HEALTH_MAX;
		player.Control.HandStatus = HandStatus::Free;
	}
}

void UpdateLara(ItemInfo* item, bool isTitle)
{
	if (isTitle && !g_GameFlow->IsLaraInTitleEnabled())
		return;

	// HACK: backup controls until proper control lock is implemented -- Lwmte, 07.12.22
	auto actionMap = ActionMap;

	if (isTitle)
		ClearAllActions();

	// Control player.
	InItemControlLoop = true;
	LaraControl(item, &LaraCollision);
	HandlePlayerFlyCheat(*item);
	InItemControlLoop = false;
	KillMoveItems();

	if (isTitle)
		ActionMap = actionMap;

	// Update player animations.
	g_Renderer.UpdateLaraAnimations(true);

	// Update player effects.
	HairEffect.Update(*item, g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young);
	HandlePlayerWetnessDrips(*item);
	HandlePlayerDiveBubbles(*item);
	ProcessEffects(item);
}

// Offset values may be used to account for the quirk of room traversal only being able to occur at portals.
bool UpdateLaraRoom(ItemInfo* item, int height, int xOffset, int zOffset)
{
	auto point = Geometry::TranslatePoint(item->Pose.Position, item->Pose.Orientation.y, zOffset, height, xOffset);

	// Hacky L-shaped Location traversal.
	item->Location = GetRoomVector(item->Location, point);
	item->Location = GetRoomVector(item->Location, Vector3i(item->Pose.Position.x, point.y, item->Pose.Position.z));
	item->Floor = GetSurfaceHeight(item->Location, item->Pose.Position.x, item->Pose.Position.z, true).value_or(NO_HEIGHT);

	if (item->RoomNumber != item->Location.RoomNumber)
	{
		ItemNewRoom(item->Index, item->Location.RoomNumber);
		return true;
	}

	return false;
}
