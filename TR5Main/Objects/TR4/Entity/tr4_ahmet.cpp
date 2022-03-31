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

using namespace TEN::Effects::Environment;

namespace TEN::Entities::TR4
{
	BITE_INFO AhmetBiteLeft = { 0, 0, 0, 16 };
	BITE_INFO AhmetBiteRight = { 0, 0, 0, 22 };
	BITE_INFO AhmetBiteJaw = { 0, 0, 0, 11 };

	enum AhmetState
	{
		AHMET_STATE_NONE,
		AHMET_STATE_IDLE,
		AHMET_STATE_WALK,
		AHMET_STATE_RUN,
		AHMET_STATE_STAND_DUALATK,
		AHMET_STATE_JUMP_BITE,
		AHMET_STATE_JUMP_DUALATK,
		AHMET_STATE_DIE
	};

	enum AhmetAnim
	{
		AHMET_ANIM_JUMP_ATTACK = 4,

		AHMET_ANIM_JUMP_START = 7,

		AHMET_ANIM_DEATH_ANIM = 10
	};

	#define AHMET_WALK_ANGLE ANGLE(5.0f)
	#define AHMET_RUN_ANGLE ANGLE(8.0f)
	#define AHMET_VIEW_ANGLE ANGLE(45.0f)
	#define AHMET_ENEMY_ANGLE ANGLE(90.0f)
	#define AHMET_AWARE_DISTANCE pow(SECTOR(1), 2)
	#define AHMET_IDLE_RANGE pow(SECTOR(1.25f), 2)
	#define AHMET_RUN_RANGE pow(SECTOR(2.5f), 2)
	#define AHMET_STAND_DUALATK_RANGE pow(682, 2)
	#define AHMET_RIGHT_TOUCH 0xF00000
	#define AHMET_LEFT_TOUCH 0x3C000
	#define AHMET_HAND_DAMAGE 80
	#define AHMET_JAW_DAMAGE 120

	static void AhmetHeavyTriggers(ITEM_INFO* item)
	{
		TestTriggers(item, true);
	}

	static void TriggerAhmetDeathEffect(ITEM_INFO* item)
	{
		// HACK: Using CreatureSpheres here in release mode results in total mess-up
		// of LaraSpheres, which looks in game as ghost Lara fire silhouette.
		// Later both CreatureSpheres and LaraSpheres globals should be eradicated.

		static SPHERE spheres[MAX_SPHERES] = {};

		if (!(Wibble & 7))
		{
			int meshCount = GetSpheres(item, spheres, SPHERES_SPACE_WORLD, Matrix::Identity);
			auto sphere = &spheres[(Wibble / 8) & 1];

			for (int i = meshCount; i > 0; i--, sphere += 2)
				TriggerFireFlame(sphere->x, sphere->y, sphere->z, -1, 1);
		}

		// NOTE: fixed light below the ground with -STEP_L!
		TriggerDynamicLight(item->Pose.Position.x, (item->Pose.Position.y - STEP_SIZE), item->Pose.Position.z, 13, (GetRandomControl() & 0x3F) - 64, (GetRandomControl() & 0x1F) + 96, 0);
		SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Pose, NULL);
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

