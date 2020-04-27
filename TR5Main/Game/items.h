#pragma once
#include "..\Global\global.h"
#include "control.h"

#define NUM_ITEMS 1024
#define NUM_EFFECTS 1024

void EffectNewRoom(short fxNumber, short roomNumber);
void ItemNewRoom(short itemNum, short roomNumber);
void AddActiveItem(short itemNumber);
void ClearItem(short itemNum);
short CreateItem();
void RemoveAllItemsInRoom(short roomNumber, short objectNumber);
void RemoveActiveItem(short itemNum);
void RemoveDrawnItem(short itemNum);
void InitialiseFXArray(int allocmem);
short CreateNewEffect(short roomNum);
void KillEffect(short fxNumber);
void InitialiseItem(short itemNum);
void InitialiseItemArray(int numitems);
void KillItem(short itemNum);
ITEM_INFO* find_a_fucking_item(short objectNum);
int FindItem(short objectNum);
