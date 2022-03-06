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

BITE_INFO MonkeyBite = { 10, 10, 11, 13 };

void InitialiseMonkey(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->AnimNumber = Objects[ID_MONKEY].animIndex + 2;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->ActiveState = 6;
	item->TargetState = 6;
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
		if (item->ActiveState != 11)
		{
			item->MeshBits = -1;
			item->AnimNumber = Objects[ID_MONKEY].animIndex + 14;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 11;
		}
	}
	else
	{
		GetAITarget(creature);

		if (creature->hurtByLara)
			creature->enemy = LaraItem;
		else
		{
			int minDistance = 0x7FFFFFFF;
			creature->enemy = NULL;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				auto* currentCreature = ActiveCreatures[i];
				if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNumber)
					continue;

				auto* target = &g_Level.Items[currentCreature->itemNum];
				if (target->ObjectNumber == ID_LARA || target->ObjectNumber == ID_MONKEY)
					continue;

				if (target->ObjectNumber == ID_SMALLMEDI_ITEM)
				{
					int x = target->Position.xPos - item->Position.xPos;
					int z = target->Position.zPos - item->Position.zPos;
					int distance = pow(x, 2) + pow(z, 2);

					if (distance < minDistance)
					{
						creature->enemy = target;
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
				item->MeshBits = -1;
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

		if (!creature->hurtByLara && creature->enemy == LaraItem)
			creature->enemy = NULL;

		AI_INFO laraAI;
		if (creature->enemy == LaraItem)
		{
			laraAI.angle = AI.angle;
			laraAI.distance = AI.distance;
		}
		else
		{
			int dx = LaraItem->Position.xPos - item->Position.xPos;
			int dz = LaraItem->Position.zPos - item->Position.zPos;

			laraAI.angle = phd_atan(dz, dz) - item->Position.yRot;
			laraAI.distance = pow(dx, 2) + pow(dz, 2);
		}

		GetCreatureMood(item, &AI, VIOLENT);

		if (Lara.Vehicle != NO_ITEM)
			creature->mood = MoodType::Escape;

		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		auto* enemy = creature->enemy;
		creature->enemy = LaraItem;

		if (item->HitStatus)
			AlertAllGuards(itemNumber);

		creature->enemy = enemy;

		switch (item->ActiveState)
		{
		case 6:
			creature->flags = 0;
			creature->maximumTurn = 0;
			torsoY = laraAI.angle;

			if (item->AIBits & GUARD)
			{
				torsoY = AIGuard(creature);
				if (!(GetRandomControl() & 0xF))
				{
					if (GetRandomControl() & 0x1)
						item->TargetState = 8;
					else
						item->TargetState = 7;
				}

				break;
			}

			else if (item->AIBits & PATROL1)
				item->TargetState = 2;
			else if (creature->mood == MoodType::Escape)
				item->TargetState = 3;
			else if (creature->mood == MoodType::Bored)
			{
				if (item->RequiredState)
					item->TargetState = item->RequiredState;
				else if (!(GetRandomControl() & 0xF))
					item->TargetState = 2;
				else if (!(GetRandomControl() & 0xF))
				{
					if (GetRandomControl() & 0x1)
						item->TargetState = 8;
					else
						item->TargetState = 7;
				}
			}
			else if ((item->AIBits & FOLLOW) && (creature->reachedGoal || laraAI.distance > pow(SECTOR(2), 2)))
			{
				if (item->RequiredState)
					item->TargetState = item->RequiredState;
				else if (AI.ahead)
					item->TargetState = 6;
				else
					item->TargetState = 3;
			}
			else if (AI.bite && AI.distance < pow(682, 2))
				item->TargetState = 3;
			else if (AI.bite && AI.distance < pow(682, 2))
				item->TargetState = 2;
			else
				item->TargetState = 3;

			break;

		case 3:
			creature->maximumTurn = 0;
			creature->flags = 0;
			torsoY = laraAI.angle;

			if (item->AIBits & GUARD)
			{
				torsoY = AIGuard(creature);

				if (!(GetRandomControl() & 15))
				{
					if (GetRandomControl() & 1)
						item->TargetState = 10;
					else
						item->TargetState = 6;
				}

				break;
			}
			else if (item->AIBits & PATROL1)
				item->TargetState = 2;
			else if (creature->mood == MoodType::Escape)
			{
				if (Lara.target != item && AI.ahead)
					item->TargetState = 3;
				else
					item->TargetState = 4;
			}
			else if (creature->mood == MoodType::Bored)
			{
				if (item->RequiredState)
					item->TargetState = item->RequiredState;
				else if (!(GetRandomControl() & 15))
					item->TargetState = 2;
				else if (!(GetRandomControl() & 15))
				{
					if (GetRandomControl() & 1)
						item->TargetState = 10;
					else
						item->TargetState = 6;
				}
			}
			else if (item->AIBits & FOLLOW && (creature->reachedGoal || laraAI.distance > pow(SECTOR(2), 2)))
			{
				if (item->RequiredState)
					item->TargetState = item->RequiredState;
				else if (AI.ahead)
					item->TargetState = 6;
				else
					item->TargetState = 4;
			}
			else if (AI.bite && AI.distance < pow(341, 2))
			{
				if (LaraItem->Position.yPos < item->Position.yPos)
					item->TargetState = 13;
				else
					item->TargetState = 12;
			}
			else if (AI.bite && AI.distance < pow(682, 2))
				item->TargetState = 14;
			else if (AI.bite && AI.distance < pow(682, 2))
				item->TargetState = 2;
			else if (AI.distance < pow(682, 2) && creature->enemy != LaraItem && creature->enemy != NULL &&
				creature->enemy->ObjectNumber != ID_AI_PATROL1 && creature->enemy->ObjectNumber != ID_AI_PATROL2 &&
				abs(item->Position.yPos - creature->enemy->Position.yPos) < 256)
			{
				item->TargetState = 5;
			}
			else if (AI.bite && AI.distance < pow(SECTOR(1), 2))
				item->TargetState = 9;
			else
				item->TargetState = 4;

			break;

		case 5:
			creature->reachedGoal = true;

			if (creature->enemy == NULL)
				break;
			else if ((creature->enemy->ObjectNumber == ID_SMALLMEDI_ITEM ||
				creature->enemy->ObjectNumber == ID_KEY_ITEM4) &&
				item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 12)
			{
				if (creature->enemy->RoomNumber == NO_ROOM ||
					creature->enemy->Status == ITEM_INVISIBLE ||
					creature->enemy->Flags & -32768)
					creature->enemy = NULL;
				else
				{
					item->CarriedItem = creature->enemy - g_Level.Items.data();
					RemoveDrawnItem(creature->enemy - g_Level.Items.data());
					creature->enemy->RoomNumber = NO_ROOM;
					creature->enemy->CarriedItem = NO_ITEM;

					for (int i = 0; i < ActiveCreatures.size(); i++)
					{
						auto* currentCreature = ActiveCreatures[i];
						if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNumber)
							continue;

						auto* target = &g_Level.Items[currentCreature->itemNum];
						if (currentCreature->enemy == creature->enemy)
							currentCreature->enemy = NULL;
					}

					creature->enemy = NULL;

					if (item->AIBits != MODIFY)
					{
						item->AIBits |= AMBUSH;
						item->AIBits |= MODIFY;
					}
				}
			}
			else if (creature->enemy->ObjectNumber == ID_AI_AMBUSH && item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 12)
			{
				item->AIBits = 0;

				auto* carriedItem = &g_Level.Items[item->CarriedItem];

				carriedItem->Position.xPos = item->Position.xPos;
				carriedItem->Position.yPos = item->Position.yPos;
				carriedItem->Position.zPos = item->Position.zPos;

				ItemNewRoom(item->CarriedItem, item->RoomNumber);
				item->CarriedItem = NO_ITEM;

				carriedItem->AIBits = GUARD;
				creature->enemy = NULL;
			}
			else
			{
				creature->maximumTurn = 0;

				if (abs(AI.angle) < ANGLE(7.0f))
					item->Position.yRot += AI.angle;
				else if (AI.angle < 0)
					item->Position.yRot -= ANGLE(7.0f);
				else
					item->Position.yRot += ANGLE(7.0f);
			}

			break;

		case 2:
			creature->maximumTurn = ANGLE(7.0f);
			torsoY = laraAI.angle;

			if (item->AIBits & PATROL1)
			{
				item->TargetState = 2;
				torsoY = 0;
			}
			else if (creature->mood == MoodType::Escape)
				item->TargetState = 4;
			else if (creature->mood == MoodType::Bored)
			{
				if (GetRandomControl() < 256)
					item->TargetState = 6;
			}
			else if (AI.bite && AI.distance < pow(682, 2))
				item->TargetState = 3;

			break;

		case 4:
			creature->maximumTurn = ANGLE(11.0f);
			tilt = angle / 2;

			if (AI.ahead)
				torsoY = AI.angle;

			if (item->AIBits & GUARD)
				item->TargetState = 3;
			else if (creature->mood == MoodType::Escape)
			{
				if (Lara.target != item && AI.ahead)
					item->TargetState = 3;
				break;
			}
			else if ((item->AIBits & FOLLOW) && (creature->reachedGoal || laraAI.distance > pow(SECTOR(2), 2)))
				item->TargetState = 3;
			else if (creature->mood == MoodType::Bored)
				item->TargetState = 9;
			else if (AI.distance < pow(682, 2))
				item->TargetState = 3;
			else if (AI.bite && AI.distance < pow(SECTOR(1), 2))
				item->TargetState = 9;

			break;

		case 12:
			creature->maximumTurn = 0;

			if (AI.ahead)
			{
				headY = AI.angle;
				headX = AI.xAngle;
			}

			if (abs(AI.angle) < ANGLE(7.0f))
				item->Position.yRot += AI.angle;
			else if (AI.angle < 0)
				item->Position.yRot -= ANGLE(7.0f);
			else
				item->Position.yRot += ANGLE(7.0f);

			if (enemy == LaraItem)
			{
				if (!creature->flags && (item->TouchBits & 0x2400))
				{
					CreatureEffect(item, &MonkeyBite, DoBloodSplat);
					creature->flags = 1;

					LaraItem->HitPoints -= 40;
					LaraItem->HitStatus = true;
				}
			}
			else
			{
				if (!creature->flags && enemy)
				{
					if (abs(enemy->Position.xPos - item->Position.xPos) < CLICK(1) &&
						abs(enemy->Position.yPos - item->Position.yPos) <= CLICK(1) &&
						abs(enemy->Position.zPos - item->Position.zPos) < CLICK(1))
					{
						CreatureEffect(item, &MonkeyBite, DoBloodSplat);
						creature->flags = 1;

						enemy->HitPoints -= 20;
						enemy->HitStatus = true;
					}
				}
			}

			break;

		case 13:
			creature->maximumTurn = 0;

			if (AI.ahead)
			{
				headY = AI.angle;
				headX = AI.xAngle;
			}

			if (abs(AI.angle) < ANGLE(7.0f))
				item->Position.yRot += AI.angle;
			else if (AI.angle < 0)
				item->Position.yRot -= ANGLE(7.0f);
			else
				item->Position.yRot += ANGLE(7.0f);

			if (enemy == LaraItem)
			{
				if (!creature->flags && item->TouchBits & 0x2400)
				{
					CreatureEffect(item, &MonkeyBite, DoBloodSplat);
					creature->flags = 1;

					LaraItem->HitPoints -= 40;
					LaraItem->HitStatus = true;
				}
			}
			else
			{
				if (!creature->flags && enemy)
				{
					if (abs(enemy->Position.xPos - item->Position.xPos) < CLICK(1) &&
						abs(enemy->Position.yPos - item->Position.yPos) <= CLICK(1) &&
						abs(enemy->Position.zPos - item->Position.zPos) < CLICK(1))
					{
						CreatureEffect(item, &MonkeyBite, DoBloodSplat);
						creature->flags = 1;

						enemy->HitPoints -= 20;
						enemy->HitStatus = true;
					}
				}
			}

			break;

		case 14:
			creature->maximumTurn = 0;

			if (AI.ahead)
			{
				headX = AI.xAngle;
				headY = AI.angle;
			}

			if (abs(AI.angle) < ANGLE(7.0f))
				item->Position.yRot += AI.angle;
			else if (AI.angle < 0)
				item->Position.yRot -= ANGLE(7.0f);
			else
				item->Position.yRot += ANGLE(7.0f);

			if (enemy == LaraItem)
			{
				if (creature->flags != 1 && item->TouchBits & 0x2400)
				{
					CreatureEffect(item, &MonkeyBite, DoBloodSplat);
					creature->flags = 1;

					LaraItem->HitPoints -= 50;
					LaraItem->HitStatus = true;
				}
			}
			else
			{
				if (creature->flags != 1 && enemy)
				{
					if (abs(enemy->Position.xPos - item->Position.xPos) < CLICK(1) &&
						abs(enemy->Position.yPos - item->Position.yPos) <= CLICK(1) &&
						abs(enemy->Position.zPos - item->Position.zPos) < CLICK(1))
					{
						CreatureEffect(item, &MonkeyBite, DoBloodSplat);
						creature->flags = 1;

						enemy->HitPoints -= 25;
						enemy->HitStatus = true;
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

	if (item->ActiveState < 15)
	{
		switch (CreatureVault(itemNumber, angle, 2, 128))
		{
		case 2:
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[ID_MONKEY].animIndex + 19;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 17;
			break;

		case 3:
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[ID_MONKEY].animIndex + 18;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 16;
			break;

		case 4:
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[ID_MONKEY].animIndex + 17;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 15;
			break;

		case -2:
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[ID_MONKEY].animIndex + 22;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 20;
			break;

		case -3:
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[ID_MONKEY].animIndex + 21;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 19;
			break;

		case -4:
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[ID_MONKEY].animIndex + 20;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 18;
			break;
		}
	}
	else
	{
		creature->maximumTurn = 0;
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
