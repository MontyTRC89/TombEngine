#include "framework.h"
#include "tr4_cog.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
    void CogControl(short itemNum)
    {
        ITEM_INFO* item = &g_Level.Items[itemNum];

        if (TriggerActive(item))
        {
            item->Status = ITEM_ACTIVE;
            AnimateItem(item);

            if (item->TriggerFlags == 666)
            {
                PHD_VECTOR pos;
                GetJointAbsPosition(item, &pos, 0);
                SoundEffect(65, (PHD_3DPOS*)&pos, 0);

                if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
                    item->Flags &= 0xC1;
            }
        }
        else if (item->TriggerFlags == 2)
        {
            item->Status |= ITEM_INVISIBLE;
        }
    }

    void CogCollision(__int16 itemNumber, ITEM_INFO* l, COLL_INFO* coll)
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];
        
        if (item->Status != ITEM_INVISIBLE)
        {
            if (TestBoundsCollide(item, l, coll->Setup.Radius))
            {
                if (TriggerActive(item))
                {
                    DoBloodSplat(
                        (GetRandomControl() & 0x3F) + l->Position.xPos - 32, 
                        (GetRandomControl() & 0x1F) + item->Position.yPos - 16, 
                        (GetRandomControl() & 0x3F) + l->Position.zPos - 32, 
                        (GetRandomControl() & 3) + 2, 
                        2 * GetRandomControl(),
                        l->RoomNumber);
                    LaraItem->HitPoints -= 10;
                }
                else if (coll->Setup.EnableObjectPush)
                {
                    ItemPushItem(item, l, coll, 0, 0);
                }
            }
        }
    }
}