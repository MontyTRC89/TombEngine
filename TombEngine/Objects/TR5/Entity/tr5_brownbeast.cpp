#include "framework.h"
#include "tr5_brownbeast.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

BITE_INFO BrownBeastBite1 = { 0, 0, 0, 16 };
BITE_INFO BrownBeastBite2 = { 0, 0, 0, 22 };

// TODO
enum BrownBeastState
{

};

// TODO
enum BrownBeastAnim
{

};

void InitialiseBrownBeast(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);
	
	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = 1;
	item->Animation.ActiveState = 1;
}

void ControlBrowsBeast(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;
	
	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle  = 0;

	if (item->HitPoints <= 0)
	{
		item->HitPoints = 0;
		if (item->Animation.ActiveState != 7)
		{
			item->Animation.AnimNumber = Objects[ID_BROWN_BEAST].animIndex + 10;
			item->Animation.ActiveState = 7;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(creature);
		else if (creature->HurtByLara)
			creature->Enemy = LaraItem;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		int distance;

		if (creature->Enemy == LaraItem)
			distance = AI.distance;
		else
		{
			int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
			int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
			phd_atan(dz, dz);

			distance = pow(dx, 2) + pow(dz, 2);
		}

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);
		creature->MaxTurn = ANGLE(7.0f);

		switch (item->Animation.ActiveState)
		{
		case 1:
			creature->Flags = 0;
			if (creature->Mood == MoodType::Attack)
			{
				if (distance <= pow(SECTOR(1), 2))
				{
					if (GetRandomControl() & 1)
						item->Animation.TargetState = 4;
					else
						item->Animation.TargetState = 6;
				}
				else if (GetRandomControl() & 1)
					item->Animation.TargetState = 2;
				else
					item->Animation.TargetState = 3;
			}
			else
				item->Animation.TargetState = 1;
			
			break;

		case 2:
		case 3:
			if (distance < pow(SECTOR(1), 2) || creature->Mood != MoodType::Attack)
				item->Animation.TargetState = 1;

			SoundEffect(SFX_TR5_IMP_BARREL_ROLL, &item->Pose);
			break;

		case 4:
		case 6:
			creature->MaxTurn = 0;

			if (abs(AI.angle) >= ANGLE(2.0f))
			{
				if (AI.angle > 0)
					item->Pose.Orientation.y += ANGLE(2.0f);
				else
					item->Pose.Orientation.y -= ANGLE(2.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;

			if (creature->Flags)
				break;

			if (item->TouchBits & 0x3C000)
			{
				if (item->Animation.AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 8)
				{
					if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 19 &&
						item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 25)
					{
						DoDamage(creature->Enemy, 150);
						CreatureEffect2(item, &BrownBeastBite1, 20, item->Pose.Orientation.y, DoBloodSplat);
						creature->Flags |= 1;
						break;
					}
				}

				if (item->Animation.AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 2)
				{
					if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 6 &&
						item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 16)
					{
						DoDamage(creature->Enemy, 150);
						CreatureEffect2(item, &BrownBeastBite1, 20, item->Pose.Orientation.y, DoBloodSplat);
						creature->Flags |= 1;
						break;
					}
				}
			}

			if (!(item->TouchBits & 0xF00000))
				break;

			if (item->Animation.AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 8)
			{
				if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 13 &&
					item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 20)
				{
					DoDamage(creature->Enemy, 150);
					CreatureEffect2(item, &BrownBeastBite2, 20, item->Pose.Orientation.y, DoBloodSplat);
					creature->Flags |= 2;
					break;
				}
			}

			if (item->Animation.AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 2)
			{
				if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 33 &&
					item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 43)
				{
					DoDamage(creature->Enemy, 150);
					CreatureEffect2(item, &BrownBeastBite2, 20, item->Pose.Orientation.y, DoBloodSplat);
					creature->Flags |= 2;
					break;
				}
			}

			break;

		default:
			break;
		}
	}

	CreatureAnimation(itemNumber, angle, 0);
}
