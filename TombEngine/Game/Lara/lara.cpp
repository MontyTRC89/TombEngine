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
	constexpr auto HEIGHT_STEP = BLOCK(1 / 32.0f);

	const auto& tri = isFloor ? sector.FloorSurface.Triangles[triID] : sector.CeilingSurface.Triangles[triID];

	relX += AXIS_OFFSET;
	relZ += AXIS_OFFSET;

	auto normal = tri.Plane.Normal();
	float relPlaneHeight = -((normal.x * relX) + (normal.z * relZ)) / normal.y;
	return (int)RoundToStep(tri.Plane.D() + relPlaneHeight, HEIGHT_STEP);
}

static Vector3 GetRawSurfaceTriangleNormal(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2)
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
	constexpr auto NORTH_WALL_NORMAL	  = Vector3(0.0f, 0.0f, 1.0f);
	constexpr auto SOUTH_WALL_NORMAL	  = Vector3(0.0f, 0.0f, -1.0f);
	constexpr auto EAST_WALL_NORMAL		  = Vector3(1.0f, 0.0f, 0.0f);
	constexpr auto WEST_WALL_NORMAL		  = Vector3(-1.0f, 0.0f, 0.0f);
	constexpr auto NORTH_EAST_WALL_NORMAL = Vector3(SQRT_2 / 2, 0.0f, SQRT_2 / 2);
	constexpr auto NORTH_WEST_WALL_NORMAL = Vector3(-SQRT_2 / 2, 0.0f, SQRT_2 / 2);
	constexpr auto SOUTH_EAST_WALL_NORMAL = Vector3(SQRT_2 / 2, 0.0f, -SQRT_2 / 2);
	constexpr auto SOUTH_WEST_WALL_NORMAL = Vector3(-SQRT_2 / 2, 0.0f, -SQRT_2 / 2);

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

		bool isSurfSplit = sector.IsSurfaceSplit(isFloor);
		bool isSurfSplitAngle0 = (surface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);

		// 1) Generate surface triangle 0.
		auto surfTri0 = CollisionTriangle();
		if (!isSurfSplit || isSurfSplitAngle0)
		{
			auto vertex0 = Vector3(corner0.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_0.x, REL_CORNER_0.y, 0, isFloor), corner0.y);
			auto vertex1 = Vector3(corner1.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner1.y);
			auto vertex2 = Vector3(corner2.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_2.x, REL_CORNER_2.y, 0, isFloor), corner2.y);
			surfTri0 = CollisionTriangle(vertex0, vertex1, vertex2, GetRawSurfaceTriangleNormal(vertex0, vertex1, vertex2) * (isFloor ? -1 : 1));
		}
		else
		{
			auto vertex0 = Vector3(corner1.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_1.x, REL_CORNER_1.y, 0, isFloor), corner1.y);
			auto vertex1 = Vector3(corner2.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_2.x, REL_CORNER_2.y, 0, isFloor), corner2.y);
			auto vertex2 = Vector3(corner3.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, 0, isFloor), corner3.y);
			surfTri0 = CollisionTriangle(vertex0, vertex1, vertex2, GetRawSurfaceTriangleNormal(vertex0, vertex1, vertex2) * (isFloor ? -1 : 1));
		}
		
		// 2) Generate surface triangle 1.
		auto surfTri1 = CollisionTriangle();
		if (!isSurfSplit || isSurfSplitAngle0)
		{
			auto vertex0 = Vector3(corner0.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_0.x, REL_CORNER_0.y, 1, isFloor), corner0.y);
			auto vertex1 = Vector3(corner2.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_2.x, REL_CORNER_2.y, 1, isFloor), corner2.y);
			auto vertex2 = Vector3(corner3.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, 1, isFloor), corner3.y);
			surfTri1 = CollisionTriangle(vertex0, vertex1, vertex2, GetRawSurfaceTriangleNormal(vertex0, vertex1, vertex2) * (isFloor ? -1 : 1));
		}
		else
		{
			auto vertex0 = Vector3(corner0.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_0.x, REL_CORNER_0.y, 1, isFloor), corner0.y);
			auto vertex1 = Vector3(corner1.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_1.x, REL_CORNER_1.y, 1, isFloor), corner1.y);
			auto vertex2 = Vector3(corner3.x, GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, 1, isFloor), corner3.y);
			surfTri1 = CollisionTriangle(vertex0, vertex1, vertex2, GetRawSurfaceTriangleNormal(vertex0, vertex1, vertex2) * (isFloor ? -1 : 1));
		}

		bool isSurf0Wall = sector.IsWall(0);
		bool isSurf1Wall = sector.IsWall(1);
		bool isSurfTri0Portal = (surface.Triangles[0].PortalRoomNumber != NO_VALUE);
		bool isSurfTri1Portal = (surface.Triangles[1].PortalRoomNumber != NO_VALUE);

		// 3) Collect surface triangles.
		if (!isSurf0Wall && !isSurfTri0Portal)
			tris.push_back(surfTri0);
		if (!isSurf1Wall && !isSurfTri1Portal)
			tris.push_back(surfTri1);

		// 4) Generate and collect diagonal wall triangles.
		if (sector.IsSurfaceSplit(isFloor) && !(isSurf0Wall && isSurf1Wall)) // Can set wall.
		{
			// TODO: when surface meets wall at one corner.
			// Full wall.
			if ((isSurf0Wall || isSurf1Wall) && isFloor)
			{
				if (isSurfSplitAngle0)
				{
					// TODO: Correct ceil triangle.
					int floorHeight0 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_0.x, REL_CORNER_0.y, isSurf0Wall ? 1 : 0, true);
					int floorHeight1 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_2.x, REL_CORNER_2.y, isSurf0Wall ? 1 : 0, true);
					int ceilHeight0 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_0.x, REL_CORNER_0.y, isSurf0Wall ? 1 : 0, false);
					int ceilHeight1 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_2.x, REL_CORNER_2.y, isSurf0Wall ? 1 : 0, false);

					auto vertex0 = Vector3(corner0.x, floorHeight0, corner0.y);
					auto vertex1 = Vector3(corner2.x, floorHeight1, corner2.y);
					auto vertex2 = Vector3(corner0.x, ceilHeight0, corner0.y);
					auto vertex3 = Vector3(corner2.x, ceilHeight1, corner2.y);

					auto wallTri0 = CollisionTriangle(vertex0, vertex1, vertex2, isSurf0Wall ? SOUTH_EAST_WALL_NORMAL : NORTH_WEST_WALL_NORMAL);
					auto wallTri1 = CollisionTriangle(vertex1, vertex2, vertex3, isSurf0Wall ? SOUTH_EAST_WALL_NORMAL : NORTH_WEST_WALL_NORMAL);
					tris.push_back(wallTri0);
					tris.push_back(wallTri1);
				}
				else
				{
					// TODO: Correct ceil triangle.
					int floorHeight0 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_1.x, REL_CORNER_1.y, isSurf0Wall ? 1 : 0, true);
					int floorHeight1 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, isSurf0Wall ? 1 : 0, true);
					int ceilHeight0 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_1.x, REL_CORNER_1.y, isSurf0Wall ? 1 : 0, false);
					int ceilHeight1 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, isSurf0Wall ? 1 : 0, false);

					auto vertex0 = Vector3(corner1.x, floorHeight0, corner1.y);
					auto vertex1 = Vector3(corner3.x, floorHeight1, corner3.y);
					auto vertex2 = Vector3(corner1.x, ceilHeight0, corner1.y);
					auto vertex3 = Vector3(corner3.x, ceilHeight1, corner3.y);

					auto wallTri0 = CollisionTriangle(vertex0, vertex1, vertex2, isSurf0Wall ? SOUTH_WEST_WALL_NORMAL : NORTH_EAST_WALL_NORMAL);
					auto wallTri1 = CollisionTriangle(vertex1, vertex2, vertex3, isSurf0Wall ? SOUTH_WEST_WALL_NORMAL : NORTH_EAST_WALL_NORMAL);
					tris.push_back(wallTri0);
					tris.push_back(wallTri1);
				}
			}
			// Step wall.
			else if (!isSurfTri0Portal || !isSurfTri1Portal)
			{
				if (isSurfSplitAngle0)
				{
					if (surfTri0.GetVertices()[0] != surfTri1.GetVertices()[0])
					{
						const auto& normal0 = ((surfTri0.GetVertices()[0].y > surfTri1.GetVertices()[0].y) ? NORTH_WEST_WALL_NORMAL : SOUTH_EAST_WALL_NORMAL) * (isFloor ? 1 : -1);
						auto wallTri0 = CollisionTriangle(surfTri0.GetVertices()[0], surfTri1.GetVertices()[0], surfTri0.GetVertices()[2], normal0);
						tris.push_back(wallTri0);
					}
					if (surfTri0.GetVertices()[2] != surfTri1.GetVertices()[1])
					{
						const auto& normal1 = ((surfTri0.GetVertices()[2].y > surfTri1.GetVertices()[1].y) ? NORTH_WEST_WALL_NORMAL : SOUTH_EAST_WALL_NORMAL) * (isFloor ? 1 : -1);
						auto wallTri1 = CollisionTriangle(surfTri1.GetVertices()[0], surfTri0.GetVertices()[2], surfTri1.GetVertices()[1], normal1);
						tris.push_back(wallTri1);
					}
				}
				else
				{
					if (surfTri0.GetVertices()[0] != surfTri1.GetVertices()[1])
					{
						const auto& normal0 = ((surfTri0.GetVertices()[0].y > surfTri1.GetVertices()[1].y) ? NORTH_EAST_WALL_NORMAL : SOUTH_WEST_WALL_NORMAL) * (isFloor ? 1 : -1);
						auto wallTri0 = CollisionTriangle(surfTri1.GetVertices()[1], surfTri0.GetVertices()[0], surfTri1.GetVertices()[2], normal0);
						tris.push_back(wallTri0);
					}
					if (surfTri0.GetVertices()[2] != surfTri1.GetVertices()[2])
					{
						const auto& normal1 = ((surfTri0.GetVertices()[2].y > surfTri1.GetVertices()[2].y) ? NORTH_EAST_WALL_NORMAL : SOUTH_WEST_WALL_NORMAL) * (isFloor ? 1 : -1);
						auto wallTri1 = CollisionTriangle(surfTri0.GetVertices()[0], surfTri1.GetVertices()[2], surfTri0.GetVertices()[2], normal1);
						tris.push_back(wallTri1);
					}
				}
			}
		}

		// 5) Collect cardinal wall triangles on X axis.
		if (prevSectorX != nullptr)
		{
			const auto& prevSurface = isFloor ? prevSectorX->FloorSurface : prevSectorX->CeilingSurface;

			bool isPrevSurfSplit = prevSectorX->IsSurfaceSplit(isFloor);
			bool isPrevSurfSplitAngle0 = (prevSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);

			bool isPrevSurf0Wall = prevSectorX->IsWall(0);
			bool isPrevSurf1Wall = prevSectorX->IsWall(1);
			bool isPrevSurfTri0Portal = (prevSurface.Triangles[0].PortalRoomNumber != NO_VALUE);
			bool isPrevSurfTri1Portal = (prevSurface.Triangles[1].PortalRoomNumber != NO_VALUE);

			// TODO: use isSplit bools instead.
			bool useTri0 = (!isSurfSplit || isSurfSplitAngle0);
			bool usePrevTri1 = (!isPrevSurfSplit || isPrevSurfSplitAngle0);

			bool isSurfWall = (useTri0 ? isSurf0Wall : isSurf1Wall);
			bool isPrevSurfWall = (usePrevTri1 ? isPrevSurf1Wall : isPrevSurf0Wall);

			// TODO: When not split.
			// Wall behind.
			if (!isSurfWall || !isPrevSurfWall) // Can set wall.
			{
				// TODO: when surface meets wall at one corner.
				// Full wall referencing current sector.
				if ((!isSurfWall && isPrevSurfWall) && isFloor)
				{
					// TODO: Correct ceil triangle.
					int floorHeight0 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, useTri0 ? 0 : 1, true);
					int floorHeight1 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_2.x, REL_CORNER_2.y, useTri0 ? 0 : 1, true);
					int ceilHeight0 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, useTri0 ? 0 : 1, false);
					int ceilHeight1 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_2.x, REL_CORNER_2.y, useTri0 ? 0 : 1, false);

					auto vertex0 = Vector3(corner0.x, floorHeight0, corner0.y);
					auto vertex1 = Vector3(corner1.x, floorHeight1, corner1.y);
					auto vertex2 = Vector3(corner0.x, ceilHeight0, corner0.y);
					auto vertex3 = Vector3(corner1.x, ceilHeight1, corner1.y);

					auto wallTri0 = CollisionTriangle(vertex0, vertex1, vertex2, EAST_WALL_NORMAL);
					auto wallTri1 = CollisionTriangle(vertex1, vertex2, vertex3, EAST_WALL_NORMAL);
					tris.push_back(wallTri0);
					tris.push_back(wallTri1);
				}
				// Full wall referencing previous sector.
				else if ((isSurfWall && !isPrevSurfWall) && isFloor)
				{
					// TODO: Correct ceil triangle.
					int floorHeight0 = GetSurfaceTriangleVertexHeight(*prevSectorX, REL_CORNER_3.x, REL_CORNER_3.y, usePrevTri1 ? 1 : 0, true);
					int floorHeight1 = GetSurfaceTriangleVertexHeight(*prevSectorX, REL_CORNER_2.x, REL_CORNER_2.y, usePrevTri1 ? 1 : 0, true);
					int ceilHeight0 = GetSurfaceTriangleVertexHeight(*prevSectorX, REL_CORNER_3.x, REL_CORNER_3.y, usePrevTri1 ? 1 : 0, false);
					int ceilHeight1 = GetSurfaceTriangleVertexHeight(*prevSectorX, REL_CORNER_2.x, REL_CORNER_2.y, usePrevTri1 ? 1 : 0, false);

					auto vertex0 = Vector3(corner0.x, floorHeight0, corner0.y);
					auto vertex1 = Vector3(corner1.x, floorHeight1, corner1.y);
					auto vertex2 = Vector3(corner0.x, ceilHeight0, corner0.y);
					auto vertex3 = Vector3(corner1.x, ceilHeight1, corner1.y);

					auto wallTri0 = CollisionTriangle(vertex0, vertex1, vertex2, WEST_WALL_NORMAL);
					auto wallTri1 = CollisionTriangle(vertex1, vertex2, vertex3, WEST_WALL_NORMAL);
					tris.push_back(wallTri0);
					tris.push_back(wallTri1);
				}
				// Step wall.
				else if (!isSurfWall && !isPrevSurfWall)
				{
					int height0 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_0.x, REL_CORNER_0.y, useTri0 ? 0 : 1, isFloor);
					int height1 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_1.x, REL_CORNER_1.y, useTri0 ? 0 : 1, isFloor);
					int prevHeight0 = GetSurfaceTriangleVertexHeight(*prevSectorX, REL_CORNER_3.x, REL_CORNER_3.y, usePrevTri1 ? 1 : 0, isFloor);
					int prevHeight1 = GetSurfaceTriangleVertexHeight(*prevSectorX, REL_CORNER_2.x, REL_CORNER_2.y, usePrevTri1 ? 1 : 0, isFloor);

					auto vertex0 = Vector3(corner0.x, height0, corner0.y);
					auto vertex1 = Vector3(corner1.x, height1, corner1.y);
					auto vertex2 = Vector3(corner0.x, prevHeight0, corner0.y);
					auto vertex3 = Vector3(corner1.x, prevHeight1, corner1.y);

					if (vertex0 != vertex2)
					{
						const auto& normal0 = ((vertex0.y > vertex2.y) ? EAST_WALL_NORMAL : WEST_WALL_NORMAL) * (isFloor ? 1 : -1);
						auto wallTri0 = CollisionTriangle(vertex0, vertex1, vertex2, normal0);
						tris.push_back(wallTri0);
					}
					if (vertex1 != vertex3)
					{
						const auto& normal1 = ((vertex1.y > vertex3.y) ? EAST_WALL_NORMAL : WEST_WALL_NORMAL) * (isFloor ? 1 : -1);
						auto wallTri1 = CollisionTriangle(vertex1, vertex2, vertex3, normal1);
						tris.push_back(wallTri1);
					}
				}
			}

			// Collect end wall.
			if (isXEnd && isFloor)
			{
				const auto& ceilSurface = isFloor ? sector.FloorSurface : sector.CeilingSurface;
				bool isCeilSurfSplit = sector.IsSurfaceSplit(false);
				bool isCeilSurfSplitAngle0 = (ceilSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);
				bool useCeilTri0 = (!isCeilSurfSplit || isCeilSurfSplitAngle0);

				int floorHeight0 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_2.x, REL_CORNER_2.y, !useTri0 ? 0 : 1, true);
				int floorHeight1 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, !useTri0 ? 0 : 1, true);
				int ceilHeight0 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_2.x, REL_CORNER_2.y, !useCeilTri0 ? 0 : 1, false);
				int ceilHeight1 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, !useCeilTri0 ? 0 : 1, false);

				auto endVertex0 = Vector3(corner2.x, floorHeight0, corner2.y);
				auto endVertex1 = Vector3(corner3.x, floorHeight1, corner3.y);
				auto endVertex2 = Vector3(corner2.x, ceilHeight0, corner2.y);
				auto endVertex3 = Vector3(corner3.x, ceilHeight1, corner3.y);

				auto endWallTri0 = CollisionTriangle(endVertex0, endVertex1, endVertex2, WEST_WALL_NORMAL);
				auto endWallTri1 = CollisionTriangle(endVertex1, endVertex2, endVertex3, WEST_WALL_NORMAL);
				tris.push_back(endWallTri0);
				tris.push_back(endWallTri1);
			}
		}

		// TODO
		// 6) Collect cardinal wall triangles on Z axis.
		if (prevSectorZ != nullptr)
		{
			const auto& prevSurface = isFloor ? prevSectorZ->FloorSurface : prevSectorZ->CeilingSurface;

			bool isPrevSurfSplit = prevSectorZ->IsSurfaceSplit(isFloor);
			bool isPrevSurfSplitAngle0 = (prevSurface.SplitAngle == SectorSurfaceData::SPLIT_ANGLE_0);

			bool isPrevSurf0Wall = prevSectorZ->IsWall(0);
			bool isPrevSurf1Wall = prevSectorZ->IsWall(1);
			bool isPrevSurfTri0Portal = (prevSurface.Triangles[0].PortalRoomNumber != NO_VALUE);
			bool isPrevSurfTri1Portal = (prevSurface.Triangles[1].PortalRoomNumber != NO_VALUE);

			// Calculate current sector corner heights.
			bool useTri1 = (!isSurfSplit || isSurfSplitAngle0);
			int height0 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_3.x, REL_CORNER_3.y, useTri1 ? 1 : 0, isFloor);
			int height1 = GetSurfaceTriangleVertexHeight(sector, REL_CORNER_0.x, REL_CORNER_0.y, useTri1 ? 1 : 0, isFloor);

			// Calculate previous sector corner heights.
			bool usePrevTri0 = (!isPrevSurfSplit || isPrevSurfSplitAngle0);
			int prevHeight0 = GetSurfaceTriangleVertexHeight(*prevSectorZ, REL_CORNER_1.x, REL_CORNER_1.y, usePrevTri0 ? 0 : 1, isFloor);
			int prevHeight1 = GetSurfaceTriangleVertexHeight(*prevSectorZ, REL_CORNER_2.x, REL_CORNER_2.y, usePrevTri0 ? 0 : 1, isFloor);

			// Determine wall vertices.
			auto vertex0 = Vector3(corner3.x, height0, corner3.y);
			auto vertex1 = Vector3(corner0.x, height1, corner0.y);
			auto vertex2 = Vector3(corner3.x, prevHeight0, corner3.y);
			auto vertex3 = Vector3(corner0.x, prevHeight1, corner0.y);

			// Collect step wall.
			if (!(usePrevTri0 ? isPrevSurf0Wall : isPrevSurf1Wall))
			{
				if (vertex0 != vertex2)
				{
					const auto& normal0 = ((vertex0.y > vertex2.y) ? NORTH_WALL_NORMAL : SOUTH_WALL_NORMAL) * (isFloor ? 1 : -1);
					auto wallTri0 = CollisionTriangle(vertex0, vertex1, vertex2, normal0);
					//tris.push_back(wallTri0);
				}
				if (vertex1 != vertex3)
				{
					const auto& normal1 = ((vertex1.y > vertex3.y) ? NORTH_WALL_NORMAL : SOUTH_WALL_NORMAL) * (isFloor ? 1 : -1);
					auto wallTri1 = CollisionTriangle(vertex1, vertex2, vertex3, normal1);
					//tris.push_back(wallTri1);
				}
			}
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

	// Run through sectors (ignoring border).
	for (int x = 1; x < (room.xSize - 1); x++)
	{
		for (int z = 1; z < (room.zSize - 1); z++)
		{
			auto& sector = room.floor[(x * room.zSize) + z];

			// Get previous X sector.
			const auto* prevSectorX = &room.floor[((x - 1) * room.zSize) + z];
			if (prevSectorX->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& prevRoomX = g_Level.Rooms[prevSectorX->SidePortalRoomNumber];
				auto prevRoomGridCoordX = GetRoomGridCoord(prevSectorX->SidePortalRoomNumber, prevSectorX->Position.x, prevSectorX->Position.y);

				prevSectorX = &prevRoomX.floor[(prevRoomGridCoordX.x * prevRoomX.zSize) + prevRoomGridCoordX.y];
			}

			// Get previous Z sector.
			const auto* prevSectorZ = &room.floor[(x * room.zSize) + (z - 1)];
			if (prevSectorZ->SidePortalRoomNumber != NO_VALUE)
			{
				const auto& prevRoomZ = g_Level.Rooms[prevSectorZ->SidePortalRoomNumber];
				auto prevRoomGridCoordZ = GetRoomGridCoord(prevSectorZ->SidePortalRoomNumber, prevSectorZ->Position.x, prevSectorZ->Position.y);

				prevSectorZ = &prevRoomZ.floor[(prevRoomGridCoordZ.x * prevRoomZ.zSize) + prevRoomGridCoordZ.y];
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
