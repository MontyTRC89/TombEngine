#include "newobjects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\effects.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"
#include "..\Game\effect2.h"
#include "..\Game\people.h"

BITE_INFO tribesmanAxeBite = { 0, 16, 265, 13 };
BITE_INFO tribesmanDartsBite1 = { 0, 0, -200, 13 };
BITE_INFO tribesmanDartsBite2 = { 8, 40, -248, 13 };

byte tribesmanAxeHit[13][3] = {
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

void __cdecl TribemanAxeControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*) item->data;

	__int16 head = 0;
	__int16 angle = 0;
	__int16 tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 9)
		{
			if (item->currentAnimState == 1 ||
				item->currentAnimState == 7)
				item->animNumber = Objects[item->objectNumber].animIndex + 21;
			else
				item->animNumber = Objects[item->objectNumber].animIndex + 20;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 9;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		if (creature->enemy == LaraItem && creature->hurtByLara && info.distance > SQUARE(3072) && info.enemyFacing < ANGLE(67) && info.enemyFacing > -ANGLE(67))
			creature->mood = MOOD_TYPE::ESCAPE_MOOD;
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);
		if (info.ahead)
			head = info.angle;

		switch (item->currentAnimState)
		{
		case 1:
			creature->maximumTurn = ANGLE(4);
			creature->flags = 0;

			if (creature->mood == MOOD_TYPE::BORED_MOOD)
			{
				creature->maximumTurn = 0;
				if (GetRandomControl() < 0x100)
					item->goalAnimState = 2;
			}
			else if (creature->mood == MOOD_TYPE::ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead && !item->hitStatus)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 3;
			}
			else if (item->itemFlags[0])
			{
				item->itemFlags[0] = 0;
				item->goalAnimState = 11;
			}
			else if (info.ahead && info.distance < SQUARE(682))
			{
				item->goalAnimState = 7;
			}
			else if (info.ahead && info.distance < SQUARE(1024))
			{
				if (GetRandomControl() < 0x4000)
					item->goalAnimState = 2;
				else
				{
					item->goalAnimState = 7;
				}
			}
			else if (info.ahead && info.distance < SQUARE(2048))
				item->goalAnimState = 2;
			else
				item->goalAnimState = 3;
			break;

		case 11:
			creature->maximumTurn = ANGLE(4);
			creature->flags = 0;

			if (creature->mood == MOOD_TYPE::BORED_MOOD)
			{
				creature->maximumTurn = 0;
				if (GetRandomControl() < 0x100)
					item->goalAnimState = 2;
			}
			else if (creature->mood == MOOD_TYPE::ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead && !item->hitStatus)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(682))
			{
				if (GetRandomControl() < 0x800)
					item->goalAnimState = 5;
				else
					item->goalAnimState = 8;
			}
			else if (info.distance < SQUARE(2048))
				item->goalAnimState = 2;
			else
				item->goalAnimState = 3;
			break;

		case 2:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(9);
			tilt = angle >> 3;

			if (creature->mood == MOOD_TYPE::BORED_MOOD)
			{
				creature->maximumTurn >>= 2;
				if (GetRandomControl() < 0x100)
				{
					if (GetRandomControl() < 0x2000)
						item->goalAnimState = 1;
					else
						item->goalAnimState = 11;
				}
			}
			else if (creature->mood == MOOD_TYPE::ESCAPE_MOOD)
				item->goalAnimState = 3;
			else if (info.ahead && info.distance < SQUARE(682))
			{
				if (GetRandomControl() < 0x2000)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 11;
			}
			else if (info.distance > SQUARE(2048))
				item->goalAnimState = 3;
			break;

		case 3:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(6);
			tilt = angle >> 2;

			if (creature->mood == MOOD_TYPE::BORED_MOOD)
			{
				creature->maximumTurn >>= 2;
				if (GetRandomControl() < 0x100)
				{
					if (GetRandomControl() < 0x4000)
						item->goalAnimState = 1;
					else
						item->goalAnimState = 11;
				}
			}
			else if (creature->mood == MOOD_TYPE::ESCAPE_MOOD && Lara.target != item && info.ahead)
				item->goalAnimState = 11;
			else if (info.bite || info.distance < SQUARE(2048))
			{
				if (GetRandomControl() < 0x4000)
					item->goalAnimState = 12;
				else if (GetRandomControl() < 0x2000)
					item->goalAnimState = 10;
				else
					item->goalAnimState = 2;
			}

			break;

		case 8:
			creature->maximumTurn = ANGLE(4);
			if (info.bite || info.distance < SQUARE(682))
				item->goalAnimState = 6;
			else
				item->goalAnimState = 11;
			break;

		case 5:
		case 6:
		case 7:
		case 10:
		case 12:
			item->itemFlags[0] = 1;
			creature->maximumTurn = ANGLE(4);
			creature->flags = item->frameNumber - Anims[item->animNumber].frameBase;

			if (creature->enemy == LaraItem)
			{
				if ((item->touchBits & 0x2000) &&
					creature->flags >= tribesmanAxeHit[item->currentAnimState][0] &&
					creature->flags <= tribesmanAxeHit[item->currentAnimState][1])
				{
					LaraItem->hitPoints -= tribesmanAxeHit[item->currentAnimState][2];
					LaraItem->hitStatus = true;

					for (__int32 i = 0; i < tribesmanAxeHit[item->currentAnimState][2]; i += 8)
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
						creature->flags >= tribesmanAxeHit[item->currentAnimState][0] &&
						creature->flags <= tribesmanAxeHit[item->currentAnimState][1])
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

void __cdecl TribesmanShotDart(ITEM_INFO* item)
{
	__int16 dartItemNumber = CreateItem();
	if (dartItemNumber != NO_ITEM)
	{
		ITEM_INFO* dartItem = &Items[dartItemNumber];
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
		pos2.z = tribesmanDartsBite2.z << 1;
		GetJointAbsPosition(item, &pos2, tribesmanDartsBite2.meshNum);

		__int16 angles[2];
		phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

		dartItem->pos.xPos = pos1.x;
		dartItem->pos.yPos = pos1.y;
		dartItem->pos.zPos = pos1.z;

		InitialiseItem(dartItemNumber);

		dartItem->pos.xRot = angles[1];
		dartItem->pos.yRot = angles[0];
		dartItem->speed = 256;

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

void __cdecl TribesmanDartsControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;
	
	__int16 headX = 0;
	__int16 headY = 0;
	__int16 torsoX = 0;
	__int16 torsoY = 0;
	__int16 angle = 0;
	__int16 tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 9)
		{
			if (item->currentAnimState == 1 || item->currentAnimState == 4)
				item->animNumber = Objects[item->objectNumber].animIndex + 21;
			else
				item->animNumber = Objects[item->objectNumber].animIndex + 20;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 9;
		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);

		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, (info.zoneNumber == info.enemyZone ? VIOLENT : TIMID));

		if (item->hitStatus && Lara.poisoned >= 0x100 && creature->mood == MOOD_TYPE::BORED_MOOD)
			creature->mood = MOOD_TYPE::ESCAPE_MOOD;

		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, creature->mood == MOOD_TYPE::BORED_MOOD ? ANGLE(2) : creature->maximumTurn);
		if (info.ahead)
		{
			headY = info.angle >> 1;
			torsoY = info.angle >> 1;
		}

		if (item->hitStatus || 
			(creature->enemy == LaraItem && (info.distance < 1024 || 
				TargetVisible(item, &info)) && (abs(LaraItem->pos.yPos - item->pos.yPos) < 2048))) 
			AlertAllGuards(itemNum);

		switch (item->currentAnimState)
		{
		case 1:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle >> 1;
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
					item->goalAnimState = 11;
				break;
			}
			else if (creature->mood == MOOD_TYPE::ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead && !item->hitStatus)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 3;
			}
			else if (info.bite && info.distance < SQUARE(512))
				item->goalAnimState = 11;
			else if (info.bite && info.distance < SQUARE(2048))
				item->goalAnimState = 2;
			else if (Targetable(item, &info) && info.distance < SQUARE(8192))
				item->goalAnimState = 4;
			else if (creature->mood == MOOD_TYPE::BORED_MOOD)
			{
				if (GetRandomControl() < 0x200)
					item->goalAnimState = 2;
				else
					break;
			}
			else
				item->goalAnimState = 3;
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
					item->goalAnimState = 1;
				break;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead && !item->hitStatus)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 3;
			}
			else if (info.bite && info.distance < SQUARE(512))
				item->goalAnimState = 6;
			else if (info.bite && info.distance < SQUARE(2048))
				item->goalAnimState = 2;
			else if (Targetable(item, &info) && info.distance < SQUARE(8192))
				item->goalAnimState = 1;
			else if (creature->mood == MOOD_TYPE::BORED_MOOD && GetRandomControl() < 0x200)
				item->goalAnimState = 2;
			else
				item->goalAnimState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(9);

			if (info.bite && info.distance < SQUARE(512))
				item->goalAnimState = 11;
			else if (info.bite && info.distance < SQUARE(2048))
				item->goalAnimState = 2;
			else if (Targetable(item, &info) && info.distance < SQUARE(8192))
				item->goalAnimState = 1;
			else if (creature->mood == MOOD_TYPE::ESCAPE_MOOD)
				item->goalAnimState = 3;
			else if (creature->mood == MOOD_TYPE::BORED_MOOD)
			{
				if (GetRandomControl() > 0x200)
					item->goalAnimState = 2;
				else if (GetRandomControl() > 0x200)
					item->goalAnimState = 11;
				else
					item->goalAnimState = 1;
			}
			else if (info.distance > SQUARE(2048))
				item->goalAnimState = 3;
			break;

		case 3:
			creature->flags &= 0x0FFF;
			creature->maximumTurn = ANGLE(6);
			tilt = angle >> 2;

			if (info.bite && info.distance < SQUARE(512))
				item->goalAnimState = 11;
			else if (Targetable(item, &info) && info.distance < SQUARE(8192))
				item->goalAnimState = 1;
			if (item->aiBits & GUARD)
				item->goalAnimState = 11;
			else if (creature->mood == MOOD_TYPE::ESCAPE_MOOD && Lara.target != item && info.ahead)
				item->goalAnimState = 11;
			else if (creature->mood == MOOD_TYPE::BORED_MOOD)
				item->goalAnimState = 1;
			break;

		case 8:
			if (!info.bite || info.distance > SQUARE(512))
				item->goalAnimState = 11;
			else
				item->goalAnimState = 6;
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


			if (item->frameNumber == Anims[item->animNumber].frameBase + 15)
			{
				TribesmanShotDart(item);
				item->goalAnimState = 1;
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


