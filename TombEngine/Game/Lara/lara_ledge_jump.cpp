#include "framework.h"
#include "lara_ledge_jump.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"

void lara_col_ledge_jump_up(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	SetAnimation(item, LA_LEDGE_JUMP_UP_START);

	if (item->Animation.AnimNumber == LA_LEDGE_JUMP_UP_START)
	{
		SetAnimation(item, LA_LEDGE_JUMP_UP_START);
	}
}

void lara_col_ledge_jump_back(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);
	
	SetAnimation(item, LA_LEDGE_JUMP_BACK_START);

	if (item->Animation.AnimNumber == LA_LEDGE_JUMP_BACK_START)
	{
		SetAnimation(item, LA_LEDGE_JUMP_BACK_START);
	}
}

void lara_as_ledge_jump_up(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	SetAnimation(item, LA_LEDGE_JUMP_UP_START);

	if (item->Animation.AnimNumber == LA_LEDGE_JUMP_UP_START)
	{
		SetAnimation(item, LA_LEDGE_JUMP_UP_START);
	}
}

void lara_as_ledge_jump_back(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	SetAnimation(item, LA_LEDGE_JUMP_BACK_START);

	if (item->Animation.AnimNumber == LA_LEDGE_JUMP_BACK_START)
	{
		SetAnimation(item, LA_LEDGE_JUMP_BACK_START);
	}
}