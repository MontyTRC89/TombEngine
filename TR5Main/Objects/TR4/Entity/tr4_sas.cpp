#include "framework.h"
#include "tr4_sas.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/people.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/input.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/itemdata/creature_info.h"
#include "Game/collision/collide_item.h"

namespace TEN::Entities::TR4
{
	BITE_INFO SASGunBite = { 0, 300, 64, 7 };

	Vector3Int SASDragBodyPosition = { 0, 0, -460 };
	OBJECT_COLLISION_BOUNDS SASDragBodyBounds =
	{
		-256, 256,
		-64, 100,
		-200, -460,
		ANGLE(-10.0f), ANGLE(10.0f),
		ANGLE(-30.0f), ANGLE(30.0f),
		0, 0
	};

	enum SASState
	{
		SAS_STATE_NONE = 0,
		SAS_STATE_IDLE = 1,
		SAS_STATE_WALK = 2,
		SAS_STATE_RUN = 3,
		SAS_STATE_WAIT = 4,
		SAS_STATE_SIGHT_SHOOT = 5,
		SAS_STATE_WALK_SHOOT = 6,
		SAS_STATE_DEATH = 7,
		SAS_STATE_SIGHT_AIM = 8,
		SAS_STATE_WALK_AIM = 9,
		SAS_STATE_HOLD_AIM = 10,
		SAS_STATE_HOLD_SHOOT = 11,
		SAS_STATE_KNEEL_AIM = 12,
		SAS_STATE_KNEEL_SHOOT = 13,
		SAS_STATE_KNEEL_STOP = 14,
		SAS_STATE_HOLD_PREPARE_GRENADE = 15,
		SAS_STATE_HOLD_SHOOT_GRENADE = 16,
		SAS_STATE_BLIND = 17
	};

	enum SASAnim
	{
		SAS_ANIM_WALK = 0,
		SAS_ANIM_RUN = 1,
		SAS_ANIM_SIGHT_SHOOT = 2,
		SAS_ANIM_STAND_TO_SIGHT_AIM = 3,
		SAS_ANIM_WALK_TO_WALK_AIM = 4,
		SAS_ANIM_RUN_TO_WALK = 5,
		SAS_ANIM_WALK_SHOOT = 6,
		SAS_ANIM_SIGHT_AIM_TO_STAND = 7,
		SAS_ANIM_WALK_AIM_TO_STAND = 8,
		SAS_ANIM_STAND_TO_RUN = 9,
		SAS_ANIM_STAND_TO_WAIT = 10,
		SAS_ANIM_STAND_TO_WALK = 11,
		SAS_ANIM_STAND = 12,
		SAS_ANIM_WAIT_TO_STAND = 13,
		SAS_ANIM_WAIT = 14,
		SAS_ANIM_WAIT_TO_SIGHT_AIM = 15,
		SAS_ANIM_WALK_TO_RUN = 16,
		SAS_ANIM_WALK_TO_STAND = 17,
		SAS_ANIM_WALK_AIM_TO_WALK = 18,
		SAS_ANIM_DEATH = 19,
		SAS_ANIM_STAND_TO_HOLD_AIM = 20,
		SAS_ANIM_HOLD_SHOOT = 21,
		SAS_ANIM_HOLD_AIM_TO_STAND = 22,
		SAS_ANIM_HOLD_PREPARE_GRENADE = 23,
		SAS_ANIM_HOLD_SHOOT_GRENADE = 24,
		SAS_ANIM_STAND_TO_KNEEL_AIM = 25,
		SAS_ANIM_KNEEL_SHOOT = 26,
		SAS_ANIM_KNEEL_AIM_TO_STAND = 27,
		SAS_ANIM_BLIND = 28,
		SAS_ANIM_BLIND_TO_STAND = 29
	};

	void InitialiseSas(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + SAS_ANIM_STAND;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = SAS_STATE_IDLE;
		item->Animation.ActiveState = SAS_STATE_IDLE;
	}

	void SasControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = (CreatureInfo*)item->Data;
		auto* enemy = creature->Enemy;

		short tilt = 0;
		short angle = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;

