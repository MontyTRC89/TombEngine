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

		int dx = phd_sin(item->Pose.Orientation.y + ANGLE(90.0f)) * 512;
		int dz = phd_cos(item->Pose.Orientation.y + ANGLE(90.0f)) * 512;

		item->Pose.Position.x += dx;
		item->Pose.Position.z += dz;

		item->ItemFlags[0] = item->Pose.Position.x / 256;
		item->ItemFlags[1] = (item->Pose.Position.y - 4608) / 256;
		item->ItemFlags[2] = item->Pose.Position.z / 256;
		item->ItemFlags[3] = 50;
	}

	void SlicerDicerControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP1, &item->Pose);
		SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP2, &item->Pose);

		item->Pose.Position.x = (item->ItemFlags[0] * 256) + 4608 * phd_cos(item->TriggerFlags) * phd_sin(item->Pose.Orientation.y);
		item->Pose.Position.y = (item->ItemFlags[1] * 256) - 4608 * phd_sin(item->TriggerFlags);
		item->Pose.Position.z = (item->ItemFlags[2] * 256) + 4608 * phd_cos(item->TriggerFlags) * phd_cos(item->Pose.Orientation.y);

		item->TriggerFlags += 170;

		auto probedRoomNumber = GetCollision(item).RoomNumber;
		if (item->RoomNumber != probedRoomNumber)
			ItemNewRoom(itemNumber, probedRoomNumber);

		AnimateItem(item);
	}
}
