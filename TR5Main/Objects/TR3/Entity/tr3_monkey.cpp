#include "framework.h"
#include "Objects/TR3/Entity/tr3_monkey.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO monkeyBite = { 10, 10, 11, 13 };

void InitialiseMonkey(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

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

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;

	short headX = 0;
	short headY = 0;
	short torsoY = 0;
	short angle = 0;
	short tilt = 0;
	int x = 0;
	int z = 0;
	int distance = 0;
	int dx = 0;
	int dz = 0;

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
				CREATURE_INFO* currentCreature = ActiveCreatures[i];
				if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNumber)
					continue;

				ITEM_INFO* target = &g_Level.Items[currentCreature->itemNum];
				if (target->ObjectNumber == ID_LARA || target->ObjectNumber == ID_MONKEY)
					continue;

				if (target->ObjectNumber == ID_SMALLMEDI_ITEM)
				{
					x = target->Position.xPos - item->Position.xPos;
					z = target->Position.zPos - item->Position.zPos;
					distance = SQUARE(x) + SQUARE(z);

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

		AI_INFO info;
		AI_INFO laraInfo;

		CreatureAIInfo(item, &info);


		if (!creature->hurtByLara && creature->enemy == LaraItem)
			creature->enemy = NULL;

		if (creature->enemy == LaraItem)
		{
			laraInfo.angle = info.angle;
			laraInfo.distance = info.distance;
		}
		else
		{
			dx = LaraItem->Position.xPos - item->Position.xPos;
			dz = LaraItem->Position.zPos - item->Position.zPos;

			laraInfo.angle = phd_atan(dz, dz) - item->Position.yRot;
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item, &info, VIOLENT);
		if (Lara.Vehicle != NO_ITEM)
			creature->mood = ESCAPE_MOOD;
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		ITEM_INFO* enemy = creature->enemy;
		creature->enemy = LaraItem;

		if (item->HitStatus)
			AlertAllGuards(itemNumber);

		creature->enemy = enemy;

		switch (item->ActiveState)
		{
		case 6:
			creature->flags = 0;
			creature->maximumTurn = 0;

			torsoY = laraInfo.angle;

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

			else if (creature->mood == ESCAPE_MOOD)
				item->TargetState = 3;
			else if (creature->mood == BORED_MOOD)
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
			else if ((item->AIBits & FOLLOW) && (creature->reachedGoal || laraInfo.distance > SQUARE(2048)))
			{
				if (item->RequiredState)
					item->TargetState = item->RequiredState;
				else if (info.ahead)
					item->TargetState = 6;
				else
					item->TargetState = 3;
			}
			else if (info.bite && info.distance < SQUARE(682))
				item->TargetState = 3;
			else if (info.bite && info.distance < SQUARE(682))
				item->TargetState = 2;
			else
				item->TargetState = 3;

			break;

		case 3:
			creature->flags = 0;
			creature->maximumTurn = 0;

			torsoY = laraInfo.angle;

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
			else if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->TargetState = 3;
				else
					item->TargetState = 4;
			}
			else if (creature->mood == BORED_MOOD)
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
			else if ((item->AIBits & FOLLOW) && (creature->reachedGoal || laraInfo.distance > SQUARE(2048)))
			{
				if (item->RequiredState)
					item->TargetState = item->RequiredState;
				else if (info.ahead)
					item->TargetState = 6;
				else
					item->TargetState = 4;
			}
			else if (info.bite && info.distance < SQUARE(341))
			{
				if (LaraItem->Position.yPos < item->Position.yPos)
					item->TargetState = 13;
				else
					item->TargetState = 12;
			}
			else if (info.bite && info.distance < SQUARE(682))
				item->TargetState = 14;
			else if (info.bite && info.distance < SQUARE(682))
				item->TargetState = 2;
			else if (info.distance < SQUARE(682) && creature->enemy != LaraItem && creature->enemy != NULL
				&& creature->enemy->ObjectNumber != ID_AI_PATROL1 && creature->enemy->ObjectNumber != ID_AI_PATROL2
				&& abs(item->Position.yPos - creature->enemy->Position.yPos) < 256)
				item->TargetState = 5;
			else if (info.bite && info.distance < SQUARE(1024))
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
						CREATURE_INFO* currentCreature = ActiveCreatures[i];
						if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNumber)
							continue;

						ITEM_INFO* target = &g_Level.Items[currentCreature->itemNum];
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

				ITEM_INFO* carriedItem = &g_Level.Items[item->CarriedItem];

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
				if (abs(info.angle) < ANGLE(7))
					item->Position.yRot += info.angle;
				else if (info.angle < 0)
					item->Position.yRot -= ANGLE(7);
				else
					item->Position.yRot += ANGLE(7);
			}

			break;

		case 2:
			torsoY = laraInfo.angle;
			creature->maximumTurn = ANGLE(7);

			if (item->AIBits & PATROL1)
			{
				item->TargetState = 2;
				torsoY = 0;
			}
			else if (creature->mood == ESCAPE_MOOD)
				item->TargetState = 4;
			else if (creature->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 256)
				{
					item->TargetState = 6;
				}
			}
			else if (info.bite && info.distance < SQUARE(682))
				item->TargetState = 3;

			break;

		case 4:
			if (info.ahead)
				torsoY = info.angle;

			creature->maximumTurn = ANGLE(11);
			tilt = angle / 2;

			if (item->AIBits & GUARD)
				item->TargetState = 3;
			else if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->TargetState = 3;
				break;
			}
			else if ((item->AIBits & FOLLOW) && (creature->reachedGoal || laraInfo.distance > SQUARE(2048)))
				item->TargetState = 3;
			else if (creature->mood == BORED_MOOD)
				item->TargetState = 9;
			else if (info.distance < SQUARE(682))
				item->TargetState = 3;
			else if (info.bite && info.distance < SQUARE(1024))
				item->TargetState = 9;

			break;

		case 12:
			if (info.ahead)
			{
				headY = info.angle;
				headX = info.xAngle;
			}

			creature->maximumTurn = 0;
			if (abs(info.angle) < ANGLE(7))
				item->Position.yRot += info.angle;
			else if (info.angle < 0)
				item->Position.yRot -= ANGLE(7);
			else
				item->Position.yRot += ANGLE(7);

			if (enemy == LaraItem)
			{
				if (!creature->flags && (item->TouchBits & 0x2400))
				{
					LaraItem->HitPoints -= 40;
					LaraItem->HitStatus = true;
					CreatureEffect(item, &monkeyBite, DoBloodSplat);

					creature->flags = 1;
				}
			}
			else
			{
				if (!creature->flags && enemy)
				{
					if (abs(enemy->Position.xPos - item->Position.xPos) < 256 &&
						abs(enemy->Position.yPos - item->Position.yPos) <= 256 &&
						abs(enemy->Position.zPos - item->Position.zPos) < 256)
					{
						enemy->HitPoints -= 20;
						enemy->HitStatus = true;
						creature->flags = 1;
						CreatureEffect(item, &monkeyBite, DoBloodSplat);
					}
				}
			}

			break;

		case 13:
			if (info.ahead)
			{
				headY = info.angle;
				headX = info.xAngle;
			}

			creature->maximumTurn = 0;
			if (abs(info.angle) < ANGLE(7))
				item->Position.yRot += info.angle;
			else if (info.angle < 0)
				item->Position.yRot -= ANGLE(7);
			else
				item->Position.yRot += ANGLE(7);

			if (enemy == LaraItem)
			{
				if (!creature->flags && (item->TouchBits & 0x2400))
				{
					LaraItem->HitPoints -= 40;
					LaraItem->HitStatus = true;
					CreatureEffect(item, &monkeyBite, DoBloodSplat);

					creature->flags = 1;
				}
			}
			else
			{
				if (!creature->flags && enemy)
				{
					if (abs(enemy->Position.xPos - item->Position.xPos) < 256 &&
						abs(enemy->Position.yPos - item->Position.yPos) <= 256 &&
						abs(enemy->Position.zPos - item->Position.zPos) < 256)
					{
						enemy->HitPoints -= 20;
						enemy->HitStatus = true;
						creature->flags = 1;
						CreatureEffect(item, &monkeyBite, DoBloodSplat);
					}
				}
			}

			break;

		case 14:
			if (info.ahead)
			{
				headY = info.angle;
				headX = info.xAngle;
			}

			creature->maximumTurn = 0;
			if (abs(info.angle) < ANGLE(7))
				item->Position.yRot += info.angle;
			else if (info.angle < 0)
				item->Position.yRot -= ANGLE(7);
			else
				item->Position.yRot += ANGLE(7);

			if (enemy == LaraItem)
			{
				if (creature->flags != 1 && (item->TouchBits & 0x2400))
				{
					LaraItem->HitPoints -= 50;
					LaraItem->HitStatus = true;
					CreatureEffect(item, &monkeyBite, DoBloodSplat);

					creature->flags = 1;
				}
			}
			else
			{
				if (creature->flags != 1 && enemy)
				{
					if (abs(enemy->Position.xPos - item->Position.xPos) < 256 &&
						abs(enemy->Position.yPos - item->Position.yPos) <= 256 &&
						abs(enemy->Position.zPos - item->Position.zPos) < 256)
					{
						enemy->HitPoints -= 25;
						enemy->HitStatus = true;
						creature->flags = 1;
						CreatureEffect(item, &monkeyBite, DoBloodSplat);
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
