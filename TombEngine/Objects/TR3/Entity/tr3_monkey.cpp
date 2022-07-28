#include "framework.h"
#include "Objects/TR3/Entity/tr3_monkey.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::TR3
{
	BITE_INFO MonkeyBite = { 10, 10, 11, 13 };
	const vector<int> MonkeyAttackJoints = { 10, 13 };

	void InitialiseMonkey(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[ID_MONKEY].animIndex + 2;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = 6;
		item->Animation.TargetState = 6;
	}

	void MonkeyControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short headX = 0;
		short headY = 0;
		short torsoY = 0;
		short angle = 0;
		short tilt = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 11)
			{
				item->MeshBits = ALL_JOINT_BITS;
				item->Animation.AnimNumber = Objects[ID_MONKEY].animIndex + 14;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 11;
			}
		}
		else
		{
			GetAITarget(creature);

			if (creature->HurtByLara)
				creature->Enemy = LaraItem;
			else
			{
				int minDistance = 0x7FFFFFFF;
				creature->Enemy = NULL;

				for (int i = 0; i < ActiveCreatures.size(); i++)
				{
					auto* currentCreature = ActiveCreatures[i];
					if (currentCreature->ItemNumber == NO_ITEM || currentCreature->ItemNumber == itemNumber)
						continue;

					auto* target = &g_Level.Items[currentCreature->ItemNumber];
					if (target->ObjectNumber == ID_LARA || target->ObjectNumber == ID_MONKEY)
						continue;

					if (target->ObjectNumber == ID_SMALLMEDI_ITEM)
					{
						int x = target->Pose.Position.x - item->Pose.Position.x;
						int z = target->Pose.Position.z - item->Pose.Position.z;
						int distance = pow(x, 2) + pow(z, 2);

						if (distance < minDistance)
						{
							creature->Enemy = target;
							minDistance = distance;
						}
					}
				}
			}

			if (item->AIBits != MODIFY)
			{
				if (item->CarriedItem != NO_ITEM)
					item->MeshBits = 0xFFFFFEFF;
				else
					item->MeshBits = ALL_JOINT_BITS;
			}
			else
			{
				if (item->CarriedItem != NO_ITEM)
					item->MeshBits = 0xFFFF6E6F;
				else
					item->MeshBits = 0xFFFF6F6F;
			}

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (!creature->HurtByLara && creature->Enemy == LaraItem)
				creature->Enemy = NULL;

			AI_INFO laraAI;
			if (creature->Enemy == LaraItem)
			{
				laraAI.angle = AI.angle;
				laraAI.distance = AI.distance;
			}
			else
			{
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;

				laraAI.angle = phd_atan(dz, dz) - item->Pose.Orientation.y;
				laraAI.distance = pow(dx, 2) + pow(dz, 2);
			}

			GetCreatureMood(item, &AI, VIOLENT);

			if (Lara.Vehicle != NO_ITEM)
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);

			auto* enemy = creature->Enemy;
			creature->Enemy = LaraItem;

			if (item->HitStatus)
				AlertAllGuards(itemNumber);

			creature->Enemy = enemy;

			switch (item->Animation.ActiveState)
			{
			case 6:
				creature->Flags = 0;
				creature->MaxTurn = 0;
				torsoY = laraAI.angle;

				if (item->AIBits & GUARD)
				{
					torsoY = AIGuard(creature);
					if (!(GetRandomControl() & 0xF))
					{
						if (GetRandomControl() & 0x1)
							item->Animation.TargetState = 8;
						else
							item->Animation.TargetState = 7;
					}

					break;
				}

				else if (item->AIBits & PATROL1)
					item->Animation.TargetState = 2;
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 3;
				else if (creature->Mood == MoodType::Bored)
				{
					if (item->Animation.RequiredState)
						item->Animation.TargetState = item->Animation.RequiredState;
					else if (!(GetRandomControl() & 0xF))
						item->Animation.TargetState = 2;
					else if (!(GetRandomControl() & 0xF))
					{
						if (GetRandomControl() & 0x1)
							item->Animation.TargetState = 8;
						else
							item->Animation.TargetState = 7;
					}
				}
				else if ((item->AIBits & FOLLOW) && (creature->ReachedGoal || laraAI.distance > pow(SECTOR(2), 2)))
				{
					if (item->Animation.RequiredState)
						item->Animation.TargetState = item->Animation.RequiredState;
					else if (AI.ahead)
						item->Animation.TargetState = 6;
					else
						item->Animation.TargetState = 3;
				}
				else if (AI.bite && AI.distance < pow(682, 2))
					item->Animation.TargetState = 3;
				else if (AI.bite && AI.distance < pow(682, 2))
					item->Animation.TargetState = 2;
				else
					item->Animation.TargetState = 3;

				break;

			case 3:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				torsoY = laraAI.angle;

				if (item->AIBits & GUARD)
				{
					torsoY = AIGuard(creature);

					if (!(GetRandomControl() & 15))
					{
						if (GetRandomControl() & 1)
							item->Animation.TargetState = 10;
						else
							item->Animation.TargetState = 6;
					}

					break;
				}
				else if (item->AIBits & PATROL1)
					item->Animation.TargetState = 2;
				else if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead)
						item->Animation.TargetState = 3;
					else
						item->Animation.TargetState = 4;
				}
				else if (creature->Mood == MoodType::Bored)
				{
					if (item->Animation.RequiredState)
						item->Animation.TargetState = item->Animation.RequiredState;
					else if (!(GetRandomControl() & 15))
						item->Animation.TargetState = 2;
					else if (!(GetRandomControl() & 15))
					{
						if (GetRandomControl() & 1)
							item->Animation.TargetState = 10;
						else
							item->Animation.TargetState = 6;
					}
				}
				else if (item->AIBits & FOLLOW && (creature->ReachedGoal || laraAI.distance > pow(SECTOR(2), 2)))
				{
					if (item->Animation.RequiredState)
						item->Animation.TargetState = item->Animation.RequiredState;
					else if (AI.ahead)
						item->Animation.TargetState = 6;
					else
						item->Animation.TargetState = 4;
				}
				else if (AI.bite && AI.distance < pow(341, 2))
				{
					if (LaraItem->Pose.Position.y < item->Pose.Position.y)
						item->Animation.TargetState = 13;
					else
						item->Animation.TargetState = 12;
				}
				else if (AI.bite && AI.distance < pow(682, 2))
					item->Animation.TargetState = 14;
				else if (AI.bite && AI.distance < pow(682, 2))
					item->Animation.TargetState = 2;
				else if (AI.distance < pow(682, 2) && creature->Enemy != LaraItem && creature->Enemy != NULL &&
					creature->Enemy->ObjectNumber != ID_AI_PATROL1 && creature->Enemy->ObjectNumber != ID_AI_PATROL2 &&
					abs(item->Pose.Position.y - creature->Enemy->Pose.Position.y) < 256)
				{
					item->Animation.TargetState = 5;
				}
				else if (AI.bite && AI.distance < pow(SECTOR(1), 2))
					item->Animation.TargetState = 9;
				else
					item->Animation.TargetState = 4;

				break;

			case 5:
				creature->ReachedGoal = true;

				if (creature->Enemy == NULL)
					break;
				else if ((creature->Enemy->ObjectNumber == ID_SMALLMEDI_ITEM ||
					creature->Enemy->ObjectNumber == ID_KEY_ITEM4) &&
					item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 12)
				{
					if (creature->Enemy->RoomNumber == NO_ROOM ||
						creature->Enemy->Status == ITEM_INVISIBLE ||
						creature->Enemy->Flags & -32768)
						creature->Enemy = NULL;
					else
					{
						item->CarriedItem = creature->Enemy - g_Level.Items.data();
						RemoveDrawnItem(creature->Enemy - g_Level.Items.data());
						creature->Enemy->RoomNumber = NO_ROOM;
						creature->Enemy->CarriedItem = NO_ITEM;

						for (int i = 0; i < ActiveCreatures.size(); i++)
						{
							auto* currentCreature = ActiveCreatures[i];
							if (currentCreature->ItemNumber == NO_ITEM || currentCreature->ItemNumber == itemNumber)
								continue;

							auto* target = &g_Level.Items[currentCreature->ItemNumber];
							if (currentCreature->Enemy == creature->Enemy)
								currentCreature->Enemy = NULL;
						}

						creature->Enemy = NULL;

						if (item->AIBits != MODIFY)
						{
							item->AIBits |= AMBUSH;
							item->AIBits |= MODIFY;
						}
					}
				}
				else if (creature->Enemy->ObjectNumber == ID_AI_AMBUSH && item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 12)
				{
					item->AIBits = 0;

					auto* carriedItem = &g_Level.Items[item->CarriedItem];

					carriedItem->Pose.Position.x = item->Pose.Position.x;
					carriedItem->Pose.Position.y = item->Pose.Position.y;
					carriedItem->Pose.Position.z = item->Pose.Position.z;

					ItemNewRoom(item->CarriedItem, item->RoomNumber);
					item->CarriedItem = NO_ITEM;

					carriedItem->AIBits = GUARD;
					creature->Enemy = NULL;
				}
				else
				{
					creature->MaxTurn = 0;

					if (abs(AI.angle) < ANGLE(7.0f))
						item->Pose.Orientation.y += AI.angle;
					else if (AI.angle < 0)
						item->Pose.Orientation.y -= ANGLE(7.0f);
					else
						item->Pose.Orientation.y += ANGLE(7.0f);
				}

				break;

			case 2:
				creature->MaxTurn = ANGLE(7.0f);
				torsoY = laraAI.angle;

				if (item->AIBits & PATROL1)
				{
					item->Animation.TargetState = 2;
					torsoY = 0;
				}
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 4;
				else if (creature->Mood == MoodType::Bored)
				{
					if (GetRandomControl() < 256)
						item->Animation.TargetState = 6;
				}
				else if (AI.bite && AI.distance < pow(682, 2))
					item->Animation.TargetState = 3;

				break;

			case 4:
				creature->MaxTurn = ANGLE(11.0f);
				tilt = angle / 2;

				if (AI.ahead)
					torsoY = AI.angle;

				if (item->AIBits & GUARD)
					item->Animation.TargetState = 3;
				else if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead)
						item->Animation.TargetState = 3;
					break;
				}
				else if ((item->AIBits & FOLLOW) && (creature->ReachedGoal || laraAI.distance > pow(SECTOR(2), 2)))
					item->Animation.TargetState = 3;
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = 9;
				else if (AI.distance < pow(682, 2))
					item->Animation.TargetState = 3;
				else if (AI.bite && AI.distance < pow(SECTOR(1), 2))
					item->Animation.TargetState = 9;

				break;

			case 12:
				creature->MaxTurn = 0;

				if (AI.ahead)
				{
					headY = AI.angle;
					headX = AI.xAngle;
				}

				if (abs(AI.angle) < ANGLE(7.0f))
					item->Pose.Orientation.y += AI.angle;
				else if (AI.angle < 0)
					item->Pose.Orientation.y -= ANGLE(7.0f);
				else
					item->Pose.Orientation.y += ANGLE(7.0f);

				if (enemy->IsLara())
				{
					if (!creature->Flags && item->TestBits(JointBitType::Touch, MonkeyAttackJoints))
					{
						CreatureEffect(item, &MonkeyBite, DoBloodSplat);
						DoDamage(enemy, 40);
						creature->Flags = 1;
					}
				}
				else
				{
					if (!creature->Flags && enemy)
					{
						if (abs(enemy->Pose.Position.x - item->Pose.Position.x) < CLICK(1) &&
							abs(enemy->Pose.Position.y - item->Pose.Position.y) <= CLICK(1) &&
							abs(enemy->Pose.Position.z - item->Pose.Position.z) < CLICK(1))
						{
							CreatureEffect(item, &MonkeyBite, DoBloodSplat);
							DoDamage(enemy, 20);
							creature->Flags = 1;
						}
					}
				}

				break;

			case 13:
				creature->MaxTurn = 0;

				if (AI.ahead)
				{
					headY = AI.angle;
					headX = AI.xAngle;
				}

				if (abs(AI.angle) < ANGLE(7.0f))
					item->Pose.Orientation.y += AI.angle;
				else if (AI.angle < 0)
					item->Pose.Orientation.y -= ANGLE(7.0f);
				else
					item->Pose.Orientation.y += ANGLE(7.0f);

				if (enemy->IsLara())
				{
					if (!creature->Flags && item->TestBits(JointBitType::Touch, MonkeyAttackJoints))
					{
						CreatureEffect(item, &MonkeyBite, DoBloodSplat);
						DoDamage(enemy, 40);
						creature->Flags = 1;
					}
				}
				else
				{
					if (!creature->Flags && enemy)
					{
						if (abs(enemy->Pose.Position.x - item->Pose.Position.x) < CLICK(1) &&
							abs(enemy->Pose.Position.y - item->Pose.Position.y) <= CLICK(1) &&
							abs(enemy->Pose.Position.z - item->Pose.Position.z) < CLICK(1))
						{
							CreatureEffect(item, &MonkeyBite, DoBloodSplat);
							DoDamage(enemy, 20);
							creature->Flags = 1;
						}
					}
				}

				break;

			case 14:
				creature->MaxTurn = 0;

				if (AI.ahead)
				{
					headX = AI.xAngle;
					headY = AI.angle;
				}

				if (abs(AI.angle) < ANGLE(7.0f))
					item->Pose.Orientation.y += AI.angle;
				else if (AI.angle < 0)
					item->Pose.Orientation.y -= ANGLE(7.0f);
				else
					item->Pose.Orientation.y += ANGLE(7.0f);

				if (enemy->IsLara())
				{
					if (creature->Flags != 1 && item->TestBits(JointBitType::Touch, MonkeyAttackJoints))
					{
						CreatureEffect(item, &MonkeyBite, DoBloodSplat);
						DoDamage(enemy, 50);
						creature->Flags = 1;
					}
				}
				else
				{
					if (creature->Flags != 1 && enemy)
					{
						if (abs(enemy->Pose.Position.x - item->Pose.Position.x) < CLICK(1) &&
							abs(enemy->Pose.Position.y - item->Pose.Position.y) <= CLICK(1) &&
							abs(enemy->Pose.Position.z - item->Pose.Position.z) < CLICK(1))
						{
							CreatureEffect(item, &MonkeyBite, DoBloodSplat);
							DoDamage(enemy, 25);
							creature->Flags = 1;
						}
					}
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, headY);
		CreatureJoint(item, 1, headX);
		CreatureJoint(item, 2, torsoY);

		if (item->Animation.ActiveState < 15)
		{
			switch (CreatureVault(itemNumber, angle, 2, 128))
			{
			case 2:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = Objects[ID_MONKEY].animIndex + 19;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 17;
				break;

			case 3:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = Objects[ID_MONKEY].animIndex + 18;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 16;
				break;

			case 4:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = Objects[ID_MONKEY].animIndex + 17;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 15;
				break;

			case -2:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = Objects[ID_MONKEY].animIndex + 22;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 20;
				break;

			case -3:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = Objects[ID_MONKEY].animIndex + 21;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 19;
				break;

			case -4:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = Objects[ID_MONKEY].animIndex + 20;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 18;
				break;
			}
		}
		else
		{
			creature->MaxTurn = 0;
			CreatureAnimation(itemNumber, angle, tilt);
		}
	}
}
