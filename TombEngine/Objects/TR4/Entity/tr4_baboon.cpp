#include "framework.h"
#include "tr4_baboon.h"
#include "Game/collision/collide_room.h"
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

namespace TEN::Entities::TR4
{
	BaboonRespawner BaboonRespawn;
	BITE_INFO BaboonBite = { 10, 10, 11, 4 };
	const vector<int> BaboonAttackJoints = { 11, 12 };
	const vector<int> BaboonAttackRightJoints = { 1, 2, 3, 5, 8, 9 };
	const vector<int> BaboonJumpAttackJoints = { 3, 4, 8 };

	constexpr auto BABOON_ATTACK_DAMAGE = 70;

	constexpr auto BABOON_ATTACK_RANGE = SECTOR(0.34f);
	constexpr auto BABOON_ATTACK_READY_RANGE = SECTOR(0.67f);
	constexpr auto BABOON_JUMP_ATTACK_2_RANGE = SECTOR(0.67f);
	constexpr auto BABOON_IDLE_RANGE = SECTOR(1);
	constexpr auto BABOON_ROLL_FORWARD_RANGE = SECTOR(1);
	constexpr auto BABOON_FOLLOW_RANGE = SECTOR(2);

	constexpr auto NO_BABOON = -1;
	constexpr auto NO_BABOON_COUNT = -2;
	constexpr auto NO_CROWBAR_SWITCH_FOUND = -1;

	#define BABOON_ATTACK_ANGLE ANGLE(7.0f)
	#define BABOON_WALK_FORWARD_TURN_ANGLE ANGLE(7.0f)
	#define BABOON_RUN_FORWARD_TURN_ANGLE ANGLE(11.0f)

	#define BABOON_STATE_WALK_ANIM 14 // TODO: What is this?

	enum BaboonState
	{
		BABOON_STATE_NULL = 0,
		BABOON_STATE_NONE = 1,
		BABOON_STATE_WALK_FORWARD = 2,
		BABOON_STATE_IDLE = 3,
		BABOON_STATE_RUN_FORWARD = 4,
		BABOON_STATE_PICKUP = 5,
		BABOON_STATE_SIT_IDLE = 6,
		BABOON_STATE_EAT = 7,
		BABOON_STATE_SIT_SCRATCH = 8,
		BABOON_STATE_RUN_FORWARD_ROLL = 9,
		BABOON_STATE_HIT_GROUND = 10,
		BABOON_STATE_WALK = 11,
		BABOON_STATE_SWIPE_ATTACK = 12,
		BABOON_STATE_JUMP_ATTACK_1 = 13,
		BABOON_STATE_JUMP_ATTACK_2 = 14,

		// Unused in TR4:
		BABOON_STATE_CLIMB_4_STEPS = 15,
		BABOON_STATE_CLIMB_3_STEPS = 16,
		BABOON_STATE_CLIMB_2_STEPS = 17,
		BABOON_STATE_FALL_4_STEPS = 18,
		BABOON_STATE_FALL_3_STEPS = 19,
		BABOON_STATE_FALL_2_STEPS = 20,

		BABOON_ACTIVATE_SWITCH = 21
	};

	enum BaboonAnim
	{
		BABOON_ANIM_WALK_FORWARD = 0,
		BABOON_ANIM_WALK_FORWARD_TO_SIT_IDLE = 1,
		BABOON_ANIM_SIT_IDLE = 2,
		BABOON_ANIM_SIT_IDLE_TO_WALK_FORWARD = 3,
		BABOON_ANIM_SIT_IDLE_EAT = 4,
		BABOON_ANIM_SIT_IDLE_SCRATCH = 5,
		BABOON_ANIM_RUN_FORWARD = 6,
		BABOON_ANIM_RUN_FORWARD_ROLL = 7,
		BABOON_ANIM_HIT_GROUND = 8,
		BABOON_ANIM_IDLE = 9,
		BABOON_ANIM_IDLE_TO_RUN_FORWARD_1 = 10,
		BABOON_ANIM_IDLE_TO_RUN_FORWARD_2 = 11,
		BABOON_ANIM_RUN_FORWARD_TO_IDLE = 12,
		BABOON_ANIM_SIT_IDLE_TO_IDLE = 13,
		BABOON_ANIM_DEATH = 14,
		BABOON_ANIM_RUN_FORWARD_TO_WALK_FORWARD = 15,
		BABOON_ANIM_IDLE_TO_SIT_IDLE = 16,

