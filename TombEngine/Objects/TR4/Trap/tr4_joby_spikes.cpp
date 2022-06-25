#include "framework.h"
#include "tr4_joby_spikes.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Sound/sound.h"
#include "Game/Lara/lara.h"
#include "Game/effects/effects.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
    void InitialiseJobySpikes(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];

		// Set bone mutators to 0 by default
		for (int i = 0; i < item->Animation.Mutator.size(); i++)
			item->Animation.Mutator[i].Scale.y = 0.0f;

        item->Pose.Orientation.y = GetRandomControl() * 1024;
        item->ItemFlags[2] = GetRandomControl() & 1;

        auto probe = GetCollision(item);

        // TODO: check this optimized division
        //v6 = 1321528399i64 * ((probe.Position.Floor - probe.Position.Ceiling) << 12);
        //item->itemFlags[3] = (HIDWORD(v6) >> 31) + (SHIDWORD(v6) >> 10);

        item->ItemFlags[3] = (short)((probe.Position.Floor - probe.Position.Ceiling) * 1024 * 12 / 13);
    }

    void JobySpikesControl(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];

        if (!TriggerActive(item))
            return;

        ANIM_FRAME* framePtr[2];
        int rate;
        SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP1, &item->Pose);
        GetFrame(LaraItem, framePtr, &rate);

        int dy = LaraItem->Pose.Position.y + framePtr[0]->boundingBox.Y1;
        int dl = 3328 * item->ItemFlags[1] / 4096;

        if (LaraItem->HitPoints > 0)
        {
            if (item->Pose.Position.y + dl > dy)
            {
                if (abs(item->Pose.Position.x - LaraItem->Pose.Position.x) < CLICK(2))
                {
                    if (abs(item->Pose.Position.z - LaraItem->Pose.Position.z) < CLICK(2))
                    {
                        int x = (GetRandomControl() & 0x7F) + LaraItem->Pose.Position.x - 64;
                        int y = dy + GetRandomControl() % (item->Pose.Position.y - dy + dl);
                        int z = (GetRandomControl() & 0x7F) + LaraItem->Pose.Position.z - 64;

                        DoBloodSplat(x, y, z, (GetRandomControl() & 3) + 2, 2 * GetRandomControl(), item->RoomNumber);
                        DoDamage(LaraItem, 8);
                    }
                }
            }
        }

        if (item->ItemFlags[2])
        {
            if (item->ItemFlags[0] > -4096)
                item->ItemFlags[0] = item->ItemFlags[1] + (item->ItemFlags[1] / 64) - 2;
        }
        else if (item->ItemFlags[0] < 4096)
            item->ItemFlags[0] = (item->ItemFlags[1] / 64) + item->ItemFlags[1] + 2;

        if (item->ItemFlags[1] < item->ItemFlags[3])
            item->ItemFlags[1] += 3;

        item->Pose.Orientation.y += item->ItemFlags[0];

		// Update bone mutators
		if (item->ItemFlags[1])
		{
			for (int i = 0; i < item->Animation.Mutator.size(); i++)
				item->Animation.Mutator[i].Scale = Vector3(1.0f, item->ItemFlags[1] / 4096.0f, 1.0f);
		}
    }
}
