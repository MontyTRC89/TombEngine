#include "framework.h"
#include "GameScriptItemInfo.h"
#include "items.h"
#include "objectslist.h"
#include "level.h"
#include "setup.h"
#include "lot.h"
#include "GameScriptPosition.h"
#include "GameScriptRotation.h"
#include "trmath.h"

extern bool const WarningsAsErrors;

GameScriptItemInfo::GameScriptItemInfo(short num) : m_item{ &g_Level.Items[num]}, m_num { num }
{};

GameScriptItemInfo::GameScriptItemInfo(GameScriptItemInfo&& other) noexcept : m_item { std::exchange(other.m_item, nullptr) }, m_num{ std::exchange(other.m_num, -1) } {};

// todo.. how to check if item is killed outside of script?
GameScriptItemInfo::~GameScriptItemInfo() {
	// todo.. see if there's a better default state than -1
	if (m_num > -1)
	{
		KillItem(m_num);
	}
}

void GameScriptItemInfo::Register(sol::state * state)
{
	state->new_usertype<GameScriptItemInfo>("ItemInfo",
		"new", sol::overload(&GameScriptItemInfo::Create, &GameScriptItemInfo::CreateEmpty),
		"Init", &GameScriptItemInfo::Init,
		"currentAnim", sol::property(&GameScriptItemInfo::GetCurrentAnim, &GameScriptItemInfo::SetCurrentAnim),
		"requiredAnim", sol::property(&GameScriptItemInfo::GetRequiredAnim, &GameScriptItemInfo::SetRequiredAnim),
		"goalAnim", sol::property(&GameScriptItemInfo::GetGoalAnim, &GameScriptItemInfo::SetGoalAnim),
		"HP", sol::property(&GameScriptItemInfo::GetHP, &GameScriptItemInfo::SetHP),
		"OCB", sol::property(&GameScriptItemInfo::GetOCB, &GameScriptItemInfo::SetOCB),
		"itemFlags", sol::property(&GameScriptItemInfo::GetItemFlags, &GameScriptItemInfo::SetItemFlags),
		"AIBits", sol::property(&GameScriptItemInfo::GetAIBits, &GameScriptItemInfo::SetAIBits),
		"status", sol::property(&GameScriptItemInfo::GetStatus, &GameScriptItemInfo::SetStatus),
		"hitStatus", sol::property(&GameScriptItemInfo::GetHitStatus, &GameScriptItemInfo::SetHitStatus),
		"active", sol::property(&GameScriptItemInfo::GetActive, &GameScriptItemInfo::SetActive),
		"room", sol::property(&GameScriptItemInfo::GetRoom, &GameScriptItemInfo::SetRoom),
		"pos", sol::property(&GameScriptItemInfo::GetPos, &GameScriptItemInfo::SetPos),
		"rot", sol::property(&GameScriptItemInfo::GetRot, &GameScriptItemInfo::SetRot),
		"Enable", &GameScriptItemInfo::EnableItem,
		"Disable", &GameScriptItemInfo::DisableItem);
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
	short room,
	short currentAnim,
	short requiredAnim,
	short goalAnim,
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
	auto ptr = std::make_unique<GameScriptItemInfo>(num);

	ITEM_INFO * item = &g_Level.Items[num];
	ptr->SetPos(pos);
	ptr->SetRot(rot);
	ptr->SetRoom(room);
	//make it a big medipack by default for now	
	item->objectNumber = ID_BIGMEDI_ITEM;
	InitialiseItem(num);

	ptr->SetCurrentAnim(currentAnim);
	ptr->SetRequiredAnim(requiredAnim);
	ptr->SetGoalAnim(requiredAnim);
	ptr->SetHP(hp);
	ptr->SetOCB(ocb);
	ptr->SetItemFlags(itemFlags);
	ptr->SetAIBits(aiBits);
	ptr->SetStatus(status);
	ptr->SetActive(active);
	ptr->SetHitStatus(hitStatus);

	return ptr;
}

void GameScriptItemInfo::Init()
{
	InitialiseItem(m_num);
}

GameScriptPosition GameScriptItemInfo::GetPos() const
{
	return GameScriptPosition(	m_item->pos	);
}

void GameScriptItemInfo::SetPos(GameScriptPosition const& pos)
{
	pos.StoreInPHDPos(m_item->pos);
}

// This does not guarantee that the returned value will be identical
// to a value written in via SetRot - only that the angle measures
// will be mathematically equal
// (e.g. 90 degrees = -270 degrees = 450 degrees)
GameScriptRotation GameScriptItemInfo::GetRot() const
{
	return GameScriptRotation(	int(TO_DEGREES(m_item->pos.xRot)) % 360,
								int(TO_DEGREES(m_item->pos.yRot)) % 360,
								int(TO_DEGREES(m_item->pos.zRot)) % 360);
}