		// Handle SAS firing.
		if (creature->FiredWeapon)
		{
			auto pos = Vector3Int(SASGunBite.x, SASGunBite.y, SASGunBite.z);
			GetJointAbsPosition(item, &pos, SASGunBite.meshNum);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 24, 16, 4);
			creature->FiredWeapon--;
		}

		if (item->HitPoints > 0)
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
				creature->Enemy = LaraItem;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			int distance = 0;
			int angle = 0;
			if (creature->Enemy == LaraItem)
			{
				angle = AI.angle;
				distance = AI.distance;
			}
			else
			{
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				int ang = phd_atan(dz, dx) - item->Pose.Orientation.y;
				distance = pow(dx, 2) + pow(dz, 2);
			}

			GetCreatureMood(item, &AI, creature->Enemy != LaraItem);

			// Vehicle handling
			if (Lara.Vehicle != NO_ITEM && AI.bite)
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, creature->Enemy != LaraItem);
			angle = CreatureTurn(item, creature->MaxTurn);

			if (item->HitStatus)
				AlertAllGuards(itemNumber);

			int angle1 = 0;
			int angle2 = 0;

			switch (item->Animation.ActiveState)
			{
			case SAS_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				joint2 = angle;

				if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + SAS_ANIM_WALK_TO_STAND)
				{
					if (abs(AI.angle) >= ANGLE(10.0f))
					{
						if (AI.angle >= 0)
							item->Pose.Orientation.y += ANGLE(10.0f);
						else
							item->Pose.Orientation.y -= ANGLE(10.0f);
					}
					else
						item->Pose.Orientation.y += AI.angle;
				}
				else if (item->AIBits & MODIFY || Lara.Vehicle != NO_ITEM)
				{
					if (abs(AI.angle) >= ANGLE(2.0f))
					{
						if (AI.angle >= 0)
							item->Pose.Orientation.y += ANGLE(2.0f);
						else
							item->Pose.Orientation.y -= ANGLE(2.0f);
					}
					else
						item->Pose.Orientation.y += AI.angle;
				}

				if (item->AIBits & GUARD)
				{
					joint2 = AIGuard(creature);

					if (!GetRandomControl())
					{
						if (item->Animation.ActiveState == SAS_STATE_IDLE)
						{
							item->Animation.TargetState = SAS_STATE_WAIT;
							break;
						}

						item->Animation.TargetState = SAS_STATE_IDLE;
					}
				}
				else if (!(item->AIBits & PATROL1) ||
					item->AIBits & MODIFY ||
					Lara.Vehicle != NO_ITEM)
				{
					if (Targetable(item, &AI))
					{
						if (AI.distance < pow(SECTOR(3), 2) ||
							AI.zoneNumber != AI.enemyZone)
						{
							if (GetRandomControl() & 1)
								item->Animation.TargetState = SAS_STATE_SIGHT_AIM;
							else if (GetRandomControl() & 1)
								item->Animation.TargetState = SAS_STATE_HOLD_AIM;
							else
								item->Animation.TargetState = SAS_STATE_KNEEL_AIM;
						}
						else if (!(item->AIBits & MODIFY))
							item->Animation.TargetState = SAS_STATE_WALK;
					}
					else
					{
						if (item->AIBits & MODIFY)
							item->Animation.TargetState = SAS_STATE_IDLE;
						else
						{
							if (creature->Mood == MoodType::Escape)
								item->Animation.TargetState = SAS_STATE_RUN;
							else
							{
								if ((creature->Alerted ||
									creature->Mood != MoodType::Bored) &&
									(!(item->AIBits & FOLLOW) ||
										!creature->ReachedGoal &&
										distance <= pow(SECTOR(2), 2)))
								{
									if (creature->Mood == MoodType::Bored ||
										AI.distance <= pow(SECTOR(2), 2))
									{
										item->Animation.TargetState = SAS_STATE_WALK;
										break;
									}
									item->Animation.TargetState = SAS_STATE_RUN;
								}
								else
									item->Animation.TargetState = SAS_STATE_IDLE;
							}
						}
					}
				}
				else
				{
					item->Animation.TargetState = SAS_STATE_WALK;
					joint2 = 0;
				}

				break;

			case SAS_STATE_WAIT:
				joint2 = angle;
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (item->AIBits & GUARD)
				{
					joint2 = AIGuard(creature);

					if (!GetRandomControl())
						item->Animation.TargetState = SAS_STATE_IDLE;
				}
				else if (Targetable(item, &AI) ||
					creature->Mood != MoodType::Bored ||
					!AI.ahead ||
					item->AIBits & MODIFY ||
					Lara.Vehicle != NO_ITEM)
				{
					item->Animation.TargetState = SAS_STATE_IDLE;
				}

				break;

			case SAS_STATE_WALK:
				joint2 = angle;
				creature->MaxTurn = ANGLE(5.0f);
				creature->Flags = 0;

				if (item->AIBits & PATROL1)
					item->Animation.TargetState = SAS_STATE_WALK;
				else if (Lara.Vehicle == NO_ITEM ||
					!(item->AIBits & MODIFY) &&
					item->AIBits)
				{
					if (creature->Mood == MoodType::Escape)
						item->Animation.TargetState = SAS_STATE_RUN;
					else
					{
						if (item->AIBits & GUARD ||
							item->AIBits & FOLLOW &&
							(creature->ReachedGoal ||
								distance > pow(SECTOR(2), 2)))
						{
							item->Animation.TargetState = SAS_STATE_IDLE;
							break;
						}
						if (Targetable(item, &AI))
						{
							if (AI.distance < pow(SECTOR(3), 2) ||
								AI.enemyZone != AI.zoneNumber)
							{
								item->Animation.TargetState = SAS_STATE_IDLE;
								break;
							}

							item->Animation.TargetState = SAS_STATE_WALK_AIM;
						}
						else if (creature->Mood != MoodType::Bored)
						{
							if (AI.distance > pow(SECTOR(2), 2))
								item->Animation.TargetState = SAS_STATE_RUN;
						}
						else if (AI.ahead)
						{
							item->Animation.TargetState = SAS_STATE_IDLE;
							break;
						}
					}
				}
				else
					item->Animation.TargetState = SAS_STATE_IDLE;
				
				break;

			case SAS_STATE_RUN:
				tilt = angle / 2;
				creature->MaxTurn = ANGLE(10.0f);

				if (AI.ahead)
					joint2 = AI.angle;

				if (Lara.Vehicle != NO_ITEM)
				{
					if (item->AIBits & MODIFY || !item->AIBits)
					{
						item->Animation.TargetState = SAS_STATE_WAIT;
						break;
					}
				}

				if (item->AIBits & GUARD ||
					item->AIBits & FOLLOW &&
					(creature->ReachedGoal ||
						distance > pow(SECTOR(2), 2)))
				{
					item->Animation.TargetState = SAS_STATE_WALK;
					break;
				}

				if (creature->Mood != MoodType::Escape)
				{
					if (Targetable(item, &AI))
						item->Animation.TargetState = SAS_STATE_WALK;
					else
					{
						if (creature->Mood != MoodType::Bored ||
							creature->Mood == MoodType::Stalk &&
							item->AIBits & FOLLOW &&
							AI.distance < pow(SECTOR(2), 2))
						{
							item->Animation.TargetState = SAS_STATE_WALK;
						}
					}
				}

				break;

			case SAS_STATE_SIGHT_AIM:
			case SAS_STATE_HOLD_AIM:
			case SAS_STATE_KNEEL_AIM:
				creature->Flags = 0;

				if (AI.ahead)
				{
					joint0 = AI.angle;
					joint1 = AI.xAngle;

					if (Targetable(item, &AI))
					{
						if (item->Animation.ActiveState == SAS_STATE_SIGHT_AIM)
							item->Animation.TargetState = SAS_STATE_SIGHT_SHOOT;
						else if (item->Animation.ActiveState == SAS_STATE_KNEEL_AIM)
							item->Animation.TargetState = SAS_STATE_KNEEL_SHOOT;
						else if (GetRandomControl() & 1)
							item->Animation.TargetState = SAS_STATE_HOLD_SHOOT;
						else
							item->Animation.TargetState = SAS_STATE_HOLD_PREPARE_GRENADE;
					}
					else
						item->Animation.TargetState = SAS_STATE_IDLE;
				}

				break;

			case SAS_STATE_WALK_AIM:
				creature->Flags = 0;

				if (AI.ahead)
				{
					joint0 = AI.angle;
					joint1 = AI.xAngle;

					if (Targetable(item, &AI))
						item->Animation.TargetState = SAS_STATE_WALK_SHOOT;
					else
						item->Animation.TargetState = SAS_STATE_WALK;
				}

				break;

			case SAS_STATE_HOLD_PREPARE_GRENADE:
				if (AI.ahead)
				{
					joint0 = AI.angle;
					joint1 = AI.xAngle;
				}

				break;

			case SAS_STATE_HOLD_SHOOT_GRENADE:
				if (AI.ahead)
				{
					angle1 = AI.angle;
					angle2 = AI.xAngle;
					joint0 = AI.angle;
					joint1 = AI.xAngle;

					if (AI.distance > pow(SECTOR(3), 2))
					{
						angle2 = sqrt(AI.distance) + AI.xAngle - ANGLE(5.6f);
						joint1 = angle2;
					}
				}
				else
				{
					angle1 = 0;
					angle2 = 0;
				}

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 20)
				{
					if (!creature->Enemy->Animation.Velocity)
					{
						angle1 += (GetRandomControl() & 0x1FF) - 256;
						angle2 += (GetRandomControl() & 0x1FF) - 256;
						joint0 = angle1;
						joint1 = angle2;
					}

					SasFireGrenade(item, angle2, angle1);

					if (Targetable(item, &AI))
						item->Animation.TargetState = SAS_STATE_HOLD_PREPARE_GRENADE;
				}

				break;

			case SAS_STATE_HOLD_SHOOT:
			case SAS_STATE_KNEEL_SHOOT:
			case SAS_STATE_SIGHT_SHOOT:
			case SAS_STATE_WALK_SHOOT:
				if (item->Animation.ActiveState == SAS_STATE_HOLD_SHOOT ||
					item->Animation.ActiveState == SAS_STATE_KNEEL_SHOOT)
				{
					if (item->Animation.TargetState != SAS_STATE_IDLE &&
						item->Animation.TargetState != SAS_STATE_KNEEL_STOP &&
						(creature->Mood == MoodType::Escape ||
							!Targetable(item, &AI)))
					{
						if (item->Animation.ActiveState == SAS_STATE_HOLD_SHOOT)
							item->Animation.TargetState = SAS_STATE_IDLE;
						else
							item->Animation.TargetState = SAS_STATE_KNEEL_STOP;
					}
				}

				if (AI.ahead)
				{
					joint0 = AI.angle;
					joint1 = AI.xAngle;
				}

				if (creature->Flags)
					creature->Flags -= 1;
				else
				{
					ShotLara(item, &AI, &SASGunBite, joint0, 15);
					creature->Flags = 5;
					creature->FiredWeapon = 3;
				}

				break;

			case SAS_STATE_BLIND:
				if (!WeaponEnemyTimer && !(GetRandomControl() & 0x7F))
					item->Animation.TargetState = SAS_STATE_WAIT;

				break;

			default:
				break;
			}

			if (WeaponEnemyTimer > 100 &&
				item->Animation.ActiveState != SAS_STATE_BLIND)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + SAS_ANIM_BLIND;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase + (GetRandomControl() & 7);
				item->Animation.ActiveState = SAS_STATE_BLIND;
				creature->MaxTurn = 0;
			}
		}
		else
		{
			if (item->Animation.ActiveState != SAS_STATE_DEATH)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + SAS_ANIM_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = SAS_STATE_DEATH;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);

		CreatureAnimation(itemNumber, angle, 0);
	}

	void SasFireGrenade(ITEM_INFO* item, short angle1, short angle2)
	{
		short itemNumber = CreateItem();
		if (itemNumber != NO_ITEM)
		{
			auto* grenadeItem = &g_Level.Items[itemNumber];

			grenadeItem->Shade = -15856;
			grenadeItem->ObjectNumber = ID_GRENADE;
			grenadeItem->RoomNumber = item->RoomNumber;

			auto pos = Vector3Int(SASGunBite.x, SASGunBite.y, SASGunBite.z);
			GetJointAbsPosition(item, &pos, SASGunBite.meshNum);

			grenadeItem->Pose.Position.x = pos.x;
			grenadeItem->Pose.Position.y = pos.y;
			grenadeItem->Pose.Position.z = pos.z;

			auto probe = GetCollision(pos.x, pos.y, pos.z, grenadeItem->RoomNumber);
			grenadeItem->RoomNumber = probe.RoomNumber;

			if (probe.Position.Floor < grenadeItem->Pose.Position.y)
			{
				grenadeItem->Pose.Position.x = item->Pose.Position.x;
				grenadeItem->Pose.Position.y = probe.Position.Floor;
				grenadeItem->Pose.Position.z = item->Pose.Position.z;
				grenadeItem->RoomNumber = item->RoomNumber;
			}

			SmokeCountL = 32;
			SmokeWeapon = (LaraWeaponType)5; // TODO: 5 is the HK. Did TEN's enum get shuffled around? @Sezz 2022.03.09

			for (int i = 0; i < 5; i++)
				TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 1, (LaraWeaponType)5, 32);

			InitialiseItem(itemNumber);

			grenadeItem->Pose.Orientation.x = angle1 + item->Pose.Orientation.x;
			grenadeItem->Pose.Orientation.y = angle2 + item->Pose.Orientation.y;
			grenadeItem->Pose.Orientation.z = 0;

			if (GetRandomControl() & 3)
				grenadeItem->ItemFlags[0] = (int)GrenadeType::Normal;
			else
				grenadeItem->ItemFlags[0] = (int)GrenadeType::Super;

			grenadeItem->ItemFlags[2] = 1;
			grenadeItem->Animation.Velocity = 128;
			grenadeItem->Animation.ActiveState = grenadeItem->Pose.Orientation.x;
			grenadeItem->Animation.VerticalVelocity = -128 * phd_sin(grenadeItem->Pose.Orientation.x);
			grenadeItem->Animation.TargetState = grenadeItem->Pose.Orientation.y;
			grenadeItem->Animation.RequiredState = 0;
			grenadeItem->HitPoints = 120;

			AddActiveItem(itemNumber);
		}
	}

	void InitialiseInjuredSas(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->TriggerFlags)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
			item->Animation.TargetState = item->Animation.ActiveState = 1;
		}
		else
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 3;
			item->Animation.TargetState = item->Animation.ActiveState = 4;
		}

		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	}

	void InjuredSasControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->Animation.ActiveState == 1)
		{
			if (!(GetRandomControl() & 0x7F))
			{
				item->Animation.TargetState = 2;
				AnimateItem(item);
			}
			else if (!(byte)GetRandomControl())
				item->Animation.TargetState = 3;
		}
		else if (item->Animation.ActiveState == 4 && !(GetRandomControl() & 0x7F))
		{
			item->Animation.TargetState = 5;
			AnimateItem(item);
		}
		else
			AnimateItem(item);
	}

	void SasDragBlokeCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
	{
		auto* item = &g_Level.Items[itemNumber];

		if ((!(TrInput & IN_ACTION) ||
			laraItem->Animation.Airborne ||
			laraItem->Animation.ActiveState != LS_IDLE ||
			laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
			Lara.Control.HandStatus != HandStatus::Free ||
			item->Flags & 0x3E00) &&
			(!(Lara.Control.IsMoving) || Lara.InteractedItem != itemNumber))
		{
			if (item->Status == ITEM_ACTIVE)
			{
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
				{
					int x = laraItem->Pose.Position.x - 512 * phd_sin(laraItem->Pose.Orientation.y);
					int y = laraItem->Pose.Position.y;
					int z = laraItem->Pose.Position.z - 512 * phd_cos(laraItem->Pose.Orientation.y);

					TestTriggers(x, y, z, laraItem->RoomNumber, true);

					RemoveActiveItem(itemNumber);
					item->Status = ITEM_NOT_ACTIVE;
				}
			}

			ObjectCollision(itemNumber, laraItem, coll);
		}
		else
		{
			if (TestLaraPosition(&SASDragBodyBounds, item, laraItem))
			{
				if (MoveLaraPosition(&SASDragBodyPosition, item, laraItem))
				{
					laraItem->Animation.AnimNumber = LA_DRAG_BODY;
					laraItem->Animation.ActiveState = LS_MISC_CONTROL;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					laraItem->Pose.Orientation.y = item->Pose.Orientation.y;
					ResetLaraFlex(laraItem);
					Lara.Control.IsMoving = false;
					Lara.Control.HandStatus = HandStatus::Busy;
					item->Flags |= 0x3E00;
					item->Status = ITEM_ACTIVE;
					AddActiveItem(itemNumber);
				}
				else
					Lara.InteractedItem = itemNumber;
			}
		}
	}
}
