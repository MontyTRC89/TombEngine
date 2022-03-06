#include "framework.h"
#include "tr5_ghost.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

BITE_INFO InvisibleGhostBite = { 0, 0, 0, 17 };

void InitialiseInvisibleGhost(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = 1;
	item->ActiveState = 1;
	item->Position.yPos += CLICK(2);
}

void InvisibleGhostControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short joint0 = 0;
	short joint2 = 0;
	short joint1 = 0;
	short angle = 0;
		
	if (item->AIBits)
		GetAITarget(info);
	else if (info->HurtByLara)
		info->Enemy = LaraItem;

	AI_INFO aiInfo;
	CreatureAIInfo(item, &aiInfo);

	angle = CreatureTurn(item, info->MaxTurn);
	if (abs(aiInfo.angle) >= ANGLE(3.0f))
	{
		if (aiInfo.angle > 0)
			item->Position.yRot += ANGLE(3.0f);
		else
			item->Position.yRot -= ANGLE(3.0f);
	}
	else
		item->Position.yRot += aiInfo.angle;

	if (aiInfo.ahead)
	{
		joint0 = aiInfo.angle / 2;
		joint2 = aiInfo.angle / 2;
		joint1 = aiInfo.xAngle;
	}

	info->MaxTurn = 0;
		
	if (item->ActiveState == 1)
	{
		info->Flags = 0;

		if (aiInfo.distance < pow(614, 2))
		{
			if (GetRandomControl() & 1)
				item->TargetState = 2;
			else
				item->TargetState = 3;
		}
	}
	else if (item->ActiveState > 1 &&
		item->ActiveState <= 3 &&
		!info->Flags &&
		item->TouchBits & 0x9470 &&
		item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 18)
	{
		CreatureEffect2(item, &InvisibleGhostBite, 10, item->Position.yRot, DoBloodSplat);
		info->Flags = 1;

		LaraItem->HitPoints -= 400;
		LaraItem->HitStatus = true;
	}

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);

	if (aiInfo.distance >= pow(SECTOR(1.5f), 2))
	{
		item->AfterDeath = 125;
		item->ItemFlags[0] = 0;
	}
	else
	{
		item->AfterDeath = sqrt(aiInfo.distance) / 16;
		if (item->ItemFlags[0] == 0)
		{
			item->ItemFlags[0] = 1;
			SoundEffect(SFX_TR5_SKELETON_APPEAR, &item->Position, 0);
		}
	}

	CreatureAnimation(itemNumber, angle, 0);
}
