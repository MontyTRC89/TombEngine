#include "../newobjects.h"
#include "../../Game/items.h"

void InitialiseGladiator(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = Anims[item->animNumber].frameBase;
    item->goalAnimState = 1;
    item->currentAnimState = 1;
    if (item->triggerFlags == 1)
        *item->pad2 = -1;
}