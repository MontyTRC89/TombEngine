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

namespace TEN::Entities::Creatures::TR2
{
	const auto SilencerGunBite = BiteInfo(Vector3(3.0f, 331.0f, 56.0f), 10);

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
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		Vector3Shrt extraHeadRot = Vector3Shrt::Zero;
		Vector3Shrt extraTorsoRot = Vector3Shrt::Zero;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 12 && item->Animation.ActiveState != 13)
				SetAnimation(item, 20);
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case 3:
				creature->MaxTurn = 0;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;

				break;

			case 4:
				creature->MaxTurn = 0;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (creature->Mood == MoodType::Escape)
				{
					item->Animation.TargetState = 3;
					item->Animation.RequiredState = 2;
				}
				else
				{
					if (Targetable(item, &AI))
					{
						item->Animation.TargetState = 3;
						item->Animation.RequiredState = (GetRandomControl() >= 0x4000 ? 10 : 6);
					}

					if (creature->Mood == MoodType::Attack || !AI.ahead)
					{
						if (AI.distance >= pow(SECTOR(2), 2))
						{
							item->Animation.TargetState = 3;
							item->Animation.RequiredState = 2;
						}
						else
						{
							item->Animation.TargetState = 3;
							item->Animation.RequiredState = 1;
						}
					}
					else
					{
						if (GetRandomControl() >= 1280)
						{
							if (GetRandomControl() < 2560)
							{
								item->Animation.TargetState = 3;
								item->Animation.RequiredState = 1;
							}
						}
						else
						{
							item->Animation.TargetState = 3;
							item->Animation.RequiredState = 5;
						}
					}
				}

				break;

			case 1:
				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				creature->MaxTurn = 910;

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 2;
				else if (Targetable(item, &AI))
				{
					item->Animation.TargetState = 3;
					item->Animation.RequiredState = (GetRandomControl() >= 0x4000 ? 10 : 6);
				}
				else
				{
					if (AI.distance > pow(SECTOR(2), 2) || !AI.ahead)
						item->Animation.TargetState = 2;
					if (creature->Mood == MoodType::Bored && GetRandomControl() < 0x300)
						item->Animation.TargetState = 3;
				}

				break;

			case 2:
				creature->MaxTurn = ANGLE(5.0f);
				creature->Flags = 0;
				tilt = angle / 4;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (creature->Mood == MoodType::Escape)
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
				else if (creature->Mood == MoodType::Attack)
					item->Animation.TargetState = (GetRandomControl() >= 0x4000) ? 3 : 2;
				else
					item->Animation.TargetState = 3;

				break;

			case 5:
				creature->MaxTurn = 0;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (Targetable(item, &AI))
				{
					item->Animation.TargetState = 3;
					item->Animation.RequiredState = 6;
				}
				else
				{
					if (creature->Mood == MoodType::Attack || GetRandomControl() < 0x100)
						item->Animation.TargetState = 3;
					if (!AI.ahead)
						item->Animation.TargetState = 3;
				}

				break;

			case 6:
			case 10:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}
				else
					extraHeadRot.y = AI.angle;

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 3;
				else if (Targetable(item, &AI))
					item->Animation.TargetState = item->Animation.ActiveState != 6 ? 11 : 7;
				else
					item->Animation.TargetState = 3;

				break;

			case 7:
			case 11:
				creature->MaxTurn = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}
				else
					extraHeadRot.y = AI.angle;

				if (!creature->Flags)
				{
					ShotLara(item, &AI, SilencerGunBite, extraTorsoRot.y, 50);
					creature->Flags = 1;
				}

				break;

			case 9:
				creature->MaxTurn = ANGLE(5.0f);

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}
				else
					extraHeadRot.y = AI.angle;

				if (!item->Animation.RequiredState)
				{
					if (!ShotLara(item, &AI, SilencerGunBite, extraTorsoRot.y, 50))
						item->Animation.TargetState = 2;

					item->Animation.RequiredState = 9;
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, extraTorsoRot.y);
		CreatureJoint(item, 1, extraTorsoRot.x);
		CreatureJoint(item, 2, extraHeadRot.y);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
