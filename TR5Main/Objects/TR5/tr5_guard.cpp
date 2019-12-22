#include "../newobjects.h"
#include "../../Game/items.h"

void InitialiseGuard(short itemNum)
{
    ITEM_INFO* item, *item2;
    short anim;
    short roomItemNumber;

    item = &Items[itemNum];
    ClearItem(itemNum);
    anim = Objects[ID_SWAT].animIndex;
    if (!Objects[ID_SWAT].loaded)
        anim = Objects[ID_BLUE_GUARD].animIndex;

    switch (item->triggerFlags)
    {
        case 0:
        case 10:
            item->animNumber = anim;
            item->goalAnimState = 1;
            break;
        case 1:
            item->goalAnimState = 11;
            item->animNumber = anim + 23;
            break;
        case 2:
            item->goalAnimState = 13;
            item->animNumber = anim + 25;
            // TODO: item->flags2 ^= (item->flags2 ^ ((item->flags2 & 0xFE) + 2)) & 6;
            break;
        case 3:
            item->goalAnimState = 15;
            item->animNumber = anim + 28;
            *item->pad2 = 9216;
            roomItemNumber = Rooms[item->roomNumber].itemNumber;
            if (roomItemNumber != NO_ITEM)
            {
                while (true)
                {
                    item2 = &Items[roomItemNumber];
                    if (item2->objectNumber >= ID_ANIMATING1 && item2->objectNumber <= ID_ANIMATING15 && item2->roomNumber == item->roomNumber && item2->triggerFlags == 3)
                        break;
                    roomItemNumber = item2->nextItem;
                    if (roomItemNumber == NO_ITEM)
                    {
                        item->frameNumber = Anims[item->animNumber].frameBase;
                        item->currentAnimState = item->goalAnimState;
                        break;
                    }
                }
                item2->meshBits = -5;
            }
            break;
        case 4:
            item->goalAnimState = 17;
            *item->pad2 = 8192;
            item->animNumber = anim + 30;
            break;
        case 5:
            FLOOR_INFO *floor;
            short roomNumber;

            item->animNumber = anim + 26;
            item->goalAnimState = 14;
            roomNumber = item->roomNumber;
            floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
            GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
            item->pos.yPos = GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) - SECTOR(2);
            break;
        case 6:
            item->goalAnimState = 19;
            item->animNumber = anim + 32;
            break;
        case 7:
        case 9:
            item->goalAnimState = 38;
            item->animNumber = anim + 59;
            item->pos.xPos -= SIN(item->pos.yRot); // 4 * not exist there ??
            item->pos.zPos -= COS(item->pos.yRot); // 4 * not exist there ??
            break;
        case 8:
            item->goalAnimState = 31;
            item->animNumber = anim + 46;
            break;
        case 11:
            item->goalAnimState = 7;
            item->animNumber = anim + 12;
            break;
        default:
            break;
    }
}

void InitialiseGuardM16(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = Anims[item->animNumber].frameBase;
    item->goalAnimState = 1;
    item->currentAnimState = 1;
    item->pos.yPos += STEP_SIZE*2;
    item->pos.xPos += SIN(item->pos.yRot);
    item->pos.zPos += COS(item->pos.yRot);
}

void InitialiseGuardLaser(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex + 6;
    item->frameNumber = Anims[item->animNumber].frameBase;
    item->goalAnimState = 1;
    item->currentAnimState = 1;
}