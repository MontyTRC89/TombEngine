#include "framework.h"
#include "tr4_ahmet.h"
#include "Game/control/control.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/effects/weather.h"
#include "Sound/sound.h"
#include "Specific/setup.h"
#include "Game/control/box.h"
#include "Specific/level.h"
#include "Game/misc.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Game/items.h"
#include "Game/control/lot.h"
#include "Game/itemdata/creature_info.h"

using std::vector;
using namespace TEN::Effects::Environment;

namespace TEN::Entities::TR4
{
	BITE_INFO AhmetBiteLeft = { 0, 0, 0, 16 };
	BITE_INFO AhmetBiteRight = { 0, 0, 0, 22 };
	BITE_INFO AhmetBiteJaw = { 0, 0, 0, 11 };
	const vector<int> AhmetSwipeAttackLeftJoints = { 14, 15, 16, 17 };
	const vector<int> AhmetSwipeAttackRightJoints = { 20, 21, 22, 23 };

	constexpr auto AHMET_SWIPE_ATTACK_DAMAGE = 80;
	constexpr auto AHMET_BITE_ATTACK_DAMAGE = 120;

	constexpr auto AHMET_ATTACK_RANGE = SECTOR(0.67f);
	constexpr auto AHMET_AWARE_RANGE = SECTOR(1);
	constexpr auto AHMET_IDLE_RANGE = SECTOR(1.25f);
	constexpr auto AHMET_RUN_RANGE = SECTOR(2.5f);

	#define AHMET_WALK_FORWARD_TURN_ANGLE ANGLE(5.0f)
	#define AHMET_RUN_FORWARD_TURN_ANGLE ANGLE(8.0f)
	#define AHMET_VIEW_ANGLE ANGLE(45.0f)
	#define AHMET_ENEMY_ANGLE ANGLE(90.0f)

	enum AhmetState
	{
		AHMET_STATE_NONE = 0,
		AHMET_STATE_IDLE = 1,
		AHMET_STATE_WALK_FORWARD = 2,
		AHMET_STATE_RUN_FORWARD = 3,
		AHMET_STATE_SWIPE_ATTACK = 4,
		AHMET_STATE_JUMP_BITE_ATTACK = 5,
		AHMET_STATE_JUMP_SWIPE_ATTACK = 6,
		AHMET_STATE_DEATH = 7
	};

	enum AhmetAnim
	{
		AHMET_ANIM_IDLE = 0,
		AHMET_ANIM_RUN_FORWARD = 1,
		AHMET_ANIM_SWIPE_ATTACK = 2,
		AHMET_ANIM_JUMP_BITE_ATTACK_START = 3,
		AHMET_ANIM_JUMP_BITE_ATTACK_CONTINUE = 4,
		AHMET_ANIM_JUMP_BITE_ATTACK_END = 5,
		AHMET_ANIM_WALK_FORWARD = 6,
		AHMET_ANIM_JUMP_SWIPE_ATTACK_START = 7,
		AHMET_ANIM_JUMP_SWIPE_ATTACK_CONTINUE = 8,
		AHMET_ANIM_JUMP_SWIPE_ATTACK_END = 9,
		AHMET_ANIM_DEATH = 10,
		AHMET_ANIM_IDLE_TO_WALK_FORWARD = 11,
		AHMET_ANIM_WALK_FORWARD_TO_IDLE = 12,
		AHMET_ANIM_IDLE_TO_RUN_FORWARD = 13,
		AHMET_ANIM_RUN_FORWARD_TO_IDLE = 14,
	};

	// TODO
	enum AhmetFlags
	{

	};

	static void AhmetHeavyTriggers(ItemInfo* item)
	{
		TestTriggers(item, true);
	}

