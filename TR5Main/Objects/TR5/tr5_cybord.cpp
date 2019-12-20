#include "../newobjects.h"
#include "../../Game/items.h"

void InitialiseCyborg(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex + 4;
    item->frameNumber = Anims[item->animNumber].frameBase;
    item->goalAnimState = 1;
    item->currentAnimState = 1;
}