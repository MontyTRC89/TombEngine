#include "../newobjects.h"

void InitialiseAutoGuns(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    item->meshBits = 1024;
    item->data = (void*)GameMalloc(5702);
}