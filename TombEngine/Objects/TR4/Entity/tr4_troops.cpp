#include "framework.h"
#include "tr4_troops.h"

#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/people.h"
#include "Game/itemdata/creature_info.h"
#include "Game/animation.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

namespace TEN::Entities::TR4
{
	BITE_INFO TroopsBite1 = { 0, 300, 64, 7 };

	enum TroopState
	{
		TROOP_STATE_IDLE = 1,
		TROOP_STATE_WALK = 2,
		TROOP_STATE_RUN = 3,
		TROOP_STATE_GUARD = 4,
		TROOP_STATE_ATTACK_1 = 5,
		TROOP_STATE_ATTACK_2 = 6,
		TROOP_STATE_DEATH = 7,
		TROOP_STATE_AIM_1 = 8,
		TROOP_STATE_AIM_2 = 9,
		TROOP_STATE_AIM_3 = 10,
		TROOP_STATE_ATTACK_3 = 11,
		TROOP_STATE_KILLED_BY_SCORPION = 15,
		TROOP_STATE_ATTACKED_BY_SCORPION = 16,
		TROOP_STATE_FLASHED = 17
	};

	// TODO
	enum TroopAnim
	{

	};

	void InitialiseTroops(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		if (item->TriggerFlags == 1)
		{
			item->Animation.TargetState = item->Animation.ActiveState = TROOP_STATE_ATTACKED_BY_SCORPION;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 27;
		}
		else
		{
			item->Animation.TargetState = item->Animation.ActiveState = TROOP_STATE_IDLE;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 12;
		}

		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	}

	void TroopsControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		auto* object = &Objects[item->ObjectNumber];

		short angle = 0;
		short tilt = 0;
		short rot = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;

		int dx = 0;
		int dy = 0;
		int dz = 0;

		int distance = 0;

		if (creature->FiredWeapon)
		{
			auto pos = Vector3Int(TroopsBite1.x, TroopsBite1.y, TroopsBite1.z);
			GetJointAbsPosition(item, &pos, TroopsBite1.meshNum);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * creature->FiredWeapon + 8, 24, 16, 4);

