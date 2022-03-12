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
    void CogControl(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];

        if (TriggerActive(item))
        {
            item->Status = ITEM_ACTIVE;
            AnimateItem(item);

            if (item->TriggerFlags == 666)
            {
                PHD_VECTOR pos;
                GetJointAbsPosition(item, &pos, 0);
                SoundEffect(65, (PHD_3DPOS*)&pos, 0);

                if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
                    item->Flags &= 0xC1;
            }
        }
        else if (item->TriggerFlags == 2)
            item->Status |= ITEM_INVISIBLE;
    }

    void CogCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
    {
        auto* cogItem = &g_Level.Items[itemNumber];
        
        if (cogItem->Status != ITEM_INVISIBLE)
        {
            if (TestBoundsCollide(cogItem, laraItem, coll->Setup.Radius))
            {
                if (TriggerActive(cogItem))
                {
                    DoBloodSplat(
                        (GetRandomControl() & 0x3F) + laraItem->Position.xPos - 32, 
                        (GetRandomControl() & 0x1F) + cogItem->Position.yPos - 16, 
                        (GetRandomControl() & 0x3F) + laraItem->Position.zPos - 32, 
                        (GetRandomControl() & 3) + 2, 
                        2 * GetRandomControl(),
                        laraItem->RoomNumber);

                    laraItem->HitPoints -= 10;
                }
                else if (coll->Setup.EnableObjectPush)
                    ItemPushItem(cogItem, laraItem, coll, 0, 0);
            }
        }
    }
}
