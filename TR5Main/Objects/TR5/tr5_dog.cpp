#include "../newobjects.h"

void InitialiseDog(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    item->currentAnimState = 1;
    item->animNumber = Objects[item->objectNumber].animIndex + 8;
    if (!item->triggerFlags)
    {
        item->animNumber = Objects[item->objectNumber].animIndex + 1;
        // TODO: item->flags2 ^= (item->flags2 ^ ((item->flags2 & 0xFE) + 2)) & 6;
    }
    item->frameNumber = Anims[item->animNumber].frameBase;
}