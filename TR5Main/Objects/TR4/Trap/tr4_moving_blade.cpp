#include "framework.h"
#include "tr4_moving_blade.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
    void MovingBladeControl(short itemNumber)
    {
        auto* item = &g_Level.Items[itemNumber];

        if (TriggerActive(item))
        {
            item->ItemFlags[3] = 50;
            AnimateItem(item);
        }
        else
            item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
    }
}
