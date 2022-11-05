#include "framework.h"
#include "Objects/TR5/Entity/tr5_brownbeast.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/setup.h"
#include "Math/Random.h"
#include "Specific/level.h"

using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::Creatures::TR5
{
	constexpr auto BROWN_BEAST_ATTACK_DAMAGE = 150;

	const auto BrownBeastBite1 = BiteInfo(Vector3::Zero, 16);
	const auto BrownBeastBite2 = BiteInfo(Vector3::Zero, 22);
	const vector<unsigned int> BrownBeastAttackJoints1 = { 14, 15, 16, 17 };
	const vector<unsigned int> BrownBeastAttackJoints2 = { 20, 21, 22, 23 };

	// TODO
	enum BrownBeastState
	{

	};

	// TODO
	enum BrownBeastAnim
	{

	};

	void InitialiseBrownBeast(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);
		SetAnimation(item, 0);
	}

	void ControlBrowsBeast(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;

		if (item->HitPoints <= 0)
		{
			item->HitPoints = 0;
			if (item->Animation.ActiveState != 7)
			{
				item->Animation.AnimNumber = Objects[ID_BROWN_BEAST].animIndex + 10;
				item->Animation.ActiveState = 7;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else if (creature->HurtByLara)
				creature->Enemy = LaraItem;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			int distance;

			if (creature->Enemy == LaraItem)
				distance = AI.distance;
			else
			{
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				phd_atan(dz, dz);

				distance = pow(dx, 2) + pow(dz, 2);
			}

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);
			creature->MaxTurn = ANGLE(7.0f);

			switch (item->Animation.ActiveState)
			{
			case 1:
				creature->Flags = 0;

				if (creature->Mood == MoodType::Attack)
				{
					if (distance <= pow(SECTOR(1), 2))
					{
						if (TestProbability(1.0f / 2))
							item->Animation.TargetState = 4;
						else
							item->Animation.TargetState = 6;
					}
					else if (TestProbability(1.0f / 2))
						item->Animation.TargetState = 2;
					else
						item->Animation.TargetState = 3;
				}
				else
					item->Animation.TargetState = 1;

				break;

			case 2:
			case 3:
				if (distance < pow(SECTOR(1), 2) || creature->Mood != MoodType::Attack)
					item->Animation.TargetState = 1;

				SoundEffect(SFX_TR5_IMP_BARREL_ROLL, &item->Pose);
				break;

			case 4:
			case 6:
				creature->MaxTurn = 0;

				if (abs(AI.angle) >= ANGLE(2.0f))
				{
					if (AI.angle > 0)
						item->Pose.Orientation.y += ANGLE(2.0f);
					else
						item->Pose.Orientation.y -= ANGLE(2.0f);
				}
				else
					item->Pose.Orientation.y += AI.angle;

				if (creature->Flags)
					break;

				if (item->TouchBits.Test(BrownBeastAttackJoints1))
				{
					if (item->Animation.AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 8)
					{
						if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 19 &&
							item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 25)
						{
							DoDamage(creature->Enemy, BROWN_BEAST_ATTACK_DAMAGE);
							CreatureEffect2(item, BrownBeastBite1, 20, item->Pose.Orientation.y, DoBloodSplat);
							creature->Flags |= 1;
							break;
						}
					}

					if (item->Animation.AnimNumber == (Objects[ID_BROWN_BEAST].animIndex + 2))
					{
						if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 6 &&
							item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 16)
						{
							DoDamage(creature->Enemy, BROWN_BEAST_ATTACK_DAMAGE);
							CreatureEffect2(item, BrownBeastBite1, 20, item->Pose.Orientation.y, DoBloodSplat);
							creature->Flags |= 1;
							break;
						}
					}
				}

				if (!item->TouchBits.Test(BrownBeastAttackJoints2))
					break;

				if (item->Animation.AnimNumber == (Objects[ID_BROWN_BEAST].animIndex + 8))
				{
					if (item->Animation.FrameNumber > (g_Level.Anims[item->Animation.AnimNumber].frameBase + 13) &&
						item->Animation.FrameNumber < (g_Level.Anims[item->Animation.AnimNumber].frameBase + 20))
					{
						DoDamage(creature->Enemy, BROWN_BEAST_ATTACK_DAMAGE);
						CreatureEffect2(item, BrownBeastBite2, 20, item->Pose.Orientation.y, DoBloodSplat);
						creature->Flags |= 2;
						break;
					}
				}

				if (item->Animation.AnimNumber == (Objects[ID_BROWN_BEAST].animIndex + 2))
				{
					if (item->Animation.FrameNumber > (g_Level.Anims[item->Animation.AnimNumber].frameBase + 33) &&
						item->Animation.FrameNumber < (g_Level.Anims[item->Animation.AnimNumber].frameBase + 43))
					{
						DoDamage(creature->Enemy, BROWN_BEAST_ATTACK_DAMAGE);
						CreatureEffect2(item, BrownBeastBite2, 20, item->Pose.Orientation.y, DoBloodSplat);
						creature->Flags |= 2;
						break;
					}
				}

				break;

			default:
				break;
			}
		}

		CreatureAnimation(itemNumber, angle, 0);
	}
}
