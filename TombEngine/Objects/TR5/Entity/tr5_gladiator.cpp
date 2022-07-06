#include "framework.h"
#include "tr5_gladiator.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"
#include "Game/animation.h"

BITE_INFO GladiatorBite = { 0, 0, 0, 16 };

// TODO
enum GladiatorState
{

};

// TODO
enum GladiatorAnim
{

};

void InitialiseGladiator(short itemNumber)
{
    auto* item = &g_Level.Items[itemNumber];

    ClearItem(itemNumber);

    item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
    item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
    item->Animation.TargetState = 1;
    item->Animation.ActiveState = 1;

    if (item->TriggerFlags == 1)
        item->MeshSwapBits = ALL_JOINT_BITS;
}

void ControlGladiator(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short tilt = 0;
	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	if (item->HitPoints <= 0)
	{
		item->HitPoints = 0;
		if (item->Animation.ActiveState != 6)
		{
			item->Animation.AnimNumber = Objects[ID_GLADIATOR].animIndex + 16;
			item->Animation.ActiveState = 6;
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

		int unknown = true;
		short rot;
		int distance;

		if (creature->Enemy == LaraItem)
		{
			distance = AI.distance;
			rot = AI.angle;
		}
		else
		{
			int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
			int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
			
			rot = phd_atan(dz, dx) - item->Pose.Orientation.y;
			if (rot <= -ANGLE(90.0f) || rot >= ANGLE(90.0f))
				unknown = false;

			distance = pow(dx, 2) + pow(dz, 2);
		}

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);
			
		if (AI.ahead)
		{
			joint0 = AI.angle / 2;
			joint2 = AI.angle / 2;
			joint1 = AI.xAngle;
		}

		switch (item->Animation.ActiveState)
		{
		case 1:
			creature->Flags = 0;
			joint2 = rot;
			creature->MaxTurn = (-(int)(creature->Mood != MoodType::Bored)) & 0x16C;
				
			if (item->AIBits & GUARD ||
				!(GetRandomControl() & 0x1F) &&
				(AI.distance > pow(SECTOR(1), 2) ||
					creature->Mood != MoodType::Attack))
			{
				joint2 = AIGuard(creature);
				break;
			}

			if (item->AIBits & PATROL1)
				item->Animation.TargetState = 2;
			else
			{
				if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item &&
						AI.ahead &&
						!item->HitStatus)
					{
						item->Animation.TargetState = 1;
						break;
					}
				}
				else
				{
					if (creature->Mood == MoodType::Bored ||
						item->AIBits & FOLLOW &&
						(creature->ReachedGoal ||
							distance > pow(SECTOR(2), 2)))
					{
						if (item->Animation.RequiredState)
							item->Animation.TargetState = item->Animation.RequiredState;
						else if (!(GetRandomControl() & 0x3F))
							item->Animation.TargetState = 2;
						
						break;
					}
						
					if (Lara.TargetEntity == item &&
						unknown &&
						distance < pow(SECTOR(1.5f), 2) &&
						GetRandomControl() & 1 &&
						(Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun ||
							!(GetRandomControl() & 0xF)) &&
						item->MeshBits == -1)
					{
						item->Animation.TargetState = 4;
						break;
					}
						
					if (AI.bite && AI.distance < pow(819, 2))
					{
						if (GetRandomControl() & 1)
							item->Animation.TargetState = 8;
						else
							item->Animation.TargetState = 9;

						break;
					}
				}

				item->Animation.TargetState = 2;
			}

			break;

		case 2:
			creature->MaxTurn = creature->Mood != MoodType::Bored ? ANGLE(7.0f) : ANGLE(2.0f);
			joint2 = rot;
			creature->Flags = 0;

			if (item->AIBits & PATROL1)
			{
				item->Animation.TargetState = 2;
				joint2 = 0;
			}
			else if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 3;
			else if (creature->Mood != MoodType::Bored)
			{
				if (AI.distance < pow(SECTOR(1), 2))
				{
					item->Animation.TargetState = 1;
					break;
				}

				if (AI.bite && AI.distance < pow(SECTOR(2), 2))
					item->Animation.TargetState = 11;
				else if (!AI.ahead || AI.distance > pow(SECTOR(1.5f), 2))
					item->Animation.TargetState = 3;
			}
			else if (!(GetRandomControl() & 0x3F))
			{
				item->Animation.TargetState = 1;
				break;
			}
				
			break;

		case 3:
			creature->MaxTurn = ANGLE(11.0f);
			creature->LOT.IsJumping = false;
			tilt = angle / 2;

			if (AI.ahead)
				joint2 = AI.angle;

			if (item->AIBits & GUARD)
			{
				creature->MaxTurn = 0;
				item->Animation.TargetState = 1;
				break;
			}

			if (creature->Mood == MoodType::Escape)
			{
				if (Lara.TargetEntity != item && AI.ahead)
				{
					item->Animation.TargetState = 1;
					break;
				}

				break;
			}

			if (item->AIBits & FOLLOW &&
				(creature->ReachedGoal ||
					distance > pow(SECTOR(2), 2)))
			{
				item->Animation.TargetState = 1;
				break;
			}

			if (creature->Mood == MoodType::Bored)
			{
				item->Animation.TargetState = 2;
				break;
			}

			if (AI.distance < pow(SECTOR(1.5f), 2))
			{
				if (AI.bite)
					item->Animation.TargetState = 10;
				else
					item->Animation.TargetState = 2;
			}

			break;

		case 4:
			if (item->HitStatus)
			{
				if (!unknown)
				{
					item->Animation.TargetState = 1;
					break;
				}
			}
			else if (Lara.TargetEntity != item ||
				!(GetRandomControl() & 0x7F))
			{
				item->Animation.TargetState = 1;
				break;
			}

			break;

		case 5:
			if (Lara.TargetEntity != item)
				item->Animation.TargetState = 1;

			break;

		case 8:
		case 9:
		case 10:
		case 11:
			creature->MaxTurn = 0;

			if (abs(AI.angle) >= ANGLE(7.0f))
			{
				if (AI.angle >= 0)
					item->Pose.Orientation.y += ANGLE(7.0f);
				else
					item->Pose.Orientation.y -= ANGLE(7.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;

			if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 10)
			{
				auto* room = &g_Level.Rooms[item->RoomNumber];

				auto pos = Vector3Int();
				GetJointAbsPosition(item, &pos, 16);

				auto* floor = GetSector(room, pos.x - room->x, pos.z - room->z);
				if (floor->Stopper)
				{
					for (int i = 0; i < room->mesh.size(); i++)
					{
						auto* mesh = &room->mesh[i];

						if (!((pos.z ^ mesh->pos.Position.z) & 0xFFFFFC00))
						{
							if (!((pos.x ^ mesh->pos.Position.x) & 0xFFFFFC00))
							{
								if (StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
								{
									ShatterObject(0, mesh, -64, LaraItem->RoomNumber, 0);
									//SoundEffect(ShatterSounds[gfCurrentLevel - 5][*(v28 + 18)], v28);
									mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;

									TestTriggers(pos.x, pos.y, pos.z, item->RoomNumber, true);
								}
							}
						}
					}
				}

				if (!creature->Flags)
				{
					if (item->TouchBits & 0x6000)
					{
						DoDamage(creature->Enemy, 120);
						CreatureEffect2(item, &GladiatorBite, 10, item->Pose.Orientation.y, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
						creature->Flags = 1;
					}
				}
			}

			break;

		default:
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);
}
