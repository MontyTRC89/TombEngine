#include "framework.h"
#include "tr4_baboon.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Game/misc.h"
#include "Game/Lara/lara.h"
#include "Game/effects/effects.h"
#include "Game/effects/weather.h"
#include "Game/effects/tomb4fx.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"

using std::vector;
using namespace TEN::Effects::Environment;

BaboonRespawnClass BaboonRespawn;
static BITE_INFO BaboonBite = { 10, 10, 11, 4 };

enum BABOON_STATE
{
	BABOON_NULL,
	BABOON_EMPTY,
	BABOON_WALK,
	BABOON_IDLE,
	BABOON_RUN,
	BABOON_PICKUP,
	BABOON_SIT_IDLE,
	BABOON_SIT_EAT,
	BABOON_SIT_SCRATCH,
	BABOON_RUN_ROLL,
	BABOON_HITGROUND,
	BABOON_DEATH,
	BABOON_ATK1,
	BABOON_JUMPATK,
	BABOON_SUPERJUMPATK,
	/// NOT USED IN TR4:
	BABOON_CLIMB_4CLICK,
	BABOON_CLIMB_3CLICK,
	BABOON_CLIMB_2CLICK,
	BABOON_FALL_4CLICK,
	BABOON_FALL_3CLICK,
	BABOON_FALL_2CLICK,
	///!END
	BABOON_ACTIVATE_SWITCH
};

constexpr auto NO_BABOON = -1;
constexpr auto NO_BABOON_COUNT = -2;
constexpr auto NO_CROWBAR_SWITCH_FOUND = -1;

#define BABOON_SIT_IDLE_ANIM 2
#define BABOON_IDLE_ANIM 9
#define BABOON_DEATH_ANIM 14
#define BABOON_SWITCH_ANIM 31

#define BABOON_DAMAGE 70
#define BABOON_IDLE_DISTANCE pow(SECTOR(1), 2)
#define BABOON_ATTACK_ANGLE ANGLE(7.0f)
#define BABOON_ATK_RANGE 0x718E4
#define BABOON_ATK_NORMALRANGE 0x1C639
#define BABOON_JUMP_RANGE 0x718E4
#define BABOON_FOLLOW_RANGE 0x400000
#define BABOON_RUNROLL_RANGE 0x100000
#define BABOON_WALK_ANGLE ANGLE(7.0f)
#define BABOON_RUN_ANGLE ANGLE(11.0f)
/// NOTE (TokyoSU): these touchbits is fixed !
/// now the baboon is a killing machine :D
#define BABOON_RIGHT_TOUCHBITS 814
#define BABOON_JUMP_TOUCHBITS 280
#define BABOON_TOUCHBITS 0x1800

static void TriggerBaboonShockwave(PHD_3DPOS pos, short xRot)
{
	short shockwaveID = GetFreeShockwave();
	if (shockwaveID != NO_ITEM)
	{
		auto* dieEffect = &ShockWaves[shockwaveID];

		dieEffect->x = pos.xPos;
		dieEffect->y = pos.yPos;
		dieEffect->z = pos.zPos;
		dieEffect->innerRad = 0x2000280;
		dieEffect->outerRad = 0x28802000;
		dieEffect->xRot = xRot;
		dieEffect->r = 255;
		dieEffect->g = 64;
		dieEffect->b = 0;
		dieEffect->speed = -600;
		dieEffect->life = 64;
	}
}

void BaboonDieEffect(ITEM_INFO* item)
{
	PHD_3DPOS pos = PHD_3DPOS(item->Position.xPos, item->Position.yPos - 128, item->Position.zPos);

	// trigger shockwave effect
	TriggerBaboonShockwave(pos, ANGLE(0.0f));
	TriggerBaboonShockwave(pos, ANGLE(45.0f));
	TriggerBaboonShockwave(pos, ANGLE(90.0f));
	TriggerBaboonShockwave(pos, ANGLE(135.0f));

	// trigger flash screen
	Weather.Flash(255, 64, 0, 0.03f);
}

