#include "framework.h"
#include "tr5_chef.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/people.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

BITE_INFO ChefBite = { 0, 200, 0 ,13 };

// TODO
enum ChefState
{
	CHEF_STATE_COOKING = 1,
	CHEF_STATE_TURN_180 = 2,
	CHEF_STATE_ATTACK = 3,
	CHEF_STATE_AIM = 4,
	CHEF_STATE_WALK = 5,

	CHEF_STATE_DEATH = 8
};

// TODO
enum ChefAnim
{
	CHEF_ANIM_DEATH = 16
};

void InitialiseChef(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	
	ClearItem(itemNumber);

	item->AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->TargetState = CHEF_STATE_COOKING;
	item->ActiveState = CHEF_STATE_COOKING;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->Position.xPos += 192 * phd_sin(item->Position.yRot);
	item->Position.zPos += 192 * phd_cos(item->Position.yRot);
}

void ControlChef(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short angle = 0;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != CHEF_STATE_DEATH)
		{
			item->HitPoints = 0;
			item->ActiveState = CHEF_STATE_DEATH;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + CHEF_ANIM_DEATH;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(info);
		else if (info->hurtByLara)
			info->enemy = LaraItem;

		AI_INFO aiInfo;
		AI_INFO aiLaraInfo;
		CreatureAIInfo(item, &aiInfo);

		if (info->enemy == LaraItem)
		{
			aiLaraInfo.angle = aiInfo.angle;
			aiLaraInfo.distance = aiInfo.distance;
		}
		else
		{
			int dx = LaraItem->Position.xPos - item->Position.xPos;
			int dz = LaraItem->Position.zPos - item->Position.zPos;

			aiLaraInfo.angle = phd_atan(dz, dx) - item->Position.yRot;
			aiLaraInfo.ahead = true;

			if (aiLaraInfo.angle <= -ANGLE(90.0f) || aiLaraInfo.angle >= ANGLE(90.0f))
				aiLaraInfo.ahead = false;

			aiLaraInfo.distance = pow(dx, 2) + pow(dz, 2);
		}

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, VIOLENT);

		short angle = CreatureTurn(item, info->maximumTurn);

		if (aiInfo.ahead)
		{
			joint0 = aiInfo.angle / 2;
			//joint1 = info.xAngle;
			joint2 = aiInfo.angle / 2;
		}

		info->maximumTurn = 0;

		switch (item->ActiveState)
		{
		case CHEF_STATE_COOKING:
			if (abs(LaraItem->Position.yPos - item->Position.yPos) < SECTOR(1) &&
				aiInfo.distance < pow(SECTOR(1.5f), 2) &&
				(item->TouchBits ||
					item->HitStatus ||
					LaraItem->Velocity > 15 ||
					TargetVisible(item, &aiLaraInfo)))
			{
				item->TargetState = CHEF_STATE_TURN_180;
				info->alerted = true;
				item->AIBits = 0;
			}

			break;

		case CHEF_STATE_TURN_180:
			info->maximumTurn = 0;

			if (aiInfo.angle > 0)
				item->Position.yRot -= ANGLE(2.0f);
			else
				item->Position.yRot += ANGLE(2.0f);
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
				item->Position.yRot += -ANGLE(180.0f);

			break;

		case CHEF_STATE_ATTACK:
			info->maximumTurn = 0;

			if (abs(aiInfo.angle) >= ANGLE(2.0f))
			{
				if (aiInfo.angle > 0)
					item->Position.yRot += ANGLE(2.0f);
				else
					item->Position.yRot -= ANGLE(2.0f);
			}
			else
				item->Position.yRot += aiInfo.angle;

			if (!info->flags)
			{
				if (item->TouchBits & 0x2000)
				{
					if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 10)
					{
						CreatureEffect2(item, &ChefBite, 20, item->Position.yRot, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Position, 0);
						info->flags = 1;

						LaraItem->HitPoints -= 80;
						LaraItem->HitStatus = true;
					}
				}
			}

			break;

		case CHEF_STATE_AIM:
			info->maximumTurn = ANGLE(2.0f);
			info->flags = 0;

			if (aiInfo.distance >= pow(682, 2))
			{
				if (aiInfo.angle > ANGLE(112.5f) || aiInfo.angle < -ANGLE(112.5f))
					item->TargetState = CHEF_STATE_TURN_180;
				else if (info->mood == ATTACK_MOOD)
					item->TargetState = CHEF_STATE_WALK;
			}
			else if (aiInfo.bite)
				item->TargetState = CHEF_STATE_ATTACK;
			
			break;

		case CHEF_STATE_WALK:
			info->maximumTurn = ANGLE(7.0f);

			if (aiInfo.distance < pow(682, 2) ||
				aiInfo.angle > ANGLE(112.5f) ||
				aiInfo.angle < -ANGLE(112.5f) ||
				info->mood != ATTACK_MOOD)
			{
				item->TargetState = CHEF_STATE_AIM;
			}

			break;

		default:
			break;
		}
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);
}
