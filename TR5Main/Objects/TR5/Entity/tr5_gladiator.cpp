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
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"
#include "Game/animation.h"

BITE_INFO GladiatorBite = { 0, 0, 0, 16 };

void InitialiseGladiator(short itemNum)
{
    ITEM_INFO* item;

    item = &g_Level.Items[itemNum];
    ClearItem(itemNum);
    item->AnimNumber = Objects[item->ObjectNumber].animIndex;
    item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
    item->TargetState = 1;
    item->ActiveState = 1;
    if (item->TriggerFlags == 1)
        item->SwapMeshFlags = -1;
}

void ControlGladiator(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		short tilt = 0;
		short angle = 0;
		short joint2 = 0;
		short joint1 = 0;
		short joint0 = 0;
		PHD_VECTOR pos;
		short roomNumber;
		ROOM_INFO* r;
		MESH_INFO* mesh;
		FLOOR_INFO* floor;
		int i;

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;

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
			{
				GetAITarget(creature);
			}
			else if (creature->hurtByLara)
			{
				creature->enemy = LaraItem;
			}

			AI_INFO info;
			AI_INFO laraInfo;

			CreatureAIInfo(item, &info);

			int unknown = true;
			short rot;
			int distance;

			if (creature->enemy == LaraItem)
			{
				distance = info.distance;
				rot = info.angle;
			}
			else
			{
				int dx = LaraItem->Position.xPos - item->Position.xPos;
				int dz = LaraItem->Position.zPos - item->Position.zPos;
			
				rot = phd_atan(dz, dx) - item->Position.yRot;
				if (rot <= -ANGLE(90) || rot >= ANGLE(90))
					unknown = false;

				distance = SQUARE(dx) + SQUARE(dz);
			}

			GetCreatureMood(item, &info, VIOLENT);
			CreatureMood(item, &info, VIOLENT);

			angle = CreatureTurn(item, creature->maximumTurn);
			
			if (info.ahead)
			{
				joint0 = info.angle / 2;
				joint2 = info.angle / 2;
				joint1 = info.xAngle;
			}

			switch (item->ActiveState)
			{
			case 1:
				creature->flags = 0;
				joint2 = rot;
				creature->maximumTurn = (-(int)(creature->mood != MoodType::Bored)) & 0x16C;
				
				if (item->AIBits & GUARD 
					|| !(GetRandomControl() & 0x1F) 
					&& (info.distance > SQUARE(1024)
						|| creature->mood != MoodType::Attack))
				{
					joint2 = AIGuard(creature);
					break;
				}

				if (item->AIBits & PATROL1)
				{
					item->TargetState = 2;
				}
				else
				{
					if (creature->mood == MoodType::Escape)
					{
						if (Lara.target != item
							&& info.ahead
							&& !item->HitStatus)
						{
							item->TargetState = 1;
							break;
						}
					}
					else
					{
						if (creature->mood == MoodType::Bored
							|| item->AIBits & FOLLOW 
							&& (creature->reachedGoal 
								|| distance > SQUARE(2048)))
						{
							if (item->RequiredState)
							{
								item->TargetState = item->RequiredState;
							}
							else if (!(GetRandomControl() & 0x3F))
							{
								item->TargetState = 2;
							}
							break;
						}
						
						if (Lara.target == item
							&& unknown
							&& distance < SQUARE(1536)
							&& GetRandomControl() & 1
							&& (Lara.Control.WeaponControl.GunType == WEAPON_SHOTGUN 
								|| !(GetRandomControl() & 0xF))
							&& item->MeshBits == -1)
						{
							item->TargetState = 4;
							break;
						}
						
						if (info.bite && info.distance < SQUARE(819))
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
				creature->flags = 0;
				joint2 = rot;
				creature->maximumTurn = creature->mood != MoodType::Bored ? ANGLE(7) : ANGLE(2);

				if (item->AIBits & PATROL1)
				{
					item->TargetState = 2;
					joint2 = 0;
				}
				else if (creature->mood == MoodType::Escape)
				{
					item->TargetState = 3;
				}
				else if (creature->mood != MoodType::Bored)
				{
					if (info.distance < SQUARE(1024))
					{
						item->TargetState = 1;
						break;
					}

					if (info.bite && info.distance < SQUARE(2048))
					{
						item->TargetState = 11;
					}
					else if (!info.ahead || info.distance > SQUARE(1536))
					{
						item->TargetState = 3;
					}
				}
				else if (!(GetRandomControl() & 0x3F))
				{
					item->TargetState = 1;
					break;
				}
				
				break;

			case 3:
				creature->LOT.isJumping = false;
				if (info.ahead)
					joint2 = info.angle;
				creature->maximumTurn = ANGLE(11);
				tilt = angle / 2;

				if (item->AIBits & GUARD)
				{
					creature->maximumTurn = 0;
					item->TargetState = 1;
					break;
				}

				if (creature->mood == MoodType::Escape)
				{
					if (Lara.target != item && info.ahead)
					{
						item->TargetState = 1;
						break;
					}
					break;
				}

				if (item->AIBits & FOLLOW 
					&& (creature->reachedGoal 
						|| distance > SQUARE(2048)))
				{
					item->TargetState = 1;
					break;
				}

				if (creature->mood == MoodType::Bored)
				{
					item->TargetState = 2;
					break;
				}

				if (info.distance < SQUARE(1536))
				{
					if (info.bite)
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
				else if (Lara.target != item 
					|| !(GetRandomControl() & 0x7F))
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
				creature->maximumTurn = 0;
				if (abs(info.angle) >= ANGLE(7))
				{
					if (info.angle >= 0)
						item->Position.yRot += ANGLE(7);
					else
						item->Position.yRot -= ANGLE(7);
				}
				else
				{
					item->Position.yRot += info.angle;
				}

				if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 10)
				{
					r = &g_Level.Rooms[item->RoomNumber];
					pos.x = 0;
					pos.y = 0;
					pos.z = 0;
					GetJointAbsPosition(item, &pos, 16);

					floor = GetSector(r, pos.x - r->x, pos.z - r->z);
					if (floor->Stopper)
					{
						for (i = 0; i < r->mesh.size(); i++)
						{
							mesh = &r->mesh[i];

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

					if (!creature->flags)
					{
						if (item->TouchBits & 0x6000)
						{
							LaraItem->HitPoints -= 120;
							LaraItem->HitStatus = true;
							CreatureEffect2(item, &GladiatorBite, 10, item->Position.yRot, DoBloodSplat);
							SoundEffect(SFX_TR4_LARA_THUD, &item->Position, 0);
							creature->flags = 1;
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
}