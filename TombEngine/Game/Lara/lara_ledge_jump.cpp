#include "framework.h"
#include "lara_ledge_jump.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"

void lara_ledge_jump_up(ItemInfo* item, CollisionInfo* coll)
{
	item->Animation.TargetState = LS_JUMP_UP;
}

void lara_ledge_jump_back(ItemInfo* item, CollisionInfo* coll)
{
	item->Animation.TargetState = LS_JUMP_BACK;
}