#include "framework.h"
#include "Objects/TR2/Entity/tr2_silencer.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO SilencerGunBite = { 3, 331, 56, 10 };

// TODO
enum SilencerState
{

};

// TODO
enum SilencerAnim
{

};

void SilencerControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short angle = 0;
	short torsoX = 0;
	short torsoY = 0;
	short head = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 12 && item->Animation.ActiveState != 13)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 20;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 13;
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, info->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case 3:
			if (AI.ahead)
				head = AI.angle;
			info->MaxTurn = 0;

			if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;

			break;

		case 4:
			if (AI.ahead)
				head = AI.angle;
			info->MaxTurn = 0;

			if (info->Mood == MoodType::Escape)
			{
				item->Animation.RequiredState = 2;
				item->Animation.TargetState = 3;
			}
			else
			{
				if (Targetable(item, &AI))
				{
					item->Animation.RequiredState = (GetRandomControl() >= 0x4000 ? 10 : 6);
					item->Animation.TargetState = 3;
				}

				if (info->Mood == MoodType::Attack || !AI.ahead)
				{
					if (AI.distance >= pow(SECTOR(2), 2))
					{
						item->Animation.RequiredState = 2;
						item->Animation.TargetState = 3;
					}
					else
					{
						item->Animation.RequiredState = 1;
						item->Animation.TargetState = 3;
					}
				}
				else
				{
					if (GetRandomControl() >= 1280)
					{
						if (GetRandomControl() < 2560)
						{
							item->Animation.RequiredState = 1;
							item->Animation.TargetState = 3;
						}
					}
					else
					{
						item->Animation.RequiredState = 5;
						item->Animation.TargetState = 3;
					}
				}
			}

			break;

		case 1:
			if (AI.ahead)
				head = AI.angle;

			info->MaxTurn = 910;

			if (info->Mood == MoodType::Escape)
				item->Animation.TargetState = 2;
			else if (Targetable(item, &AI))
			{
				item->Animation.RequiredState = (GetRandomControl() >= 0x4000 ? 10 : 6);
				item->Animation.TargetState = 3;
			}
			else
			{
				if (AI.distance > pow(SECTOR(2), 2) || !AI.ahead)
					item->Animation.TargetState = 2;
				if (info->Mood == MoodType::Bored && GetRandomControl() < 0x300)
					item->Animation.TargetState = 3;
			}

			break;

		case 2:
			if (AI.ahead)
				head = AI.angle;

			info->MaxTurn = ANGLE(5.0f);
			info->Flags = 0;
			tilt = angle / 4;

			if (info->Mood == MoodType::Escape)
			{
				if (Targetable(item, &AI))
					item->Animation.TargetState = 9;

				break;

			}

			if (Targetable(item, &AI))
			{
				if (AI.distance >= pow(SECTOR(2), 2) && AI.zoneNumber == AI.enemyZone)
					item->Animation.TargetState = 9;

				break;
			}
			else if (info->Mood == MoodType::Attack)
				item->Animation.TargetState = (GetRandomControl() >= 0x4000) ? 3 : 2;
			else
				item->Animation.TargetState = 3;

			break;

		case 5:
			if (AI.ahead)
				head = AI.angle;

			info->MaxTurn = 0;

			if (Targetable(item, &AI))
			{
				item->Animation.RequiredState = 6;
				item->Animation.TargetState = 3;
			}
			else
			{
				if (info->Mood == MoodType::Attack || GetRandomControl() < 0x100)
					item->Animation.TargetState = 3;
				if (!AI.ahead)
					item->Animation.TargetState = 3;
			}

			break;

		case 6:
		case 10:
			info->MaxTurn = 0;
			info->Flags = 0;

			if (AI.ahead)
			{
				torsoY = AI.angle;
				torsoX = AI.xAngle;
			}
			else
				head = AI.angle;

			if (info->Mood == MoodType::Escape)
				item->Animation.TargetState = 3;
			else if (Targetable(item, &AI))
				item->Animation.TargetState = item->Animation.ActiveState != 6 ? 11 : 7;
			else
				item->Animation.TargetState = 3;

			break;

		case 7:
		case 11:
			info->MaxTurn = 0;

			if (AI.ahead)
			{
				torsoY = AI.angle;
				torsoX = AI.xAngle;
			}
			else
				head = AI.angle;

			if (!info->Flags)
			{
				ShotLara(item, &AI, &SilencerGunBite, torsoY, 50);
				info->Flags = 1;
			}

			break;

		case 9:
			info->MaxTurn = ANGLE(5.0f);

			if (AI.ahead)
			{
				torsoY = AI.angle;
				torsoX = AI.xAngle;
			}
			else
				head = AI.angle;

			if (!item->Animation.RequiredState)
			{
				if (!ShotLara(item, &AI, &SilencerGunBite, torsoY, 50))
					item->Animation.TargetState = 2;

				item->Animation.RequiredState = 9;
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torsoY);
	CreatureJoint(item, 1, torsoX);
	CreatureJoint(item, 2, head);
	CreatureAnimation(itemNumber, angle, tilt);
}
