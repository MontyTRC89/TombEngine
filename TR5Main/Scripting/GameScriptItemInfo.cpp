#include "framework.h"
#include "GameScriptItemInfo.h"
#include "items.h"
#include "objectslist.h"
#include "level.h"

void GameScriptItemInfo::Register(sol::state * state)
{
	state->new_usertype<GameScriptItemInfo>("ItemInfo",
		"new", sol::factories(&GameScriptItemInfo::Create));
}

std::unique_ptr<GameScriptItemInfo> GameScriptItemInfo::Create(
	short hp,
	short currentAnim,
	short requiredAnimState,
	sol::as_table_t<std::array<int, 3>> pos,
	sol::as_table_t<std::array<short, 3>> rot,
	sol::as_table_t<std::array<short, 8>> itemFlags,
	short ocb,
	byte aiBits,
	short status,
	bool active,
	bool hitStatus)
{
	short num = CreateItem();	
	ITEM_INFO * item = &g_Level.Items[num];
	
	auto p = pos.value();
	auto r = rot.value();
	item->pos = PHD_3DPOS(
		p[0],
		p[1],
		p[2],
		r[0],
		r[1],
		r[2]
	);

	//make it a big medipack by default for now	
	item->objectNumber = ID_BIGMEDI_ITEM;
	InitialiseItem(num);
	
	item->hitPoints = hp;
	item->currentAnimState = currentAnim;
	item->requiredAnimState = requiredAnimState;
	memcpy(item->itemFlags, itemFlags.value().data(), sizeof(item->itemFlags));
	item->triggerFlags = ocb;
	item->aiBits = aiBits;
	item->status = status;
	item->active = active;
	item->hitStatus = hitStatus;

	return std::make_unique<GameScriptItemInfo>(num);
}

GameScriptItemInfo::GameScriptItemInfo(short num) : m_num{ num } {};

GameScriptItemInfo::~GameScriptItemInfo() {
	KillItem(m_num);
}

