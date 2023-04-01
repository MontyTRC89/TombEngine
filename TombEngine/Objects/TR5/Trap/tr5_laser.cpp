#include "framework.h"
#include "Objects/TR5/Trap/tr5_laser.h"
#include "Objects/TR5/Trap/tr5_laser_info.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"

static LaserStructInfo& GetLaserInfo(ItemInfo& item)
{
	return (LaserStructInfo&)item.Data;
}

void InitialiseLasers(short itemNumber)
{

	auto* item = &g_Level.Items[itemNumber];
	auto* laser = &GetLaserInfo(*item);

    short xAdd, height, zAdd, Lh, yAdd, hAdd, width; //width,
	short roomNumber;

		width = (item->TriggerFlags & 255) * SECTOR(1);

		if (!(item->TriggerFlags & 1))
		{
			xAdd = (width / 2) - CLICK(2);

			item->Pose.Position.z += short(xAdd * phd_cos(item->Pose.Orientation.y + ANGLE(180.0f)));// >> 14;  //32768 = ANGLE(180.0f))
			item->Pose.Position.x += short(xAdd * phd_sin(item->Pose.Orientation.y + ANGLE(180.0f)));// >> 14;
		}

		if ((item->TriggerFlags & 255) == 1)
			item->ItemFlags[1] = 1;

		roomNumber = item->RoomNumber;

		auto floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
		item->Pose.Position.y = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
		item->ItemFlags[0] = short(item->Pose.Position.y - GetCeiling(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z));
		height = item->ItemFlags[0];
		item->TriggerFlags >>= 8;
		yAdd = height / 8;

		zAdd = abs(int(width * phd_cos(item->Pose.Orientation.y + ANGLE(180.0f))));// >> 15);
		xAdd = abs(int(width * phd_sin(item->Pose.Orientation.y + ANGLE(180.0f))));// >> 15);
		Lh = yAdd >> 1;
		height = -yAdd;


		//move vertex position to Laser struct
		for (int i = 0; i < 3; i++)
		{
			hAdd = (Lh >> 1) * (i - 1);
			laser->vert1[i].x = short(xAdd);
			laser->vert1[i].y = short(height - Lh + hAdd);
			laser->vert1[i].z = short(zAdd);
			laser->vert2[i].x = short(-xAdd);
			laser->vert2[i].y = short(height - Lh + hAdd);
			laser->vert2[i].z = short(-zAdd);
			laser->vert3[i].x = short(-xAdd);
			laser->vert3[i].y = short(height + Lh + hAdd);
			laser->vert3[i].z = short(-zAdd);
			laser->vert4[i].x = short(xAdd);
			laser->vert4[i].y = short(height + Lh + hAdd);
			laser->vert4[i].z = short(zAdd);
			height -= yAdd * 3;
		}

		for (int i = 0; i < 18; i++)
			laser->Rand[i] = short(GetRandomControl() << 1);
	}

	void ControlLasers(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* laser = &GetLaserInfo(*item);

		if (!TriggerActive(item))
			return;
		//including sound later
		//SoundEffect(SFX_RICH_DOOR_BEAM, &item->pos, SFX_DEFAULT);

		if (item->ItemFlags[3])
			item->ItemFlags[3] -= 2;
		
		//TODO: make collision

	}
