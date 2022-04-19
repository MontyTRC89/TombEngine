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
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
    item->goalAnimState = 1;
    item->currentAnimState = 1;
    if (item->triggerFlags == 1)
        item->swapMeshFlags = -1;
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
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

		if (item->hitPoints <= 0)
		{
			item->hitPoints = 0;
			if (item->currentAnimState != 6)
			{
				item->animNumber = Objects[ID_GLADIATOR].animIndex + 16;
				item->currentAnimState = 6;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
		}
		else
		{
			if (item->aiBits)
			{
				GetAITarget(creature);
			}
			else if (creature->hurtByLara)
			{
				creature->enemy = LaraItem;
			}

			AI_INFO info;
			AI_INFO laraInfo;

			CreatureAIInfo(item,&info);

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
				int dx = LaraItem->pos.xPos - item->pos.xPos;
				int dz = LaraItem->pos.zPos - item->pos.zPos;
			
				rot = phd_atan(dz, dx) - item->pos.yRot;
				if (rot <= -ANGLE(90) || rot >= ANGLE(90))
					unknown = false;

				distance = SQUARE(dx) + SQUARE(dz);
			}

			GetCreatureMood(item,&info, VIOLENT);
			CreatureMood(item,&info, VIOLENT);

			angle = CreatureTurn(item, creature->maximumTurn);
			
			if (info.ahead)
			{
				joint0 = info.angle / 2;
				joint2 = info.angle / 2;
				joint1 = info.xAngle;
			}

			switch (item->currentAnimState)
			{
			case 1:
				creature->flags = 0;
				joint2 = rot;
				creature->maximumTurn = (-(creature->mood != 0)) & 0x16C;
				
				if (item->aiBits & GUARD 
					|| !(GetRandomControl() & 0x1F) 
					&& (info.distance > SQUARE(1024)
						|| creature->mood != ATTACK_MOOD))
				{
					joint2 = AIGuard(creature);
					break;
				}

				if (item->aiBits & PATROL1)
				{
					item->goalAnimState = 2;
				}
				else
				{
					if (creature->mood == ESCAPE_MOOD)
					{
						if (Lara.target != item
							&& info.ahead
							&& !item->hitStatus)
						{
							item->goalAnimState = 1;
							break;
						}
					}
					else
					{
						if (!creature->mood 
							|| item->aiBits & FOLLOW 
							&& (creature->reachedGoal 
								|| distance > SQUARE(2048)))
						{
							if (item->requiredAnimState)
							{
								item->goalAnimState = item->requiredAnimState;
							}
							else if (!(GetRandomControl() & 0x3F))
							{
								item->goalAnimState = 2;
							}
							break;
						}
						
						if (Lara.target == item
							&& unknown
							&& distance < SQUARE(1536)
							&& GetRandomControl() & 1
							&& (Lara.gunType == WEAPON_SHOTGUN 
								|| !(GetRandomControl() & 0xF))
							&& item->meshBits == -1)
						{
							item->goalAnimState = 4;
							break;
						}
						
						if (info.bite && info.distance < SQUARE(819))
						{
							if (GetRandomControl() & 1)
								item->goalAnimState = 8;
							else
								item->goalAnimState = 9;
							break;
						}
					}
					item->goalAnimState = 2;
				}
				break;

			case 2:
				creature->flags = 0;
				joint2 = rot;
				creature->maximumTurn = creature->mood != BORED_MOOD ? ANGLE(7) : ANGLE(2);

				if (item->aiBits & PATROL1)
				{
					item->goalAnimState = 2;
					joint2 = 0;
				}
				else if (creature->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 3;
				}
				else if (creature->mood)
				{
					if (info.distance < SQUARE(1024))
					{
						item->goalAnimState = 1;
						break;
					}

					if (info.bite && info.distance < SQUARE(2048))
					{
						item->goalAnimState = 11;
					}
					else if (!info.ahead || info.distance > SQUARE(1536))
					{
						item->goalAnimState = 3;
					}
				}
				else if (!(GetRandomControl() & 0x3F))
				{
					item->goalAnimState = 1;
					break;
				}
				
				break;

			case 3:
				creature->LOT.isJumping = false;
				if (info.ahead)
					joint2 = info.angle;
				creature->maximumTurn = ANGLE(11);
				tilt = angle / 2;

				if (item->aiBits & GUARD)
				{
					creature->maximumTurn = 0;
					item->goalAnimState = 1;
					break;
				}

				if (creature->mood == ESCAPE_MOOD)
				{
					if (Lara.target != item && info.ahead)
					{
						item->goalAnimState = 1;
						break;
					}
					break;
				}

				if (item->aiBits & FOLLOW 
					&& (creature->reachedGoal 
						|| distance > SQUARE(2048)))
				{
					item->goalAnimState = 1;
					break;
				}

				if (!creature->mood)
				{
					item->goalAnimState = 2;
					break;
				}

				if (info.distance < SQUARE(1536))
				{
					if (info.bite)
						item->goalAnimState = 10;
					else
						item->goalAnimState = 2;
				}
				break;

			case 4:
				if (item->hitStatus)
				{
					if (!unknown)
					{
						item->goalAnimState = 1;
						break;
					}
				}
				else if (Lara.target != item 
					|| !(GetRandomControl() & 0x7F))
				{
					item->goalAnimState = 1;
					break;
				}
				break;

			case 5:
				if (Lara.target != item)
					item->goalAnimState = 1;
				break;

			case 8:
			case 9:
			case 10:
			case 11:
				creature->maximumTurn = 0;
				if (abs(info.angle) >= ANGLE(7))
				{
					if (info.angle >= 0)
						item->pos.yRot += ANGLE(7);
					else
						item->pos.yRot -= ANGLE(7);
				}
				else
				{
					item->pos.yRot += info.angle;
				}

				if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 10)
				{
					r = &g_Level.Rooms[item->roomNumber];
					pos.x = 0;
					pos.y = 0;
					pos.z = 0;
					GetJointAbsPosition(item,&pos, 16);

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
										ShatterObject(0, mesh, -64, LaraItem->roomNumber, 0);
										//SoundEffect(ShatterSounds[gfCurrentLevel - 5][*(v28 + 18)], v28, 0);
										mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;

										TestTriggers(pos.x, pos.y, pos.z, item->roomNumber, true);
									}
								}
							}
						}
					}

					if (!creature->flags)
					{
						if (item->touchBits & 0x6000)
						{
							LaraItem->hitPoints -= 120;
							LaraItem->hitStatus = true;
							CreatureEffect2(item,&GladiatorBite, 10, item->pos.yRot, DoBloodSplat);
							SoundEffect(SFX_TR4_LARA_THUD,&item->pos, 0);
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