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

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = 1;
	item->Animation.ActiveState = 1;
	item->Pose.Position.y += CLICK(2);
}

void InvisibleGhostControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;
	short joint0 = 0;
	short joint2 = 0;
	short joint1 = 0;
		
	if (item->AIBits)
		GetAITarget(creature);
	else if (creature->HurtByLara)
		creature->Enemy = LaraItem;

	AI_INFO AI;
	CreatureAIInfo(item, &AI);

	angle = CreatureTurn(item, creature->MaxTurn);
	if (abs(AI.angle) >= ANGLE(3.0f))
	{
		if (AI.angle > 0)
			item->Pose.Orientation.y += ANGLE(3.0f);
		else
			item->Pose.Orientation.y -= ANGLE(3.0f);
	}
	else
		item->Pose.Orientation.y += AI.angle;

	if (AI.ahead)
	{
		joint0 = AI.angle / 2;
		joint2 = AI.angle / 2;
		joint1 = AI.xAngle;
	}

	creature->MaxTurn = 0;
		
	if (item->Animation.ActiveState == 1)
	{
		creature->Flags = 0;

		if (AI.distance < pow(614, 2))
		{
			if (GetRandomControl() & 1)
				item->Animation.TargetState = 2;
			else
				item->Animation.TargetState = 3;
		}
	}
	else if (item->Animation.ActiveState > 1 &&
		item->Animation.ActiveState <= 3 &&
		!creature->Flags &&
		item->TouchBits & 0x9470 &&
		item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 18)
	{
		DoDamage(creature->Enemy, 400);
		CreatureEffect2(item, &InvisibleGhostBite, 10, item->Pose.Orientation.y, DoBloodSplat);
		creature->Flags = 1;
	}

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);

	if (AI.distance >= pow(SECTOR(1.5f), 2))
	{
		item->AfterDeath = 125;
		item->ItemFlags[0] = 0;
	}
	else
	{
		item->AfterDeath = sqrt(AI.distance) / 16;
		if (item->ItemFlags[0] == 0)
		{
			item->ItemFlags[0] = 1;
			SoundEffect(SFX_TR5_SKELETON_GHOST_APPEAR, &item->Pose);
		}
	}

	CreatureAnimation(itemNumber, angle, 0);
}
