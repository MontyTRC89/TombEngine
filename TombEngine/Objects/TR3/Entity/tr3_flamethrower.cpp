#include "framework.h"
#include "Objects/TR3/Entity/tr3_flamethrower.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;

namespace TEN::Entities::Creatures::TR3
{
	const auto FlamethrowerOffset = Vector3Int(0, 340, 0);
	const auto FlamethrowerBite = BiteInfo(Vector3(0.0f, 340.0f, 64.0f), 7);

	// TODO
	enum FlamethrowerState
	{

	};

	// TODO
	enum FlamethrowerAnim
	{

	};

	void FlameThrowerControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		auto extraHeadRot = Vector3Shrt::Zero;
		auto extraTorsoRot = Vector3Shrt::Zero;

		auto pos = Vector3Int(FlamethrowerBite.Position);
		GetJointAbsPosition(item, &pos, FlamethrowerBite.meshNum);

		int randomInt = GetRandomControl();
		if (item->Animation.ActiveState != 6 && item->Animation.ActiveState != 11)
		{
			TriggerDynamicLight(pos.x, pos.y, pos.z, (randomInt & 3) + 6, 24 - ((randomInt / 16) & 3), 16 - ((randomInt / 64) & 3), randomInt & 3);
			TriggerPilotFlame(itemNumber, 9);
		}
		else
			TriggerDynamicLight(pos.x, pos.y, pos.z, (randomInt & 3) + 10, 31 - ((randomInt / 16) & 3), 24 - ((randomInt / 64) & 3), randomInt & 7);

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 7)
				SetAnimation(item, 19);
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else if (creature->HurtByLara)
				creature->Enemy = LaraItem;
			else
			{
				creature->Enemy = nullptr;

				ItemInfo* target = nullptr;
				int minDistance = INT_MAX;

				for (auto& currentCreature : ActiveCreatures)
				{
					if (currentCreature->ItemNumber == NO_ITEM || currentCreature->ItemNumber == itemNumber)
						continue;

					target = &g_Level.Items[currentCreature->ItemNumber];
					if (target->ObjectNumber == ID_LARA || target->HitPoints <= 0)
						continue;

					int x = target->Pose.Position.x - item->Pose.Position.x;
					int z = target->Pose.Position.z - item->Pose.Position.z;

					int distance = pow(x, 2) + pow(z, 2);
					if (distance < minDistance)
					{
						creature->Enemy = target;
						minDistance = distance;
					}
				}
			}

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			AI_INFO laraAI;
			if (creature->Enemy == LaraItem)
			{
				laraAI.angle = AI.angle;
				laraAI.distance = AI.distance;

				if (!creature->HurtByLara)
					creature->Enemy = nullptr;
			}
			else
			{
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;

				laraAI.angle = phd_atan(dz, dz) - item->Pose.Orientation.y;
				laraAI.distance = pow(dx, 2) + pow(dz, 2);

				AI.xAngle -= ANGLE(11.25f);
			}

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			auto* realEnemy = creature->Enemy;

			if (item->HitStatus || laraAI.distance < pow(SECTOR(1), 2) || TargetVisible(item, &laraAI))
			{
				if (!creature->Alerted)
					SoundEffect(SFX_TR3_AMERCAN_HOY, &item->Pose);

				AlertAllGuards(itemNumber);
			}

			switch (item->Animation.ActiveState)
			{
			case 1:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				extraHeadRot.y = laraAI.angle;

				if (item->AIBits & GUARD)
				{
					extraHeadRot.y = AIGuard(creature);

					if (TestProbability(0.008f))
						item->Animation.TargetState = 4;

					break;
				}
				else if (item->AIBits & PATROL1)
					item->Animation.TargetState = 2;
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 2;
				else if (Targetable(item, &AI) && (realEnemy != LaraItem || creature->HurtByLara))
				{
					if (AI.distance < pow(SECTOR(2), 2))
						item->Animation.TargetState = 10;
					else
						item->Animation.TargetState = 2;
				}
				else if (creature->Mood == MoodType::Bored && AI.ahead && TestProbability(0.008f))
					item->Animation.TargetState = 4;
				else if (creature->Mood == MoodType::Attack || TestProbability(0.008f))
					item->Animation.TargetState = 2;

				break;

			case 4:
				extraHeadRot.y = laraAI.angle;

				if (item->AIBits & GUARD)
				{
					extraHeadRot.y = AIGuard(creature);

					if (TestProbability(0.008f))
						item->Animation.TargetState = 1;

					break;
				}
				else if ((Targetable(item, &AI) &&
					AI.distance < pow(SECTOR(4), 2) &&
					(realEnemy != LaraItem || creature->HurtByLara) ||
					creature->Mood != MoodType::Bored ||
					TestProbability(0.008f)))
				{
					item->Animation.TargetState = 1;
				}

				break;

			case 2:
				creature->MaxTurn = ANGLE(5.0f);
				creature->Flags = 0;
				extraHeadRot.y = laraAI.angle;

				if (item->AIBits & GUARD)
					SetAnimation(item, 12);
				else if (item->AIBits & PATROL1)
					item->Animation.TargetState = 2;
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 2;
				else if (Targetable(item, &AI) &&
					(realEnemy != LaraItem || creature->HurtByLara))
				{
					if (AI.distance < pow(SECTOR(4), 2))
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 9;
				}
				else if (creature->Mood == MoodType::Bored && AI.ahead)
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 2;

				break;

			case 10:
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;

					if (Targetable(item, &AI) &&
						AI.distance < pow(SECTOR(4), 2) &&
						(realEnemy != LaraItem || creature->HurtByLara))
					{
						item->Animation.TargetState = 11;
					}
					else
						item->Animation.TargetState = 1;
				}

				break;

			case 9:
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;

					if (Targetable(item, &AI) &&
						AI.distance < pow(SECTOR(4), 2) &&
						(realEnemy != LaraItem || creature->HurtByLara))
					{
						item->Animation.TargetState = 6;
					}
					else
						item->Animation.TargetState = 2;
				}

				break;

			case 11:
				if (creature->Flags < 40)
					creature->Flags += (creature->Flags / 4) + 1;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;

					if (Targetable(item, &AI) &&
						AI.distance < pow(SECTOR(4), 2) &&
						(realEnemy != LaraItem || creature->HurtByLara))
					{
						item->Animation.TargetState = 11;
					}
					else
						item->Animation.TargetState = 1;
				}
				else
					item->Animation.TargetState = 1;

				if (creature->Flags < 40)
					ThrowFire(itemNumber, FlamethrowerBite.meshNum, FlamethrowerOffset, Vector3Int(0, creature->Flags * 1.5f, 0));
				else
				{
					ThrowFire(itemNumber, FlamethrowerBite.meshNum, FlamethrowerOffset, Vector3Int(0, (GetRandomControl() & 63) + 12, 0));
					if (realEnemy)
					{
						/*code*/
					}
				}

				SoundEffect(SFX_TR4_FLAME_EMITTER, &item->Pose);
				break;

			case 6:
				if (creature->Flags < 40)
					creature->Flags += (creature->Flags / 4) + 1;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;

					if (Targetable(item, &AI) &&
						AI.distance < pow(SECTOR(4), 2) &&
						(!realEnemy->IsLara() || creature->HurtByLara))
					{
						item->Animation.TargetState = 6;
					}
					else
						item->Animation.TargetState = 2;
				}
				else
					item->Animation.TargetState = 2;

				if (creature->Flags < 40)
					ThrowFire(itemNumber, FlamethrowerBite.meshNum, FlamethrowerOffset, Vector3Int(0, creature->Flags * 1.5f, 0));
				else
				{
					ThrowFire(itemNumber, FlamethrowerBite.meshNum, FlamethrowerOffset, Vector3Int(0, (GetRandomControl() & 63) + 12, 0));
					if (realEnemy)
					{
						/*code*/
					}
				}

				SoundEffect(SFX_TR4_FLAME_EMITTER, &item->Pose);
				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, extraTorsoRot.y);
		CreatureJoint(item, 1, extraTorsoRot.x);
		CreatureJoint(item, 2, extraHeadRot.y);

		CreatureAnimation(itemNumber, angle, 0);
	}
}
