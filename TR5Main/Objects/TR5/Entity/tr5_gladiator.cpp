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

    item->AnimNumber = Objects[item->ObjectNumber].animIndex;
    item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
    item->TargetState = 1;
    item->ActiveState = 1;

    if (item->TriggerFlags == 1)
        item->SwapMeshFlags = -1;
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
		if (item->ActiveState != 6)
		{
			item->AnimNumber = Objects[ID_GLADIATOR].animIndex + 16;
			item->ActiveState = 6;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
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
			int dx = LaraItem->Position.xPos - item->Position.xPos;
			int dz = LaraItem->Position.zPos - item->Position.zPos;
			
			rot = phd_atan(dz, dx) - item->Position.yRot;
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

		switch (item->ActiveState)
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
				item->TargetState = 2;
			else
			{
				if (creature->Mood == MoodType::Escape)
				{
					if (Lara.target != item &&
						AI.ahead &&
						!item->HitStatus)
					{
						item->TargetState = 1;
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
						if (item->RequiredState)
							item->TargetState = item->RequiredState;
						else if (!(GetRandomControl() & 0x3F))
							item->TargetState = 2;
						
						break;
					}
						
					if (Lara.target == item &&
						unknown &&
						distance < pow(SECTOR(1.5f), 2) &&
						GetRandomControl() & 1 &&
						(Lara.Control.WeaponControl.GunType == WEAPON_SHOTGUN ||
							!(GetRandomControl() & 0xF)) &&
						item->MeshBits == -1)
					{
						item->TargetState = 4;
						break;
					}
						
					if (AI.bite && AI.distance < pow(819, 2))
					{
						if (GetRandomControl() & 1)
							item->TargetState = 8;
						else
							item->TargetState = 9;

						break;
					}
				}

				item->TargetState = 2;
			}

			break;

		case 2:
			creature->MaxTurn = creature->Mood != MoodType::Bored ? ANGLE(7.0f) : ANGLE(2.0f);
			joint2 = rot;
			creature->Flags = 0;

			if (item->AIBits & PATROL1)
			{
				item->TargetState = 2;
				joint2 = 0;
			}
			else if (creature->Mood == MoodType::Escape)
				item->TargetState = 3;
			else if (creature->Mood != MoodType::Bored)
			{
				if (AI.distance < pow(SECTOR(1), 2))
				{
					item->TargetState = 1;
					break;
				}

				if (AI.bite && AI.distance < pow(SECTOR(2), 2))
					item->TargetState = 11;
				else if (!AI.ahead || AI.distance > pow(SECTOR(1.5f), 2))
					item->TargetState = 3;
			}
			else if (!(GetRandomControl() & 0x3F))
			{
				item->TargetState = 1;
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
				item->TargetState = 1;
				break;
			}

			if (creature->Mood == MoodType::Escape)
			{
				if (Lara.target != item && AI.ahead)
				{
					item->TargetState = 1;
					break;
				}

				break;
			}

			if (item->AIBits & FOLLOW &&
				(creature->ReachedGoal ||
					distance > pow(SECTOR(2), 2)))
			{
				item->TargetState = 1;
				break;
			}

			if (creature->Mood == MoodType::Bored)
			{
				item->TargetState = 2;
				break;
			}

			if (AI.distance < pow(SECTOR(1.5f), 2))
			{
				if (AI.bite)
					item->TargetState = 10;
				else
					item->TargetState = 2;
			}

			break;

		case 4:
			if (item->HitStatus)
			{
				if (!unknown)
				{
					item->TargetState = 1;
					break;
				}
			}
			else if (Lara.target != item ||
				!(GetRandomControl() & 0x7F))
			{
				item->TargetState = 1;
				break;
			}

			break;

		case 5:
			if (Lara.target != item)
				item->TargetState = 1;

			break;

		case 8:
		case 9:
		case 10:
		case 11:
			creature->MaxTurn = 0;

			if (abs(AI.angle) >= ANGLE(7.0f))
			{
				if (AI.angle >= 0)
					item->Position.yRot += ANGLE(7.0f);
				else
					item->Position.yRot -= ANGLE(7.0f);
			}
			else
				item->Position.yRot += AI.angle;

			if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 10)
			{
				auto* room = &g_Level.Rooms[item->RoomNumber];

				PHD_VECTOR pos = { 0, 0, 0 };
				GetJointAbsPosition(item, &pos, 16);

				auto* floor = GetSector(room, pos.x - room->x, pos.z - room->z);
				if (floor->Stopper)
				{
					for (int i = 0; i < room->mesh.size(); i++)
					{
						auto* mesh = &room->mesh[i];

						if (!((pos.z ^ mesh->pos.zPos) & 0xFFFFFC00))
						{
							if (!((pos.x ^ mesh->pos.xPos) & 0xFFFFFC00))
							{
								if (StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
								{
									ShatterObject(0, mesh, -64, LaraItem->RoomNumber, 0);
									//SoundEffect(ShatterSounds[gfCurrentLevel - 5][*(v28 + 18)], v28, 0);
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
						CreatureEffect2(item, &GladiatorBite, 10, item->Position.yRot, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Position, 0);
						creature->Flags = 1;

						LaraItem->HitPoints -= 120;
						LaraItem->HitStatus = true;
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
