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
	short torsoY = 0;
	short torsoX = 0;
	short head = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 12 && item->ActiveState != 13)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 20;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 13;
		}
	}
	else
	{
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, VIOLENT);

		angle = CreatureTurn(item, info->maximumTurn);

		switch (item->ActiveState)
		{
		case 3:
			if (aiInfo.ahead)
				head = aiInfo.angle;
			info->maximumTurn = 0;

			if (item->RequiredState)
				item->TargetState = item->RequiredState;

			break;

		case 4:
			if (aiInfo.ahead)
				head = aiInfo.angle;
			info->maximumTurn = 0;

			if (info->mood == ESCAPE_MOOD)
			{
				item->RequiredState = 2;
				item->TargetState = 3;
			}
			else
			{
				if (Targetable(item, &aiInfo))
				{
					item->RequiredState = (GetRandomControl() >= 0x4000 ? 10 : 6);
					item->TargetState = 3;
				}

				if (info->mood == ATTACK_MOOD || !aiInfo.ahead)
				{
					if (aiInfo.distance >= 0x400000)
					{
						item->RequiredState = 2;
						item->TargetState = 3;
					}
					else
					{
						item->RequiredState = 1;
						item->TargetState = 3;
					}
				}
				else
				{
					if (GetRandomControl() >= 1280)
					{
						if (GetRandomControl() < 2560)
						{
							item->RequiredState = 1;
							item->TargetState = 3;
						}
					}
					else
					{
						item->RequiredState = 5;
						item->TargetState = 3;
					}
				}
			}

			break;

		case 1:
			if (aiInfo.ahead)
				head = aiInfo.angle;

			info->maximumTurn = 910;

			if (info->mood == ESCAPE_MOOD)
				item->TargetState = 2;
			else if (Targetable(item, &aiInfo))
			{
				item->RequiredState = (GetRandomControl() >= 0x4000 ? 10 : 6);
				item->TargetState = 3;
			}
			else
			{
				if (aiInfo.distance > 0x400000 || !aiInfo.ahead)
					item->TargetState = 2;
				if (info->mood == BORED_MOOD && GetRandomControl() < 0x300)
					item->TargetState = 3;
			}

			break;

		case 2:
			if (aiInfo.ahead)
				head = aiInfo.angle;

			info->maximumTurn = ANGLE(5.0f);
			info->flags = 0;
			tilt = angle / 4;

			if (info->mood == ESCAPE_MOOD)
			{
				if (Targetable(item, &aiInfo))
					item->TargetState = 9;

				break;

			}

			if (Targetable(item, &aiInfo))
			{
				if (aiInfo.distance >= 0x400000 && aiInfo.zoneNumber == aiInfo.enemyZone)
					item->TargetState = 9;

				break;
			}
			else if (info->mood == ATTACK_MOOD)
				item->TargetState = (GetRandomControl() >= 0x4000) ? 3 : 2;
			else
				item->TargetState = 3;

			break;

		case 5:
			if (aiInfo.ahead)
				head = aiInfo.angle;

			info->maximumTurn = 0;

			if (Targetable(item, &aiInfo))
			{
				item->RequiredState = 6;
				item->TargetState = 3;
			}
			else
			{
				if (info->mood == ATTACK_MOOD || GetRandomControl() < 0x100)
					item->TargetState = 3;
				if (!aiInfo.ahead)
					item->TargetState = 3;
			}

			break;

		case 6:
		case 10:
			info->maximumTurn = 0;
			info->flags = 0;

			if (aiInfo.ahead)
			{
				torsoY = aiInfo.angle;
				torsoX = aiInfo.xAngle;
			}
			else
				head = aiInfo.angle;

			if (info->mood == ESCAPE_MOOD)
				item->TargetState = 3;
			else if (Targetable(item, &aiInfo))
				item->TargetState = item->ActiveState != 6 ? 11 : 7;
			else
				item->TargetState = 3;

			break;

		case 7:
		case 11:
			info->maximumTurn = 0;

			if (aiInfo.ahead)
			{
				torsoY = aiInfo.angle;
				torsoX = aiInfo.xAngle;
			}
			else
				head = aiInfo.angle;

			if (!info->flags)
			{
				ShotLara(item, &aiInfo, &SilencerGunBite, torsoY, 50);
				info->flags = 1;
			}

			break;

		case 9:
			info->maximumTurn = ANGLE(5.0f);

			if (aiInfo.ahead)
			{
				torsoY = aiInfo.angle;
				torsoX = aiInfo.xAngle;
			}
			else
				head = aiInfo.angle;

			if (!item->RequiredState)
			{
				if (!ShotLara(item, &aiInfo, &SilencerGunBite, torsoY, 50))
					item->TargetState = 2;

				item->RequiredState = 9;
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
