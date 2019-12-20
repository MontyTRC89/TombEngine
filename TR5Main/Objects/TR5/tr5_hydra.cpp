#include "../newobjects.h"
#include "../../Game/items.h"

void InitialiseHydra(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = 30 * item->triggerFlags + Anims[item->animNumber].frameBase;
    item->goalAnimState = 0;
    item->currentAnimState = 0;

    if (item->triggerFlags == 1)
        item->pos.zPos += STEPUP_HEIGHT;

    if (item->triggerFlags == 2)
        item->pos.zPos -= STEPUP_HEIGHT;

    item->pos.yRot = ANGLE(90);
    item->pos.xPos -= STEP_SIZE;
}