	static void TriggerAhmetDeathEffect(ItemInfo* item)
	{
		// HACK: Using CreatureSpheres here in release mode results in total mess-up
		// of LaraSpheres, which in-game appears as a ghostly Lara fire silhouette.
		// Later, both CreatureSpheres and LaraSpheres globals should be eradicated.

		static SPHERE spheres[MAX_SPHERES] = {};

		if (!(Wibble & 7))
		{
			int meshCount = GetSpheres(item, spheres, SPHERES_SPACE_WORLD, Matrix::Identity);
			auto sphere = &spheres[(Wibble / 8) & 1];

			for (int i = meshCount; i > 0; i--, sphere += 2)
				TriggerFireFlame(sphere->x, sphere->y, sphere->z, -1, 1);
		}

		TriggerDynamicLight(
			item->Pose.Position.x,
			item->Pose.Position.y - CLICK(1),
			item->Pose.Position.z,
			13,
			(GetRandomControl() & 0x3F) - 64,
			(GetRandomControl() & 0x1F) + 96,
			0);

		SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Pose);
	}

	void InitialiseAhmet(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = AHMET_STATE_IDLE;
		item->Animation.ActiveState = AHMET_STATE_IDLE;
		item->ItemFlags[0] = item->Pose.Position.x / SECTOR(1);
		item->ItemFlags[1] = item->Pose.Position.y * 4 / SECTOR(1);
		item->ItemFlags[2] = item->Pose.Position.z / SECTOR(1);
	}

	void AhmetControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		if (item->TriggerFlags == 1)
		{
			item->TriggerFlags = 0;
			return;
		}

		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short headY = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState == AHMET_STATE_DEATH)
			{
				// Don't clear.
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
				{
					item->Collidable = false;
					item->Animation.FrameNumber = (g_Level.Anims[item->Animation.AnimNumber].frameEnd - 1);
				}
			}
			else
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + AHMET_ANIM_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = AHMET_STATE_DEATH;
				item->Animation.TargetState = AHMET_STATE_DEATH;
				Lara.InteractedItem = itemNumber;
			}

			TriggerAhmetDeathEffect(item);
		}
		else
		{
			if (item->AIBits & ALL_AIOBJ)
				GetAITarget(creature);

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			AI_INFO laraAI;
			if (creature->Enemy == LaraItem)
			{
				laraAI.angle = AI.angle;
				laraAI.distance = AI.distance;
			}
			else
			{
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				int angle = phd_atan(dx, dz);
				laraAI.angle = angle - item->Pose.Orientation.y;
				laraAI.distance = pow(dx, 2) + pow(dz, 2);
			}

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);
			creature->Enemy = LaraItem;

			if (laraAI.distance < pow(AHMET_AWARE_RANGE, 2) ||
				item->HitStatus ||
				TargetVisible(item, &laraAI))
			{
				AlertAllGuards(itemNumber);
			}

			if (AI.ahead)
				headY = AI.angle;

			switch (item->Animation.ActiveState)
			{
			case AHMET_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (item->AIBits & GUARD)
				{
					item->Animation.TargetState = AHMET_STATE_IDLE;
					headY = AIGuard(creature);
				}
				else if (item->AIBits & PATROL1)
				{
					item->Animation.TargetState = AHMET_STATE_WALK_FORWARD;
					headY = 0;
				}
				else if (creature->Mood == MoodType::Attack && creature->Mood != MoodType::Escape)
				{
					if (AI.bite && AI.distance < pow(AHMET_ATTACK_RANGE, 2))
						item->Animation.TargetState = AHMET_STATE_SWIPE_ATTACK;
					else if ((AI.angle >= AHMET_VIEW_ANGLE || AI.angle <= -AHMET_VIEW_ANGLE) || AI.distance >= pow(AHMET_IDLE_RANGE, 2))
					{
						if (item->Animation.RequiredState)
							item->Animation.TargetState = item->Animation.RequiredState;
						else
						{
							if (!AI.ahead || AI.distance >= pow(AHMET_RUN_RANGE, 2))
								item->Animation.TargetState = AHMET_STATE_RUN_FORWARD;
							else
								item->Animation.TargetState = AHMET_STATE_WALK_FORWARD;
						}
					}
					else if (GetRandomControl() & 1)
						item->Animation.TargetState = AHMET_STATE_JUMP_BITE_ATTACK;
					else
						item->Animation.TargetState = AHMET_STATE_JUMP_SWIPE_ATTACK;
				}
				else
				{
					if (Lara.TargetEntity == item || !AI.ahead)
						item->Animation.TargetState = AHMET_STATE_RUN_FORWARD;
					else
						item->Animation.TargetState = AHMET_STATE_IDLE;
				}

				break;

			case AHMET_STATE_WALK_FORWARD:
				creature->MaxTurn = AHMET_WALK_FORWARD_TURN_ANGLE;

				if (item->AIBits & PATROL1)
				{
					item->Animation.TargetState = AHMET_STATE_WALK_FORWARD;
					headY = 0;
				}
				else if (AI.bite && AI.distance < pow(AHMET_IDLE_RANGE, 2))
					item->Animation.TargetState = AHMET_STATE_IDLE;
				else if (creature->Mood == MoodType::Escape || AI.distance > pow(AHMET_RUN_RANGE, 2) || !AI.ahead || (AI.enemyFacing > -AHMET_ENEMY_ANGLE || AI.enemyFacing < AHMET_ENEMY_ANGLE))
					item->Animation.TargetState = AHMET_STATE_RUN_FORWARD;
				
				break;

			case AHMET_STATE_RUN_FORWARD:
				creature->MaxTurn = AHMET_RUN_FORWARD_TURN_ANGLE;
				creature->Flags = 0;

				if (item->AIBits & GUARD ||
					((creature->Mood == MoodType::Bored || creature->Mood == MoodType::Escape) &&
						(Lara.TargetEntity == item && AI.ahead)) ||
					(AI.bite && AI.distance < pow(AHMET_IDLE_RANGE, 2)))
				{
					item->Animation.TargetState = AHMET_STATE_IDLE;
				}
				else if (AI.ahead && AI.distance < pow(AHMET_RUN_RANGE, 2) &&
					(AI.enemyFacing < -AHMET_ENEMY_ANGLE || AI.enemyFacing > AHMET_ENEMY_ANGLE))
				{
					item->Animation.TargetState = AHMET_STATE_WALK_FORWARD;
				}

				break;

			case AHMET_STATE_SWIPE_ATTACK:
				creature->MaxTurn = 0;

				if (abs(AI.angle) >= 910)
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += ANGLE(5.0f);
					else
						item->Pose.Orientation.y -= ANGLE(5.0f);
				}
				else
					item->Pose.Orientation.y += AI.angle;

				if (!(creature->Flags & 1) && item->Animation.FrameNumber > (g_Level.Anims[item->Animation.AnimNumber].frameBase + 7) &&
					item->TestBits(JointBitType::Touch, AhmetSwipeAttackLeftJoints))
				{
					CreatureEffect2(item, &AhmetBiteLeft, 10, -1, DoBloodSplat);
					DoDamage(creature->Enemy, AHMET_SWIPE_ATTACK_DAMAGE);
					creature->Flags |= 1;
				}
				else if (!(creature->Flags & 2) && item->Animation.FrameNumber > (g_Level.Anims[item->Animation.AnimNumber].frameBase + 32) &&
					item->TestBits(JointBitType::Touch, AhmetSwipeAttackRightJoints))
				{
					CreatureEffect2(item, &AhmetBiteRight, 10, -1, DoBloodSplat);
					DoDamage(creature->Enemy, AHMET_SWIPE_ATTACK_DAMAGE);
					creature->Flags |= 2;
				}

				break;

			case AHMET_STATE_JUMP_BITE_ATTACK:
				creature->MaxTurn = 0;

				if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + AHMET_ANIM_JUMP_SWIPE_ATTACK_START)
				{
					if (abs(AI.angle) >= ANGLE(5.0f))
					{
						if (AI.angle >= 0)
							item->Pose.Orientation.y += ANGLE(5.0f);
						else
							item->Pose.Orientation.y -= ANGLE(5.0f);
					}
					else
						item->Pose.Orientation.y += AI.angle;
				}
				else
				{
					if (!(creature->Flags & 1) && item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + AHMET_ANIM_JUMP_BITE_ATTACK_CONTINUE)
					{
						if (item->Animation.FrameNumber > (g_Level.Anims[item->Animation.AnimNumber].frameBase + 11) &&
							item->TestBits(JointBitType::Touch, AhmetSwipeAttackLeftJoints))
						{
							CreatureEffect2(item, &AhmetBiteJaw, 10, -1, DoBloodSplat);
							DoDamage(creature->Enemy, AHMET_BITE_ATTACK_DAMAGE);
							creature->Flags |= 1;
						}
					}
				}

				break;

			case AHMET_STATE_JUMP_SWIPE_ATTACK:
				creature->MaxTurn = 0;

				if (item->Animation.AnimNumber == (Objects[item->ObjectNumber].animIndex + AHMET_ANIM_JUMP_SWIPE_ATTACK_START))
				{
					if (abs(AI.angle) >= ANGLE(5.0f))
					{
						if (AI.angle >= 0)
							item->Pose.Orientation.y += ANGLE(5.0f);
						else
							item->Pose.Orientation.y -= ANGLE(5.0f);
					}
					else
						item->Pose.Orientation.y += AI.angle;
				}
				else
				{
					if (!(creature->Flags & 1) &&
						item->Animation.FrameNumber > (g_Level.Anims[item->Animation.AnimNumber].frameBase + 14) &&
						item->TestBits(JointBitType::Touch, AhmetSwipeAttackLeftJoints))
					{
						CreatureEffect2(item, &AhmetBiteLeft, 10, -1, DoBloodSplat);
						DoDamage(creature->Enemy, AHMET_SWIPE_ATTACK_DAMAGE);
						creature->Flags |= 1;
					}
					else if (!(creature->Flags & 2) &&
						item->Animation.FrameNumber > (g_Level.Anims[item->Animation.AnimNumber].frameBase + 22) &&
						item->TestBits(JointBitType::Touch, AhmetSwipeAttackRightJoints))
					{
						CreatureEffect2(item, &AhmetBiteRight, 10, -1, DoBloodSplat);
						DoDamage(creature->Enemy, AHMET_SWIPE_ATTACK_DAMAGE);
						creature->Flags |= 2;
					}
				}

				break;
			}
		}

		CreatureTilt(item, 0);
		CreatureJoint(item, 0, headY);
		AhmetHeavyTriggers(item);
		CreatureAnimation(itemNumber, angle, 0);
	}

	bool RespawnAhmet(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->Animation.ActiveState != AHMET_STATE_DEATH || item->Animation.FrameNumber != g_Level.Anims[item->Animation.AnimNumber].frameEnd)
			return false;

		Weather.Flash(255, 64, 0, 0.03f);

		item->Pose.Position.x = (item->ItemFlags[0] * SECTOR(1)) + CLICK(2);
		item->Pose.Position.y = (item->ItemFlags[1] * CLICK(1));
		item->Pose.Position.z = (item->ItemFlags[2] * SECTOR(1)) + CLICK(2);

		auto outsideRoom = IsRoomOutside(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
		if (item->RoomNumber != outsideRoom)
			ItemNewRoom(itemNumber, outsideRoom);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
		item->Animation.TargetState = AHMET_STATE_IDLE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = AHMET_STATE_IDLE;
		item->HitPoints = Objects[item->ObjectNumber].HitPoints;

		AddActiveItem(itemNumber);

		item->Flags &= 0xFE;
		item->AfterDeath = 0;
		item->Status = ITEM_ACTIVE;
		item->Collidable = true;

		EnableEntityAI(itemNumber, true);

		item->TriggerFlags = 1;
		return true;
	}
}
