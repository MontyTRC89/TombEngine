#include "framework.h"
#include "tr4_element_puzzle.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/input.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"

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

        if (item->TriggerFlags == 1)
        {
            SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Position, 0);

            byte r = (GetRandomControl() & 0x3F) + 192;
            byte g = (GetRandomControl() & 0x1F) + 96;
            byte b = 0;
            int on = 0;

            if (item->ItemFlags[3])
            {
                item->ItemFlags[3]--;
                on = 255 - GetRandomControl() % (4 * (91 - item->ItemFlags[3]));
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

            AddFire(item->Position.xPos, item->Position.yPos - 620, item->Position.zPos, 1, item->RoomNumber, on);
            TriggerDynamicLight(item->Position.xPos, item->Position.yPos - 768, item->Position.zPos, 12, r, g, b);
            return;
        }

        if (item->TriggerFlags != 3)
        {
            return;
        }

        if (item->ItemFlags[1] > 90)
        {
            SoundEffect(SFX_TR4_JOBY_WIND, &item->Position, 0);
        }

        if (item->ItemFlags[1] < 60)
        {
            item->ItemFlags[1]++;
            return;
        }

        item->ItemFlags[0]++;

        if (item->ItemFlags[0] == 90)
        {
            short itemNos[256];
            int sw = GetSwitchTrigger(item, itemNos, 0);
            if (sw > 0)
            {
                for (int i = 0; i < sw; i++)
                {
                    AddActiveItem(itemNos[i]);
                    g_Level.Items[itemNos[i]].Status = ITEM_ACTIVE;
                    g_Level.Items[itemNos[i]].Flags |= 0x3E00;
                }
            }

            KillItem(itemNumber);
            return;
        }

        short currentItemNumber = g_Level.Rooms[item->RoomNumber].itemNumber;
        if (currentItemNumber == NO_ITEM)
        {
            return;
        }

        while (currentItemNumber != NO_ITEM)
        {
            ITEM_INFO* currentItem = &g_Level.Items[currentItemNumber];

            if (currentItem->ObjectNumber != ID_FLAME_EMITTER2)
            {
                if (currentItem->ObjectNumber == ID_ELEMENT_PUZZLE && currentItem->TriggerFlags == 1 && !currentItem->ItemFlags[3])
                {
                    currentItem->ItemFlags[3] = 90;
                }
                currentItemNumber = currentItem->NextItem;
                continue;
            }

            if (item->ItemFlags[0] != 89)
            {
                currentItem->ItemFlags[3] = 255 - GetRandomControl() % (4 * item->ItemFlags[0]);
                if (currentItem->ItemFlags[3] >= 2)
                {
                    currentItemNumber = currentItem->NextItem;
                    continue;
                }
                currentItem->ItemFlags[3] = 2;
            }

            RemoveActiveItem(currentItemNumber);
            currentItem->Status = ITEM_NOT_ACTIVE;
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

        if (item->TriggerFlags)
        {
            if (item->TriggerFlags == 1)
            {
                flags = 26;
            }
            else
            {
                if (item->TriggerFlags != 2)
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

        if ((l->AnimNumber == LA_WATERSKIN_POUR_LOW
            || l->AnimNumber == LA_WATERSKIN_POUR_HIGH)
            && !item->ItemFlags[0])
        {
            BOUNDING_BOX* box = GetBoundsAccurate(item);

            ElementPuzzleBounds.boundingBox.X1 = box->X1;
            ElementPuzzleBounds.boundingBox.X2 = box->X2;
            ElementPuzzleBounds.boundingBox.Z1 = box->Z1 - 200;
            ElementPuzzleBounds.boundingBox.Z2 = box->Z2 + 200;

            short oldRot = item->Position.yRot;
            item->Position.yRot = l->Position.yRot;

            if (TestLaraPosition(&ElementPuzzleBounds, item, l))
            {
                if (l->AnimNumber == LA_WATERSKIN_POUR_LOW && LaraItem->ItemFlags[2] == flags)
                {
                    l->AnimNumber = LA_WATERSKIN_POUR_HIGH;
                    l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
                }

                if (l->FrameNumber == g_Level.Anims[LA_WATERSKIN_POUR_HIGH].frameBase + 74
                    && LaraItem->ItemFlags[2] == flags)
                {
                    if (!item->TriggerFlags)
                    {
                        item->MeshBits = 48;
                        TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
                        item->ItemFlags[0] = 1;
                        item->Position.yRot = oldRot;

                        return;
                    }

                    if (item->TriggerFlags == 1)
                    {
                        item->MeshBits = 3;
                        Lara.Pickups[1]--;
                        item->ItemFlags[0] = 1;
                        item->Position.yRot = oldRot;
                        return;
                    }

                    item->MeshBits = 12;
                    TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
                    Lara.Pickups[0]--;
                    item->ItemFlags[0] = 1;

                }
            }

            item->Position.yRot = oldRot;
        }
        else
        {
            if (Lara.gunType != WEAPON_TORCH
                || Lara.gunStatus != LG_READY
                || Lara.LeftArm.lock
                || !(TrInput & IN_ACTION)
                || item->TriggerFlags != 1 
                || item->ItemFlags[0] != 1
                || l->ActiveState != LS_IDLE
                || l->AnimNumber != LA_STAND_IDLE
                || !Lara.litTorch
                || l->Airborne)
            {
                if (l->AnimNumber != LA_TORCH_LIGHT_3
                    || g_Level.Anims[LA_TORCH_LIGHT_3].frameBase + 16
                    || item->ItemFlags[0] != 2)
                {
                    ElementPuzzleDoCollision(itemNumber, l, c);
                }
                else
                {
                    TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
                    AddActiveItem(itemNumber);
                    item->Status = ITEM_ACTIVE;
                    item->ItemFlags[0] = 3;
                    item->Flags |= 0x3E00;
                }
            }
            else
            {
                BOUNDING_BOX* box = GetBoundsAccurate(item);

                ElementPuzzleBounds.boundingBox.X1 = box->X1;
                ElementPuzzleBounds.boundingBox.X2 = box->X2;
                ElementPuzzleBounds.boundingBox.Z1 = box->Z1 - 200;
                ElementPuzzleBounds.boundingBox.Z2 = box->Z2 + 200;

                short oldRot = item->Position.yRot;
                item->Position.yRot = l->Position.yRot;

                if (TestLaraPosition(&ElementPuzzleBounds, item, l))
                {
                    l->AnimNumber = (abs(item->Position.yPos - l->Position.yPos) >> 8) + LA_TORCH_LIGHT_3;
                    l->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
                    l->ActiveState = LS_MISC_CONTROL;
                    Lara.Flare.ControlLeft = false;
                    Lara.LeftArm.lock = true;
                    item->ItemFlags[0] = 2;
                }
                item->Position.yRot = oldRot;
            }
        }
    }

    void InitialiseElementPuzzle(short itemNumber)
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];

        if (item->TriggerFlags)
        {
            if (item->TriggerFlags == 1)
            {
                item->MeshBits = 65;
            }
            else if (item->TriggerFlags == 2)
            {
                item->MeshBits = 68;
            }
            else
            {
                item->MeshBits = 0;
            }
        }
        else
        {
            item->MeshBits = 80;
        }
    }
}