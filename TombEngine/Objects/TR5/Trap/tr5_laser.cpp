#include "framework.h"
#include "Objects/TR5/Trap/tr5_laser.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/effects/effects.h"
#include "Game/effects/item_fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"

using namespace TEN::Effects::Items;

namespace TEN::Traps::TR5
{
	// NOTES:
	// - Negative OCB gives not lethal and activates a heavy trigger, positive OCB lethal.
	// - OCB value defines the width in blocks.

	extern std::unordered_map<int, LaserBarrier> LaserBarriers = {};

	// TODO: Simplify.
	void InitializeLaserBarrier(short itemNumber)
	{
		constexpr auto BEAM_COUNT = 3; // TODO: Make beam counts an attribute.

		auto& item = g_Level.Items[itemNumber];

		auto barrier = LaserBarrier{};

		barrier.Lethal = item.TriggerFlags;
		int width = abs(item.TriggerFlags) * BLOCK(1);
		barrier.Color = item.Model.Color;
		barrier.Color.w = 0.0f;

		item.ItemFlags[1] = item.TriggerFlags;

		int xAdd = 0;
		if (!(item.TriggerFlags & 1))
		{
			xAdd = (width / 2) - BLOCK(0.5);

			item.Pose.Position.z += xAdd * phd_cos(item.Pose.Orientation.y + ANGLE(TO_RAD(180.0f))) / 16384;
			item.Pose.Position.x += xAdd * phd_sin(item.Pose.Orientation.y + ANGLE(TO_RAD(180.0f))) / 16384;
		}

		if ((item.TriggerFlags & 255) == 1)
			item.ItemFlags[1] = 1;
		
		auto pointColl = GetCollision(&item);

		item.Pose.Position.y = pointColl.Position.Floor;

		item.ItemFlags[0] = short(item.Pose.Position.y - pointColl.Position.Ceiling);
		short height = item.ItemFlags[0];
		int yAdd = height / 8;

		int zAdd = abs((width * phd_cos(item.Pose.Orientation.y)) / 2);
		xAdd = abs((width * phd_sin(item.Pose.Orientation.y)) / 2);
		int lH = yAdd / 2;
		height = -yAdd;

		auto basePos = item.Pose.Position.ToVector3();

		// Set vertex positions.
		int i = 0;
		barrier.Beams.resize(BEAM_COUNT);
		for (auto& beam : barrier.Beams)
		{
			int hAdd = (lH / 2) * (i - 1);

			beam.VertexPoints = std::array<Vector3, LaserBarrierBeam::VERTEX_COUNT>
			{
				basePos + Vector3(xAdd, height - lH + hAdd, zAdd),
				basePos + Vector3(-xAdd, height - lH + hAdd, -zAdd),
				basePos + Vector3(-xAdd, height + lH + hAdd, -zAdd),
				basePos + Vector3(xAdd, height + lH + hAdd, zAdd)
			};

			height -= yAdd * 3;
			i++;
		}

		LaserBarriers.insert({ itemNumber, barrier });
	}

	// TODO: Make it a line of light.
	static void SpawnLaserBarrierLight(const ItemInfo& item, float intensity, float amplitude)
	{
		float intensityNorm = intensity - Random::GenerateFloat(0.0f, amplitude);
		TriggerDynamicLight(
			item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z,
			8,
			intensityNorm * (item.Model.Color.x / 2),
			intensityNorm * (item.Model.Color.y / 2),
			intensityNorm * (item.Model.Color.z / 2));
	}

	void ControlLaserBarrier(short itemNumber)
	{
		constexpr auto LIGHT_INTENSITY = 150.0f;
		constexpr auto LIGHT_AMPLITUDE = 31.0f;

		if (!LaserBarriers.count(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& barrier = LaserBarriers.at(itemNumber);

		if (!TriggerActive(&item))
		{
			barrier.IsActive = false;
			barrier.Color.w = 0.0f;
			item.Model.Color.w = 0.0f;
			return;
		}

		// Brightness fade-in and distortion.
		if (item.Model.Color.w < 1.0f)
			item.Model.Color.w += 0.02f;

		if (barrier.Color.w < 1.0f)
			barrier.Color.w += 0.02f;

		if (item.Model.Color.w > 8.0f)
		{
			barrier.Color.w = 0.8f;
			item.Model.Color.w = 0.8f;
		}
			
		barrier.IsActive = true;

		if (item.Model.Color.w >= 0.8f)
			SpawnLaserBarrierLight(item, LIGHT_INTENSITY, LIGHT_AMPLITUDE);

		SoundEffect(SFX_TR5_DOOR_BEAM, &item.Pose);
	}

	void CollideLaserBarrier(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		constexpr auto LIGHT_INTENSITY = 255.0f;
		constexpr auto LIGHT_AMPLITUDE = 100.0f;

		if (!LaserBarriers.count(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& barrier = LaserBarriers.at(itemNumber);

		if (!barrier.IsActive)
			return;

		// Determine points.
		auto point0 = barrier.Beams.back().VertexPoints[0];
		auto point1 = barrier.Beams.back().VertexPoints[1];
		auto point2 = barrier.Beams.front().VertexPoints[2];
		auto point3 = barrier.Beams.front().VertexPoints[3];

		// Update bounding box center.
		barrier.BoundingBox.Center = (point0 + point1 + point2 + point3) / 4;

		// Calculate and update relative bounding box dimensions.
		float halfWidth = std::abs(point0.x - point1.x) / 2;
		float halfHeight = std::abs(point0.y - point2.y) / 2;
		float halfDepth = std::abs(point0.z - point2.z) / 2;
		barrier.BoundingBox.Extents = Vector3(halfWidth, halfHeight, halfDepth);

		auto playerBox = GameBoundingBox(playerItem).ToBoundingOrientedBox(playerItem->Pose);
		if (barrier.BoundingBox.Intersects(playerBox))
		{
			if (barrier.Lethal > 0 &&
				playerItem->HitPoints > 0 && playerItem->Effect.Type != EffectType::Smoke)
			{
				ItemRedLaserBurn(playerItem, 2.0f * FPS);
				DoDamage(playerItem, MAXINT);
			}

			if (barrier.Lethal < 0)
				TestTriggers(&item, true, item.Flags & IFLAG_ACTIVATION_MASK);

			barrier.Color.w = Random::GenerateFloat(0.6f, 1.0f);
			SpawnLaserBarrierLight(item, LIGHT_INTENSITY, LIGHT_AMPLITUDE);
		}		
	}

	void ClearLaserBarriers()
	{
		LaserBarriers.clear();
	}
}