		AI_INFO laraAI, AI;

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
			if (item->Animation.ActiveState == AHMET_STATE_DIE)
			{
				// dont clear it !
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
				{
					item->Collidable = false; // NOTE: not exist in the original game, avoid wreid collision with lara...
					item->Animation.FrameNumber = (g_Level.Anims[item->Animation.AnimNumber].frameEnd - 1);
				}
			}
			else
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + AHMET_ANIM_DEATH_ANIM;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = AHMET_STATE_DIE;
				item->Animation.TargetState = AHMET_STATE_DIE;
				Lara.InteractedItem = itemNumber;
			}

			TriggerAhmetDeathEffect(item);
		}
		else
		{
			if (item->AIBits & ALL_AIOBJ)
				GetAITarget(creature);

			CreatureAIInfo(item, &AI);

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

			GetCreatureMood(item, &AI, TRUE);
			CreatureMood(item, &AI, TRUE);

			angle = CreatureTurn(item, creature->MaxTurn);
			creature->Enemy = LaraItem;

			if (laraAI.distance < AHMET_AWARE_DISTANCE ||
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
					item->Animation.TargetState = AHMET_STATE_WALK;
					headY = 0;
				}
				else if (creature->Mood == MoodType::Attack && creature->Mood != MoodType::Escape)
				{
					if (AI.bite && AI.distance < AHMET_STAND_DUALATK_RANGE)
						item->Animation.TargetState = AHMET_STATE_STAND_DUALATK;
					else if ((AI.angle >= AHMET_VIEW_ANGLE || AI.angle <= -AHMET_VIEW_ANGLE) || AI.distance >= AHMET_IDLE_RANGE)
					{
						if (item->Animation.RequiredState)
							item->Animation.TargetState = item->Animation.RequiredState;
						else
						{
							if (!AI.ahead || AI.distance >= AHMET_RUN_RANGE)
								item->Animation.TargetState = AHMET_STATE_RUN;
							else
								item->Animation.TargetState = AHMET_STATE_WALK;
						}
					}
					else if (GetRandomControl() & 1)
						item->Animation.TargetState = AHMET_STATE_JUMP_BITE;
					else
						item->Animation.TargetState = AHMET_STATE_JUMP_DUALATK;
				}
				else
				{
					if (Lara.TargetEntity == item || !AI.ahead)
						item->Animation.TargetState = AHMET_STATE_RUN;
					else
						item->Animation.TargetState = AHMET_STATE_IDLE;
				}

				break;

			case AHMET_STATE_WALK:
				creature->MaxTurn = AHMET_WALK_ANGLE;

				if (item->AIBits & PATROL1)
				{
					item->Animation.TargetState = AHMET_STATE_WALK;
					headY = 0;
				}
				else if (AI.bite && AI.distance < AHMET_IDLE_RANGE)
					item->Animation.TargetState = AHMET_STATE_IDLE;
				else if (creature->Mood == MoodType::Escape || AI.distance > AHMET_RUN_RANGE || !AI.ahead || (AI.enemyFacing > -AHMET_ENEMY_ANGLE || AI.enemyFacing < AHMET_ENEMY_ANGLE))
					item->Animation.TargetState = AHMET_STATE_RUN;
				
				break;

			case AHMET_STATE_RUN:
				creature->MaxTurn = AHMET_RUN_ANGLE;
				creature->Flags = 0;

				if (item->AIBits & GUARD || (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Escape) && (Lara.TargetEntity == item && AI.ahead) || (AI.bite && AI.distance < AHMET_IDLE_RANGE))
					item->Animation.TargetState = AHMET_STATE_IDLE;
				else if (AI.ahead && AI.distance < AHMET_RUN_RANGE && (AI.enemyFacing < -AHMET_ENEMY_ANGLE || AI.enemyFacing > AHMET_ENEMY_ANGLE))
					item->Animation.TargetState = AHMET_STATE_WALK;

				break;

			case AHMET_STATE_STAND_DUALATK:
				creature->MaxTurn = 0;

				if (abs(AI.angle) >= 910)
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += 910;
					else
						item->Pose.Orientation.y -= 910;
				}
				else
					item->Pose.Orientation.y += AI.angle;

				if (!(creature->Flags & 1) && item->Animation.FrameNumber > (g_Level.Anims[item->Animation.AnimNumber].frameBase + 7) && (item->TouchBits & AHMET_LEFT_TOUCH))
				{
					CreatureEffect2(item, &AhmetBiteLeft, 10, -1, DoBloodSplat);
					creature->Flags |= 1;

					LaraItem->HitStatus = true;
					LaraItem->HitPoints -= AHMET_HAND_DAMAGE;
				}
				else if (!(creature->Flags & 2) && item->Animation.FrameNumber > (g_Level.Anims[item->Animation.AnimNumber].frameBase + 32) && (item->TouchBits & AHMET_RIGHT_TOUCH))
				{
					CreatureEffect2(item, &AhmetBiteRight, 10, -1, DoBloodSplat);
					creature->Flags |= 2;

					LaraItem->HitStatus = true;
					LaraItem->HitPoints -= AHMET_HAND_DAMAGE;
				}

				break;

			case AHMET_STATE_JUMP_BITE:
				creature->MaxTurn = 0;

				if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + AHMET_ANIM_JUMP_START)
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
					if (!(creature->Flags & 1) && item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + AHMET_ANIM_JUMP_ATTACK)
					{
						if (item->Animation.FrameNumber > (g_Level.Anims[item->Animation.AnimNumber].frameBase + 11) &&
							item->TouchBits & AHMET_LEFT_TOUCH)
						{
							CreatureEffect2(item, &AhmetBiteJaw, 10, -1, DoBloodSplat);
							creature->Flags |= 1;

							LaraItem->HitStatus = true;
							LaraItem->HitPoints -= AHMET_JAW_DAMAGE;
						}
					}
				}

				break;

			case AHMET_STATE_JUMP_DUALATK:
				creature->MaxTurn = 0;

				if (item->Animation.AnimNumber == (Objects[item->ObjectNumber].animIndex + AHMET_ANIM_JUMP_START))
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
						item->TouchBits & AHMET_LEFT_TOUCH)
					{
						CreatureEffect2(item, &AhmetBiteLeft, 10, -1, DoBloodSplat);
						creature->Flags |= 1;

						LaraItem->HitStatus = true;
						LaraItem->HitPoints -= AHMET_HAND_DAMAGE;
					}
					else if (!(creature->Flags & 2) &&
						item->Animation.FrameNumber > (g_Level.Anims[item->Animation.AnimNumber].frameBase + 22) &&
						item->TouchBits & AHMET_RIGHT_TOUCH)
					{
						CreatureEffect2(item, &AhmetBiteRight, 10, -1, DoBloodSplat);
						creature->Flags |= 2;

						LaraItem->HitStatus = true;
						LaraItem->HitPoints -= AHMET_HAND_DAMAGE;
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

		if (item->Animation.ActiveState != 7 || item->Animation.FrameNumber != g_Level.Anims[item->Animation.AnimNumber].frameEnd)
			return false;

		Weather.Flash(255, 64, 0, 0.03f);

		item->Pose.Position.x = (item->ItemFlags[0] * SECTOR(1)) + CLICK(2);
		item->Pose.Position.y = (item->ItemFlags[1] * CLICK(1));
		item->Pose.Position.z = (item->ItemFlags[2] * SECTOR(1)) + CLICK(2);

		auto outsideRoom = IsRoomOutside(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
		if (item->RoomNumber != outsideRoom)
			ItemNewRoom(itemNumber, outsideRoom);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
		item->Animation.TargetState = 1;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = 1;
		item->HitPoints = Objects[item->ObjectNumber].HitPoints;

		AddActiveItem(itemNumber);

		item->Flags &= 0xFE;
		item->AfterDeath = 0;
		item->Status = ITEM_ACTIVE;
		item->Collidable = true;

		EnableBaddieAI(itemNumber, 1);

		item->TriggerFlags = 1;
		return true;
	}
}
