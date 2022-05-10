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

// TODO
enum CobraState
{

};

// TODO
enum CobraAnim
{

};

void InitialiseCobra(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 2;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase + 45;
	item->Animation.ActiveState = item->Animation.TargetState = 3;
	item->ItemFlags[2] = item->HitStatus;
}

void CobraControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	float head = 0;
	float angle = 0;
	float tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 4)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 4;
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		AI.angle += Angle::DegToRad(16.8f);

		GetCreatureMood(item, &AI, 1);
		CreatureMood(item, &AI, 1);

		info->Target.x = LaraItem->Pose.Position.x;
		info->Target.z = LaraItem->Pose.Position.z;
		angle = CreatureTurn(item, info->MaxTurn);

		if (AI.ahead)
			head = AI.angle;

		if (abs(AI.angle) < Angle::DegToRad(10.0f))
			item->Pose.Orientation.SetY(item->Pose.Orientation.GetY() + AI.angle);
		else if (AI.angle < 0)
			item->Pose.Orientation.SetY(item->Pose.Orientation.GetY() - Angle::DegToRad(10.0f));
		else
			item->Pose.Orientation.SetY(item->Pose.Orientation.GetY() + Angle::DegToRad(10.0f));

		switch (item->Animation.ActiveState)
		{
		case 1:
			info->Flags = 0;

			if (AI.distance > pow(SECTOR(2.5f), 2))
				item->Animation.TargetState = 3;
			else if (LaraItem->HitPoints > 0 &&
				((AI.ahead && AI.distance < pow(SECTOR(1), 2)) || item->HitStatus || LaraItem->Animation.Velocity > 15))
			{
				item->Animation.TargetState = 2;
			}

			break;

		case 3:
			info->Flags = 0;

			if (item->HitPoints != -16384)
			{
				item->ItemFlags[2] = item->HitPoints;
				item->HitPoints = -16384;
			}
			if (AI.distance < pow(SECTOR(1.5f), 2) && LaraItem->HitPoints > 0)
			{
				item->Animation.TargetState = 0;
				item->HitPoints = item->ItemFlags[2];
			}

			break;

		case 2:
			if (info->Flags != 1 && item->TouchBits & 0x2000)
			{
				CreatureEffect(item, &CobraBite, DoBloodSplat);
				info->Flags = 1;

				LaraItem->HitPoints -= 80;
				LaraItem->HitStatus = true;
				Lara.PoisonPotency += 1;
			}

			break;

		case 0:
			item->HitPoints = item->ItemFlags[2];
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head / 2);
	CreatureJoint(item, 1, head / 2);
	CreatureAnimation(itemNumber, angle, tilt);
}
