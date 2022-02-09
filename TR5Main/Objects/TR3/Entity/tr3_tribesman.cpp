#include "framework.h"
#include "Objects/TR3/Entity/tr3_tribesman.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Objects/Generic/Traps/dart_emitter.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Entities::Traps;

BITE_INFO tribesmanAxeBite = { 0, 16, 265, 13 };
BITE_INFO tribesmanDartsBite1 = { 0, 0, -200, 13 };
BITE_INFO tribesmanDartsBite2 = { 8, 40, -248, 13 };

unsigned char tribesmanAxeHit[13][3] = {
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{2,12,8},
	{8,9,32},
	{19,28,8},	
	{0,0,0}, 
	{0,0,0}, 
	{7,14,8}, 
	{0,0,0}, 
	{15,19,32}
};

void TribemanAxeControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*) item->data;

	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->activeState != 9)
		{
			if (item->activeState == 1 ||
				item->activeState == 7)
				item->animNumber = Objects[item->objectNumber].animIndex + 21;
			else
				item->animNumber = Objects[item->objectNumber].animIndex + 20;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = 9;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		if (creature->enemy == LaraItem && creature->hurtByLara && info.distance > SQUARE(3072) && info.enemyFacing < ANGLE(67) && info.enemyFacing > -ANGLE(67))
			creature->mood = ESCAPE_MOOD;
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);
		if (info.ahead)
			head = info.angle;

		switch (item->activeState)
		{
		case 1:
			creature->maximumTurn = ANGLE(4);
			creature->flags = 0;

			if (creature->mood == BORED_MOOD)
			{
				creature->maximumTurn = 0;
				if (GetRandomControl() < 0x100)
					item->targetState = 2;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead && !item->hitStatus)
					item->targetState = 1;
				else
					item->targetState = 3;
			}
			else if (item->itemFlags[0])
			{
				item->itemFlags[0] = 0;
				item->targetState = 11;
			}
			else if (info.ahead && info.distance < SQUARE(682))
			{
				item->targetState = 7;
			}
			else if (info.ahead && info.distance < SQUARE(1024))
			{
				if (GetRandomControl() < 0x4000)
					item->targetState = 2;
				else
				{
					item->targetState = 7;
				}
			}
			else if (info.ahead && info.distance < SQUARE(2048))
				item->targetState = 2;
			else
				item->targetState = 3;
			break;

		case 11:
			creature->maximumTurn = ANGLE(4);
			creature->flags = 0;

			if (creature->mood == BORED_MOOD)
			{
				creature->maximumTurn = 0;
				if (GetRandomControl() < 0x100)
					item->targetState = 2;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead && !item->hitStatus)
					item->targetState = 1;
				else
					item->targetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(682))
			{
				if (GetRandomControl() < 0x800)
					item->targetState = 5;
				else
					item->targetState = 8;
			}
			else if (info.distance < SQUARE(2048))
				item->targetState = 2;
			else
				item->targetState = 3;
			break;

		case 2:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(9);
			tilt = angle / 8;

			if (creature->mood == BORED_MOOD)
			{
				creature->maximumTurn /= 4;
				if (GetRandomControl() < 0x100)
				{
					if (GetRandomControl() < 0x2000)
						item->targetState = 1;
					else
						item->targetState = 11;
				}
			}
			else if (creature->mood == ESCAPE_MOOD)
				item->targetState = 3;
			else if (info.ahead && info.distance < SQUARE(682))
			{
				if (GetRandomControl() < 0x2000)
					item->targetState = 1;
				else
					item->targetState = 11;
			}
			else if (info.distance > SQUARE(2048))
				item->targetState = 3;
			break;

		case 3:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(6);
			tilt = angle / 4;

			if (creature->mood == BORED_MOOD)
			{
				creature->maximumTurn /= 4;
				if (GetRandomControl() < 0x100)
				{
					if (GetRandomControl() < 0x4000)
						item->targetState = 1;
					else
						item->targetState = 11;
				}
			}
			else if (creature->mood == ESCAPE_MOOD && Lara.target != item && info.ahead)
				item->targetState = 11;
			else if (info.bite || info.distance < SQUARE(2048))
			{
				if (GetRandomControl() < 0x4000)
					item->targetState = 12;
				else if (GetRandomControl() < 0x2000)
					item->targetState = 10;
				else
					item->targetState = 2;
			}

			break;

		case 8:
			creature->maximumTurn = ANGLE(4);
			if (info.bite || info.distance < SQUARE(682))
				item->targetState = 6;
			else
				item->targetState = 11;
			break;

		case 5:
		case 6:
		case 7:
		case 10:
		case 12:
			item->itemFlags[0] = 1;
			creature->maximumTurn = ANGLE(4);
			creature->flags = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;

			if (creature->enemy == LaraItem)
			{
				if ((item->touchBits & 0x2000) &&
					creature->flags >= tribesmanAxeHit[item->activeState][0] &&
					creature->flags <= tribesmanAxeHit[item->activeState][1])
				{
					LaraItem->hitPoints -= tribesmanAxeHit[item->activeState][2];
					LaraItem->hitStatus = true;

					for (int i = 0; i < tribesmanAxeHit[item->activeState][2]; i += 8)
						CreatureEffect(item, &tribesmanAxeBite, DoBloodSplat);

					SoundEffect(70, &item->pos, 0);
				}
			}
			else
			{
				if (creature->enemy)
				{
					if (abs(creature->enemy->pos.xPos - item->pos.xPos) < 512 &&
						abs(creature->enemy->pos.yPos - item->pos.yPos) < 512 &&
						abs(creature->enemy->pos.zPos - item->pos.zPos) < 512 &&
						creature->flags >= tribesmanAxeHit[item->activeState][0] &&
						creature->flags <= tribesmanAxeHit[item->activeState][1])
					{
						creature->enemy->hitPoints -= 2;
						creature->enemy->hitStatus = true;

						CreatureEffect(item, &tribesmanAxeBite, DoBloodSplat);

						SoundEffect(70, &item->pos, 0);
					}
				}
			}
			break;

		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head >> 1);
	CreatureJoint(item, 1, head >> 1);

	CreatureAnimation(itemNum, angle, 0);
}

