#include "framework.h"
#include "Objects/TR3/Entity/tr3_tribesman.h"

#include "Game/animation.h"
#include "Game/collision/sphere.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Objects/Generic/Traps/dart_emitter.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"

using namespace TEN::Entities::Traps;
using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::TR3
{
	const auto TribesmanAxeBite	  = BiteInfo(Vector3(0.0f, 16.0f, 265.0f), 13);
	const auto TribesmanDartBite1 = BiteInfo(Vector3(0.0f, 0.0f, -200.0f), 13);
	const auto TribesmanDartBite2 = BiteInfo(Vector3(8.0f, 40.0f, -248.0f), 13);
	const vector<int> TribesmanAxeAttackJoints	= { 13 };
	const vector<int> TribesmanDartAttackJoints = { 10, 13 }; // TODO: Check.

	unsigned char TribesmanAxeHit[13][3] =
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 2, 12, 8 },
		{ 8, 9, 32 },
		{ 19, 28, 8 },
		{ 0, 0 ,0 },
		{ 0, 0, 0 },
		{ 7, 14, 8 },
		{ 0, 0, 0 },
		{ 15, 19, 32 }
	};

	// TODO
	enum TribesmanState
	{

	};

	// TODO
	enum TribesmanAnim
	{

	};

	void TribemanAxeControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short head = 0;
		short angle = 0;
		short tilt = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 9)
			{
				if (item->Animation.ActiveState == 1 ||
					item->Animation.ActiveState == 7)
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 21;
				else
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 20;

				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 9;
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, true);

			if (creature->Enemy == LaraItem &&
				creature->HurtByLara && AI.distance > pow(SECTOR(3), 2) &&
				AI.enemyFacing < ANGLE(67.0f) && AI.enemyFacing > -ANGLE(67.0f))
			{
				creature->Mood = MoodType::Escape;
			}

			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			if (AI.ahead)
				head = AI.angle;

			switch (item->Animation.ActiveState)
			{
			case 1:
				creature->MaxTurn = ANGLE(4.0f);
				creature->Flags = 0;

				if (creature->Mood == MoodType::Bored)
				{
					creature->MaxTurn = 0;

					if (TestProbability(0.008f))
						item->Animation.TargetState = 2;
				}
				else if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead && !item->HitStatus)
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 3;
				}
				else if (item->ItemFlags[0])
				{
					item->ItemFlags[0] = 0;
					item->Animation.TargetState = 11;
				}
				else if (AI.ahead && AI.distance < pow(682, 2))
					item->Animation.TargetState = 7;
				else if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
				{
					if (TestProbability(0.5f))
						item->Animation.TargetState = 2;
					else
						item->Animation.TargetState = 7;
				}
				else if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
					item->Animation.TargetState = 2;
				else
					item->Animation.TargetState = 3;

				break;

			case 11:
				creature->MaxTurn = ANGLE(4.0f);
				creature->Flags = 0;

				if (creature->Mood == MoodType::Bored)
				{
					creature->MaxTurn = 0;
					if (TestProbability(0.008f))
						item->Animation.TargetState = 2;
				}
				else if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead && !item->HitStatus)
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 3;
				}
				else if (AI.ahead && AI.distance < pow(682, 2))
				{
					if (TestProbability(0.0625f))
						item->Animation.TargetState = 5;
					else
						item->Animation.TargetState = 8;
				}
				else if (AI.distance < pow(SECTOR(2), 2))
					item->Animation.TargetState = 2;
				else
					item->Animation.TargetState = 3;

				break;

			case 2:
				creature->MaxTurn = ANGLE(9.0f);
				creature->Flags = 0;
				tilt = angle / 8;

				if (creature->Mood == MoodType::Bored)
				{
					creature->MaxTurn /= 4;
					if (TestProbability(0.008f))
					{
						if (TestProbability(0.25f))
							item->Animation.TargetState = 1;
						else
							item->Animation.TargetState = 11;
					}
				}
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 3;
				else if (AI.ahead && AI.distance < pow(682, 2))
				{
					if (TestProbability(0.25f))
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 11;
				}
				else if (AI.distance > pow(SECTOR(2), 2))
					item->Animation.TargetState = 3;

				break;

			case 3:
				tilt = angle / 4;
				creature->MaxTurn = ANGLE(6.0f);
				creature->Flags = 0;

				if (creature->Mood == MoodType::Bored)
				{
					creature->MaxTurn /= 4;
					if (TestProbability(0.008f))
					{
						if (TestProbability(0.5f))
							item->Animation.TargetState = 1;
						else
							item->Animation.TargetState = 11;
					}
				}
				else if (creature->Mood == MoodType::Escape && Lara.TargetEntity != item && AI.ahead)
					item->Animation.TargetState = 11;
				else if (AI.bite || AI.distance < pow(SECTOR(2), 2))
				{
					if (TestProbability(0.5f))
						item->Animation.TargetState = 12;
					else if (TestProbability(0.25f))
						item->Animation.TargetState = 10;
					else
						item->Animation.TargetState = 2;
				}

				break;

			case 8:
				creature->MaxTurn = ANGLE(4.0f);
				if (AI.bite || AI.distance < pow(682, 2))
					item->Animation.TargetState = 6;
				else
					item->Animation.TargetState = 11;

				break;

			case 5:
			case 6:
			case 7:
			case 10:
			case 12:
				item->ItemFlags[0] = 1;
				creature->MaxTurn = ANGLE(4.0f);
				creature->Flags = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;

				if (creature->Enemy->IsLara())
				{
					if (item->TestBits(JointBitType::Touch, TribesmanAxeAttackJoints) &&
						creature->Flags >= TribesmanAxeHit[item->Animation.ActiveState][0] &&
						creature->Flags <= TribesmanAxeHit[item->Animation.ActiveState][1])
					{
						for (int i = 0; i < TribesmanAxeHit[item->Animation.ActiveState][2]; i += 8)
							CreatureEffect(item, TribesmanAxeBite, DoBloodSplat);

						DoDamage(creature->Enemy, TribesmanAxeHit[item->Animation.ActiveState][2]);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
					}
				}
				else
				{
					if (creature->Enemy)
					{
						auto direction = creature->Enemy->Pose.Position - item->Pose.Position;
						if (abs(direction.x) < SECTOR(0.5f) &&
							abs(direction.y) < SECTOR(0.5f) &&
							abs(direction.z) < SECTOR(0.5f) &&
							creature->Flags >= TribesmanAxeHit[item->Animation.ActiveState][0] &&
							creature->Flags <= TribesmanAxeHit[item->Animation.ActiveState][1])
						{
							DoDamage(creature->Enemy, 2);
							CreatureEffect(item, TribesmanAxeBite, DoBloodSplat);
							SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
						}
					}
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, head >> 1);
		CreatureJoint(item, 1, head >> 1);

		CreatureAnimation(itemNumber, angle, 0);
	}

	static void TribesmanShotDart(ItemInfo* item)
	{
		short dartItemNumber = CreateItem();
		if (dartItemNumber != NO_ITEM)
		{
			auto* dartItem = &g_Level.Items[dartItemNumber];
			dartItem->ObjectNumber = ID_DARTS;
			dartItem->RoomNumber = item->RoomNumber;

			auto pos1 = Vector3Int(TribesmanDartBite2.x, TribesmanDartBite2.y, TribesmanDartBite2.z);
			GetJointAbsPosition(item, &pos1, TribesmanDartBite2.meshNum);

			auto pos2 = pos1;
			pos2.z *= 2;
			GetJointAbsPosition(item, &pos2, TribesmanDartBite2.meshNum);

			auto angles = GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z);

			dartItem->Pose.Position = pos1;

			InitialiseItem(dartItemNumber);

			dartItem->Pose.Orientation = angles;
			dartItem->Animation.Velocity = CLICK(1);

			AddActiveItem(dartItemNumber);

			dartItem->Status = ITEM_ACTIVE;

			pos1 = Vector3Int(TribesmanDartBite2.x, TribesmanDartBite2.y, TribesmanDartBite2.z + 96);
			GetJointAbsPosition(item, &pos1, TribesmanDartBite2.meshNum);

			TriggerDartSmoke(pos1.x, pos1.y, pos1.z, 0, 0, 1);
			TriggerDartSmoke(pos1.x, pos1.y, pos1.z, 0, 0, 1);
		}
	}

	void TribemanDartsControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		short headX = 0;
		short headY = 0;
		short torsoX = 0;
		short torsoY = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 9)
			{
				if (item->Animation.ActiveState == 1 || item->Animation.ActiveState == 4)
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 21;
				else
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 20;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;

				item->Animation.ActiveState = 9;
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, (AI.zoneNumber == AI.enemyZone ? true : false));

			if (item->HitStatus && Lara.PoisonPotency && creature->Mood == MoodType::Bored)
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, false);

			angle = CreatureTurn(item, creature->Mood == MoodType::Bored ? ANGLE(2.0f) : creature->MaxTurn);
			if (AI.ahead)
			{
				headY = AI.angle / 2;
				torsoY = AI.angle / 2;
			}

			if (item->HitStatus ||
				(creature->Enemy == LaraItem && (AI.distance < pow(SECTOR(1), 2) ||
					TargetVisible(item, &AI)) && (abs(LaraItem->Pose.Position.y - item->Pose.Position.y) < SECTOR(2))))
			{
				AlertAllGuards(itemNumber);
			}

			switch (item->Animation.ActiveState)
			{
			case 1:
				creature->MaxTurn = ANGLE(2.0f);
				creature->Flags &= 0x0FFF;

				if (AI.ahead)
				{
					torsoY = AI.angle;
					torsoX = AI.xAngle / 2;
				}

				if (item->AIBits & GUARD)
				{
					creature->MaxTurn = 0;
					headY = AIGuard(creature);
					torsoX = 0;
					torsoY = 0;

					if (!(GetRandomControl() & 0xFF))
						item->Animation.TargetState = 11;

					break;
				}
				else if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead && !item->HitStatus)
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 3;
				}
				else if (AI.bite && AI.distance < pow(SECTOR(0.5f), 2))
					item->Animation.TargetState = 11;
				else if (AI.bite && AI.distance < pow(SECTOR(2), 2))
					item->Animation.TargetState = 2;
				else if (Targetable(item, &AI) && AI.distance < pow(MAX_VISIBILITY_DISTANCE, 2))
					item->Animation.TargetState = 4;
				else if (creature->Mood == MoodType::Bored)
				{
					if (TestProbability(0.015f))
						item->Animation.TargetState = 2;
					else
						break;
				}
				else
					item->Animation.TargetState = 3;

				break;

			case 11:
				creature->MaxTurn = ANGLE(2.0f);
				creature->Flags &= 0x0FFF;

				if (item->AIBits & GUARD)
				{
					creature->MaxTurn = 0;
					headY = AIGuard(creature);
					torsoX = 0;
					torsoY = 0;

					if (!(GetRandomControl() & 0xFF))
						item->Animation.TargetState = 1;

					break;
				}
				else if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead && !item->HitStatus)
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 3;
				}
				else if (AI.bite && AI.distance < pow(SECTOR(0.5f), 2))
					item->Animation.TargetState = 6;
				else if (AI.bite && AI.distance < pow(SECTOR(2), 2))
					item->Animation.TargetState = 2;
				else if (Targetable(item, &AI) && AI.distance < pow(MAX_VISIBILITY_DISTANCE, 2))
					item->Animation.TargetState = 1;
				else if (creature->Mood == MoodType::Bored && TestProbability(0.015f))
					item->Animation.TargetState = 2;
				else
					item->Animation.TargetState = 3;

				break;

			case 2:
				creature->MaxTurn = ANGLE(9.0f);

				if (AI.bite && AI.distance < pow(SECTOR(0.5f), 2))
					item->Animation.TargetState = 11;
				else if (AI.bite && AI.distance < pow(SECTOR(2), 2))
					item->Animation.TargetState = 2;
				else if (Targetable(item, &AI) && AI.distance < pow(MAX_VISIBILITY_DISTANCE, 2))
					item->Animation.TargetState = 1;
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 3;
				else if (creature->Mood == MoodType::Bored)
				{
					if (TestProbability(0.985f))
						item->Animation.TargetState = 2;
					else if (TestProbability(0.985f))
						item->Animation.TargetState = 11;
					else
						item->Animation.TargetState = 1;
				}
				else if (AI.distance > pow(SECTOR(2), 2))
					item->Animation.TargetState = 3;

				break;

			case 3:
				creature->MaxTurn = ANGLE(6.0f);
				creature->Flags &= 0x0FFF;
				tilt = angle / 4;

				if (AI.bite && AI.distance < pow(SECTOR(0.5f), 2))
					item->Animation.TargetState = 11;
				else if (Targetable(item, &AI) && AI.distance < pow(MAX_VISIBILITY_DISTANCE, 2), 2)
					item->Animation.TargetState = 1;

				if (item->AIBits & GUARD)
					item->Animation.TargetState = 11;
				else if (creature->Mood == MoodType::Escape && Lara.TargetEntity != item && AI.ahead)
					item->Animation.TargetState = 11;
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = 1;

				break;

			case 8:
				if (!AI.bite || AI.distance > pow(SECTOR(0.5f), 2))
					item->Animation.TargetState = 11;
				else
					item->Animation.TargetState = 6;

				break;

			case 4:
				creature->MaxTurn = 0;

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (abs(AI.angle) < ANGLE(2.0f))
					item->Pose.Orientation.y += AI.angle;
				else if (AI.angle < 0)
					item->Pose.Orientation.y -= ANGLE(2.0f);
				else
					item->Pose.Orientation.y += ANGLE(2.0f);

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 15)
				{
					TribesmanShotDart(item);
					item->Animation.TargetState = 1;
				}

				break;

			case 6:
				if (creature->Enemy->IsLara())
				{
					if (!(creature->Flags & 0xf000) && item->TestBits(JointBitType::Touch, TribesmanDartAttackJoints))
					{
						DoDamage(creature->Enemy, 100);
						CreatureEffect(item, TribesmanDartBite1, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
						creature->Flags |= 0x1000;
					}
				}
				else
				{
					if (!(creature->Flags & 0xf000) && creature->Enemy)
					{
						if (abs(creature->Enemy->Pose.Position.x - item->Pose.Position.x) < pow(SECTOR(0.5f), 2) &&
							abs(creature->Enemy->Pose.Position.y - item->Pose.Position.y) < pow(SECTOR(0.5f), 2) &&
							abs(creature->Enemy->Pose.Position.z - item->Pose.Position.z) < pow(SECTOR(0.5f), 2))
						{
							creature->Flags |= 0x1000;

							DoDamage(creature->Enemy, 5);
							SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
						}
					}
				}

				break;
			}
		}

		CreatureTilt(item, tilt);

		headY -= torsoY;

		CreatureJoint(item, 0, torsoY);
		CreatureJoint(item, 1, torsoX);
		CreatureJoint(item, 2, headY);
		CreatureJoint(item, 3, headX);

		CreatureAnimation(itemNumber, angle, 0);
	}
}
