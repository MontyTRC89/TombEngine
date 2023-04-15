#include "framework.h"
#include "creature_info.h"
#include "Game/items.h"

bool CreatureInfo::TargetIsAlive()
{
    return Enemy != nullptr && Enemy->HitPoints > 0;
}
