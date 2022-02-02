#include "framework.h"
#include "tr5_willowwisp.h"
#include "Game/items.h"
#include "Specific/setup.h"
#include "Specific/level.h"

void InitialiseLightingGuide(short itemNum)
{
    ITEM_INFO* item;

    item = &g_Level.Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
    item->targetState = 1;
    item->activeState = 1;
}