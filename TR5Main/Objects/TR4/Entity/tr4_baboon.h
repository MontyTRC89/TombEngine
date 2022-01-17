#pragma once
#include "Specific/phd_global.h"
#include "Game/items.h"

struct BaboonRespawnStruct
{
    int id;
    PHD_3DPOS pos;
    int count;
    int max_count; // used to limit the number of respawn !
};

class BaboonRespawnClass
{
private:
    std::vector<BaboonRespawnStruct> baboonRespawnArray;
public:
    void Free(void);
    void Add(ITEM_INFO* item, int max_count);
    void Remove(int id);
    int GetBaboonFreePlace(void);
    BaboonRespawnStruct* GetBaboonRespawn(int id);
    int GetCount(int id);
    int GetCountMax(int id);
};


extern BaboonRespawnClass BaboonRespawn;

extern void InitialiseBaboon(short itemNumber);
extern void BaboonControl(short itemNumber);