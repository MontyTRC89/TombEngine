#include "framework.h"
#include "Objects/TR5/Trap/tr5_laser.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/effects/effects.h"
#include "Specific/level.h"

namespace TEN::Traps::TR5
{
	// NOTES:
	// - Negative OCB gives green laser, positive OCB gives red laser.
	// - OCB value defines the width in blocks.

	std::vector<LaserBarrier> LaserBarriers = {};

	void InitializeLaserBarriers(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		auto barrier = LaserBarrier{};

		int width = abs(item.TriggerFlags) * BLOCK(1);
		barrier.Color = item.Model.Color;

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
		item.TriggerFlags >>= 8;
		int yAdd = height / 8;

		int zAdd = abs((width * phd_cos(item.Pose.Orientation.y)) / 2);
		xAdd = abs((width * phd_sin(item.Pose.Orientation.y)) / 2);
		int lH = yAdd / 2;
		height = -yAdd;

		// Set vertex positions.
		for (int i = 0; i < 3; i++)
		{
			int hAdd = (lH / 2) * (i - 1);

			barrier.vert1[i].x = item.Pose.Position.x + xAdd;
			barrier.vert1[i].y = item.Pose.Position.y + (height - lH + hAdd);
			barrier.vert1[i].z = item.Pose.Position.z + zAdd;
			barrier.vert2[i].x = item.Pose.Position.x + (-xAdd);
			barrier.vert2[i].y = item.Pose.Position.y + (height - lH + hAdd);
			barrier.vert2[i].z = item.Pose.Position.z + (-zAdd);
			barrier.vert3[i].x = item.Pose.Position.x + (-xAdd);
			barrier.vert3[i].y = item.Pose.Position.y + (height + lH + hAdd);
			barrier.vert3[i].z = item.Pose.Position.z + (-zAdd);
			barrier.vert4[i].x = item.Pose.Position.x + xAdd;
			barrier.vert4[i].y = item.Pose.Position.y + (height + lH + hAdd);
			barrier.vert4[i].z = item.Pose.Position.z + zAdd;

			height -= yAdd * 3;
		}

		for (int i = 0; i < 18; i++)
			barrier.Rand[i] = short(GetRandomControl() << 1);

		LaserBarriers.push_back(barrier);
	}

	void ControlLaserBarriers(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		for (auto& barrier : LaserBarriers)
		{
			if (!TriggerActive(&item))
			{
				item.Model.Color.w = 0.0f;
				barrier.Color.w = 1.0f;
				return;
			}

			item.Model.Color.w = 1.0f;
			barrier.Color.w = 0.8f;

			/*auto effectBounds = GameBoundingBox::Zero;

			effectBounds.Y1 = item.Pose.Position.y + bounds.Y1;
			effectBounds.Y2 = item.Pose.Position.y + bounds.Y2;*/

			TriggerDynamicLight(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, 6, item.Model.Color.x, item.Model.Color.y, item.Model.Color.z);

			//The next line makes the effect flicker as soon Lara steps in. Yay! - moving it into collision code.
			//laser.Color.w = Random::GenerateFloat(1.0f, 0.8f);

			//including sound later. Bzzz
			//SoundEffect(SFX_RICH_DOOR_BEAM, &item.pos, SFX_DEFAULT);

			if (item.ItemFlags[3])
				item.ItemFlags[3] -= 2;
		}

		// TODO: Make this an object collision routine.
		CollideLaserBarriers();
	}

	void CollideLaserBarriers()
	{
		// TODO
	}

	void UpdateLaserBarriers()
	{
		if (LaserBarriers.empty())
			return;

		for (auto& barrier : LaserBarriers)
		{
			// Not needed.
		}
	}

	void ClearLaserBarriers()
	{
		LaserBarriers.clear();
	}
}