static void TribesmanShotDart(ITEM_INFO* item)
{
	short dartItemNumber = CreateItem();
	if (dartItemNumber != NO_ITEM)
	{
		ITEM_INFO* dartItem = &g_Level.Items[dartItemNumber];
		dartItem->objectNumber = ID_DARTS;
		dartItem->roomNumber = item->roomNumber;

		PHD_VECTOR pos1;
		pos1.x = tribesmanDartsBite2.x;
		pos1.y = tribesmanDartsBite2.y;
		pos1.z = tribesmanDartsBite2.z;
		GetJointAbsPosition(item, &pos1, tribesmanDartsBite2.meshNum);

		PHD_VECTOR pos2;
		pos2.x = tribesmanDartsBite2.x;
		pos2.y = tribesmanDartsBite2.y;
		pos2.z = tribesmanDartsBite2.z * 2;
		GetJointAbsPosition(item, &pos2, tribesmanDartsBite2.meshNum);

		short angles[2];
		phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

		dartItem->pos.xPos = pos1.x;
		dartItem->pos.yPos = pos1.y;
		dartItem->pos.zPos = pos1.z;

		InitialiseItem(dartItemNumber);

		dartItem->pos.xRot = angles[1];
		dartItem->pos.yRot = angles[0];
		dartItem->Velocity = 256;

		AddActiveItem(dartItemNumber);

		dartItem->status = ITEM_ACTIVE;

		pos1.x = tribesmanDartsBite2.x;
		pos1.y = tribesmanDartsBite2.y;
		pos1.z = tribesmanDartsBite2.z + 96;

		GetJointAbsPosition(item, &pos1, tribesmanDartsBite2.meshNum);
		
		TriggerDartSmoke(pos1.x, pos1.y, pos1.z, 0, 0, 1);
		TriggerDartSmoke(pos1.x, pos1.y, pos1.z, 0, 0, 1);
	}
}

void TribemanDartsControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;
	
	short headX = 0;
	short headY = 0;
	short torsoX = 0;
	short torsoY = 0;
	short angle = 0;
	short tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->activeState != 9)
		{
			if (item->activeState == 1 || item->activeState == 4)
				item->animNumber = Objects[item->objectNumber].animIndex + 21;
			else
				item->animNumber = Objects[item->objectNumber].animIndex + 20;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = 9;
		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);

		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, (info.zoneNumber == info.enemyZone ? VIOLENT : TIMID));

		if (item->hitStatus && Lara.poisoned >= 0x100 && creature->mood == BORED_MOOD)
			creature->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, creature->mood == BORED_MOOD ? ANGLE(2) : creature->maximumTurn);
		if (info.ahead)
		{
			headY = info.angle / 2;
			torsoY = info.angle / 2;
		}

		if (item->hitStatus || 
			(creature->enemy == LaraItem && (info.distance < 1024 || 
				TargetVisible(item, &info)) && (abs(LaraItem->pos.yPos - item->pos.yPos) < 2048))) 
			AlertAllGuards(itemNum);

		switch (item->activeState)
		{
		case 1:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle / 2;
			}
			creature->flags &= 0x0FFF;
			creature->maximumTurn = ANGLE(2);
			if (item->aiBits & GUARD)
			{
				headY = AIGuard(creature);
				torsoY = 0;
				torsoX = 0;
				creature->maximumTurn = 0;
				if (!(GetRandomControl() & 0xFF))
					item->targetState = 11;
				break;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead && !item->hitStatus)
					item->targetState = 1;
				else
					item->targetState = 3;
			}
			else if (info.bite && info.distance < SQUARE(WALL_SIZE / 2))
				item->targetState = 11;
			else if (info.bite && info.distance < SQUARE(WALL_SIZE * 2))
				item->targetState = 2;
			else if (Targetable(item, &info) && info.distance < SQUARE(MAX_VISIBILITY_DISTANCE))
				item->targetState = 4;
			else if (creature->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x200)
					item->targetState = 2;
				else
					break;
			}
			else
				item->targetState = 3;
			break;

		case 11:
			creature->flags &= 0x0FFF;
			creature->maximumTurn = ANGLE(2);
			if (item->aiBits & GUARD)
			{
				headY = AIGuard(creature);
				torsoY = 0;
				torsoX = 0;
				creature->maximumTurn = 0;
				if (!(GetRandomControl() & 0xFF))
					item->targetState = 1;
				break;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead && !item->hitStatus)
					item->targetState = 1;
				else
					item->targetState = 3;
			}
			else if (info.bite && info.distance < SQUARE(WALL_SIZE / 2))
				item->targetState = 6;
			else if (info.bite && info.distance < SQUARE(WALL_SIZE * 2))
				item->targetState = 2;
			else if (Targetable(item, &info) && info.distance < SQUARE(MAX_VISIBILITY_DISTANCE))
				item->targetState = 1;
			else if (creature->mood == BORED_MOOD && GetRandomControl() < 0x200)
				item->targetState = 2;
			else
				item->targetState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(9);

			if (info.bite && info.distance < SQUARE(WALL_SIZE / 2))
				item->targetState = 11;
			else if (info.bite && info.distance < SQUARE(WALL_SIZE * 2))
				item->targetState = 2;
			else if (Targetable(item, &info) && info.distance < SQUARE(MAX_VISIBILITY_DISTANCE))
				item->targetState = 1;
			else if (creature->mood == ESCAPE_MOOD)
				item->targetState = 3;
			else if (creature->mood == BORED_MOOD)
			{
				if (GetRandomControl() > 0x200)
					item->targetState = 2;
				else if (GetRandomControl() > 0x200)
					item->targetState = 11;
				else
					item->targetState = 1;
			}
			else if (info.distance > SQUARE(2048))
				item->targetState = 3;
			break;

		case 3:
			creature->flags &= 0x0FFF;
			creature->maximumTurn = ANGLE(6);
			tilt = angle / 4;

			if (info.bite && info.distance < SQUARE(WALL_SIZE / 2))
				item->targetState = 11;
			else if (Targetable(item, &info) && info.distance < SQUARE(MAX_VISIBILITY_DISTANCE))
				item->targetState = 1;
			if (item->aiBits & GUARD)
				item->targetState = 11;
			else if (creature->mood == ESCAPE_MOOD && Lara.target != item && info.ahead)
				item->targetState = 11;
			else if (creature->mood == BORED_MOOD)
				item->targetState = 1;
			break;

		case 8:
			if (!info.bite || info.distance > SQUARE(512))
				item->targetState = 11;
			else
				item->targetState = 6;
			break;

		case 4:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->maximumTurn = 0;
			if (abs(info.angle) < ANGLE(2))
				item->pos.yRot += info.angle;
			else if (info.angle < 0)
				item->pos.yRot -= ANGLE(2);
			else
				item->pos.yRot += ANGLE(2);


			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 15)
			{
				TribesmanShotDart(item);
				item->targetState = 1;
			}
			break;

		case 6:
			if (creature->enemy == LaraItem)
			{
				if (!(creature->flags & 0xf000) && (item->touchBits & 0x2400))
				{
					LaraItem->hitPoints -= 100;
					LaraItem->hitStatus = true;

					creature->flags |= 0x1000;

					SoundEffect(70, &item->pos, 0);

					CreatureEffect(item, &tribesmanDartsBite1, DoBloodSplat);
				}
			}
			else
			{
				if (!(creature->flags & 0xf000) && creature->enemy)
				{
					if (abs(creature->enemy->pos.xPos - item->pos.xPos) < SQUARE(512) &&
						abs(creature->enemy->pos.yPos - item->pos.yPos) < SQUARE(512) &&
						abs(creature->enemy->pos.zPos - item->pos.zPos) < SQUARE(512))
					{
						creature->enemy->hitPoints -= 5;
						creature->enemy->hitStatus = true;
						creature->flags |= 0x1000;

						SoundEffect(70, &item->pos, 0);
					}
				}
			}
			break;
		}
	}

	CreatureTilt(item, tilt);

	headY -= torsoY;

	CreatureJoint(item, 0, torsoY);
	CreatureJoint(item, 1, torsoX);
	CreatureJoint(item, 2, headY);
	CreatureJoint(item, 3, headX);

	CreatureAnimation(itemNum, angle, 0);
}


