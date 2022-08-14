#pragma once
#include "Game/control/box.h"

constexpr auto MAX_VISIBILITY_DISTANCE = SECTOR(8);

bool ShotLara(ItemInfo* item, AI_INFO* AI, BiteInfo gun, short extraRotation, int damage);
short GunMiss(int x, int y, int z, short velocity, short yRot, short roomNumber);
short GunHit(int x, int y, int z, short velocity, short yRot, short roomNumber);
short GunShot(int x, int y, int z, short velocity, short yRot, short roomNumber);
bool Targetable(ItemInfo* item, AI_INFO* AI);
bool TargetVisible(ItemInfo* item, AI_INFO* AI, float maxAngle = 45.0f);
bool TargetVisible(ItemInfo* item, CreatureInfo* creature, AI_INFO* AI, float maxAngle = 45.0f);
