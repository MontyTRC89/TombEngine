#include "../newobjects.h"
#include "../../Game/items.h"

void InitialiseImp(short itemNum)
{
    ITEM_INFO* item;
    short stateid;

    item = &Items[itemNum];
    ClearItem(itemNum);
    if (item->triggerFlags == 2 || item->triggerFlags == 12)
    {
        stateid = 8;
        item->animNumber = Objects[ID_IMP].animIndex + 8;
    }
    else if (item->triggerFlags == 1 || item->triggerFlags == 11)
    {
        stateid = 7;
        item->animNumber = Objects[ID_IMP].animIndex + 7;
    }
    else
    {
        stateid = 1;
        item->animNumber = Objects[ID_IMP].animIndex + 1;
    }
    item->goalAnimState = stateid;
    item->currentAnimState = stateid;
    item->frameNumber = Anims[item->animNumber].frameBase;
}