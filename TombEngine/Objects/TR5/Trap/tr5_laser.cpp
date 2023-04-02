#include "framework.h"
#include "Objects/TR5/Trap/tr5_laser.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"

std::vector<LaserStructInfo> Lasers = {};

void InitialiseLasers(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	
	auto laser = LaserStructInfo();
	short xAdd, height, zAdd, Lh, yAdd, hAdd, width;
	short roomNumber;

	//negative OCB means green laser, positive red laser. OCB value defines the width in blocks
	if (item->TriggerFlags >= 0)
		width =  item->TriggerFlags * BLOCK(1);
	else
		width = abs(item->TriggerFlags) * BLOCK(1);

	if (!(item->TriggerFlags & 1))
	{
		xAdd = (width / 2) - BLOCK(0.5);

		item->Pose.Position.z += xAdd * phd_cos(item->Pose.Orientation.y + ANGLE(TO_RAD(180.0f))) / 16384;
		item->Pose.Position.x += xAdd * phd_sin(item->Pose.Orientation.y + ANGLE(TO_RAD(180.0f))) / 16384;
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

	zAdd = abs((width * phd_cos(item->Pose.Orientation.y )) / 2 );
	xAdd = abs((width * phd_sin(item->Pose.Orientation.y )) / 2 );
	Lh = yAdd >> 1;
	height = -yAdd;

	//move vertex position to Laser struct
	for (int i = 0; i < 3; i++)
	{
		hAdd = (Lh >> 1) * (i - 1);
		laser.vert1[i].x = item->Pose.Position.x + xAdd;
		laser.vert1[i].y = item->Pose.Position.y + (height - Lh + hAdd);
		laser.vert1[i].z = item->Pose.Position.z + zAdd;
		laser.vert2[i].x = item->Pose.Position.x + (-xAdd);
		laser.vert2[i].y = item->Pose.Position.y + (height - Lh + hAdd);
		laser.vert2[i].z = item->Pose.Position.z + (-zAdd);
		laser.vert3[i].x = item->Pose.Position.x + (-xAdd);
		laser.vert3[i].y = item->Pose.Position.y + (height + Lh + hAdd);
		laser.vert3[i].z = item->Pose.Position.z + (-zAdd);
		laser.vert4[i].x = item->Pose.Position.x + xAdd;
		laser.vert4[i].y = item->Pose.Position.y + (height + Lh + hAdd);
		laser.vert4[i].z = item->Pose.Position.z + zAdd;
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


	}

}

void RemoveLasers()
{
		// Despawn inactive effects.
		Lasers.erase(
			std::remove_if(
				Lasers.begin(), Lasers.end(),
				[](const LaserStructInfo& laser) { return (laser.life <= 0.0f); }), Lasers.end());
}

	void ControlLasers(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!TriggerActive(item))
			return;

		//including sound later
		//SoundEffect(SFX_RICH_DOOR_BEAM, &item->pos, SFX_DEFAULT);

		if (item->ItemFlags[3])
			item->ItemFlags[3] -= 2;
		
		//TODO: make collision
	}
