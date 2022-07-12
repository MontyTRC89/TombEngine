#include "framework.h"
#include "Objects/TR3/Entity/tr3_tiger.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::TR3
{
	BITE_INFO TigerBite = { 19, -13, 3, 26 };
	const vector<int> TigerAttackJoints = { 14, 15, 16, 18, 19, 20, 21, 22, 23, 24, 25, 26 };

	constexpr auto TIGER_ATTACK_DAMAGE = 90;

	// TODO
	enum TigerState
	{

	};

	// TODO
	enum TigerAnim
	{

	};

	void TigerControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* info = GetCreatureInfo(item);

		short head = 0;
		short angle = 0;
		short tilt = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 9)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 11;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 9;
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, 1);

			if (info->Alerted && AI.zoneNumber != AI.enemyZone)
				info->Mood = MoodType::Escape;

			CreatureMood(item, &AI, 1);

			angle = CreatureTurn(item, info->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case 1:
				info->MaxTurn = 0;
				info->Flags = 0;

				if (info->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead)
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 3;
				}
				else if (info->Mood == MoodType::Bored)
				{
					short random = GetRandomControl();
					if (random < 0x60)
						item->Animation.TargetState = 5;
					else if (random < 0x460)
						item->Animation.TargetState = 2;
				}
				else if (AI.bite && AI.distance < pow(340, 2))
					item->Animation.TargetState = 6;
				else if (AI.bite && AI.distance < pow(SECTOR(1), 2))
				{
					info->MaxTurn = ANGLE(3.0f);
					item->Animation.TargetState = 8;
				}
				else if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (info->Mood != MoodType::Attack && GetRandomControl() < 0x60)
					item->Animation.TargetState = 5;
				else
					item->Animation.TargetState = 3;

				break;

			case 2:
				info->MaxTurn = ANGLE(3.0f);

				if (info->Mood == MoodType::Escape || info->Mood == MoodType::Attack)
					item->Animation.TargetState = 3;
				else if (GetRandomControl() < 0x60)
				{
					item->Animation.TargetState = 1;
					item->Animation.RequiredState = 5;
				}

				break;

			case 3:
				info->MaxTurn = ANGLE(6.0f);

				if (info->Mood == MoodType::Bored)
					item->Animation.TargetState = 1;
				else if (info->Flags && AI.ahead)
					item->Animation.TargetState = 1;
				else if (AI.bite && AI.distance < pow(SECTOR(1.5f), 2))
				{
					if (LaraItem->Animation.Velocity == 0)
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 7;
				}
				else if (info->Mood != MoodType::Attack && GetRandomControl() < 0x60)
				{
					item->Animation.RequiredState = 5;
					item->Animation.TargetState = 1;
				}
				else if (info->Mood == MoodType::Escape && Lara.TargetEntity != item && AI.ahead)
					item->Animation.TargetState = 1;

				info->Flags = 0;
				break;

			case 6:
			case 7:
			case 8:
				if (!info->Flags && item->TestBits(JointBitType::Touch, TigerAttackJoints))
				{
					CreatureEffect(item, &TigerBite, DoBloodSplat);
					DoDamage(info->Enemy, TIGER_ATTACK_DAMAGE);
					info->Flags = 1;
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
