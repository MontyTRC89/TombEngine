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
            item->status = ITEM_ACTIVE;
            AnimateItem(item);

            if (item->triggerFlags == 666)
            {
                PHD_VECTOR pos;
                GetJointAbsPosition(item, &pos, 0);
                SoundEffect(65, (PHD_3DPOS*)&pos, 0);

                if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
                    item->flags &= 0xC1;
            }
        }
        else if (item->triggerFlags == 2)
        {
            item->status |= ITEM_INVISIBLE;
        }
    }

    void CogCollision(__int16 itemNumber, ITEM_INFO* l, COLL_INFO* coll)
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];
        
        if (item->status != ITEM_INVISIBLE)
        {
            if (TestBoundsCollide(item, l, coll->Setup.Radius))
            {
                if (TriggerActive(item))
                {
                    DoBloodSplat(
                        (GetRandomControl() & 0x3F) + l->pos.xPos - 32, 
                        (GetRandomControl() & 0x1F) + item->pos.yPos - 16, 
                        (GetRandomControl() & 0x3F) + l->pos.zPos - 32, 
                        (GetRandomControl() & 3) + 2, 
                        2 * GetRandomControl(),
                        l->roomNumber);
                    LaraItem->hitPoints -= 10;
                }
                else if (coll->Setup.EnableObjectPush)
                {
                    ItemPushItem(item, l, coll, 0, 0);
                }
            }
        }
    }
}