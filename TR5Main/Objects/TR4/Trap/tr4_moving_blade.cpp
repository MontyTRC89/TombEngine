#include "framework.h"
#include "tr4_moving_blade.h"
#include "level.h"
#include "control.h"
#include "Sound\sound.h"
#include "animation.h"
#include <lara.h>
#include <sphere.h>
#include <effects\effects.h>

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