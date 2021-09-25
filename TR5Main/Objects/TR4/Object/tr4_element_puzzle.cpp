#include "framework.h"
#include "tr4_element_puzzle.h"
#include "level.h"
#include "control/control.h"
#include "Sound/sound.h"
#include "animation.h"
#include "lara.h"
#include "sphere.h"
#include "effects/effects.h"
#include "effects/tomb4fx.h"
#include "input.h"
#include "generic_switch.h"
#include "collide.h"
#include "items.h"

using namespace TEN::Entities::Switches;

namespace TEN::Entities::TR4
{
    OBJECT_COLLISION_BOUNDS ElementPuzzleBounds = {
        0, 0, 
        -64, 0, 
        0, 0,
        -ANGLE(10), ANGLE(10), 
        -ANGLE(30), ANGLE(30), 
        -ANGLE(10), ANGLE(10)
    };

    void ElementPuzzleControl(short itemNumber)
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];

        if (!TriggerActive(item))
            return;

        if (item->triggerFlags == 1)
        {
            SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->pos, 0);

            byte r = (GetRandomControl() & 0x3F) + 192;
            byte g = (GetRandomControl() & 0x1F) + 96;
            byte b = 0;
            int on = 0;

            if (item->itemFlags[3])
            {
                item->itemFlags[3]--;
                on = 255 - GetRandomControl() % (4 * (91 - item->itemFlags[3]));
                if (on < 1)
                {
                    on = 1;
                    r = (r * on) / 256;
                    g = (g * on) / 256;
                }
                else if (on <= 255)
                {
                    r = (r * on) / 256;
                    g = (g * on) / 256;
                }
            }
            else
            {
                on = 0;
            }

            AddFire(item->pos.xPos, item->pos.yPos - 620, item->pos.zPos, 1, item->roomNumber, on);
            TriggerDynamicLight(item->pos.xPos, item->pos.yPos - 768, item->pos.zPos, 12, r, g, b);
            return;
        }

        if (item->triggerFlags != 3)
        {
            return;
        }

        if (item->itemFlags[1] > 90)
        {
            SoundEffect(SFX_TR4_JOBY_WIND, &item->pos, 0);
        }

        if (item->itemFlags[1] < 60)
        {
            item->itemFlags[1]++;
            return;
        }

        item->itemFlags[0]++;

        if (item->itemFlags[0] == 90)
        {
            short itemNos[256];
            int sw = GetSwitchTrigger(item, itemNos, 0);
            if (sw > 0)
            {
                for (int i = 0; i < sw; i++)
                {
                    AddActiveItem(itemNos[i]);
                    g_Level.Items[itemNos[i]].status = ITEM_ACTIVE;
                    g_Level.Items[itemNos[i]].flags |= 0x3E00;
                }
            }

            KillItem(itemNumber);
            return;
        }

        short currentItemNumber = g_Level.Rooms[item->roomNumber].itemNumber;
        if (currentItemNumber == NO_ITEM)
        {
            return;
        }

        while (currentItemNumber != NO_ITEM)
        {
            ITEM_INFO* currentItem = &g_Level.Items[currentItemNumber];

            if (currentItem->objectNumber != ID_FLAME_EMITTER2)
            {
                if (currentItem->objectNumber == ID_ELEMENT_PUZZLE && currentItem->triggerFlags == 1 && !currentItem->itemFlags[3])
                {
                    currentItem->itemFlags[3] = 90;
                }
                currentItemNumber = currentItem->nextItem;
                continue;
            }

            if (item->itemFlags[0] != 89)
            {
                currentItem->itemFlags[3] = 255 - GetRandomControl() % (4 * item->itemFlags[0]);
                if (currentItem->itemFlags[3] >= 2)
                {
                    currentItemNumber = currentItem->nextItem;
                    continue;
                }
                currentItem->itemFlags[3] = 2;
            }

            RemoveActiveItem(currentItemNumber);
            currentItem->status = ITEM_NOT_ACTIVE;
        }
    }

    void ElementPuzzleDoCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];

        if (TestBoundsCollide(item, l, coll->Setup.Radius))
        {
            if (TestCollision(item, l))
            {
                if (coll->Setup.EnableObjectPush)
                {
                    ItemPushItem(item, l, coll, 0, 0);
                }
            }
        }
    }

    void ElementPuzzleCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c)
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];

        int flags = 0;

        if (item->triggerFlags)
        {
            if (item->triggerFlags == 1)
            {
                flags = 26;
            }
            else
            {
                if (item->triggerFlags != 2)
                {
                    return;
                }
                flags = 27;
            }
        }
        else
        {
            flags = 25;
        }

        if ((l->animNumber == LA_WATERSKIN_POUR_LOW
            || l->animNumber == LA_WATERSKIN_POUR_HIGH)
            && !item->itemFlags[0])
        {
            BOUNDING_BOX* box = GetBoundsAccurate(item);

            ElementPuzzleBounds.boundingBox.X1 = box->X1;
            ElementPuzzleBounds.boundingBox.X2 = box->X2;
            ElementPuzzleBounds.boundingBox.Z1 = box->Z1 - 200;
            ElementPuzzleBounds.boundingBox.Z2 = box->Z2 + 200;

            short oldRot = item->pos.yRot;
            item->pos.yRot = l->pos.yRot;

            if (TestLaraPosition(&ElementPuzzleBounds, item, l))
            {
                if (l->animNumber == LA_WATERSKIN_POUR_LOW && LaraItem->itemFlags[2] == flags)
                {
                    l->animNumber = LA_WATERSKIN_POUR_HIGH;
                    l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
                }

                if (l->frameNumber == g_Level.Anims[LA_WATERSKIN_POUR_HIGH].frameBase + 74
                    && LaraItem->itemFlags[2] == flags)
                {
                    if (!item->triggerFlags)
                    {
                        item->meshBits = 48;
                        TestTriggers(item, true, item->flags & IFLAG_ACTIVATION_MASK);
                        item->itemFlags[0] = 1;
                        item->pos.yRot = oldRot;

                        return;
                    }

                    if (item->triggerFlags == 1)
                    {
                        item->meshBits = 3;
                        Lara.Pickups[1]--;
                        item->itemFlags[0] = 1;
                        item->pos.yRot = oldRot;
                        return;
                    }

                    item->meshBits = 12;
                    TestTriggers(item, true, item->flags & IFLAG_ACTIVATION_MASK);
                    Lara.Pickups[0]--;
                    item->itemFlags[0] = 1;

                }
            }

            item->pos.yRot = oldRot;
        }
        else
        {
            if (Lara.gunType != WEAPON_TORCH
                || Lara.gunStatus != LG_READY
                || Lara.leftArm.lock
                || !(TrInput & IN_ACTION)
                || item->triggerFlags != 1 
                || item->itemFlags[0] != 1
                || l->currentAnimState != LS_STOP
                || l->animNumber != LA_STAND_IDLE
                || !Lara.litTorch
                || l->gravityStatus)
            {
                if (l->animNumber != LA_TORCH_LIGHT_3
                    || g_Level.Anims[LA_TORCH_LIGHT_3].frameBase + 16
                    || item->itemFlags[0] != 2)
                {
                    ElementPuzzleDoCollision(itemNumber, l, c);
                }
                else
                {
                    TestTriggers(item, true, item->flags & IFLAG_ACTIVATION_MASK);
                    AddActiveItem(itemNumber);
                    item->status = ITEM_ACTIVE;
                    item->itemFlags[0] = 3;
                    item->flags |= 0x3E00;
                }
            }
            else
            {
                BOUNDING_BOX* box = GetBoundsAccurate(item);

                ElementPuzzleBounds.boundingBox.X1 = box->X1;
                ElementPuzzleBounds.boundingBox.X2 = box->X2;
                ElementPuzzleBounds.boundingBox.Z1 = box->Z1 - 200;
                ElementPuzzleBounds.boundingBox.Z2 = box->Z2 + 200;

                short oldRot = item->pos.yRot;
                item->pos.yRot = l->pos.yRot;

                if (TestLaraPosition(&ElementPuzzleBounds, item, l))
                {
                    l->animNumber = (abs(item->pos.yPos - l->pos.yPos) >> 8) + LA_TORCH_LIGHT_3;
                    l->frameNumber = g_Level.Anims[item->animNumber].frameBase;
                    l->currentAnimState = LS_MISC_CONTROL;
                    Lara.flareControlLeft = false;
                    Lara.leftArm.lock = 3;
                    item->itemFlags[0] = 2;
                }
                item->pos.yRot = oldRot;
            }
        }
    }

    void InitialiseElementPuzzle(short itemNumber)
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];

        if (item->triggerFlags)
        {
            if (item->triggerFlags == 1)
            {
                item->meshBits = 65;
            }
            else if (item->triggerFlags == 2)
            {
                item->meshBits = 68;
            }
            else
            {
                item->meshBits = 0;
            }
        }
        else
        {
            item->meshBits = 80;
        }
    }
}