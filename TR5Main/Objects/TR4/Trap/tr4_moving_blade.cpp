#include "framework.h"
#include "tr4_moving_blade.h"
#include "level.h"
#include "control/control.h"
#include "Sound/sound.h"
#include "animation.h"
#include "lara.h"
#include "Game/collision/sphere.h"
#include "effects/effects.h"
#include "items.h"

namespace TEN::Entities::TR4
{
    void MovingBladeControl(short itemNumber)
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];

        if (TriggerActive(item))
        {
            item->itemFlags[3] = 50;
            AnimateItem(item);
        }
        else
        {
            item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
        }
    }
}