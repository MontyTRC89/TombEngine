#include "framework.h"
#include "tr4_smallscorpion.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

BITE_INFO SmallScorpionBiteInfo1 = { 0, 0, 0, 0 };
BITE_INFO SmallScorpionBiteInfo2 = { 0, 0, 0, 23 };

enum SmallScorionState
{
	SSCORPION_STATE_IDLE = 1,
	SSCORPION_STATE_WALK = 2,
	SSCORPION_STATE_RUN = 3,
	SSCORPION_STATE_ATTACK_1 = 4,
	SSCORPION_STATE_ATTACK_2 = 5,
	SSCORPION_STATE_DEATH_1 = 6,
	SSCORPION_STATE_DEATH_2 = 7
};

// TODO
enum SmallScorpionAnim
{

};

void InitialiseSmallScorpion(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->AnimNumber = Objects[ID_SMALL_SCORPION].animIndex + 2;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = SSCORPION_STATE_IDLE;
	item->ActiveState = SSCORPION_STATE_IDLE;
}

void SmallScorpionControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short angle = 0;
	short head = 0;
	short neck = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;

	if (item->HitPoints <= 0)
	{
		item->HitPoints = 0;
		if (item->ActiveState != SSCORPION_STATE_DEATH_1 &&
			item->ActiveState != SSCORPION_STATE_DEATH_2)
		{
			item->AnimNumber = Objects[ID_SMALL_SCORPION].animIndex + 5;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = SSCORPION_STATE_DEATH_1;
		}
	}
	else
	{
		int dx = LaraItem->Position.xPos - item->Position.xPos;
		int dz = LaraItem->Position.zPos - item->Position.zPos;
		int laraDistance = dx * dx + dz * dz;

		if (item->AIBits & GUARD)
			GetAITarget(info);
		else
			info->Enemy = LaraItem;

		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, VIOLENT);

		angle = CreatureTurn(item, info->MaxTurn);

		switch (item->ActiveState)
		{
		case SSCORPION_STATE_IDLE:
			info->MaxTurn = 0;
			info->Flags = 0;

			if (aiInfo.distance > pow(341, 2))
				item->TargetState = SSCORPION_STATE_WALK;
			else if (aiInfo.bite)
			{
				info->MaxTurn = ANGLE(6.0f);
				if (GetRandomControl() & 1)
					item->TargetState = SSCORPION_STATE_ATTACK_1;
				else
					item->TargetState = SSCORPION_STATE_ATTACK_2;
			}
			else if (!aiInfo.ahead)
				item->TargetState = SSCORPION_STATE_RUN;
			
			break;

		case SSCORPION_STATE_WALK:
			info->MaxTurn = ANGLE(6.0f);
			if (aiInfo.distance >= pow(341, 2))
			{
				if (aiInfo.distance > pow(213, 2))
					item->TargetState = SSCORPION_STATE_RUN;
			}
			else
				item->TargetState = SSCORPION_STATE_IDLE;
			
			break;

		case SSCORPION_STATE_RUN:
			info->MaxTurn = ANGLE(8.0f);

			if (aiInfo.distance < pow(341, 2))
				item->TargetState = SSCORPION_STATE_IDLE;
			
			break;

		case SSCORPION_STATE_ATTACK_1:
		case SSCORPION_STATE_ATTACK_2:
			info->MaxTurn = 0;

			if (abs(aiInfo.angle) >= ANGLE(6.0f))
			{
				if (aiInfo.angle >= 0)
					item->Position.yRot += ANGLE(6.0f);
				else
					item->Position.yRot -= ANGLE(6.0f);
			}
			else
				item->Position.yRot += aiInfo.angle;
			
			if (!info->Flags)
			{
				if (item->TouchBits & 0x1B00100)
				{
					if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 20 &&
						item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 32)
					{
						Lara.Poisoned += 512;
						LaraItem->HitPoints -= 20;
						LaraItem->HitStatus = true;

						short rotation;
						BITE_INFO* biteInfo;
						if (item->ActiveState == SSCORPION_STATE_ATTACK_1)
						{
							rotation = item->Position.yRot + -ANGLE(180.0f);
							biteInfo = &SmallScorpionBiteInfo1;
						}
						else
						{
							rotation = item->Position.yRot + -ANGLE(180.0f);
							biteInfo = &SmallScorpionBiteInfo2;
						}

						CreatureEffect2(item, biteInfo, 3, rotation, DoBloodSplat);
						info->Flags = 1;
					}
				}
			}

			break;
		}
	}

	CreatureAnimation(itemNumber, angle, 0);
}
