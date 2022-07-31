#include "framework.h"
#include "tr4_slicerdicer.h"
#include "Specific/level.h"
#include "Sound/sound.h"
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/animation.h"
#include "Specific/trmath.h"

namespace TEN::Entities::TR4
{
	void InitialiseSlicerDicer(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		int dx = sin(item->Pose.Orientation.y + Angle::DegToRad(90.0f)) * SECTOR(0.5f);
		int dz = cos(item->Pose.Orientation.y + Angle::DegToRad(90.0f)) * SECTOR(0.5f);

		item->Pose.Position.x += dx;
		item->Pose.Position.z += dz;

		item->ItemFlags[0] = item->Pose.Position.x / CLICK(1);
		item->ItemFlags[1] = (item->Pose.Position.y - 4608) / CLICK(1);
		item->ItemFlags[2] = item->Pose.Position.z / CLICK(1);
		item->ItemFlags[3] = 50;
	}

	void SlicerDicerControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP1, &item->Pose);
		SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP2, &item->Pose);

		item->Pose.Position.x = (item->ItemFlags[0] * 256) + 4608 * cos(item->TriggerFlags) * sin(item->Pose.Orientation.y);
		item->Pose.Position.y = (item->ItemFlags[1] * 256) - 4608 * sin(item->TriggerFlags);
		item->Pose.Position.z = (item->ItemFlags[2] * 256) + 4608 * cos(item->TriggerFlags) * cos(item->Pose.Orientation.y);

		item->TriggerFlags += 170;

		auto probedRoomNumber = GetCollision(item).RoomNumber;
		if (item->RoomNumber != probedRoomNumber)
			ItemNewRoom(itemNumber, probedRoomNumber);

		AnimateItem(item);
	}
}
