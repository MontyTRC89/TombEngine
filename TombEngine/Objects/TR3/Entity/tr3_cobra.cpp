#include "framework.h"
#include "Objects/TR3/Entity/tr3_cobra.h"

#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO CobraBite = { 0, 0, 0, 13 };


enum CobraState
{
	COBRA_STATE_NONE = 0,
	COBRA_STATE_IDLE = 1,
	COBRA_STATE_ATTACK = 2,
	COBRA_STATE_SLEEP = 3,
};


enum CobraAnim
{
	COBRA_ANIM_IDLE = 0,
	COBRA_ANIM_WAKE_UP = 1,
	COBRA_ANIM_BACK_TO_SLEEP = 2,
	COBRA_ANIM_BITE_ATTACK = 3,
	COBRA_ANIM_DEATH = 4,
};

void InitialiseCobra(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + COBRA_ANIM_BACK_TO_SLEEP;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase + 45;
	item->Animation.ActiveState = item->Animation.TargetState = COBRA_STATE_SLEEP;
	item->ItemFlags[2] = item->HitStatus;
}

void CobraControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->HitPoints <= 0 && item->HitPoints != NOT_TARGETABLE)
	{
		if (item->Animation.ActiveState != 4)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + COBRA_ANIM_DEATH;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 4; //Core prob made a mistake putting 4 as this block of code clearly makes cobra die but I'll leave it for now - Kubsy 2022/06/01
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		AI.angle += ANGLE(16.8f);

		GetCreatureMood(item, &AI, 1);
		CreatureMood(item, &AI, 1);

		info->Target.x = LaraItem->Pose.Position.x;
		info->Target.z = LaraItem->Pose.Position.z;
		angle = CreatureTurn(item, info->MaxTurn);

		if (AI.ahead)
			head = AI.angle;

		if (abs(AI.angle) < ANGLE(10.0f))
			item->Pose.Orientation.y += AI.angle;
		else if (AI.angle < 0)
			item->Pose.Orientation.y -= ANGLE(10.0f);
		else
			item->Pose.Orientation.y += ANGLE(10.0f);

		switch (item->Animation.ActiveState)
		{
		case COBRA_STATE_IDLE:
			info->Flags = 0;

			if (AI.distance > pow(SECTOR(2.5f), 2))
				item->Animation.TargetState = COBRA_STATE_SLEEP;
			else if (LaraItem->HitPoints > 0 &&
				((AI.ahead && AI.distance < pow(SECTOR(1), 2)) || item->HitStatus || LaraItem->Animation.Velocity > 15))
			{
				item->Animation.TargetState = COBRA_STATE_ATTACK;
			}

			break;

		case COBRA_STATE_SLEEP:
			info->Flags = 0;

			if (item->HitPoints != NOT_TARGETABLE)
			{
				item->ItemFlags[2] = item->HitPoints;
				item->HitPoints = NOT_TARGETABLE;
			}
			if (AI.distance < pow(SECTOR(1.5f), 2) && LaraItem->HitPoints > 0)
			{
				item->Animation.TargetState = 0;
				item->HitPoints = item->ItemFlags[2];
			}

			break;

		case COBRA_STATE_ATTACK:
			if (info->Flags != 1 && item->TouchBits & 0x2000)
			{
				CreatureEffect(item, &CobraBite, DoBloodSplat);
				info->Flags = 1;

				LaraItem->HitPoints -= 80;
				LaraItem->HitStatus = true;
				Lara.PoisonPotency += 1;
			}

			break;

		case 0: //No Idea what this is - Kubsy 2022/06/01
			item->HitPoints = item->ItemFlags[2];
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head / 2);
	CreatureJoint(item, 1, head / 2);
	CreatureAnimation(itemNumber, angle, tilt);
}
