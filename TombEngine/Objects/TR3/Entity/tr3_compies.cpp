#include "framework.h"
#include "Objects/TR3/Entity/tr3_compies.h"

#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto COMPSOGNATHUS_ATTACK_DAMAGE = 90;
	constexpr auto COMPSOGNATHUS_RUN_TURN = ANGLE(10);
	constexpr auto COMPSOGNATHUS_STOP_TURN = ANGLE(3);

	constexpr auto COMPSOGNATHUS_UPSET_SPEED = 15;

	constexpr auto COMPSOGNATHUS_ATTACK_RANGE = SQUARE(BLOCK(0.4f));
	constexpr auto COMPSOGNATHUS_SCARED_RANGE = SQUARE(BLOCK(5));

	constexpr auto COMPSOGNATHUS_ATTACK_ANGLE = 0x3000;
	constexpr auto COMPSOGNATHUS_JUMP_ATTACK_CHANCE = 0x1000;
	constexpr auto COMPSOGNATHUS_ATTACK_CHANCE = 31;

	constexpr auto COMPSOGNATHUS_HIT_FLAG = 1;

	const auto CompyBite = BiteInfo(Vector3(0.0f, 0.0f, 0.0f), 2);
	const auto CompyAttackJoints = std::vector<unsigned int>{ 1, 2 };
	
	enum CompyState
	{
		COMPSOGNATHUS_STATE_IDLE = 0,
		COMPSOGNATHUS_STATE_RUN = 1,
		COMPSOGNATHUS_STATE_JUMP_ATTACK = 2,
		COMPSOGNATHUS_STATE_ATTACK = 3,
	};

	enum  CompyAnim
	{
		COMPSOGNATHUS_ANIM_IDLE = 0,
		COMPSOGNATHUS_ANIM_RUN = 1,
		COMPSOGNATHUS_ANIM_PREPARE_JUMP_ATTACK = 2,
		COMPSOGNATHUS_ANIM_JUMP_ATTACK = 3,
		COMPSOGNATHUS_ANIM_JUMP_ATTACK_LAND = 4,
		COMPSOGNATHUS_ANIM_ATTACK = 5,
		COMPSOGNATHUS_ANIM_DIE = 6,
		COMPSOGNATHUS_ANIM_IDLE_TO_RUN = 7,
		COMPSOGNATHUS_ANIM_RUN_TO_IDLE = 8,
	};

	enum CompyTarget
	{	
		ATTACK_CADAVER = 0,
		ATTACK_LARA = 1
	};

	void InitialiseCompsognathus(short itemNumber)
	{	
		auto& item = g_Level.Items[itemNumber];
		InitialiseCreature(itemNumber);

		//set friendly
		item.ItemFlags[1] = ATTACK_CADAVER;		
	}

	void CompsognathusControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		int angle, head, neck, tilt, random, RoomNumber, target;
		Vector3 cadaverCoordinates;
		
		head = neck = angle = tilt = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != COMPSOGNATHUS_STATE_IDLE)
			{
				SetAnimation(item, COMPSOGNATHUS_ANIM_DIE);
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			if (cadaverCoordinates == Vector3::Zero)
			{
				float minDistance = INFINITY;

				int i;
				ItemInfo* targetItem;
				for (i = 0, targetItem = &g_Level.Items[0]; i < g_Level.NumItems; i++, targetItem++)
				{
					if (targetItem->ObjectNumber == NO_ITEM || targetItem->Index == itemNumber || targetItem->RoomNumber == NO_ROOM)
						continue;

					if (SameZone(creature, targetItem) )
					{
						int distance = Vector3i::Distance(item->Pose.Position, targetItem->Pose.Position);

						if (distance < minDistance && targetItem->Effect.Type == EffectType::Cadaver)
						{
							cadaverCoordinates = Vector3(targetItem->Pose.Position.x, targetItem->Pose.Position.y, targetItem->Pose.Position.z);
							minDistance = distance;
							item->ItemFlags[1] = ATTACK_CADAVER;							
						}
					}
				}
			}

			creature->Enemy = LaraItem;

			target = item->ItemFlags[1];

			if (creature->HurtByLara)
				AlertAllGuards(itemNumber);

			if (creature->Mood == MoodType::Bored && item->ItemFlags[1] == ATTACK_CADAVER)
			{
				int dx = cadaverCoordinates.x - item->Pose.Position.x;
				int dz = cadaverCoordinates.z - item->Pose.Position.z;
				AI.distance = (dx * dx) + (dz * dz);
				AI.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
				AI.ahead = (AI.angle > -FRONT_ARC && AI.angle < FRONT_ARC);
			}

			random = ((itemNumber & 0x7) * 0x200) - 0x700;
		
			if (!item->ItemFlags[3] 
				&& !item->ItemFlags[1] == ATTACK_LARA
				&& ((AI.enemyFacing < COMPSOGNATHUS_ATTACK_ANGLE && AI.enemyFacing > -COMPSOGNATHUS_ATTACK_ANGLE && LaraItem->Animation.Velocity.z > COMPSOGNATHUS_UPSET_SPEED)
				|| LaraItem->Animation.ActiveState == LS_ROLL_180_FORWARD 
				|| item->HitStatus))
			{
				if (AI.distance > COMPSOGNATHUS_ATTACK_RANGE * 4)
				{
					item->ItemFlags[0] = (random + 0x700) >> 7;
					item->ItemFlags[3] = 2 * LaraItem->Animation.Velocity.z;		//Scared for less time the more Compys there are - adjust this when we've got lots of them
				}
			}
			else if (item->ItemFlags[3])
			{
				if (item->ItemFlags[0] > 0)
				{
					item->ItemFlags[0]--;
				}
				else
				{
					creature->Mood = MoodType::Escape;					
					item->ItemFlags[3]--;
				}

				if (GetRandomControl() < COMPSOGNATHUS_ATTACK_CHANCE && item->Timer > 180  )
				{
						item->ItemFlags[1] = ATTACK_LARA;
				}
			}
			else if (AI.zoneNumber != AI.enemyZone)
			{
				creature->Mood = MoodType::Bored;						
			}
			else
				creature->Mood = MoodType::Attack;	

			switch (creature->Mood)
			{
			case MoodType::Attack:
				
				creature->Target.x = creature->Enemy->Pose.Position.x + (SECTOR(1) * phd_sin(item->Pose.Orientation.y + random)) ;
				creature->Target.y = creature->Enemy->Pose.Position.y;
				creature->Target.z = creature->Enemy->Pose.Position.z +(SECTOR(1) * phd_cos(item->Pose.Orientation.y + random));

				break;

			case MoodType::Stalk:
			case MoodType::Escape:	//Turn & Run

				creature->Target.x = item->Pose.Position.x + (SECTOR(1) * phd_sin(AI.angle + 0x8000 + random));
				creature->Target.z = item->Pose.Position.z + (SECTOR(1) * phd_cos(AI.angle + 0x8000 + random));
				RoomNumber = item->RoomNumber;

				if (AI.distance > COMPSOGNATHUS_SCARED_RANGE || !item->ItemFlags[3])
				{
					creature->Mood = MoodType::Bored;
					item->ItemFlags[0] = item->ItemFlags[3];										
				}

				break;

			case MoodType::Bored:
				
				if (cadaverCoordinates != Vector3::Zero && item->ItemFlags[1] == ATTACK_CADAVER)
				{
				creature->Target.x = cadaverCoordinates.x;
				creature->Target.z = cadaverCoordinates.z;
				}
				else
				{
					cadaverCoordinates = Vector3::Zero;
				}

				break;
			}

			if (item->AIBits)
				GetAITarget(creature);

			angle = CreatureTurn(item, creature->MaxTurn);

			if (AI.ahead)
				head = AI.angle;

			neck = -(AI.angle / 4);

			if (item->Timer < 250)
			item->Timer++;

			if (item->HitStatus && item->Timer > 200 && GetRandomControl() < COMPSOGNATHUS_ATTACK_CHANCE * 100 || (creature->Alerted && AI.zoneNumber == AI.enemyZone))
			{
					item->ItemFlags[1] = ATTACK_LARA;					
			}

			switch (item->Animation.ActiveState)
			{
			case COMPSOGNATHUS_STATE_IDLE:
				creature->MaxTurn = COMPSOGNATHUS_STOP_TURN;
				creature->Flags &= ~COMPSOGNATHUS_HIT_FLAG;
				if (creature->Mood == MoodType::Attack)
				{					
					if (AI.ahead && AI.distance < COMPSOGNATHUS_ATTACK_RANGE * 4)
					{
						if (item->ItemFlags[1] == ATTACK_LARA)
						{
							if (GetRandomControl() < 0x4000)
								item->Animation.TargetState = COMPSOGNATHUS_STATE_ATTACK;
							else
								item->Animation.TargetState = COMPSOGNATHUS_STATE_JUMP_ATTACK;
						}
						else
							item->Animation.TargetState = COMPSOGNATHUS_STATE_IDLE;
					}
					else if (AI.distance > COMPSOGNATHUS_ATTACK_RANGE * (9 - 4 * target))
						item->Animation.TargetState = COMPSOGNATHUS_STATE_RUN;
				}
				else if (creature->Mood == MoodType::Bored)
				{					
					if (AI.ahead && AI.distance < (COMPSOGNATHUS_ATTACK_RANGE * 3 ) && item->ItemFlags[1] == ATTACK_CADAVER)
					{
						if (GetRandomControl() < 0x4000)
							item->Animation.TargetState = COMPSOGNATHUS_STATE_ATTACK;
						else
							item->Animation.TargetState = COMPSOGNATHUS_STATE_JUMP_ATTACK;	
					}
					else if (AI.distance > COMPSOGNATHUS_ATTACK_RANGE * 3)
					{
						item->Animation.TargetState = COMPSOGNATHUS_STATE_RUN;
					}
				}
				else 
				{
					if (GetRandomControl() < COMPSOGNATHUS_JUMP_ATTACK_CHANCE)
						item->Animation.TargetState = COMPSOGNATHUS_STATE_JUMP_ATTACK;
					else
						item->Animation.TargetState = COMPSOGNATHUS_STATE_RUN;
				}

				break;

			case COMPSOGNATHUS_STATE_RUN:

				creature->Flags &= ~COMPSOGNATHUS_HIT_FLAG;
				creature->MaxTurn = COMPSOGNATHUS_RUN_TURN;

				if (AI.angle < COMPSOGNATHUS_ATTACK_ANGLE && AI.angle > -COMPSOGNATHUS_ATTACK_ANGLE && AI.distance < COMPSOGNATHUS_ATTACK_RANGE * (9 - 4 * target))
				{
					item->Animation.TargetState = COMPSOGNATHUS_STATE_IDLE;

				}

				break;

			case COMPSOGNATHUS_STATE_ATTACK:
			case COMPSOGNATHUS_STATE_JUMP_ATTACK:

				creature->MaxTurn = COMPSOGNATHUS_RUN_TURN;

				if (!(creature->Flags & COMPSOGNATHUS_HIT_FLAG) && (item->TouchBits.Test(CompyAttackJoints) && item->ItemFlags[1] == ATTACK_LARA))
				{
					creature->Flags |= COMPSOGNATHUS_HIT_FLAG;

					DoDamage(creature->Enemy, COMPSOGNATHUS_ATTACK_DAMAGE);
					LaraItem->HitStatus = 1;
					CreatureEffect(item, CompyBite, DoBloodSplat);
				}
				else if (!(creature->Flags & COMPSOGNATHUS_HIT_FLAG) && AI.distance < COMPSOGNATHUS_ATTACK_RANGE && AI.ahead && item->ItemFlags[1] == ATTACK_CADAVER)
				{
					creature->Flags |= COMPSOGNATHUS_HIT_FLAG;				
					CreatureEffect(item, CompyBite, DoBloodSplat);
				}

				break;
			}
		}
		tilt = angle >> 1;
		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, head);
		CreatureJoint(item, 1, neck);

		CreatureAnimation(itemNumber, angle, tilt);
	}
}
