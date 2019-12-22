#include "../newobjects.h"
#include "../../Game/items.h"

void InitialiseSubmarine(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = Anims[item->animNumber].frameBase;
    item->goalAnimState = 0;
    item->currentAnimState = 0;
    if (item->triggerFlags)
        item->triggerFlags = 120;
}