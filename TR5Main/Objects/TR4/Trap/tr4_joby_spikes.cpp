#include "framework.h"
#include "tr4_joby_spikes.h"
#include "level.h"
#include "control.h"
#include "animation.h"
#include "Sound/sound.h"
#include "lara.h"
#include "effects/effects.h"

void InitialiseJobySpikes(short itemNumber)
{
    ITEM_INFO* item = &g_Level.Items[itemNumber];

    item->pos.yRot = GetRandomControl() * 1024;
    item->itemFlags[2] = GetRandomControl() & 1;

    short roomNumber = item->roomNumber;
    FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
    int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

    // TODO: check this optimized division
    //v6 = 1321528399i64 * ((height - GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos)) << 12);
    //item->itemFlags[3] = (HIDWORD(v6) >> 31) + (SHIDWORD(v6) >> 10);

    item->itemFlags[3] = (height - GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos)) * 1024 * 12 / 13;
}

void JobySpikesControl(short itemNumber)
{
    ITEM_INFO* item = &g_Level.Items[itemNumber];

    if (!TriggerActive(item))
        return;

    ANIM_FRAME* framePtr;
    int rate;

    SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP1, &item->pos, 0);
    GetFrame(LaraItem, &framePtr, &rate);

    int dy = LaraItem->pos.yPos + framePtr->boundingBox.Y1;
    int dl = 3328 * item->itemFlags[1] / 4096;

    if (LaraItem->hitPoints > 0)
    {
        if (item->pos.yPos + dl > dy)
        {
            if (abs(item->pos.xPos - LaraItem->pos.xPos) < 512)
            {
                if (abs(item->pos.zPos - LaraItem->pos.zPos) < 512)
                {
                    int x = (GetRandomControl() & 0x7F) + LaraItem->pos.xPos - 64;
                    int y = dy + GetRandomControl() % (item->pos.yPos - dy + dl);
                    int z = (GetRandomControl() & 0x7F) + LaraItem->pos.zPos - 64;

                    DoBloodSplat(x, y, z, (GetRandomControl() & 3) + 2, 2 * GetRandomControl(), item->roomNumber);
                    LaraItem->hitPoints -= 8;
                }
            }
        }
    }

    if (item->itemFlags[2])
    {
        if (item->itemFlags[0] > -4096)
            item->itemFlags[0] = item->itemFlags[1] + (item->itemFlags[1] / 64) - 2;
    }
    else if (item->itemFlags[0] < 4096)
        item->itemFlags[0] = (item->itemFlags[1] / 64) + item->itemFlags[1] + 2;

    if (item->itemFlags[1] < item->itemFlags[3])
        item->itemFlags[1] += 3;

    item->pos.yRot += item->itemFlags[0];
}
