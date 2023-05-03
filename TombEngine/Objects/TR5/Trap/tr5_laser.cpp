#include "framework.h"
#include "Objects/TR5/Trap/tr5_laser.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/effects/effects.h"
#include "Specific/level.h"

namespace TEN::Traps::TR5
{
	std::vector<LaserStructInfo> Lasers = {};

	void InitializeLasers(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		auto laser = LaserStructInfo();
		short xAdd, height, zAdd, Lh, yAdd, hAdd, width;
		short roomNumber;

		//negative OCB means green laser, positive red laser. OCB value defines the width in blocks.
		if (item.TriggerFlags >= 0)
		{
			width = item.TriggerFlags * BLOCK(1);
			laser.Color = item.Model.Color;// Vector4(240.0f, 0.0f, 0.0f, 0.0f);
		}
		else
		{
			width = abs(item.TriggerFlags) * BLOCK(1);
			item.Model.Color = laser.Color = Vector4(0.0f, 240.0f, 0.0f, 0.0f);
		}

		if (!(item.TriggerFlags & 1))
		{
			xAdd = (width / 2) - BLOCK(0.5);

			item.Pose.Position.z += xAdd * phd_cos(item.Pose.Orientation.y + ANGLE(TO_RAD(180.0f))) / 16384;
			item.Pose.Position.x += xAdd * phd_sin(item.Pose.Orientation.y + ANGLE(TO_RAD(180.0f))) / 16384;
		}

		if ((item.TriggerFlags & 255) == 1)
			item.ItemFlags[1] = 1;

		roomNumber = item.RoomNumber;

		auto floor = GetFloor(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, &roomNumber);
		item.Pose.Position.y = GetFloorHeight(floor, item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z);
		item.ItemFlags[0] = short(item.Pose.Position.y - GetCeiling(floor, item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z));
		height = item.ItemFlags[0];
		item.TriggerFlags >>= 8;
		yAdd = height / 8;

		zAdd = abs((width * phd_cos(item.Pose.Orientation.y)) / 2);
		xAdd = abs((width * phd_sin(item.Pose.Orientation.y)) / 2);
		Lh = yAdd >> 1;
		height = -yAdd;

		//move vertex position to Laser struct.
		for (int i = 0; i < 3; i++)
		{
			hAdd = (Lh >> 1) * (i - 1);
			laser.vert1[i].x = item.Pose.Position.x + xAdd;
			laser.vert1[i].y = item.Pose.Position.y + (height - Lh + hAdd);
			laser.vert1[i].z = item.Pose.Position.z + zAdd;
			laser.vert2[i].x = item.Pose.Position.x + (-xAdd);
			laser.vert2[i].y = item.Pose.Position.y + (height - Lh + hAdd);
			laser.vert2[i].z = item.Pose.Position.z + (-zAdd);
			laser.vert3[i].x = item.Pose.Position.x + (-xAdd);
			laser.vert3[i].y = item.Pose.Position.y + (height + Lh + hAdd);
			laser.vert3[i].z = item.Pose.Position.z + (-zAdd);
			laser.vert4[i].x = item.Pose.Position.x + xAdd;
			laser.vert4[i].y = item.Pose.Position.y + (height + Lh + hAdd);
			laser.vert4[i].z = item.Pose.Position.z + zAdd;
			height -= yAdd * 3;
		}

		for (int i = 0; i < 18; i++)
			laser.Rand[i] = short(GetRandomControl() << 1);

		Lasers.push_back(laser);
	}

	void UpdateLasers()
	{
		if (Lasers.empty())
			return;

		for (auto& laser : Lasers)
		{
			//not needed..
		}
	}

	void ClearLasers()
	{
		Lasers.erase(
			std::remove_if(
				Lasers.begin(), Lasers.end(),
				[](const LaserStructInfo& laser) { return (laser.life <= 0.0f); }), Lasers.end());
	}

	void ControlLasers(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		for (auto& laser : Lasers)
		{
			if (!TriggerActive(&item))
			{
				item.Model.Color.w = 0.0f;
				laser.Color.w = 1.0f;
				return;
			}

			item.Model.Color.w = 1.0f;
			laser.Color.w = 0.8f;

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
		//TODO: make collision. Time to decrypt next function and prepare for a few more nights of work.
	}

	/*long CheckLaserBox(long* bounds)
	{
		short* lbounds;
		long swp;
		short rbounds[6];

		if (bounds[0] > bounds[1])
		{
			swp = bounds[0];
			bounds[0] = bounds[1];
			bounds[1] = swp;
		}

		if (bounds[4] > bounds[5])
		{
			swp = bounds[4];
			bounds[4] = bounds[5];
			bounds[5] = swp;
		}

		lbounds = GetBoundsAccurate(LaraItem);
		phd_PushUnitMatrix();
		phd_RotYXZ(lara_item.pos.y_rot, lara_item.pos.x_rot, lara_item.pos.z_rot);
		phd_SetTrans(0, 0, 0);
		mRotBoundingBoxNoPersp(lbounds, rbounds);
		phd_PopMatrix();

		DeadlyBounds[0] = lara_item.pos.x_pos + rbounds[0];
		DeadlyBounds[1] = lara_item.pos.x_pos + rbounds[1];
		DeadlyBounds[2] = lara_item.pos.y_pos + rbounds[2];
		DeadlyBounds[3] = lara_item.pos.y_pos + rbounds[3];
		DeadlyBounds[4] = lara_item.pos.z_pos + rbounds[4];
		DeadlyBounds[5] = lara_item.pos.z_pos + rbounds[5];

		return bounds[1] >= DeadlyBounds[0] && bounds[0] <= DeadlyBounds[1] &&
			bounds[3] >= DeadlyBounds[2] && bounds[2] <= DeadlyBounds[3] &&
			bounds[5] >= DeadlyBounds[4] && bounds[4] <= DeadlyBounds[5];
	}*/
}
