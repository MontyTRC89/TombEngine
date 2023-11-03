#include "framework.h"
#include "Game/collision/AttractorDebug.h"

#include <ois/OISKeyboard.h>

#include "Game/collision/Attractors.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Specific/Input/Input.h"

using namespace TEN::Input;

// NOTE: Temporary file.

namespace TEN::Collision::Attractors
{
	static void InitAttractors(ItemInfo& item)
	{
		auto& player = GetLaraInfo(item);

		auto points = std::vector<Vector3>{ Vector3::Zero, Vector3::Zero };

		player.Context.DebugAttracs.Attrac0 = Attractor(AttractorType::Edge, points, item.RoomNumber);
		player.Context.DebugAttracs.Attrac1 = Attractor(AttractorType::Edge, points, item.RoomNumber);
		player.Context.DebugAttracs.Attrac2 = Attractor(AttractorType::Edge, points, item.RoomNumber);
	}

	static void SpawnAttractorPentagon(ItemInfo& item, bool isOuter)
	{
		constexpr auto RADIUS	  = BLOCK(0.5f);
		constexpr auto STEP_COUNT = 5;
		constexpr auto STEP_ANGLE = PI_MUL_2 / STEP_COUNT;
		constexpr auto REL_OFFSET = Vector3(0.0f, 0.0f, RADIUS);

		auto& player = GetLaraInfo(item);

		auto center = item.Pose.Position.ToVector3() + Vector3(0.0f, -CLICK(5), 0.0f);

		float angle = 0.0f;
		auto points = std::vector<Vector3>{};
		for (int i = 0; i < STEP_COUNT; i++)
		{
			auto rotMatrix = Matrix::CreateRotationY(angle);
			auto point = center + Vector3::Transform(REL_OFFSET, rotMatrix);
			points.push_back(point);

			angle += STEP_ANGLE;
		}

		if (isOuter)
			std::reverse(points.begin(), points.end());

		auto attrac = GenerateAttractorFromPoints(points, item.RoomNumber, AttractorType::Edge);
		player.Context.DebugAttracs.Attrac2 = attrac;
	}

	static void SetDebugAttractors(ItemInfo& item)
	{
		auto& player = GetLaraInfo(item);

		auto basePos = item.Pose.Position.ToVector3();

		auto relOffset = Vector3(0.0f, -CLICK(5), LARA_RADIUS);
		auto rotMatrix = item.Pose.Orientation.ToRotationMatrix();
		auto offset = Vector3::Transform(relOffset, rotMatrix);

		auto attracPoint = basePos + offset;

		// Set debug attractor 0.
		if (KeyMap[OIS::KeyCode::KC_E])
		{
			auto pos0 = attracPoint;
			auto pos1 = player.Context.DebugAttracs.Attrac0.GetPoints().empty() ? pos0 : player.Context.DebugAttracs.Attrac0.GetPoints().back();
			player.Context.DebugAttracs.Attrac0 = Attractor(AttractorType::Edge, { pos0, pos1 }, item.RoomNumber);
		}
		if (KeyMap[OIS::KeyCode::KC_R])
		{
			auto pos1 = attracPoint;
			auto pos0 = player.Context.DebugAttracs.Attrac0.GetPoints().empty() ? pos1 : player.Context.DebugAttracs.Attrac0.GetPoints().front();
			player.Context.DebugAttracs.Attrac0 = Attractor(AttractorType::Edge, { pos0, pos1 }, item.RoomNumber);
		}

		// Set debug attractor 1.
		if (KeyMap[OIS::KeyCode::KC_Q])
		{
			auto pos0 = attracPoint;
			auto pos1 = player.Context.DebugAttracs.Attrac1.GetPoints().empty() ? pos0 : player.Context.DebugAttracs.Attrac1.GetPoints().back();
			player.Context.DebugAttracs.Attrac1 = Attractor(AttractorType::Edge, { pos0, pos1 }, item.RoomNumber);
		}
		if (KeyMap[OIS::KeyCode::KC_W])
		{
			auto pos1 = attracPoint;
			auto pos0 = player.Context.DebugAttracs.Attrac1.GetPoints().empty() ? pos1 : player.Context.DebugAttracs.Attrac1.GetPoints().front();
			player.Context.DebugAttracs.Attrac1 = Attractor(AttractorType::Edge, { pos0, pos1 }, item.RoomNumber);
		}

		// Spawn attractor pentagon.
		if (KeyMap[OIS::KeyCode::KC_T])
			SpawnAttractorPentagon(item, true);
		if (KeyMap[OIS::KeyCode::KC_Y])
			SpawnAttractorPentagon(item, false);

		// Modify pentagon point.
		if (KeyMap[OIS::KeyCode::KC_U])
		{
			auto pos = LaraItem->Pose.Position.ToVector3() + Vector3::Transform(Vector3(0.0f, -CLICK(5), LARA_RADIUS), rotMatrix);
			auto points = player.Context.DebugAttracs.Attrac2.GetPoints();
			points[1] = pos;
			player.Context.DebugAttracs.Attrac2.Update(points, player.Context.DebugAttracs.Attrac2.GetRoomNumber());
		}
	}

