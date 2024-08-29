#include "framework.h"
#include "Game/collision/AttractorDebug.h"

#include <ois/OISKeyboard.h>

#include "Game/collision/Attractor.h"
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
// G: Spawn wall edge attractor stack.

namespace TEN::Collision::Attractor
{
	static void InitAttractors(ItemInfo& item)
	{
		auto& player = GetLaraInfo(item);

		auto points = std::vector<Vector3>{ Vector3::Zero, Vector3::Zero };

		player.Context.DebugAttracs.Attrac0 = AttractorObject(AttractorType::Edge, Vector3::Zero, Quaternion::Identity,item.RoomNumber, points);
		player.Context.DebugAttracs.Attrac1 = AttractorObject(AttractorType::Edge, Vector3::Zero, Quaternion::Identity,item.RoomNumber, points);
		player.Context.DebugAttracs.Attrac2 = AttractorObject(AttractorType::Edge, Vector3::Zero, Quaternion::Identity,item.RoomNumber, points);

		for (int i = 0; i < 8; i++)
			player.Context.DebugAttracs.Attracs.push_back(AttractorObject(AttractorType::Edge, Vector3::Zero, Quaternion::Identity,item.RoomNumber, points));
	}

	static void SpawnAttractorCircle(ItemInfo& item, bool isOuter)
	{
		constexpr auto RADIUS	  = BLOCK(0.4f);
		constexpr auto STEP_COUNT = 16;
		constexpr auto STEP_ANGLE = PI_MUL_2 / STEP_COUNT;
		constexpr auto REL_OFFSET = Vector3(0.0f, 0.0f, RADIUS);

		auto& player = GetLaraInfo(item);

		auto center = Vector3(0.0f, -CLICK(5), 0.0f);

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

		player.Context.DebugAttracs.Attrac2 = AttractorObject(AttractorType::Edge, item.Pose.Position.ToVector3(), item.Pose.Orientation.ToQuaternion(), item.RoomNumber, points);
	}

	static void SetDebugAttractors(ItemInfo& item)
	{
		auto& player = GetLaraInfo(item);

		auto basePos = item.Pose.Position.ToVector3();

		auto relOffset = Vector3(0.0f, -CLICK(5), LARA_RADIUS);
		auto rotMatrix = item.Pose.Orientation.ToRotationMatrix();
		auto offset = Vector3::Transform(relOffset, rotMatrix);

		auto attracPoint = basePos + offset;

		static auto attracAPoint0 = Vector3::Zero;
		static auto attracAPoint1 = Vector3::Zero;
		static auto attracBPoint0 = Vector3::Zero;
		static auto attracBPoint1 = Vector3::Zero;

		// Set debug attractor 0.
		if (KeyMap[OIS::KeyCode::KC_Q])
		{
			attracAPoint0 = attracPoint;
			player.Context.DebugAttracs.Attrac0 = AttractorObject(AttractorType::Edge, Vector3::Zero, Quaternion::Identity, item.RoomNumber, { attracAPoint0, attracAPoint1 });
		}
		if (KeyMap[OIS::KeyCode::KC_W])
		{
			attracAPoint1 = attracPoint;
			player.Context.DebugAttracs.Attrac0 = AttractorObject(AttractorType::Edge, Vector3::Zero, Quaternion::Identity, item.RoomNumber, { attracAPoint0, attracAPoint1 });
		}

		// Set debug attractor 1.
		if (KeyMap[OIS::KeyCode::KC_E])
		{
			attracBPoint0 = attracPoint;
			player.Context.DebugAttracs.Attrac0 = AttractorObject(AttractorType::Edge, Vector3::Zero, Quaternion::Identity, item.RoomNumber, { attracBPoint0, attracBPoint1 });
		}
		if (KeyMap[OIS::KeyCode::KC_R])
		{
			attracBPoint1 = attracPoint;
			player.Context.DebugAttracs.Attrac0 = AttractorObject(AttractorType::Edge, Vector3::Zero, Quaternion::Identity, item.RoomNumber, { attracBPoint0, attracBPoint1 });
		}

		// Spawn attractor circle.
		if (KeyMap[OIS::KeyCode::KC_T])
			SpawnAttractorCircle(item, true);
		if (KeyMap[OIS::KeyCode::KC_Y])
			SpawnAttractorCircle(item, false);

		// Spawn climbable wall attractor stack.
		if (KeyMap[OIS::KeyCode::KC_G])
		{
			auto vPos = Vector3(item.Pose.Position.x, (floor(item.Pose.Position.y / CLICK(1)) * CLICK(1)) - CLICK(1), item.Pose.Position.z);
			int inc = 0;
			for (auto& attrac : player.Context.DebugAttracs.Attracs)
			{
				auto points = std::vector<Vector3>
				{
					Geometry::TranslatePoint(vPos, item.Pose.Orientation.y, 100, inc, -BLOCK(0.5f)),
					Geometry::TranslatePoint(vPos, item.Pose.Orientation.y, 100, inc, BLOCK(0.5f)),
				};
				inc -= CLICK(1);

				attrac = AttractorObject(AttractorType::WallEdge, Vector3::Zero, Quaternion::Identity, item.RoomNumber, points);
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
