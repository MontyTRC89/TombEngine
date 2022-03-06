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
	CREATURE_INFO* creature = (CREATURE_INFO*) item->Data;

	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 9)
		{
			if (item->ActiveState == 1 ||
				item->ActiveState == 7)
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 21;
			else
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 20;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 9;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		if (creature->enemy == LaraItem && creature->hurtByLara && info.distance > SQUARE(3072) && info.enemyFacing < ANGLE(67) && info.enemyFacing > -ANGLE(67))
			creature->mood = MoodType::Escape;
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);
		if (info.ahead)
			head = info.angle;

		switch (item->ActiveState)
		{
		case 1:
			creature->maximumTurn = ANGLE(4);
			creature->flags = 0;

			if (creature->mood == MoodType::Bored)
			{
				creature->maximumTurn = 0;
				if (GetRandomControl() < 0x100)
					item->TargetState = 2;
			}
			else if (creature->mood == MoodType::Escape)
			{
				if (Lara.target != item && info.ahead && !item->HitStatus)
					item->TargetState = 1;
				else
					item->TargetState = 3;
			}
			else if (item->ItemFlags[0])
			{
				item->ItemFlags[0] = 0;
				item->TargetState = 11;
			}
			else if (info.ahead && info.distance < SQUARE(682))
			{
				item->TargetState = 7;
			}
			else if (info.ahead && info.distance < SQUARE(1024))
			{
				if (GetRandomControl() < 0x4000)
					item->TargetState = 2;
				else
				{
					item->TargetState = 7;
				}
			}
			else if (info.ahead && info.distance < SQUARE(2048))
				item->TargetState = 2;
			else
				item->TargetState = 3;
			break;

		case 11:
			creature->maximumTurn = ANGLE(4);
			creature->flags = 0;

			if (creature->mood == MoodType::Bored)
			{
				creature->maximumTurn = 0;
				if (GetRandomControl() < 0x100)
					item->TargetState = 2;
			}
			else if (creature->mood == MoodType::Escape)
			{
				if (Lara.target != item && info.ahead && !item->HitStatus)
					item->TargetState = 1;
				else
					item->TargetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(682))
			{
				if (GetRandomControl() < 0x800)
					item->TargetState = 5;
				else
					item->TargetState = 8;
			}
			else if (info.distance < SQUARE(2048))
				item->TargetState = 2;
			else
				item->TargetState = 3;
			break;

		case 2:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(9);
			tilt = angle / 8;

			if (creature->mood == MoodType::Bored)
			{
				creature->maximumTurn /= 4;
				if (GetRandomControl() < 0x100)
				{
					if (GetRandomControl() < 0x2000)
						item->TargetState = 1;
					else
						item->TargetState = 11;
				}
			}
			else if (creature->mood == MoodType::Escape)
				item->TargetState = 3;
			else if (info.ahead && info.distance < SQUARE(682))
			{
				if (GetRandomControl() < 0x2000)
					item->TargetState = 1;
				else
					item->TargetState = 11;
			}
			else if (info.distance > SQUARE(2048))
				item->TargetState = 3;
			break;

		case 3:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(6);
			tilt = angle / 4;

			if (creature->mood == MoodType::Bored)
			{
				creature->maximumTurn /= 4;
				if (GetRandomControl() < 0x100)
				{
					if (GetRandomControl() < 0x4000)
						item->TargetState = 1;
					else
						item->TargetState = 11;
				}
			}
			else if (creature->mood == MoodType::Escape && Lara.target != item && info.ahead)
				item->TargetState = 11;
			else if (info.bite || info.distance < SQUARE(2048))
			{
				if (GetRandomControl() < 0x4000)
					item->TargetState = 12;
				else if (GetRandomControl() < 0x2000)
					item->TargetState = 10;
				else
					item->TargetState = 2;
			}

			break;

		case 8:
			creature->maximumTurn = ANGLE(4);
			if (info.bite || info.distance < SQUARE(682))
				item->TargetState = 6;
			else
				item->TargetState = 11;
			break;

		case 5:
		case 6:
		case 7:
		case 10:
		case 12:
			item->ItemFlags[0] = 1;
			creature->maximumTurn = ANGLE(4);
			creature->flags = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;

			if (creature->enemy == LaraItem)
			{
				if ((item->TouchBits & 0x2000) &&
					creature->flags >= tribesmanAxeHit[item->ActiveState][0] &&
					creature->flags <= tribesmanAxeHit[item->ActiveState][1])
				{
					LaraItem->HitPoints -= tribesmanAxeHit[item->ActiveState][2];
					LaraItem->HitStatus = true;

					for (int i = 0; i < tribesmanAxeHit[item->ActiveState][2]; i += 8)
						CreatureEffect(item, &tribesmanAxeBite, DoBloodSplat);

					SoundEffect(70, &item->Position, 0);
				}
			}
			else
			{
				if (creature->enemy)
				{
					if (abs(creature->enemy->Position.xPos - item->Position.xPos) < 512 &&
						abs(creature->enemy->Position.yPos - item->Position.yPos) < 512 &&
						abs(creature->enemy->Position.zPos - item->Position.zPos) < 512 &&
						creature->flags >= tribesmanAxeHit[item->ActiveState][0] &&
						creature->flags <= tribesmanAxeHit[item->ActiveState][1])
					{
						creature->enemy->HitPoints -= 2;
						creature->enemy->HitStatus = true;

						CreatureEffect(item, &tribesmanAxeBite, DoBloodSplat);

						SoundEffect(70, &item->Position, 0);
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
		dartItem->ObjectNumber = ID_DARTS;
		dartItem->RoomNumber = item->RoomNumber;

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

		dartItem->Position.xPos = pos1.x;
		dartItem->Position.yPos = pos1.y;
		dartItem->Position.zPos = pos1.z;

		InitialiseItem(dartItemNumber);

		dartItem->Position.xRot = angles[1];
		dartItem->Position.yRot = angles[0];
		dartItem->Velocity = 256;

		AddActiveItem(dartItemNumber);

		dartItem->Status = ITEM_ACTIVE;

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
	CREATURE_INFO* creature = (CREATURE_INFO *)item->Data;
	
	short headX = 0;
	short headY = 0;
	short torsoX = 0;
	short torsoY = 0;
	short angle = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 9)
		{
			if (item->ActiveState == 1 || item->ActiveState == 4)
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 21;
			else
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 20;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 9;
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(creature);

		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, (info.zoneNumber == info.enemyZone ? VIOLENT : TIMID));

		if (item->HitStatus && Lara.Poisoned >= 0x100 && creature->mood == MoodType::Bored)
			creature->mood = MoodType::Escape;

		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, creature->mood == MoodType::Bored ? ANGLE(2) : creature->maximumTurn);
		if (info.ahead)
		{
			headY = info.angle / 2;
			torsoY = info.angle / 2;
		}

		if (item->HitStatus || 
			(creature->enemy == LaraItem && (info.distance < 1024 || 
				TargetVisible(item, &info)) && (abs(LaraItem->Position.yPos - item->Position.yPos) < 2048))) 
			AlertAllGuards(itemNum);

		switch (item->ActiveState)
		{
		case 1:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle / 2;
			}
			creature->flags &= 0x0FFF;
			creature->maximumTurn = ANGLE(2);
			if (item->AIBits & GUARD)
			{
				headY = AIGuard(creature);
				torsoY = 0;
				torsoX = 0;
				creature->maximumTurn = 0;
				if (!(GetRandomControl() & 0xFF))
					item->TargetState = 11;
				break;
			}
			else if (creature->mood == MoodType::Escape)
			{
				if (Lara.target != item && info.ahead && !item->HitStatus)
					item->TargetState = 1;
				else
					item->TargetState = 3;
			}
			else if (info.bite && info.distance < SQUARE(WALL_SIZE / 2))
				item->TargetState = 11;
			else if (info.bite && info.distance < SQUARE(WALL_SIZE * 2))
				item->TargetState = 2;
			else if (Targetable(item, &info) && info.distance < SQUARE(MAX_VISIBILITY_DISTANCE))
				item->TargetState = 4;
			else if (creature->mood == MoodType::Bored)
			{
				if (GetRandomControl() < 0x200)
					item->TargetState = 2;
				else
					break;
			}
			else
				item->TargetState = 3;
			break;

		case 11:
			creature->flags &= 0x0FFF;
			creature->maximumTurn = ANGLE(2);
			if (item->AIBits & GUARD)
			{
				headY = AIGuard(creature);
				torsoY = 0;
				torsoX = 0;
				creature->maximumTurn = 0;
				if (!(GetRandomControl() & 0xFF))
					item->TargetState = 1;
				break;
			}
			else if (creature->mood == MoodType::Escape)
			{
				if (Lara.target != item && info.ahead && !item->HitStatus)
					item->TargetState = 1;
				else
					item->TargetState = 3;
			}
			else if (info.bite && info.distance < SQUARE(WALL_SIZE / 2))
				item->TargetState = 6;
			else if (info.bite && info.distance < SQUARE(WALL_SIZE * 2))
				item->TargetState = 2;
			else if (Targetable(item, &info) && info.distance < SQUARE(MAX_VISIBILITY_DISTANCE))
				item->TargetState = 1;
			else if (creature->mood == MoodType::Bored && GetRandomControl() < 0x200)
				item->TargetState = 2;
			else
				item->TargetState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(9);

			if (info.bite && info.distance < SQUARE(WALL_SIZE / 2))
				item->TargetState = 11;
			else if (info.bite && info.distance < SQUARE(WALL_SIZE * 2))
				item->TargetState = 2;
			else if (Targetable(item, &info) && info.distance < SQUARE(MAX_VISIBILITY_DISTANCE))
				item->TargetState = 1;
			else if (creature->mood == MoodType::Escape)
				item->TargetState = 3;
			else if (creature->mood == MoodType::Bored)
			{
				if (GetRandomControl() > 0x200)
					item->TargetState = 2;
				else if (GetRandomControl() > 0x200)
					item->TargetState = 11;
				else
					item->TargetState = 1;
			}
			else if (info.distance > SQUARE(2048))
				item->TargetState = 3;
			break;

		case 3:
			creature->flags &= 0x0FFF;
			creature->maximumTurn = ANGLE(6);
			tilt = angle / 4;

			if (info.bite && info.distance < SQUARE(WALL_SIZE / 2))
				item->TargetState = 11;
			else if (Targetable(item, &info) && info.distance < SQUARE(MAX_VISIBILITY_DISTANCE))
				item->TargetState = 1;
			if (item->AIBits & GUARD)
				item->TargetState = 11;
			else if (creature->mood == MoodType::Escape && Lara.target != item && info.ahead)
				item->TargetState = 11;
			else if (creature->mood == MoodType::Bored)
				item->TargetState = 1;
			break;

		case 8:
			if (!info.bite || info.distance > SQUARE(512))
				item->TargetState = 11;
			else
				item->TargetState = 6;
			break;

		case 4:
			if (info.ahead)
			{
				torsoY = info.angle;
				torsoX = info.xAngle;
			}
			creature->maximumTurn = 0;
			if (abs(info.angle) < ANGLE(2))
				item->Position.yRot += info.angle;
			else if (info.angle < 0)
				item->Position.yRot -= ANGLE(2);
			else
				item->Position.yRot += ANGLE(2);


			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 15)
			{
				TribesmanShotDart(item);
				item->TargetState = 1;
			}
			break;

		case 6:
			if (creature->enemy == LaraItem)
			{
				if (!(creature->flags & 0xf000) && (item->TouchBits & 0x2400))
				{
					LaraItem->HitPoints -= 100;
					LaraItem->HitStatus = true;

					creature->flags |= 0x1000;

					SoundEffect(70, &item->Position, 0);

					CreatureEffect(item, &tribesmanDartsBite1, DoBloodSplat);
				}
			}
			else
			{
				if (!(creature->flags & 0xf000) && creature->enemy)
				{
					if (abs(creature->enemy->Position.xPos - item->Position.xPos) < SQUARE(512) &&
						abs(creature->enemy->Position.yPos - item->Position.yPos) < SQUARE(512) &&
						abs(creature->enemy->Position.zPos - item->Position.zPos) < SQUARE(512))
					{
						creature->enemy->HitPoints -= 5;
						creature->enemy->HitStatus = true;
						creature->flags |= 0x1000;

						SoundEffect(70, &item->Position, 0);
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


