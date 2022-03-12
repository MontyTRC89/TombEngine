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

        item->Position.yRot = GetRandomControl() * 1024;
        item->ItemFlags[2] = GetRandomControl() & 1;

        auto probe = GetCollisionResult(item);

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
        SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP1, &item->Position, 0);
        GetFrame(LaraItem, framePtr, &rate);

        int dy = LaraItem->Position.yPos + framePtr[0]->boundingBox.Y1;
        int dl = 3328 * item->ItemFlags[1] / 4096;

        if (LaraItem->HitPoints > 0)
        {
            if (item->Position.yPos + dl > dy)
            {
                if (abs(item->Position.xPos - LaraItem->Position.xPos) < CLICK(2))
                {
                    if (abs(item->Position.zPos - LaraItem->Position.zPos) < CLICK(2))
                    {
                        int x = (GetRandomControl() & 0x7F) + LaraItem->Position.xPos - 64;
                        int y = dy + GetRandomControl() % (item->Position.yPos - dy + dl);
                        int z = (GetRandomControl() & 0x7F) + LaraItem->Position.zPos - 64;

                        DoBloodSplat(x, y, z, (GetRandomControl() & 3) + 2, 2 * GetRandomControl(), item->RoomNumber);
                        LaraItem->HitPoints -= 8;
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

        item->Position.yRot += item->ItemFlags[0];

		// Update bone mutators
		if (item->ItemFlags[1])
		{
			for (int i = 0; i < item->Animation.Mutator.size(); i++)
				item->Animation.Mutator[i].Scale = Vector3(1.0f, item->ItemFlags[1] / 4096.0f, 1.0f);
		}
    }
}