		// TODO: 17-22?

		BABOON_ANIM_SWIPE_ATTACK = 23,
		BABOON_ANIM_JUMP_ATTACK_1 = 24,
		BABOON_ANIM_PICKUP = 25,
		BABOON_ANIM_JUMP_ATTACK_2_START = 26,
		BABOON_ANIM_JUMP_ATTACK_2_CONTINUE = 27,
		BABOON_ANIM_JUMP_ATTACK_2_END = 28,
		BABOON_ANIM_IDLE_TO_WALK_FORWARD = 29,
		BABOON_ANIM_WALK_FORWARD_TO_IDLE = 30,
		BABOON_ANIM_ACTIVATE_SWITCH = 31
	};

	static void TriggerBaboonShockwave(PHD_3DPOS pos, short xRot)
	{
		short shockwaveID = GetFreeShockwave();
		if (shockwaveID != NO_ITEM)
		{
			auto* deathEffect = &ShockWaves[shockwaveID];

			deathEffect->x = pos.Position.x;
			deathEffect->y = pos.Position.y;
			deathEffect->z = pos.Position.z;
			deathEffect->innerRad = 512;
			deathEffect->outerRad = 640;
			deathEffect->xRot = xRot;
			deathEffect->r = 255;
			deathEffect->g = 64;
			deathEffect->b = 0;
			deathEffect->speed = -600;
			deathEffect->life = 64;
		}
	}

	void BaboonDieEffect(ItemInfo* item)
	{
		auto pos = PHD_3DPOS(item->Pose.Position.x, item->Pose.Position.y - CLICK(0.5f), item->Pose.Position.z);

		// Trigger shockwave effect.
		TriggerBaboonShockwave(pos, ANGLE(0.0f));
		TriggerBaboonShockwave(pos, ANGLE(45.0f));
		TriggerBaboonShockwave(pos, ANGLE(90.0f));
		TriggerBaboonShockwave(pos, ANGLE(135.0f));

		// Trigger flash screen.
		Weather.Flash(255, 64, 0, 0.03f);
	}

	static void KillRespawnedBaboon(short itemNumber, bool remove = false)
	{
		auto* item = &g_Level.Items[itemNumber];

		DoDamage(item, INT_MAX);
		RemoveActiveItem(itemNumber);	// Remove it from the active item list.

		item->Flags = IFLAG_CLEAR_BODY;
		item->AfterDeath = 128;				// Disappear.
		item->Status = ITEM_DEACTIVATED;	// Don't trigger again.

		if (remove)
			item->ItemFlags[0] = NO_BABOON;

		DisableEntityAI(itemNumber);	// Deactivate AI to prevent crash.
	}