void GameScriptItemInfo::SetRot(GameScriptRotation const& rot)
{
	m_item->pos.xRot = FROM_DEGREES(rot.x);
	m_item->pos.yRot = FROM_DEGREES(rot.y);
	m_item->pos.zRot = FROM_DEGREES(rot.z);
}

short GameScriptItemInfo::GetGoalAnim() const
{
	return m_item->goalAnimState;
}

void GameScriptItemInfo::SetGoalAnim(short state)
{
	m_item->goalAnimState = state;
}

short GameScriptItemInfo::GetHP() const
{
	return(m_item->hitPoints);
}

void GameScriptItemInfo::SetHP(short hp)
{
	if (hp < 0 || hp > Objects[m_item->objectNumber].hitPoints)
	{
		if (WarningsAsErrors)
			throw std::runtime_error("invalid HP");
		if (hp < 0)
		{
			hp = 0;
		}
		else if (hp > Objects[m_item->objectNumber].hitPoints)
		{
			hp = Objects[m_item->objectNumber].hitPoints;
		}
	}

	m_item->hitPoints = hp;
}

short GameScriptItemInfo::GetOCB() const
{
	return m_item->triggerFlags;
}

void GameScriptItemInfo::SetOCB(short ocb)
{
	m_item->triggerFlags = ocb;
}

byte GameScriptItemInfo::GetAIBits() const
{
	return m_item->aiBits;
}

void GameScriptItemInfo::SetAIBits(byte bits)
{
	m_item->aiBits = bits;
}

sol::as_table_t<std::array<short, 8>> GameScriptItemInfo::GetItemFlags() const
{	
	std::array<short, 8> ret{};
	memcpy(ret.data(), m_item->itemFlags, sizeof(m_item->itemFlags));
	return ret;
}

void GameScriptItemInfo::SetItemFlags(sol::as_table_t<std::array<short, 8>> const& arr)
{	
	memcpy(m_item->itemFlags, arr.value().data(), sizeof(m_item->itemFlags));
}

short GameScriptItemInfo::GetCurrentAnim() const
{
	return m_item->currentAnimState;
}

void GameScriptItemInfo::SetCurrentAnim(short anim)
{
	m_item->currentAnimState = anim;
}

short GameScriptItemInfo::GetRequiredAnim() const
{
	return m_item->requiredAnimState;
}

void GameScriptItemInfo::SetRequiredAnim(short anim)
{
	m_item->requiredAnimState = anim;
}

short GameScriptItemInfo::GetStatus() const
{
	return m_item->status;
}

void GameScriptItemInfo::SetStatus(short status)
{
	m_item->status = status;
}

bool GameScriptItemInfo::GetActive() const
{
	return m_item->active;
}

void GameScriptItemInfo::SetActive(bool active)
{
	m_item->active = active;
}

bool GameScriptItemInfo::GetHitStatus() const
{
	return m_item->hitStatus;
}

void GameScriptItemInfo::SetHitStatus(bool hitStatus)
{
	m_item->hitStatus = hitStatus;
}

short GameScriptItemInfo::GetRoom() const
{
	return m_item->roomNumber;
}

void GameScriptItemInfo::SetRoom(short room)
{	
	if (room < 0 || room >= g_Level.Rooms.size())
	{
		if (WarningsAsErrors)
			throw std::runtime_error("invalid room number");
		return;
	}

	m_item->roomNumber = room;
}


void GameScriptItemInfo::EnableItem()
{
	if (!m_item->active)
	{
		if (Objects[m_item->objectNumber].intelligent)
		{
			if (m_item->status == ITEM_DEACTIVATED)
			{
				m_item->touchBits = 0;
				m_item->status = ITEM_ACTIVE;
				AddActiveItem(m_num);
				EnableBaddieAI(m_num, 1);
			}
			else if (m_item->status == ITEM_INVISIBLE)
			{
				m_item->touchBits = 0;
				if (EnableBaddieAI(m_num, 0))
					m_item->status = ITEM_ACTIVE;
				else
					m_item->status = ITEM_INVISIBLE;
				AddActiveItem(m_num);
			}
		}
		else
		{
			m_item->touchBits = 0;
			AddActiveItem(m_num);
			m_item->status = ITEM_ACTIVE;
		}
	}
}

void GameScriptItemInfo::DisableItem()
{
	if (m_item->active)
	{
		if (Objects[m_item->objectNumber].intelligent)
		{
			if (m_item->status == ITEM_ACTIVE)
			{
				m_item->touchBits = 0;
				m_item->status = ITEM_DEACTIVATED;
				RemoveActiveItem(m_num);
				DisableBaddieAI(m_num);
			}
		}
		else
		{
			m_item->touchBits = 0;
			RemoveActiveItem(m_num);
			m_item->status = ITEM_DEACTIVATED;
		}
	}
}

