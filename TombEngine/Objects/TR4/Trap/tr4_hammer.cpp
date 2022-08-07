#include "framework.h"
#include "tr4_element_puzzle.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Objects/Generic/Switches/switch.h"
#include "Specific/input.h"
#include <Game/effects/debris.h>

namespace TEN::Entities::TR4
{

    enum HammerState
    {
        HAMMER_STATE_NONE = 0,
        HAMMER_STATE_IDLE = 1,
        HAMMER_STATE_ACTIVE = 2,
    };

    enum HammerAnim
    {
        HAMMER_ANIM_NOT_ACTIVED = 0,
        HAMMER_ANIM_ACTIVATED = 1
    };


    void HammerControl(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];

        int frameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
        item->ItemFlags[3] = 150;

        if (!TriggerActive(item))
        {
            *(long*)&item->ItemFlags[0] = 0;
            return;
        }

        int hammerTouched = 0;

        if (!item->TriggerFlags)
        {
            if (frameNumber < 52)
                *(long*)&item->ItemFlags[0] = 0xE0;
            else
                *(long*)&item->ItemFlags[0] = 0;
        }
        else if (item->Animation.ActiveState == HAMMER_STATE_IDLE && item->Animation.TargetState == HAMMER_STATE_IDLE)
        {
            if (item->ItemFlags[2])
            {
                if (item->TriggerFlags == 3)
                {
                    item->Flags &= ~CODE_BITS;
                    item->ItemFlags[2] = 0;
                }
                else if (item->TriggerFlags == 4)
                    item->ItemFlags[2]--;
                else
                    item->ItemFlags[2] = 0;
            }
            else
            {
                item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
                item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
                item->Animation.ActiveState = HAMMER_STATE_ACTIVE;
                item->Animation.TargetState = HAMMER_STATE_ACTIVE;
                item->ItemFlags[2] = 60;
            }
        }
        else
        {
            item->Animation.TargetState = HAMMER_STATE_IDLE;

            if (frameNumber < 52)
                *(long*)&item->ItemFlags[0] = 0x7E0;
            else
                *(long*)&item->ItemFlags[0] = 0;

            if (frameNumber == 8)
            {
                if (item->TriggerFlags == 2)
                {
                    short targetItem = g_Level.Rooms[item->RoomNumber].itemNumber;

                    if (targetItem != NO_ITEM)
                    {
                        auto* target = &g_Level.Items[targetItem];
                        for (; targetItem != NO_ITEM; targetItem = target->NextItem)
                        {
                            target = &g_Level.Items[targetItem];

                            if (target->ObjectNumber == ID_OBELISK && target->Pose.Orientation.y == -ANGLE(270) &&
                                g_Level.Items[target->ItemFlags[0]].Pose.Orientation.y == ANGLE(90) &&
                                g_Level.Items[target->ItemFlags[1]].Pose.Orientation.y == 0)
                            {
                                target->Flags |= CODE_BITS;
                                g_Level.Items[target->ItemFlags[0]].Flags |= CODE_BITS;
                                g_Level.Items[target->ItemFlags[1]].Flags |= CODE_BITS;
                                break;
                            }
                        }
                    }

                    SoundEffect(SFX_TR4_GENERIC_HEAVY_THUD, &item->Pose);
                    SoundEffect(SFX_TR4_EXPLOSION2, &item->Pose);
                }
                else
                {
                    short targetItem = g_Level.Rooms[item->RoomNumber].itemNumber;

                    if (targetItem != NO_ITEM)
                    {
                        auto* target = &g_Level.Items[targetItem];
                        for (; targetItem != NO_ITEM; targetItem = target->NextItem)
                        {
                            target = &g_Level.Items[targetItem];

                            if (target->ObjectNumber >= ID_PUSHABLE_OBJECT1 && target->ObjectNumber <= ID_PUSHABLE_OBJECT10)
                            {
                                if (item->Pose.Position.x == target->Pose.Position.x &&
                                    item->Pose.Position.z == target->Pose.Position.z)
                                {
                                    ExplodeItemNode(target, 0, 0, 128);
                                    KillItem(targetItem);
                                    hammerTouched = 1;
                                }
                            }
                        }
                    }

                    if (hammerTouched)
                    {
                        targetItem = g_Level.Rooms[item->RoomNumber].itemNumber;

                        if (targetItem != NO_ITEM)
                        {
                            auto* target = &g_Level.Items[targetItem];
                            for (; targetItem != NO_ITEM; targetItem = target->NextItem)
                            {
                                target = &g_Level.Items[targetItem];

                                //changed to take all puzzle items, keys and their combos. Original is hardcoded to a few slots. -Troye
                                if ((target->ObjectNumber >= ID_PUZZLE_ITEM1 && target->ObjectNumber <= ID_PUZZLE_ITEM16) ||
                                    (target->ObjectNumber >= ID_PUZZLE_ITEM1_COMBO1 && target->ObjectNumber <= ID_PUZZLE_ITEM16_COMBO2) ||
                                    (target->ObjectNumber >= ID_KEY_ITEM1 && target->ObjectNumber <= ID_KEY_ITEM16) ||
                                    (target->ObjectNumber >= ID_KEY_ITEM1_COMBO1 && target->ObjectNumber <= ID_KEY_ITEM16_COMBO2))
                                {
                                    if (item->Pose.Position.x == target->Pose.Position.x &&
                                        item->Pose.Position.z == target->Pose.Position.z)
                                    {
                                        target->Status = ITEM_NOT_ACTIVE;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if (frameNumber > 52 && item->TriggerFlags == 2)
                item->Flags &= ~CODE_BITS;
        }

        AnimateItem(item);
    }
}
