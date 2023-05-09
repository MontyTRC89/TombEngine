#include "framework.h"
#include "Objects/TR5/Trap/tr5_laser.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/effects/effects.h"
#include "Game/effects/item_fx.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"

using namespace TEN::Effects::Items;

namespace TEN::Traps::TR5
{
	// NOTES:
	// - Negative OCB gives not lethal and activates a heavy trigger, positive OCB lethal.
	// - OCB value defines the width in blocks.

	std::vector<LaserBarrier> LaserBarriers = {};

	void InitializeLaserBarriers(short itemNumber)
	{
		constexpr auto BEAM_COUNT = 3; // TODO: Make beam counts an attribute.
		auto& item = g_Level.Items[itemNumber];

		auto barrier = LaserBarrier{};

		barrier.lethal = item.TriggerFlags;
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

		LaserBarriers.push_back(barrier);
	}
	
	void ControlLaserBarriers(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		for (auto& barrier : LaserBarriers)
		{
			if (!TriggerActive(item))
			{
				item->Model.Color.w = 0.0f;
				barrier.Color.w = 0.0f;
				barrier.On = false;
				return;
			}

			//brightness fade in and distortion
			if (item->Model.Color.w < 1.0f)
				item->Model.Color.w += 0.02f;

			if (barrier.Color.w < 1.0f)
				barrier.Color.w += 0.02f;

			if (item->Model.Color.w > 8.0f)
			{
				barrier.Color.w = 0.8f;
				item->Model.Color.w = 0.8f;
			}
			
			barrier.On = true;
		}

		CollideLaserBarriers(itemNumber);

		if (item->Model.Color.w >= 0.8f)
		{
			LaserBarrierLight(itemNumber, 150, 31);			
		}

		SoundEffect(SFX_TR5_DOOR_BEAM, &item->Pose);
	}

	void CollideLaserBarriers(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		for (auto& barrier : LaserBarriers)
		{
			auto corner1 = barrier.Beams[2].VertexPoints[0];
			auto corner2 = barrier.Beams[2].VertexPoints[1];

			auto corner3 = barrier.Beams[0].VertexPoints[2];
			auto corner4 = barrier.Beams[0].VertexPoints[3];

			// Calculate center of the bounding box
			barrier.BoundingBox.Center = (corner1 + corner2 + corner3 + corner4) / 4.0f;

			// Calculate relative dimensions of the bounding box
			auto halfWidth = std::abs(corner1.x - corner2.x) / 2.0f;
			auto halfHeight = std::abs(corner1.y - corner3.y) / 2.0f;
			auto halfDepth = std::abs(corner1.z - corner3.z) / 2.0f;
			barrier.BoundingBox.Extents = Vector3(halfWidth, halfHeight, halfDepth);

			auto itemBBox = GameBoundingBox(LaraItem);
			auto itemBounds = itemBBox.ToBoundingOrientedBox(LaraItem->Pose);

			if (barrier.BoundingBox.Intersects(itemBounds))
			{
				if (barrier.lethal > 0 && LaraItem->Effect.Type != EffectType::Smoke && LaraItem->HitPoints > 0)
				{
					ItemRedLaserBurn(LaraItem, 2 * FPS);
					DoDamage(LaraItem, MAXINT);
				}

				if (barrier.lethal < 0)
				{
					TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
				}

				barrier.Color.w = Random::GenerateFloat(1.0f, 0.7f);
				LaserBarrierLight(itemNumber, 255, 100);
			}		
		}
	}

	void ClearLaserBarriers()
	{
		LaserBarriers.clear();
	}

	void LaserBarrierLight(short itemNumber, int lightIntensity, int amplitude)
	{
		for (auto& barrier : LaserBarriers)
		{
			auto* item = &g_Level.Items[itemNumber];

			int intensity = 0;

			intensity = lightIntensity - (Random::GenerateInt(0, amplitude));

			TriggerDynamicLight(
				item->Pose.Position.x,
				item->Pose.Position.y,
				item->Pose.Position.z,
				8,
				(intensity * (item->Model.Color.x / 2)),
				(intensity * (item->Model.Color.y / 2)),
				(intensity * (item->Model.Color.z / 2)));
		}
	}
}
