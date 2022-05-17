#pragma once
#include "Game/items.h"
#include "Specific/phd_global.h"

struct LOCUST_INFO
{
    bool on;
    PHD_3DPOS pos;
    ItemInfo* target;
    short roomNumber;
    short randomRotation;
    short escapeXrot;
    short escapeYrot;
    short escapeZrot;
    BYTE counter;
};

namespace TEN::Entities::TR4 {
    constexpr auto MAX_LOCUSTS = 64;
    constexpr auto LOCUST_LARA_DAMAGE = 3;
    constexpr auto LOCUST_ENTITY_DAMAGE = 1;

    extern LOCUST_INFO Locusts[MAX_LOCUSTS];

    extern int CreateLocust(void);
    extern void SpawnLocust(ItemInfo* item);
    extern void InitialiseLocust(short itemNumber);
    extern void LocustControl(short itemNumber);
    extern void UpdateLocusts(void);
    extern void DrawLocust(void);
}