	void HandleAttractorDebug(ItemInfo& item)
	{
		auto& player = GetLaraInfo(item);

		static bool hasAttracInit = false;
		if (!hasAttracInit)
		{
			InitAttractors(item);
			hasAttracInit = true;
		}

		SetDebugAttractors(item);

		// Generate sector attractor.
		auto pointColl = GetCollision(item);
		auto attracs = GenerateSectorAttractors(pointColl);

		player.Context.DebugAttracs.Attracs = attracs;

		// Draw debug.
		auto attracColls = GetAttractorCollisions(item, item.Pose.Position.ToVector3(), BLOCK(5));
		for (const auto& attracColl : attracColls)
			attracColl.Attrac.DrawDebug();
	}

	std::vector<Attractor*> GetDebugAttractorPtrs(ItemInfo& item)
	{
		auto& player = GetLaraInfo(item);

		auto nearbyAttracPtrs = std::vector<Attractor*>{};
		nearbyAttracPtrs.push_back(&player.Context.DebugAttracs.Attrac0);
		nearbyAttracPtrs.push_back(&player.Context.DebugAttracs.Attrac1);
		nearbyAttracPtrs.push_back(&player.Context.DebugAttracs.Attrac2);

		for (auto& attrac : player.Context.DebugAttracs.Attracs)
			nearbyAttracPtrs.push_back(&attrac);

		return nearbyAttracPtrs;
	}

	static Attractor GenerateBridgeAttractor(const ItemInfo& bridgeItem)
	{
		constexpr auto TILT_STEP = CLICK(1);

		// Determine tilt offset.
		int tiltOffset = 0;
		switch (bridgeItem.ObjectNumber)
		{
		default:
		case ID_BRIDGE_FLAT:
			break;

		case ID_BRIDGE_TILT1:
			tiltOffset = TILT_STEP;
			break;

		case ID_BRIDGE_TILT2:
			tiltOffset = TILT_STEP * 2;
			break;

		case ID_BRIDGE_TILT3:
			tiltOffset = TILT_STEP * 3;
			break;

		case ID_BRIDGE_TILT4:
			tiltOffset = TILT_STEP * 4;
			break;
		}

		// Determine relative corner points.
		auto box = GameBoundingBox(&bridgeItem).ToBoundingOrientedBox(bridgeItem.Pose);
		auto point0 = Vector3(box.Extents.x, -box.Extents.y + tiltOffset, box.Extents.z);
		auto point1 = Vector3(-box.Extents.x, -box.Extents.y, box.Extents.z);
		auto point2 = Vector3(-box.Extents.x, -box.Extents.y, -box.Extents.z);
		auto point3 = Vector3(box.Extents.x, -box.Extents.y + tiltOffset, -box.Extents.z);

		// Calculate absolute corner points.
		auto rotMatrix = Matrix::CreateFromQuaternion(box.Orientation);
		auto points = std::vector<Vector3>
		{
			box.Center + Vector3::Transform(point0, rotMatrix),
			box.Center + Vector3::Transform(point1, rotMatrix),
			box.Center + Vector3::Transform(point2, rotMatrix),
			box.Center + Vector3::Transform(point3, rotMatrix)
		};

		// Generate and return attractor.
		return GenerateAttractorFromPoints(points, bridgeItem.RoomNumber, AttractorType::Edge);
	}

	std::vector<Attractor> GenerateSectorAttractors(const CollisionResult& pointColl)
	{
		// Invalid sector; return empty vector.
		if (pointColl.Position.Floor == NO_HEIGHT)
			return {};

		auto attracs = std::vector<Attractor>{};

		// Generate bridge attractors.
		for (int bridgeItemNumber : pointColl.BottomBlock->BridgeItemNumbers)
		{
			const auto& bridgeItem = g_Level.Items[bridgeItemNumber];
			attracs.push_back(GenerateBridgeAttractor(bridgeItem));
		}

		// Generate floor attractors.
		auto pointGroups = pointColl.BottomBlock->GetSurfaceVertices(pointColl.Coordinates.x, pointColl.Coordinates.z, true);
		for (const auto& points : pointGroups)
		{
			if (!points.empty())
				attracs.push_back(GenerateAttractorFromPoints(points, pointColl.RoomNumber, AttractorType::Edge));
		}

		// Return generated attractors.
		return attracs;
	}
}