	static bool CheckRespawnedBaboon(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		if (item->ItemFlags[0] == NO_BABOON)	// NORMAL/INV for now.
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

		item->Pose = baboonRespawn->Pose;

		auto outsideRoom = IsRoomOutside(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
		if (item->RoomNumber != outsideRoom && outsideRoom != NO_ROOM)
			ItemNewRoom(itemNumber, outsideRoom);

		if (baboonRespawn->Count < baboonRespawn->MaxCount)
			baboonRespawn->Count++;

		item->Animation.AnimNumber = object->animIndex + BABOON_ANIM_SIT_IDLE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = BABOON_STATE_SIT_IDLE;
		item->Animation.TargetState = BABOON_STATE_SIT_IDLE;
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

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BABOON_ANIM_SIT_IDLE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = BABOON_STATE_SIT_IDLE;
		item->Animation.TargetState = BABOON_STATE_SIT_IDLE;

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
			if (item->Animation.ActiveState == BABOON_STATE_WALK)
			{
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
					BaboonRespawnFunction(itemNumber);
			}
			else if (item->Animation.ActiveState != BABOON_ACTIVATE_SWITCH)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BABOON_STATE_WALK_ANIM;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = BABOON_STATE_WALK;
				item->Animation.TargetState = BABOON_STATE_WALK;
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
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;

				laraAI.angle = phd_atan(dx, dz) - item->Pose.Orientation.y;
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
					abs(item->Pose.Position.x - creature->Enemy->Pose.Position.x) < CLICK(1) &&
					abs(item->Pose.Position.y - creature->Enemy->Pose.Position.y) < CLICK(1) &&
					abs(item->Pose.Position.z - creature->Enemy->Pose.Position.z) < CLICK(1))
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BABOON_ANIM_ACTIVATE_SWITCH;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = BABOON_ACTIVATE_SWITCH;
					item->Animation.TargetState = BABOON_ACTIVATE_SWITCH;
					item->Pose.Position = creature->Enemy->Pose.Position;
					item->Pose.Orientation.y = creature->Enemy->Pose.Orientation.y;
					item->AIBits &= ~(FOLLOW);

					TestTriggers(item, true);

					creature->Enemy = nullptr;
				}
			}

			switch (item->Animation.ActiveState)
			{
			case BABOON_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (item->AIBits & GUARD)
				{
					AIGuard(creature);
					if (!(GetRandomControl() & 0xF))
					{
						if (GetRandomControl() & 1)
							item->Animation.TargetState = BABOON_STATE_HIT_GROUND;
						else
							item->Animation.TargetState = BABOON_STATE_SIT_IDLE;
					}
				}
				else if (item->AIBits & PATROL1)
					item->Animation.TargetState = BABOON_STATE_WALK_FORWARD;
				else if (creature->Mood == MoodType::Escape)
				{
					if (AI.ahead && Lara.TargetEntity != item)
						item->Animation.TargetState = BABOON_STATE_IDLE;
					else
						item->Animation.TargetState = BABOON_STATE_RUN_FORWARD;
				}
				else if (creature->Mood == MoodType::Attack)
				{
					if (!(item->AIBits & FOLLOW) || (!item->Animation.IsAirborne && AI.distance <= pow(BABOON_ROLL_FORWARD_RANGE, 2)))
					{
						if (AI.bite && AI.distance < pow(BABOON_ATTACK_RANGE, 2))
						{
							if (LaraItem->Pose.Position.y >= item->Pose.Position.y)
								item->Animation.TargetState = BABOON_STATE_SWIPE_ATTACK;
							else
								item->Animation.TargetState = BABOON_STATE_JUMP_ATTACK_1;
						}
						else if (AI.bite && AI.distance < pow(BABOON_JUMP_ATTACK_2_RANGE, 2))
							item->Animation.TargetState = BABOON_STATE_JUMP_ATTACK_2;
						else if (AI.bite && AI.distance < pow(BABOON_ROLL_FORWARD_RANGE, 2))
							item->Animation.TargetState = BABOON_STATE_RUN_FORWARD_ROLL;
						else
							item->Animation.TargetState = BABOON_STATE_RUN_FORWARD;
					}
					else if (item->Animation.RequiredState)
						item->Animation.TargetState = item->Animation.RequiredState;
					else if (GetRandomControl() & 1)
						item->Animation.TargetState = BABOON_STATE_SIT_IDLE;
				}
				else if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (!(GetRandomControl() & 3))
					item->Animation.TargetState = BABOON_STATE_WALK_FORWARD;
				else if (!(GetRandomControl() & 1))
					item->Animation.TargetState = BABOON_STATE_RUN_FORWARD_ROLL;
				else if (GetRandomControl() & 4)
					item->Animation.TargetState = BABOON_STATE_HIT_GROUND;

				break;

			case BABOON_STATE_SIT_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (item->AIBits & GUARD)
				{
					AIGuard(creature);
					if (GetRandomControl() & 0xF)
						item->Animation.TargetState = BABOON_STATE_EAT;
					else if (GetRandomControl() & 0xA)
						item->Animation.TargetState = BABOON_STATE_SIT_SCRATCH;
				}
				else if (item->AIBits & PATROL1)
					item->Animation.TargetState = BABOON_STATE_WALK_FORWARD;
				else if (creature->Mood != MoodType::Escape)
				{
					if (creature->Mood == MoodType::Bored)
					{
						// NOTE: It's not true to the original functionality, but to avoid repetitive actions, I gave
						// the SIT_IDLE state a higher chance of occurring. The EAT state was also added here. @TokyoSU

						if (item->Animation.RequiredState)
							item->Animation.TargetState = item->Animation.RequiredState;
						else if (GetRandomControl() & 0x10)
							item->Animation.TargetState = BABOON_STATE_SIT_IDLE;
						else if (GetRandomControl() & 0x500)
						{
							if (GetRandomControl() & 0x200)
								item->Animation.TargetState = BABOON_STATE_SIT_SCRATCH;
							else if (GetRandomControl() & 0x250)
								item->Animation.TargetState = BABOON_STATE_EAT;
						}
						else if (GetRandomControl() & 0x1000 || item->AIBits & FOLLOW)
							item->Animation.TargetState = BABOON_STATE_WALK_FORWARD;
					}
					else if ((item->AIBits & FOLLOW) && AI.distance > pow(BABOON_IDLE_RANGE, 2))
					{
						if (item->Animation.RequiredState)
							item->Animation.TargetState = item->Animation.RequiredState;
						else
							item->Animation.TargetState = BABOON_STATE_WALK_FORWARD;
					}
					else
						item->Animation.TargetState = BABOON_STATE_WALK_FORWARD;
				}
				else
					item->Animation.TargetState = BABOON_STATE_IDLE;

				break;

			case BABOON_STATE_WALK_FORWARD:
				creature->MaxTurn = BABOON_WALK_FORWARD_TURN_ANGLE;

				if (item->AIBits & PATROL1)
					item->Animation.TargetState = BABOON_STATE_WALK_FORWARD;
				else if (creature->Mood == MoodType::Bored)
				{
					if (item->AIBits & FOLLOW)
						item->Animation.TargetState = BABOON_STATE_WALK_FORWARD;
					else if (GetRandomControl() < 256)
						item->Animation.TargetState = BABOON_STATE_SIT_IDLE;
				}
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = BABOON_STATE_RUN_FORWARD;
				else if (creature->Mood == MoodType::Attack)
				{
					if (AI.bite && AI.distance < pow(BABOON_ATTACK_READY_RANGE, 2))
						item->Animation.TargetState = BABOON_STATE_IDLE;
				}
				else if (GetRandomControl() < 256)
					item->Animation.TargetState = BABOON_STATE_SIT_IDLE;

				break;

			case BABOON_STATE_RUN_FORWARD:
				creature->MaxTurn = BABOON_RUN_FORWARD_TURN_ANGLE;
				tilt = angle / 2;

				if (item->AIBits & GUARD)
					item->Animation.TargetState = BABOON_STATE_IDLE;
				else if (creature->Mood == MoodType::Escape)
				{
					if (AI.ahead && Lara.TargetEntity != item)
						item->Animation.TargetState = BABOON_STATE_IDLE;
				}
				else if (item->AIBits & FOLLOW && (item->Animation.IsAirborne || AI.distance > pow(BABOON_FOLLOW_RANGE, 2)))
					item->Animation.TargetState = BABOON_STATE_IDLE;
				else if (creature->Mood == MoodType::Attack)
				{
					if (AI.distance < pow(BABOON_ATTACK_READY_RANGE, 2))
						item->Animation.TargetState = BABOON_STATE_IDLE;
					else if (AI.bite && AI.distance < pow(BABOON_ROLL_FORWARD_RANGE, 2))
						item->Animation.TargetState = BABOON_STATE_RUN_FORWARD_ROLL;
				}
				else
					item->Animation.TargetState = BABOON_STATE_RUN_FORWARD_ROLL;

				break;

			case BABOON_STATE_PICKUP:
				creature->MaxTurn = 0;
				// NOTE: baboon not use it ! (only TR3 one)
				break;

			case BABOON_ACTIVATE_SWITCH:
				creature->MaxTurn = 0;
				item->HitPoints = NOT_TARGETABLE;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 212)
				{
					auto pos = Vector3Int();
					if (item->Pose.Orientation.y == ANGLE(270.0f))
					{
						pos.x = item->Pose.Position.x - SECTOR(1);
						pos.z = item->Pose.Position.z;
					}
					else if (item->Pose.Orientation.y == ANGLE(90.0f))
					{
						pos.x = item->Pose.Position.x + SECTOR(1);
						pos.z = item->Pose.Position.z;
					}
					else if (item->Pose.Orientation.y == ANGLE(0.0f))
					{
						pos.x = item->Pose.Position.x;
						pos.z = item->Pose.Position.z + SECTOR(1);
					}
					else if (item->Pose.Orientation.y == ANGLE(180.0f))
					{
						pos.x = item->Pose.Position.x;
						pos.z = item->Pose.Position.z - SECTOR(1);
					}

					pos.y = item->Pose.Position.y;

					auto probe = GetCollision(pos.x, pos.y, pos.z, item->RoomNumber);
					item->Floor = probe.Position.Floor;
					TestTriggers(pos.x, pos.y, pos.z, probe.RoomNumber, true);
					item->TriggerFlags = 1;
				}

				break;

			case BABOON_STATE_SWIPE_ATTACK:
			case BABOON_STATE_JUMP_ATTACK_1:
			case BABOON_STATE_JUMP_ATTACK_2:
				creature->MaxTurn = 0;

				if (AI.ahead)
					headY = AI.angle;

				if (abs(AI.angle) >= BABOON_ATTACK_ANGLE)
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += BABOON_ATTACK_ANGLE;
					else
						item->Pose.Orientation.y -= BABOON_ATTACK_ANGLE;
				}
				else
					item->Pose.Orientation.y += AI.angle;

				if (creature->Flags == 0 &&
					(item->TestBits(JointBitType::Touch, BaboonAttackJoints) ||
					 item->TestBits(JointBitType::Touch, BaboonAttackRightJoints) ||
					 item->TestBits(JointBitType::Touch, BaboonJumpAttackJoints)))
				{
					CreatureEffect2(item, &BaboonBite, 10, -1, DoBloodSplat);
					DoDamage(creature->Enemy, BABOON_ATTACK_DAMAGE);
					creature->Flags = 1;
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, headY);
		CreatureAnimation(itemNumber, angle, tilt);
	}

	void BaboonRespawner::Free(void)
	{
		BaboonRespawnArray.clear();
	}

	void BaboonRespawner::Add(ItemInfo* item, unsigned int maxCount)
	{
		BaboonRespawnData toAdd;
		toAdd.ID = GetBaboonFreePlace();
		toAdd.Pose = item->Pose;
		toAdd.Count = 0;
		toAdd.MaxCount = maxCount;

		item->ItemFlags[0] = toAdd.ID; // conserve the id of baboon respawn position on the array...
		BaboonRespawnArray.push_back(toAdd);
	}

	void BaboonRespawner::Remove(int ID)
	{
		if (BaboonRespawnArray.empty())
			return;

		for (auto i = BaboonRespawnArray.begin(); i != BaboonRespawnArray.end(); i++)
		{
			if (i->ID == ID)
				BaboonRespawnArray.erase(i);
		}
	}

	int BaboonRespawner::GetBaboonFreePlace()
	{
		if (BaboonRespawnArray.empty())
			return 0;

		int j = 0;
		for (auto i = BaboonRespawnArray.begin(); i != BaboonRespawnArray.end(); i++, j++)
		{
			if (i->ID == NO_BABOON)
				return j;
		}

		return NO_BABOON;
	}

	BaboonRespawnData* BaboonRespawner::GetBaboonRespawn(int ID)
	{
		if (BaboonRespawnArray.empty())
			return nullptr;

		for (auto i = BaboonRespawnArray.begin(); i != BaboonRespawnArray.end(); i++)
		{
			if (i->ID == ID)
				return &*i;
		}

		return nullptr;
	}

	int BaboonRespawner::GetCount(int ID)
	{
		if (BaboonRespawnArray.empty())
			return NO_BABOON_COUNT;

		for (auto i = BaboonRespawnArray.begin(); i != BaboonRespawnArray.end(); i++)
		{
			if (i->ID == ID)
				return i->Count;
		}

		return NO_BABOON_COUNT;
	}

	int BaboonRespawner::GetCountMax(int ID)
	{
		if (BaboonRespawnArray.empty())
			return NO_BABOON_COUNT;

		for (auto i = BaboonRespawnArray.begin(); i != BaboonRespawnArray.end(); i++)
		{
			if (i->ID == ID)
				return i->MaxCount;
		}

		return NO_BABOON_COUNT;
	}
}
