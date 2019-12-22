#include "../newobjects.h"
#include "../../Game/items.h"

void InitialiseReaper(short itemNum)
{
    ITEM_INFO* item;

    ClearItem(itemNum);
    item = &Items[itemNum];
    item->animNumber = Objects[item->objectNumber].animIndex + 1;
    item->frameNumber = Anims[item->animNumber].frameBase;
    item->goalAnimState = 2;
    item->currentAnimState = 2;
}