			creature->FiredWeapon--;
		}

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != TROOP_STATE_DEATH &&
				item->Animation.ActiveState != TROOP_STATE_KILLED_BY_SCORPION)
			{
				if (creature->Enemy &&
					creature->Enemy->ObjectNumber == ID_BIG_SCORPION &&
					item->ItemFlags[0] < 80)
				{
					if (creature->Enemy->Animation.AnimNumber == Objects[ID_BIG_SCORPION].animIndex + 6)
					{
						item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 23;

						if (item->Animation.ActiveState == TROOP_STATE_ATTACKED_BY_SCORPION)
							item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase + 37;
						else
							item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;

						item->Animation.ActiveState = TROOP_STATE_KILLED_BY_SCORPION;
						item->Animation.TargetState = TROOP_STATE_KILLED_BY_SCORPION;

						angle = 0;

						item->Pose.Position = creature->Enemy->Pose.Position;
						item->Pose.Orientation = creature->Enemy->Pose.Orientation;

						creature->Enemy->TriggerFlags = 99;
					}
					else
					{
						item->ItemFlags[0]++;
						angle = 0;
					}
				}
				else
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 19;
					item->Animation.ActiveState = TROOP_STATE_DEATH;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				}
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
			{
				// Search for active troops.
				creature->Enemy = nullptr;
				CreatureInfo* currentCreature = ActiveCreatures[0];

				int minDistance = INT_MAX;

				for (int i = 0; i < ActiveCreatures.size(); i++)
				{
					currentCreature = ActiveCreatures[i];

					if (currentCreature->ItemNumber != NO_ITEM && currentCreature->ItemNumber != itemNumber)
					{
						auto* currentItem = &g_Level.Items[currentCreature->ItemNumber];

						if (currentItem->ObjectNumber != ID_LARA)
						{
							if (currentItem->ObjectNumber != ID_TROOPS &&
								(currentItem != LaraItem || creature->HurtByLara))
							{
								dx = currentItem->Pose.Position.x - item->Pose.Position.x;
								dy = currentItem->Pose.Position.y - item->Pose.Position.y;
								dz = currentItem->Pose.Position.z - item->Pose.Position.z;

								distance = pow(dx, 2) + pow(dy, 2) + pow(dz, 2);

								if (distance < minDistance)
								{
									minDistance = distance;
									creature->Enemy = currentItem;
								}
							}
						}
					}
				}
			}

			if (creature->HurtByLara && item->Animation.ActiveState != TROOP_STATE_ATTACKED_BY_SCORPION)
				creature->Enemy = LaraItem;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			int distance = 0;
			if (creature->Enemy == LaraItem)
			{
				distance = AI.distance;
				rot = AI.angle;
			}
			else
			{
				dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				distance = pow(dx, 2) + pow(dz, 2);
				rot = phd_atan(dz, dx) - item->Pose.Orientation.y;
			}

			if (!creature->HurtByLara && creature->Enemy == LaraItem)
				creature->Enemy = nullptr;

			GetCreatureMood(item, &AI, TIMID);
			CreatureMood(item, &AI, TIMID);

			// Vehicle handling
			if (Lara.Vehicle != NO_ITEM && AI.bite)
				creature->Mood = MoodType::Escape;

			angle = CreatureTurn(item, creature->MaxTurn);

			if (item->HitStatus)
				AlertAllGuards(itemNumber);

			switch (item->Animation.ActiveState)
			{
			case TROOP_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				joint2 = rot;

				if (item->Animation.AnimNumber == object->animIndex + 17)
				{
					if (abs(AI.angle) >= ANGLE(10.0f))
					{
						if (AI.angle >= 0)
							item->Pose.Orientation.y += ANGLE(10.0f);
						else
							item->Pose.Orientation.y -= ANGLE(10.0f);
					}
					else
						item->Pose.Orientation.y += AI.angle;
				}

				if (item->AIBits & GUARD)
				{
					joint2 = AIGuard(creature);

					if (!GetRandomControl())
					{
						if (item->Animation.ActiveState == TROOP_STATE_IDLE)
							item->Animation.TargetState = TROOP_STATE_GUARD;
						else
							item->Animation.TargetState = TROOP_STATE_IDLE;
					}
				}
				else if (item->AIBits & PATROL1)
				{
					item->Animation.TargetState = TROOP_STATE_WALK;
					joint2 = 0;
				}
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = TROOP_STATE_RUN;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < pow(SECTOR(3), 2) || AI.zoneNumber != AI.enemyZone)
					{
						if (GetRandomControl() >= 16384)
							item->Animation.TargetState = TROOP_STATE_AIM_3;
						else
							item->Animation.TargetState = TROOP_STATE_AIM_1;
					}
					else
						item->Animation.TargetState = TROOP_STATE_WALK;
				}
				else
				{
					if ((creature->Alerted || creature->Mood != MoodType::Bored) &&
						(!(item->AIBits & FOLLOW) || !(item->AIBits & MODIFY) && distance <= pow(SECTOR(2), 2)))
					{
						if (creature->Mood == MoodType::Bored || AI.distance <= pow(SECTOR(2), 2))
						{
							item->Animation.TargetState = TROOP_STATE_WALK;
							break;
						}

						item->Animation.TargetState = TROOP_STATE_RUN;
					}
					else
						item->Animation.TargetState = TROOP_STATE_IDLE;
				}

				break;

			case TROOP_STATE_WALK:
				creature->MaxTurn = ANGLE(5.0f);
				creature->Flags = 0;
				joint2 = rot;

				if (item->AIBits & PATROL1)
					item->Animation.TargetState = TROOP_STATE_WALK;
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = TROOP_STATE_RUN;
				else
				{
					if (item->AIBits & GUARD ||
						item->AIBits & FOLLOW &&
						(creature->ReachedGoal ||
							distance > pow(SECTOR(2), 2)))
					{
						item->Animation.TargetState = TROOP_STATE_IDLE;
					}
					else if (Targetable(item, &AI))
					{
						if (AI.distance < pow(SECTOR(3), 2) || AI.enemyZone != AI.zoneNumber)
							item->Animation.TargetState = TROOP_STATE_IDLE;
						else
							item->Animation.TargetState = TROOP_STATE_AIM_2;
					}
					else if (creature->Mood != MoodType::Bored)
					{
						if (AI.distance > pow(SECTOR(2), 2))
							item->Animation.TargetState = TROOP_STATE_RUN;
					}
					else if (AI.ahead)
						item->Animation.TargetState = TROOP_STATE_IDLE;
				}

				break;

			case TROOP_STATE_RUN:
				creature->MaxTurn = ANGLE(10.0f);
				tilt = angle / 2;

				if (AI.ahead)
					joint2 = AI.angle;

				if (item->AIBits & GUARD ||
					item->AIBits & FOLLOW &&
					(creature->ReachedGoal ||
						distance > pow(SECTOR(2), 2)))
				{
					item->Animation.TargetState = TROOP_STATE_WALK;
				}
				else if (creature->Mood != MoodType::Escape)
				{
					if (Targetable(item, &AI))
						item->Animation.TargetState = TROOP_STATE_WALK;
					else if (creature->Mood == MoodType::Bored ||
						creature->Mood == MoodType::Stalk &&
						!(item->AIBits & FOLLOW) &&
						AI.distance < pow(SECTOR(2), 2))
					{
						item->Animation.TargetState = TROOP_STATE_WALK;
					}
				}

				break;

			case TROOP_STATE_GUARD:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				joint2 = rot;

				if (item->AIBits & GUARD)
				{
					joint2 = AIGuard(creature);
					if (!GetRandomControl())
						item->Animation.TargetState = TROOP_STATE_IDLE;
				}
				else if (Targetable(item, &AI))
					item->Animation.TargetState = TROOP_STATE_ATTACK_1;
				else if (creature->Mood != MoodType::Bored || !AI.ahead)
					item->Animation.TargetState = TROOP_STATE_IDLE;

				break;

			case TROOP_STATE_ATTACK_1:
			case TROOP_STATE_ATTACK_2:
				if (AI.ahead)
				{
					joint0 = AI.angle;
					joint1 = AI.xAngle;
				}

				if (creature->Flags)
					creature->Flags--;
				else
				{
					creature->FiredWeapon = 1;
					ShotLara(item, &AI, &TroopsBite1, joint0, 23);
					creature->Flags = 5;
				}

				break;

			case TROOP_STATE_AIM_1:
			case TROOP_STATE_AIM_3:
				creature->Flags = 0;

				if (AI.ahead)
				{
					joint1 = AI.xAngle;
					joint0 = AI.angle;

					if (Targetable(item, &AI))
						item->Animation.TargetState = item->Animation.ActiveState != TROOP_STATE_AIM_1 ? TROOP_STATE_ATTACK_3 : TROOP_STATE_ATTACK_1;
					else
						item->Animation.TargetState = TROOP_STATE_IDLE;
				}

				break;

			case TROOP_STATE_AIM_2:
				creature->Flags = 0;

				if (AI.ahead)
				{
					joint1 = AI.xAngle;
					joint0 = AI.angle;

					if (Targetable(item, &AI))
						item->Animation.TargetState = TROOP_STATE_ATTACK_2;
					else
						item->Animation.TargetState = TROOP_STATE_WALK;
				}

				break;

			case TROOP_STATE_ATTACK_3:
				if (item->Animation.TargetState != TROOP_STATE_IDLE &&
					(creature->Mood == MoodType::Escape ||
						AI.distance > pow(SECTOR(3), 2) ||
						!Targetable(item, &AI)))
				{
					item->Animation.TargetState = TROOP_STATE_IDLE;
				}

				if (AI.ahead)
				{
					joint0 = AI.angle;
					joint1 = AI.xAngle;
				}

				if (creature->Flags)
					creature->Flags--;
				else
				{
					creature->FiredWeapon = 1;
					ShotLara(item, &AI, &TroopsBite1, joint0, 23);
					creature->Flags = 5;
				}

				break;

			case TROOP_STATE_ATTACKED_BY_SCORPION:
				creature->MaxTurn = 0;
				break;

			case TROOP_STATE_FLASHED:
				if (!FlashGrenadeAftershockTimer && !(GetRandomControl() & 0x7F))
					item->Animation.TargetState = TROOP_STATE_GUARD;

				break;

			default:
				break;
			}

			if (FlashGrenadeAftershockTimer > 100)
			{
				if (item->Animation.ActiveState != TROOP_STATE_FLASHED &&
					item->Animation.ActiveState != TROOP_STATE_ATTACKED_BY_SCORPION)
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 28;
					item->Animation.ActiveState = TROOP_STATE_FLASHED;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase + (GetRandomControl() & 7);
					creature->MaxTurn = 0;
				}
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