static void KillRespawnedBaboon(short itemNumber, bool remove = false)
{
	auto* item = &g_Level.Items[itemNumber];

	item->HitPoints = 0;
	RemoveActiveItem(itemNumber); // remove it from the active item list

	item->Flags = IFLAG_CLEAR_BODY;
	item->AfterDeath = 128; // instant disappear !
	item->Status = ITEM_DEACTIVATED; // wont triggered again...

	if (remove)
		item->ItemFlags[0] = NO_BABOON;

	DisableEntityAI(itemNumber); // desactivate this AI or you will get crash later...
}

static bool CheckRespawnedBaboon(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	if (item->ItemFlags[0] == NO_BABOON) // NORMAL/INV for now
	{
		KillRespawnedBaboon(itemNumber);
		return false;
	}

	auto* baboonRespawn = BaboonRespawn.GetBaboonRespawn(item->ItemFlags[0]);
	if (baboonRespawn == nullptr)
		return false;

	if (baboonRespawn->Count == baboonRespawn->MaxCount)
	{
		KillRespawnedBaboon(itemNumber, true);
		return false;
	}

	return true;
}

static void UpdateRespawnedBaboon(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* object = &Objects[item->ObjectNumber];

	auto* baboonRespawn = BaboonRespawn.GetBaboonRespawn(item->ItemFlags[0]);
	if (baboonRespawn == nullptr)
		return;

	item->Position = baboonRespawn->Pos;

	auto outsideRoom = IsRoomOutside(item->Position.xPos, item->Position.yPos, item->Position.zPos);
	if (item->RoomNumber != outsideRoom && outsideRoom != NO_ROOM)
		ItemNewRoom(itemNumber, outsideRoom);

	if (baboonRespawn->Count < baboonRespawn->MaxCount)
		baboonRespawn->Count++;

	item->Animation.AnimNumber = object->animIndex + BABOON_SIT_IDLE_ANIM;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.ActiveState = BABOON_SIT_IDLE;
	item->Animation.TargetState = BABOON_SIT_IDLE;
	item->HitPoints = object->HitPoints;

	RemoveActiveItem(itemNumber);
	item->Flags = NULL;
	item->AfterDeath = 0;
	item->Status = ITEM_INVISIBLE;

	DisableEntityAI(itemNumber);
	if (item->ObjectNumber == ID_BABOON_NORMAL)
	{
		if (item->TriggerFlags == 1)
			return;
		else
			item->Collidable = true;
	}
	else if (item->TriggerFlags == 0)
		item->Collidable = true;
}

void BaboonRespawnFunction(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	BaboonDieEffect(item);

	if (!CheckRespawnedBaboon(itemNumber))
		return;
	UpdateRespawnedBaboon(itemNumber);
}

void InitialiseBaboon(short itemNumber)
{
	InitialiseCreature(itemNumber);

	auto* item = &g_Level.Items[itemNumber];

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BABOON_SIT_IDLE_ANIM;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = BABOON_SIT_IDLE;
	item->Animation.ActiveState = BABOON_SIT_IDLE;

	if (item->ObjectNumber == ID_BABOON_SILENT && item->TriggerFlags != 0)
		BaboonRespawn.Add(item, item->TriggerFlags);
	else
		item->ItemFlags[0] = NO_BABOON;
}

void BaboonControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short headY = 0;
	short tilt = 0;
	short angle = 0;

	if (item->HitPoints <= 0 && item->HitPoints != NOT_TARGETABLE)
	{
		if (item->Animation.ActiveState == BABOON_DEATH)
		{
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
				BaboonRespawnFunction(itemNumber);
		}
		else if (item->Animation.ActiveState != BABOON_ACTIVATE_SWITCH)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BABOON_DEATH_ANIM;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = BABOON_DEATH;
			item->Animation.TargetState = BABOON_DEATH;
		}
	}
	else
	{
		GetAITarget(creature);

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		AI_INFO laraAI;
		if (!item->HitStatus && item->ObjectNumber == ID_BABOON_NORMAL)
		{
			int dx = LaraItem->Position.xPos - item->Position.xPos;
			int dz = LaraItem->Position.zPos - item->Position.zPos;

			laraAI.angle = phd_atan(dx, dz) - item->Position.yRot;
			laraAI.distance = pow(dx, 2) + pow(dz, 2);

			if (creature->Enemy == nullptr || creature->Enemy == LaraItem)
				creature->Enemy = nullptr;
		}
		else
		{
			laraAI.angle = AI.angle;
			laraAI.distance = AI.distance;
			creature->Enemy = LaraItem;
		}

		GetCreatureMood(item, &AI, TRUE);
		CreatureMood(item, &AI, TRUE);
		angle = CreatureTurn(item, creature->MaxTurn);

		if (creature->Enemy != nullptr && creature->Enemy != LaraItem && creature->Enemy->ObjectNumber == ID_AI_FOLLOW)
		{
			if (creature->ReachedGoal &&
				abs(item->Position.xPos - creature->Enemy->Position.xPos) < CLICK(1) &&
				abs(item->Position.yPos - creature->Enemy->Position.yPos) < CLICK(1) &&
				abs(item->Position.zPos - creature->Enemy->Position.zPos) < CLICK(1))
			{
				item->Position.xPos = creature->Enemy->Position.xPos;
				item->Position.yPos = creature->Enemy->Position.yPos;
				item->Position.zPos = creature->Enemy->Position.zPos;
				item->Position.yRot = creature->Enemy->Position.yRot;
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BABOON_SWITCH_ANIM;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.TargetState = BABOON_ACTIVATE_SWITCH;
				item->Animation.ActiveState = BABOON_ACTIVATE_SWITCH;
				item->AIBits &= ~(FOLLOW);

				TestTriggers(item, true);

				creature->Enemy = nullptr;
			}
		}

		switch (item->Animation.ActiveState)
		{
		case BABOON_IDLE:
			creature->MaxTurn = 0;
			creature->Flags = 0;

			if (item->AIBits & GUARD)
			{
				AIGuard(creature);
				if (!(GetRandomControl() & 0xF))
				{
					if (GetRandomControl() & 1)
						item->Animation.TargetState = BABOON_HITGROUND;
					else
						item->Animation.TargetState = BABOON_SIT_IDLE;
				}
			}
			else if (item->AIBits & PATROL1)
				item->Animation.TargetState = BABOON_WALK;
			else if (creature->Mood == MoodType::Escape)
			{
				if (AI.ahead && Lara.TargetEntity != item)
					item->Animation.TargetState = BABOON_IDLE;
				else
					item->Animation.TargetState = BABOON_RUN;
			}
			else if (creature->Mood == MoodType::Attack)
			{
				if (!(item->AIBits & FOLLOW) || (!item->Animation.Airborne && AI.distance <= BABOON_RUNROLL_RANGE))
				{
					if (AI.bite && AI.distance < BABOON_ATK_NORMALRANGE)
					{
						if (LaraItem->Position.yPos >= item->Position.yPos)
							item->Animation.TargetState = BABOON_ATK1;
						else
							item->Animation.TargetState = BABOON_JUMPATK;
					}
					else if (AI.bite && AI.distance < BABOON_JUMP_RANGE)
						item->Animation.TargetState = BABOON_SUPERJUMPATK;
					else if (AI.bite && AI.distance < BABOON_RUNROLL_RANGE)
						item->Animation.TargetState = BABOON_RUN_ROLL;
					else
						item->Animation.TargetState = BABOON_RUN;
				}
				else if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (GetRandomControl() & 1)
					item->Animation.TargetState = BABOON_SIT_IDLE;
			}
			else if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;
			else if (!(GetRandomControl() & 3))
				item->Animation.TargetState = BABOON_WALK;
			else if (!(GetRandomControl() & 1))
				item->Animation.TargetState = BABOON_RUN_ROLL;
			else if (GetRandomControl() & 4)
				item->Animation.TargetState = BABOON_HITGROUND;

			break;

		case BABOON_SIT_IDLE:
			creature->MaxTurn = 0;
			creature->Flags = 0;

			if (item->AIBits & GUARD)
			{
				AIGuard(creature);
				if (GetRandomControl() & 0xF)
					item->Animation.TargetState = BABOON_SIT_EAT;
				else if (GetRandomControl() & 0xA)
					item->Animation.TargetState = BABOON_SIT_SCRATCH;
			}
			else if (item->AIBits & PATROL1)
				item->Animation.TargetState = BABOON_WALK;
			else if (creature->Mood != MoodType::Escape)
			{
				if (creature->Mood == MoodType::Bored)
				{
					if (item->Animation.RequiredState)
						item->Animation.TargetState = item->Animation.RequiredState;
					// NOTE: it's not the original code, but it's too wreid 
					// that the baboon repeat the same move so i included the sit_idle with more random number
					// (the eat not exist in the bored mood, i added it !)
					else if (GetRandomControl() & 0x10)
						item->Animation.TargetState = BABOON_SIT_IDLE;
					else if (GetRandomControl() & 0x500)
					{
						if (GetRandomControl() & 0x200)
							item->Animation.TargetState = BABOON_SIT_SCRATCH;
						else if (GetRandomControl() & 0x250)
							item->Animation.TargetState = BABOON_SIT_EAT;
					}
					else if (GetRandomControl() & 0x1000 || item->AIBits & FOLLOW)
						item->Animation.TargetState = BABOON_WALK;
				}
				else if ((item->AIBits & FOLLOW) && AI.distance > BABOON_IDLE_DISTANCE)
				{
					if (item->Animation.RequiredState)
						item->Animation.TargetState = item->Animation.RequiredState;
					else
						item->Animation.TargetState = BABOON_WALK;
				}
				else
					item->Animation.TargetState = BABOON_WALK;
			}
			else
				item->Animation.TargetState = BABOON_IDLE;

			break;

		case BABOON_WALK:
			creature->MaxTurn = BABOON_WALK_ANGLE;

			if (item->AIBits & PATROL1)
				item->Animation.TargetState = BABOON_WALK;
			else if (creature->Mood == MoodType::Bored)
			{
				if (item->AIBits & FOLLOW)
					item->Animation.TargetState = BABOON_WALK;
				else if (GetRandomControl() < 256)
					item->Animation.TargetState = BABOON_SIT_IDLE;
			}
			else if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = BABOON_RUN;
			else if (creature->Mood == MoodType::Attack)
			{
				if (AI.bite && AI.distance < BABOON_ATK_RANGE)
					item->Animation.TargetState = BABOON_IDLE;
			}
			else if (GetRandomControl() < 256)
				item->Animation.TargetState = BABOON_SIT_IDLE;

			break;

		case BABOON_RUN:
			creature->MaxTurn = BABOON_RUN_ANGLE;
			tilt = angle / 2;

			if (item->AIBits & GUARD)
				item->Animation.TargetState = BABOON_IDLE;
			else if (creature->Mood == MoodType::Escape)
			{
				if (AI.ahead && Lara.TargetEntity != item)
					item->Animation.TargetState = BABOON_IDLE;
			}
			else if (item->AIBits & FOLLOW && (item->Animation.Airborne || AI.distance > BABOON_FOLLOW_RANGE))
				item->Animation.TargetState = BABOON_IDLE;
			else if (creature->Mood == MoodType::Attack)
			{
				if (AI.distance < BABOON_ATK_RANGE)
					item->Animation.TargetState = BABOON_IDLE;
				else if (AI.bite && AI.distance < BABOON_RUNROLL_RANGE)
					item->Animation.TargetState = BABOON_RUN_ROLL;
			}
			else
				item->Animation.TargetState = BABOON_RUN_ROLL;

			break;

		case BABOON_PICKUP:
			creature->MaxTurn = 0;
			// NOTE: baboon not use it ! (only TR3 one)
			break;

		case BABOON_ACTIVATE_SWITCH:
			creature->MaxTurn = 0;
			item->HitPoints = NOT_TARGETABLE;

			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 212)
			{
				GAME_VECTOR pos = { 0, 0, 0 };
				pos.boxNumber = 0;
				pos.roomNumber = NO_ROOM;

				switch (item->Position.yRot)
				{
				case -0x4000: // WEST (OK)
					pos.x = item->Position.xPos - SECTOR(1);
					pos.z = item->Position.zPos;
					break;

				case 0x4000: // EAST (OK)
					pos.x = item->Position.xPos + SECTOR(1);
					pos.z = item->Position.zPos;
					break;

				case 0:      // NORTH (NOP) maybe okay now with TombEngine
					pos.x = item->Position.xPos;
					pos.z = item->Position.zPos + SECTOR(1);
					break;

				case -0x8000: // SOUTH (OK)
					pos.x = item->Position.xPos;
					pos.z = item->Position.zPos - SECTOR(1);
					break;
				}

				pos.y = item->Position.yPos;
				pos.roomNumber = item->RoomNumber;
				FLOOR_INFO* floor = GetFloor(pos.x, pos.y, pos.z, &pos.roomNumber);
				int height = GetFloorHeight(floor, pos.x, pos.y, pos.z);
				item->Floor = height;
				TestTriggers(pos.x, pos.y, pos.z, pos.roomNumber, TRUE);
				item->TriggerFlags = 1;
			}

			break;

		case BABOON_ATK1:
		case BABOON_JUMPATK:
		case BABOON_SUPERJUMPATK:
			creature->MaxTurn = 0;

			if (AI.ahead)
				headY = AI.angle;

			if (abs(AI.angle) >= BABOON_ATTACK_ANGLE)
			{
				if (AI.angle >= 0)
					item->Position.yRot += BABOON_ATTACK_ANGLE;
				else
					item->Position.yRot -= BABOON_ATTACK_ANGLE;
			}
			else
				item->Position.yRot += AI.angle;

			if (creature->Flags == 0 &&
				(item->TouchBits & BABOON_TOUCHBITS ||
					item->TouchBits & BABOON_RIGHT_TOUCHBITS ||
					item->TouchBits & BABOON_JUMP_TOUCHBITS))
			{
				CreatureEffect2(item, &BaboonBite, 10, -1, DoBloodSplat);
				creature->Flags = 1;

				LaraItem->HitPoints -= BABOON_DAMAGE;
				LaraItem->HitStatus = true;
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, headY);
	CreatureAnimation(itemNumber, angle, tilt);
}

