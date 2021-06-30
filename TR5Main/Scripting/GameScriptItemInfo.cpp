#include "framework.h"
#include "GameScriptItemInfo.h"
#include "items.h"
#include "objectslist.h"
#include "level.h"
#include "GameScriptPosition.h"
#include "GameScriptRotation.h"

void GameScriptItemInfo::Register(sol::state * state)
{
	state->new_usertype<GameScriptItemInfo>("ItemInfo",
		"new", sol::overload(&GameScriptItemInfo::Create, &GameScriptItemInfo::CreateEmpty),
		"Init", &GameScriptItemInfo::Init,
		"GetPos", &GameScriptItemInfo::GetPos,
		"GetRot", &GameScriptItemInfo::GetRot,
		"GetCurrentAnim", &GameScriptItemInfo::GetCurrentAnim,
		"GetRequiredAnim", &GameScriptItemInfo::GetRequiredAnim,
		"GetHP", &GameScriptItemInfo::GetHP,
		"GetOCB", &GameScriptItemInfo::GetOCB,
		"GetItemFlags", &GameScriptItemInfo::GetItemFlags,
		"GetAIBits", &GameScriptItemInfo::GetAIBits,
		"GetStatus", &GameScriptItemInfo::GetStatus,
		"GetHitStatus", &GameScriptItemInfo::GetHitStatus,
		"GetActive", &GameScriptItemInfo::GetActive,
		"SetPos", &GameScriptItemInfo::SetPos,
		"SetRot", &GameScriptItemInfo::SetRot,
		"SetCurrentAnim", &GameScriptItemInfo::SetCurrentAnim,
		"SetRequiredAnim", &GameScriptItemInfo::SetRequiredAnim,
		"SetHP", &GameScriptItemInfo::SetHP,
		"SetOCB", &GameScriptItemInfo::SetOCB,
		"SetItemFlags", &GameScriptItemInfo::SetItemFlags,
		"SetAIBits", &GameScriptItemInfo::SetAIBits,
		"SetStatus", &GameScriptItemInfo::SetStatus,
		"SetHitStatus", &GameScriptItemInfo::SetHitStatus,
		"SetActive", &GameScriptItemInfo::SetActive);
		}

std::unique_ptr<GameScriptItemInfo> GameScriptItemInfo::CreateEmpty()
{
	short num = CreateItem();	
	ITEM_INFO * item = &g_Level.Items[num];
	item->objectNumber = ID_SMALLMEDI_ITEM;
	return std::make_unique<GameScriptItemInfo>(num);
}

std::unique_ptr<GameScriptItemInfo> GameScriptItemInfo::Create(
	GameScriptPosition pos,
	GameScriptRotation rot,
	short currentAnim,
	short requiredAnim,
	short hp,
	short ocb,
	sol::as_table_t<std::array<short, 8>> itemFlags,
	byte aiBits,
	short status,
	bool active,
	bool hitStatus
	)
{
	short num = CreateItem();	
	ITEM_INFO * item = &g_Level.Items[num];

	item->pos = PHD_3DPOS(
		pos.GetX(),
		pos.GetY(),
		pos.GetZ(),
		rot.GetX(),
		rot.GetY(),
		rot.GetZ()	
	);

	//make it a big medipack by default for now	
	item->objectNumber = ID_BIGMEDI_ITEM;
	InitialiseItem(num);
	
	item->currentAnimState = currentAnim;
	item->requiredAnimState = requiredAnim;
	memcpy(item->itemFlags, itemFlags.value().data(), sizeof(item->itemFlags));
	item->hitPoints = hp;
	item->triggerFlags = ocb;
	item->aiBits = aiBits;
	item->status = status;
	item->active = active;
	item->hitStatus = hitStatus;

	return std::make_unique<GameScriptItemInfo>(num);
}

void GameScriptItemInfo::Init()
{
	InitialiseItem(m_num);
}

GameScriptPosition GameScriptItemInfo::GetPos() const
{
	ITEM_INFO * item = &g_Level.Items[m_num];
	return GameScriptPosition(	item->pos.xPos,
								item->pos.yPos,
								item->pos.zPos);
}

GameScriptRotation GameScriptItemInfo::GetRot() const
{
	ITEM_INFO * item = &g_Level.Items[m_num];
	return GameScriptRotation(	item->pos.xRot,
								item->pos.yRot,
								item->pos.zRot);
}

short GameScriptItemInfo::GetHP() const
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	return(item->hitPoints);
}

short GameScriptItemInfo::GetOCB() const
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	return item->triggerFlags;
}

byte GameScriptItemInfo::GetAIBits() const
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	return item->aiBits;
}

sol::as_table_t<std::array<short, 8>> GameScriptItemInfo::GetItemFlags() const
{	
	ITEM_INFO* item = &g_Level.Items[m_num];
	std::array<short, 8> ret;
	memcpy(ret.data(), item->itemFlags, sizeof(item->itemFlags));
	return ret;
}

short GameScriptItemInfo::GetCurrentAnim() const
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	return item->currentAnimState;
}

short GameScriptItemInfo::GetRequiredAnim() const
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	return item->requiredAnimState;
}


short GameScriptItemInfo::GetStatus() const
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	return item->status;
}

bool GameScriptItemInfo::GetHitStatus() const
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	return item->hitStatus;
}

bool GameScriptItemInfo::GetActive() const
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	return item->active;
}

void GameScriptItemInfo::SetPos(GameScriptPosition const& pos)
{
	ITEM_INFO * item = &g_Level.Items[m_num];
	item->pos.xPos = pos.x;
	item->pos.yPos = pos.y;
	item->pos.zPos = pos.z;
}

void GameScriptItemInfo::SetRot(GameScriptRotation const& rot)
{
	ITEM_INFO * item = &g_Level.Items[m_num];
	item->pos.xRot = rot.GetX();
	item->pos.yRot = rot.GetY();
	item->pos.zRot = rot.GetZ();
}

void GameScriptItemInfo::SetHP(short hp)
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	item->hitPoints = hp;
}

void GameScriptItemInfo::SetOCB(short ocb)
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	item->triggerFlags = ocb;
}

void GameScriptItemInfo::SetAIBits(byte bits)
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	item->aiBits = bits;
}

void GameScriptItemInfo::SetItemFlags(sol::as_table_t<std::array<short, 8>>  const& arr)
{	
	ITEM_INFO* item = &g_Level.Items[m_num];
	memcpy(item->itemFlags, arr.value().data(), sizeof(item->itemFlags));
}

void GameScriptItemInfo::SetCurrentAnim(short anim)
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	item->currentAnimState = anim;
}

void GameScriptItemInfo::SetRequiredAnim(short anim)
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	item->requiredAnimState = anim;
}

void GameScriptItemInfo::SetStatus(short status)
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	item->status = status;
}

void GameScriptItemInfo::SetHitStatus(bool hitStatus)
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	item->hitStatus = hitStatus;
}

void GameScriptItemInfo::SetActive(bool active)
{
	ITEM_INFO* item = &g_Level.Items[m_num];
	item->active = active;
}

GameScriptItemInfo::GameScriptItemInfo(short num) : m_num{ num } {};

// todo.. how to check if item is killed outside of script?
GameScriptItemInfo::~GameScriptItemInfo() {
	KillItem(m_num);
}