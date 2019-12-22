#include "../newobjects.h"

void InitialiseDoberman(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    if (item->triggerFlags)
    {
        item->currentAnimState = 5;
        item->animNumber = Objects[item->objectNumber].animIndex + 6;
        // TODO: item->flags2 ^= (item->flags2 ^ ((item->flags2 & 0xFE) + 2)) & 6;
    }
    else
    {
        item->currentAnimState = 6;
        item->animNumber = Objects[item->objectNumber].animIndex + 10;
    }
    item->frameNumber = Anims[item->animNumber].frameBase;
}