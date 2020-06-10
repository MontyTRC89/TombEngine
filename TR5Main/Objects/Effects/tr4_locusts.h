#pragma once
#include "phd_global.h"
#include "items.h"

extern int CreateLocust(void);
extern void SpawnLocust(ITEM_INFO* item);
extern void InitialiseLocust(short itemNumber);
extern void LocustControl(short itemNumber);
extern void UpdateLocusts(void);
extern void DrawLocust(void);