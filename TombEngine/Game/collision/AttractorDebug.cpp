#include "framework.h"
#include "Game/collision/AttractorDebug.h"

#include <ois/OISKeyboard.h>

#include "Game/collision/Attractor.h"
#include "Game/collision/AttractorCollision.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Specific/Input/Input.h"

using namespace TEN::Input;
using namespace TEN::Math;

// NOTE: Temporary file.

// Controls:
// Q: Spawn edge attractor A point 0.
// W: Spawn edge attractor A point 1.
// E: Spawn edge attractor B point 0.
// R: Spawn edge attractor B point 1.
// T: Spawn outer edge attractor circle.
// Y: Spawn inner edge attractor circle.
// U: Modify edge attractor circle point.
// G: Spawn wall edge attractor stack.

namespace TEN::Collision::Attractor
{
	static void InitAttractors(ItemInfo& item)
	{
		auto& player = GetLaraInfo(item);

		auto points = std::vector<Vector3>{ Vector3::Zero, Vector3::Zero };

		player.Context.DebugAttracs.Attrac0 = Attractor(AttractorType::Edge, points, item.RoomNumber);
		player.Context.DebugAttracs.Attrac1 = Attractor(AttractorType::Edge, points, item.RoomNumber);
		player.Context.DebugAttracs.Attrac2 = Attractor(AttractorType::Edge, points, item.RoomNumber);

		for (int i = 0; i < 8; i++)
			player.Context.DebugAttracs.Attracs.push_back(Attractor(AttractorType::Edge, points, item.RoomNumber));
	}

	static void SpawnAttractorCircle(ItemInfo& item, bool isOuter)
	{
		constexpr auto RADIUS	  = BLOCK(0.4f);
		constexpr auto STEP_COUNT = 16;
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
		points.push_back(points.front());

		if (isOuter)
			std::reverse(points.begin(), points.end());

		player.Context.DebugAttracs.Attrac2 = Attractor(AttractorType::Edge, points, item.RoomNumber);
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
		if (KeyMap[OIS::KeyCode::KC_Q])
		{
			auto pos0 = attracPoint;
			auto pos1 = player.Context.DebugAttracs.Attrac0->GetPoints().empty() ? pos0 : player.Context.DebugAttracs.Attrac0->GetPoints().back();
			player.Context.DebugAttracs.Attrac0 = Attractor(AttractorType::Edge, { pos0, pos1 }, item.RoomNumber);
		}
		if (KeyMap[OIS::KeyCode::KC_W])
		{
			auto pos1 = attracPoint;
			auto pos0 = player.Context.DebugAttracs.Attrac0->GetPoints().empty() ? pos1 : player.Context.DebugAttracs.Attrac0->GetPoints().front();
			player.Context.DebugAttracs.Attrac0 = Attractor(AttractorType::Edge, { pos0, pos1 }, item.RoomNumber);
		}

		// Set debug attractor 1.
		if (KeyMap[OIS::KeyCode::KC_E])
		{
			auto pos0 = attracPoint;
			auto pos1 = player.Context.DebugAttracs.Attrac1->GetPoints().empty() ? pos0 : player.Context.DebugAttracs.Attrac1->GetPoints().back();
			player.Context.DebugAttracs.Attrac1 = Attractor(AttractorType::Edge, { pos0, pos1 }, item.RoomNumber);
		}
		if (KeyMap[OIS::KeyCode::KC_R])
		{
			auto pos1 = attracPoint;
			auto pos0 = player.Context.DebugAttracs.Attrac1->GetPoints().empty() ? pos1 : player.Context.DebugAttracs.Attrac1->GetPoints().front();
			player.Context.DebugAttracs.Attrac1 = Attractor(AttractorType::Edge, { pos0, pos1 }, item.RoomNumber);
		}

		// Spawn attractor circle.
		if (KeyMap[OIS::KeyCode::KC_T])
			SpawnAttractorCircle(item, true);
		if (KeyMap[OIS::KeyCode::KC_Y])
			SpawnAttractorCircle(item, false);

		// Modify circle point.
		if (KeyMap[OIS::KeyCode::KC_U])
		{
			auto pos = LaraItem->Pose.Position.ToVector3() + Vector3::Transform(Vector3(0.0f, -CLICK(5), LARA_RADIUS), rotMatrix);
			auto points = player.Context.DebugAttracs.Attrac2->GetPoints();
			points[1] = pos;
			player.Context.DebugAttracs.Attrac2->Update(points, player.Context.DebugAttracs.Attrac2->GetRoomNumber());
		}

		// Spawn climbable wall attractor stack.
		if (KeyMap[OIS::KeyCode::KC_G])
		{
			auto vPos = Vector3(item.Pose.Position.x, floor(item.Pose.Position.y / CLICK(1)) * CLICK(1), item.Pose.Position.z);
			int inc = 0;
			for (auto& attrac : player.Context.DebugAttracs.Attracs)
			{
				auto points = std::vector<Vector3>
				{
					Geometry::TranslatePoint(vPos, item.Pose.Orientation.y, 100, inc, -BLOCK(0.5f)),
					Geometry::TranslatePoint(vPos, item.Pose.Orientation.y, 100, inc, BLOCK(0.5f)),
				};
				inc -= CLICK(1);

				attrac = Attractor(AttractorType::WallEdge, points, item.RoomNumber);
			}
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
	}
}
