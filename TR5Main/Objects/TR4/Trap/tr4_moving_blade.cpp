#include "framework.h"
#include "tr4_moving_blade.h"
#include "level.h"
#include "control.h"
#include <sound.h>
#include <draw.h>
#include <lara.h>
#include <Game\sphere.h>
#include <Game\effect2.h>

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