void BaboonRespawnClass::Free(void)
{
	baboonRespawnArray.clear();
}

void BaboonRespawnClass::Add(ITEM_INFO* item, unsigned int maxCount)
{
	BaboonRespawnStruct toAdd;
	toAdd.ID = GetBaboonFreePlace();
	toAdd.Pos = item->Position;
	toAdd.Count = 0;
	toAdd.MaxCount = maxCount;

	item->ItemFlags[0] = toAdd.ID; // conserve the id of baboon respawn position on the array...
	baboonRespawnArray.push_back(toAdd);
}

void BaboonRespawnClass::Remove(int ID)
{
	if (baboonRespawnArray.empty())
		return;

	for (auto i = baboonRespawnArray.begin(); i != baboonRespawnArray.end(); i++)
	{
		if (i->ID == ID)
			baboonRespawnArray.erase(i);
	}
}

int BaboonRespawnClass::GetBaboonFreePlace()
{
	if (baboonRespawnArray.empty())
		return 0;

	int j = 0;
	for (auto i = baboonRespawnArray.begin(); i != baboonRespawnArray.end(); i++, j++)
	{
		if (i->ID == NO_BABOON)
			return j;
	}

	return NO_BABOON;
}

BaboonRespawnStruct* BaboonRespawnClass::GetBaboonRespawn(int ID)
{
	if (baboonRespawnArray.empty())
		return nullptr;

	for (auto i = baboonRespawnArray.begin(); i != baboonRespawnArray.end(); i++)
	{
		if (i->ID == ID)
			return &*i;
	}

	return nullptr;
}

int BaboonRespawnClass::GetCount(int ID)
{
	if (baboonRespawnArray.empty())
		return NO_BABOON_COUNT;

	for (auto i = baboonRespawnArray.begin(); i != baboonRespawnArray.end(); i++)
	{
		if (i->ID == ID)
			return i->Count;
	}

	return NO_BABOON_COUNT;
}

int BaboonRespawnClass::GetCountMax(int ID)
{
	if (baboonRespawnArray.empty())
		return NO_BABOON_COUNT;

	for (auto i = baboonRespawnArray.begin(); i != baboonRespawnArray.end(); i++)
	{
		if (i->ID == ID)
			return i->MaxCount;
	}

	return NO_BABOON_COUNT;
}
