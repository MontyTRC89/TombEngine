#include "../newobjects.h"
#include "../../Game/items.h"

void InitialiseLarson(short itemNum)
{
    ITEM_INFO* item;
    short rotY;

    item = &Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = Anims[item->animNumber].frameBase;
    item->goalAnimState = 1;
    item->currentAnimState = 1;
    if (!item->triggerFlags)
        return;
    item->itemFlags[3] = 0;
    rotY = item->pos.yRot;

    // TODO: check if it's ok !
    if (rotY > 4096 && rotY < 28672)
    {
        item->pos.xPos += STEPUP_HEIGHT;
    }
    else if (rotY < -4096 && rotY > -28672)
    {
        item->pos.xPos += STEPUP_HEIGHT;
    }
    else if (rotY <= -8192 || rotY >= 8192)
    {
        if (rotY < -20480 || rotY > 20480)
            item->pos.zPos -= STEPUP_HEIGHT;
    }
    else
    {
        item->pos.zPos += STEPUP_HEIGHT;
    }
}