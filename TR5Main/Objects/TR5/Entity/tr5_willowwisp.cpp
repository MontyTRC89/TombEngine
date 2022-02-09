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
    item->AnimNumber = Objects[item->ObjectNumber].animIndex;
    item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
    item->TargetState = 1;
    item->ActiveState = 1;
}