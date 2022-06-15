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
#include "Specific/setup.h"

namespace TEN::Entities::TR3
{
	BITE_INFO FlamethrowerBite = { 0, 340, 64, 7 };
	Vector3Int FlamethrowerOffset = { 0, 340, 0 };

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

		short torsoX = 0;
		short torsoY = 0;
		short angle = 0;
		short tilt = 0;
		short head = 0;

		Vector3Int pos = { FlamethrowerBite.x, FlamethrowerBite.y, FlamethrowerBite.z };
		GetJointAbsPosition(item, &pos, FlamethrowerBite.meshNum);

		int random = GetRandomControl();
		if (item->Animation.ActiveState != 6 && item->Animation.ActiveState != 11)
		{
			TriggerDynamicLight(pos.x, pos.y, pos.z, (random & 3) + 6, 24 - ((random / 16) & 3), 16 - ((random / 64) & 3), random & 3);
			TriggerPilotFlame(itemNumber, 9);
		}
		else
		{
			TriggerDynamicLight(pos.x, pos.y, pos.z, (random & 3) + 10, 31 - ((random / 16) & 3), 24 - ((random / 64) & 3), random & 7);
		}

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 7)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 19;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 7;
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else if (creature->HurtByLara)
				creature->Enemy = LaraItem;
			else
			{
				creature->Enemy = NULL;

				ItemInfo* target = NULL;
				int minDistance = INT_MAX;

				for (int i = 0; i < ActiveCreatures.size(); i++)
				{
					auto* currentCreature = ActiveCreatures[i];
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
					creature->Enemy = NULL;
			}
			else
			{
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;

				laraAI.angle = phd_atan(dz, dz) - item->Pose.Orientation.y;
				laraAI.distance = pow(dx, 2) + pow(dz, 2);

				AI.xAngle -= 0x800;
			}

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

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
				head = laraAI.angle;

				if (item->AIBits & GUARD)
				{
					head = AIGuard(creature);

					if (!(GetRandomControl() & 0xFF))
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
				else if (creature->Mood == MoodType::Bored && AI.ahead && !(GetRandomControl() & 0xFF))
					item->Animation.TargetState = 4;
				else if (creature->Mood == MoodType::Attack || !(GetRandomControl() & 0xFF))
					item->Animation.TargetState = 2;

				break;

			case 4:
				head = laraAI.angle;

				if (item->AIBits & GUARD)
				{
					head = AIGuard(creature);

					if (!(GetRandomControl() & 0xFF))
						item->Animation.TargetState = 1;

					break;
				}
				else if ((Targetable(item, &AI) &&
					AI.distance < pow(SECTOR(4), 2) &&
					(realEnemy != LaraItem || creature->HurtByLara) ||
					creature->Mood != MoodType::Bored ||
					!(GetRandomControl() & 0xFF)))
				{
					item->Animation.TargetState = 1;
				}

				break;

			case 2:
				creature->Flags = 0;
				creature->MaxTurn = ANGLE(5.0f);
				head = laraAI.angle;

				if (item->AIBits & GUARD)
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 12;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = 1;
					item->Animation.TargetState = 1;
				}
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
					torsoX = AI.xAngle;
					torsoY = AI.angle;

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
					torsoX = AI.xAngle;
					torsoY = AI.angle;

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
					torsoX = AI.xAngle;
					torsoY = AI.angle;

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
					torsoX = AI.xAngle;
					torsoY = AI.angle;

					if (Targetable(item, &AI) &&
						AI.distance < pow(SECTOR(4), 2) &&
						(realEnemy != LaraItem || creature->HurtByLara))
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
		CreatureJoint(item, 0, torsoY);
		CreatureJoint(item, 1, torsoX);
		CreatureJoint(item, 2, head);

		CreatureAnimation(itemNumber, angle, 0);
	}
}
