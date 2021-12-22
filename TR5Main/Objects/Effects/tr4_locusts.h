#pragma once
#include "Specific/phd_global.h"
#include "Game/items.h"

struct LOCUST_INFO
{
    bool on;
    PHD_3DPOS pos;
    ITEM_INFO* target;
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
    extern void SpawnLocust(ITEM_INFO* item);
    extern void InitialiseLocust(short itemNumber);
    extern void LocustControl(short itemNumber);
    extern void UpdateLocusts(void);
    extern void DrawLocust(void);
}