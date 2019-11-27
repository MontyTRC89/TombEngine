#include "newobjects.h"
#include "../Game/Box.h"
#include "../Game/items.h"
#include "../Game/lot.h"
#include "../Game/control.h"
#include "../Game/effects.h"
#include "../Game/draw.h"
#include "../Game/sphere.h"
#include "../Game/effect2.h"
#include "../Game/people.h"
#include "../Game/lara.h"

void __cdecl InitialiseDemigod(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[item->objectNumber].animIndex;
	item->goalAnimState = 0;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = 0;

	/*if (LevelItems > 0)
	{
		ITEM_INFO* currentItem = &Items[0];
		int k = 0;

		while (item == currentItem || currentItem->objectNumber != ID_DEMIGOD3 || currentItem->itemFlags[0])
		{
			k++;
			currentItem++;
			if (k >= LevelItems)
				return;
		}

		item->itemFlags[0] = k;
	}*/
}

void __cdecl DemigodControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	
	int someItemNum = item->itemFlags[0];
	
	if (someItemNum && Items[someItemNum].status == ITEM_ACTIVE && Items[someItemNum].active)
	{
		item->hitPoints = Objects[item->objectNumber].hitPoints;
		return;
	}

	if (!CreatureActive(itemNum))
		return;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short tilt = 0;
	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;

	/*if (CurrentLevel == 24)
	{
		ROOM_INFO* room = &Rooms[item->roomNumber];

		short* zone = GroundZones[FlipStatus * 2 + creature->LOT.zone];
		
		LaraItem->boxNumber = room->floor[((LaraItem->pos.zPos - room->z) >> WALL_SHIFT) +
			((LaraItem->pos.xPos - room->x) >> WALL_SHIFT) * room->xSize].box;

		if (zone[item->boxNumber] == zone[LaraItem->boxNumber])
		{
			item->aiBits = 0;
			creature->enemy = LaraItem;
		}
		else
		{
			item->aiBits = FOLLOW;
			item->itemFlags[3] = Lara.location;
			creature->enemy = NULL;
		}
	}*/

	if (item->hitPoints <= 0)
	{
		item->hitPoints = 0;
		if (item->currentAnimState != 8 && item->currentAnimState != 15)
		{
			if (item->currentAnimState == 1 || item->currentAnimState == 2)
			{
				item->animNumber = Objects[item->objectNumber].animIndex + 27;
				item->currentAnimState = 15;
				item->frameNumber = Anims[item->animNumber].frameBase;
			}
			else
			{
				item->animNumber = Objects[item->objectNumber].animIndex + 12;
				item->currentAnimState = 8;
				item->frameNumber = Anims[item->animNumber].frameBase;
			}
		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);

		AI_INFO info;
		AI_INFO laraInfo;

		int dx = 0;
		int dy = 0;
		int dz = 0;

		CreatureAIInfo(item, &info);

		if (creature->enemy == LaraItem)
		{
			laraInfo.ahead = info.ahead;
			laraInfo.angle = info.angle;
			laraInfo.xAngle = 0;
		}
		else
		{
			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;
			laraInfo.angle = ATAN(dz, dx) - item->pos.yRot;
			laraInfo.xAngle = 0;

			laraInfo.ahead = true;
			if (laraInfo.angle <= -ANGLE(90) || laraInfo.angle >= ANGLE(90))
				laraInfo.ahead = false;

			dx = abs(dx);
			dy = item->pos.yPos - LaraItem->pos.yPos;
			dz = abs(dz);

			if (dx <= dz)
				laraInfo.xAngle = ATAN(dz + (dx >> 1), dy);
			else
				laraInfo.xAngle = ATAN(dx + (dz >> 1), dy);
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (laraInfo.ahead)
		{
			joint0 = laraInfo.angle >> 1;
			joint1 = -laraInfo.xAngle;
			joint2 = laraInfo.angle >> 1;
			joint3 = laraInfo.angle >> 1;
		}
		else if (info.ahead)
		{
			joint0 = info.angle >> 1;
			joint1 = -info.xAngle;
			joint2 = info.angle >> 1;
			joint3 = info.angle >> 1;
		}

		switch (item->currentAnimState)
		{
		case 0:
			creature->maximumTurn = 0;
			if (info.ahead)
				joint1 = -info.xAngle;

			if (item->objectNumber == ID_DEMIGOD1)
			{
				if (info.distance >= SQUARE(3072))
				{
					item->goalAnimState = 1;
					break;
				}
				if (info.bite
					|| LaraItem->currentAnimState >= 56
					&& LaraItem->currentAnimState <= 61
					&& !Lara.location)
				{
					item->goalAnimState = 13;
					break;
				}
			}
			else
			{
				if (Targetable(item, &info))
				{
					creature->flags = 1;
					if (item->objectNumber == ID_DEMIGOD2)
						item->goalAnimState = 3;
					else
						item->goalAnimState = 11;
					break;
				}

				if (item->objectNumber == ID_DEMIGOD3)
				{
					if (info.distance <= SQUARE(2048) || info.distance >= SQUARE(5120))
					{
						item->goalAnimState = 1;
						break;
					}
					if (!(GetRandomControl() & 3))
					{
						item->goalAnimState = 9;
						break;
					}
				}
			}

			if (info.distance <= SQUARE(3072) || item->objectNumber != ID_DEMIGOD2)
			{
				item->goalAnimState = 1;
				break;
			}

			item->goalAnimState = 5;

			break;

		case 1:
			creature->maximumTurn = 1274;

			if (info.distance < SQUARE(2048))
			{
				item->goalAnimState = 0;
				break;
			}

			if (item->objectNumber == ID_DEMIGOD1)
			{
				if (info.distance < SQUARE(3072))
				{
					item->goalAnimState = 0;
					break;
				}
			}
			else
			{
				if (Targetable(item, &info))
				{
					item->goalAnimState = 0;
					break;
				}

			}

			if (info.distance > SQUARE(3072))
			{
				if (item->objectNumber == ID_DEMIGOD2)
				{
					item->goalAnimState = 5;
				}
				else
				{
					item->goalAnimState = 2;
				}
			}

			break;

		case 2:
			creature->maximumTurn = 1274;

			if (info.distance < SQUARE(2048))
			{
				item->goalAnimState = 0;
				break;
			}
			if (item->objectNumber == ID_DEMIGOD1)
			{
				if (info.distance < SQUARE(3072))
				{
					item->goalAnimState = 0;
					break;
				}
			}
			else
			{
				if (Targetable(item, &info) || item->objectNumber == ID_DEMIGOD3 && info.distance > SQUARE(2048))
				{
					item->goalAnimState = 0;
					break;
				}

				if (info.distance < SQUARE(3072))
				{
					item->goalAnimState = 1;
				}
			}

			break;

		case 3:
			if (info.ahead)
			{
				joint1 = -info.xAngle;
			}

			creature->maximumTurn = 0;

			if (item->animNumber == Objects[item->objectNumber].animIndex + 6)
			{
				if (abs(info.angle) >= ANGLE(7))
				{
					if (info.angle >= 0)
					{
						item->pos.yRot += ANGLE(7);
					}
					else
					{
						item->pos.yRot -= ANGLE(7);
					}
				}
				else
				{
					item->pos.yRot += info.angle;
				}
			}

			if (Targetable(item, &info) || creature->flags)
			{
				item->goalAnimState = 4;
				creature->flags = 0;
			}
			else
			{
				item->goalAnimState = 0;
				creature->flags = 0;
			}

			break;

		case 4:
		case 12:
			DemigodEnergyAttack(itemNum);
			break;

		case 6:
			creature->maximumTurn = ANGLE(7);
			if (Targetable(item, &info))
			{
				item->goalAnimState = 7;
			}

			break;

		case 9:
			creature->maximumTurn = ANGLE(7);
			if (!Targetable(item, &info) && info.distance < SQUARE(5120))
			{
				item->goalAnimState = 10;
			}

			break;

		case 10:
			creature->maximumTurn = ANGLE(7);

			DemigodEnergyAttack(itemNum);

			if (!Targetable(item, &info) || info.distance < SQUARE(5120) || !GetRandomControl())
			{
				item->goalAnimState = 0;
				break;
			}

			break;

		case 11:
			joint2 = joint0;
			joint0 = 0;

			if (info.ahead)
			{
				joint1 = -info.xAngle;
			}

			creature->maximumTurn = 0;

			if (item->animNumber == Objects[(signed short)item->objectNumber].animIndex + 6)
			{
				if (abs(info.angle) >= ANGLE(7))
				{
					if (info.angle >= 0)
					{
						item->pos.yRot += ANGLE(7);
					}
					else
					{
						item->pos.yRot -= ANGLE(7);
					}
				}
				else
				{
					item->pos.yRot += info.angle;
				}
			}
			if (Targetable(item, &info) || creature->flags)
			{
				item->goalAnimState = 12;
				creature->flags = 0;
			}
			else
			{
				item->goalAnimState = 0;
				creature->flags = 0;
			}

			break;

		case 13:
			// Aiming
			creature->maximumTurn = 0;
			joint2 = joint0;
			joint0 = 0;
			if (abs(info.angle) >= ANGLE(7))
			{
				if (info.angle >= 0)
				{
					item->pos.yRot += ANGLE(7);
				}
				else
				{
					item->pos.yRot -= ANGLE(7);
				}
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (info.distance >= SQUARE(3072)
				|| !info.bite
				&& (LaraItem->currentAnimState < 56 || LaraItem->currentAnimState > 61 || Lara.location))
			{
				item->goalAnimState = 0;
			}
			else
			{
				item->goalAnimState = 14;
			}

			break;

		case 14:
			// Hammer attack
			if (item->frameNumber - Anims[item->animNumber].frameBase == 26)
			{
				PHD_VECTOR pos;

				pos.x = 80;
				pos.y = -8;
				pos.z = -40;

				GetJointAbsPosition(item, &pos, 17);

				short roomNumber = item->roomNumber;
				FLOOR_INFO* floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);
				int height = GetFloorHeight(floor, pos.x, pos.y, pos.z);
				if (height == -32512)
				{
					pos.y = pos.y - 128;
				}
				else
				{
					pos.y = height - 128;
				}

				TriggerShockwave((PHD_3DPOS *)&pos, 0x00580018, 256, 545292416, 0x20000, 0);
				DemigodHammerAttack(pos.x, pos.y + 128, pos.z, 8);

				Camera.bounce = -128;

				if (LaraItem->currentAnimState >= 56 && LaraItem->currentAnimState <= 61 && !Lara.location)
				{
					Lara.torsoXrot = 0; // dword_80DF6C 
					Lara.torsoYrot = 0; // dword_80DF6C 
					Lara.headXrot = 0;
					Lara.headYrot = 0;
					LaraItem->goalAnimState = 3;
					LaraItem->currentAnimState = 3;
					LaraItem->animNumber = 34;
					LaraItem->frameNumber = Anims[item->animNumber].frameBase;
					LaraItem->hitStatus = true;
					LaraItem->speed = 2;
					LaraItem->fallspeed = 1;
					Lara.gunStatus = LG_NO_ARMS;
				}
			}

		default:
			break;

		}
	}

	CreatureTilt(item, 0);

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureJoint(item, 3, joint3);

	CreatureAnimation(itemNum, angle, 0);
}

void __cdecl DemigodThrowEnergyAttack(PHD_3DPOS* pos, short roomNumber, int flags)
{
	short fxNum = CreateNewEffect(roomNumber);
	if (fxNum != -1)
	{
		FX_INFO* fx = &Effects[fxNum];

		fx->pos.xPos = pos->xPos;
		fx->pos.yPos = pos->yPos - (GetRandomControl() & 0x3F) - 32;
		fx->pos.zPos = pos->zPos;
		fx->pos.xRot = pos->xRot;
		if (flags < 4)
		{
			fx->pos.yRot = pos->yRot;
		}
		else
		{
			fx->pos.yRot = pos->yRot + (GetRandomControl() & 0x7FF) - 1024;
		}

		printf("X: %d, Y: %d\n", fx->pos.xRot, fx->pos.yRot);

		OBJECT_INFO* obj = &Objects[ID_ENERGY_BUBBLES];

		fx->pos.zRot = 0;
		fx->roomNumber = roomNumber;
		fx->counter = 2 * GetRandomControl() + -ANGLE(180);
		fx->flag1 = flags;
		fx->speed = (GetRandomControl() & 0x1F) + 96;
		fx->objectNumber = ID_ENERGY_BUBBLES;
		if (flags >= 4)
			flags--;
		fx->frameNumber = Objects[ID_ENERGY_BUBBLES].meshIndex + 2 * flags;
	}
}

void __cdecl DemigodEnergyAttack(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	short animIndex = item->animNumber - Objects[item->objectNumber].animIndex;

	if (animIndex == 8)
	{
		if (item->frameNumber == Anims[item->animNumber].frameBase)
		{
			PHD_VECTOR pos1;
			PHD_VECTOR pos2;

			pos1.x = -544;
			pos1.y = 96;
			pos1.z = 0;

			GetJointAbsPosition(item, &pos1, 16);

			pos2.x = -900;
			pos2.y = 96;
			pos2.z = 0;

			GetJointAbsPosition(item, &pos2, 16);

			short angles[2];
			phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

			PHD_3DPOS pos;
			pos.xPos = pos1.x;
			pos.yPos = pos1.y;
			pos.zPos = pos1.z;
			pos.xRot = angles[1];
			pos.yRot = angles[0];
			pos.zRot = 0;

			if (item->objectNumber == ID_DEMIGOD3)
			{
				DemigodThrowEnergyAttack(&pos, item->roomNumber, 3);
			}
			else
			{
				DemigodThrowEnergyAttack(&pos, item->roomNumber, 5);
			}
		}

		return;
	}

	if (animIndex != 16)
	{
		if (animIndex != 19)
			return;

		if (item->frameNumber == Anims[item->animNumber].frameBase)
		{
			PHD_VECTOR pos1;
			PHD_VECTOR pos2;

			pos1.x = -544;
			pos1.y = 96;
			pos1.z = 0;

			GetJointAbsPosition(item, &pos1, 16);

			pos2.x = -900;
			pos2.y = 96;
			pos2.z = 0;

			GetJointAbsPosition(item, &pos2, 16);

			short angles[2];
			phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

			PHD_3DPOS pos;
			pos.xPos = pos1.x;
			pos.yPos = pos1.y;
			pos.zPos = pos1.z;
			pos.xRot = angles[1];
			pos.yRot = angles[0];
			pos.zRot = 0;

			if (item->objectNumber == ID_DEMIGOD3)
			{
				DemigodThrowEnergyAttack(&pos, item->roomNumber, 3);
			}
			else
			{
				DemigodThrowEnergyAttack(&pos, item->roomNumber, 5);
			}
		}

		return;
	}

	// Animation 16 (State 10) is the big circle attack of DEMIGOD_3
	int frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;

	if (frameNumber >= 8 && frameNumber <= 64)
	{
		PHD_VECTOR pos1;
		PHD_VECTOR pos2;

		pos1.x = 0;
		pos1.y = 0;
		pos1.z = 192;

		pos2.x = 0;
		pos2.y = 0;
		pos2.z = 384;

		if (GlobalCounter & 1)
		{
			GetJointAbsPosition(item, &pos1, 18);
			GetJointAbsPosition(item, &pos2, 18);
		}
		else
		{
			GetJointAbsPosition(item, &pos1, 17);
			GetJointAbsPosition(item, &pos2, 17);
		}

		short angles[2];
		phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);

		PHD_3DPOS pos;
		pos.xPos = pos1.x;
		pos.yPos = pos1.y;
		pos.zPos = pos1.z;
		pos.xRot = angles[1];
		pos.yRot = angles[0];
		pos.zRot = 0;

		DemigodThrowEnergyAttack(&pos, item->roomNumber, 4);
	}
}

void __cdecl DemigodHammerAttack(int x, int y, int z, int something)
{
	int angle = 2 * GetRandomControl();
	int deltaAngle = 0x10000 / something;

	if (something > 0)
	{
		for (int i = 0; i < something; i++)
		{
			SMOKE_SPARKS* spark = &SmokeSparks[GetFreeSmokeSpark()];

			spark->On = true;
			spark->sShade = 0;
			spark->ColFadeSpeed = 4;
			spark->dShade = (GetRandomControl() & 0x1F) + 96;
			spark->FadeToBlack = 24 - (GetRandomControl() & 7);
			spark->TransType = 2;
			spark->Life = spark->sLife = (GetRandomControl() & 7) + 48;
			spark->x = (GetRandomControl() & 0x1F) + x - 16;
			spark->y = (GetRandomControl() & 0x1F) + y - 16;
			spark->z = (GetRandomControl() & 0x1F) + z - 16;
			spark->Xvel = (byte)(GetRandomControl() + 256) * SIN(angle) >> W2V_SHIFT;
			spark->Yvel = -32 - (GetRandomControl() & 0x3F);
			spark->Zvel = (byte)(GetRandomControl() + 256) * COS(angle) >> W2V_SHIFT;
			spark->Friction = 9;

			if (GetRandomControl() & 1)
			{
				spark->Flags = 16;
				spark->RotAng = GetRandomControl() & 0xFFF;
				if (GetRandomControl() & 1)
				{
					spark->RotAdd = -64 - (GetRandomControl() & 0x3F);
				}
				else
				{
					spark->RotAdd = (GetRandomControl() & 0x3F) + 64;
				}
			}
			else if (Rooms[LaraItem->roomNumber].flags & ENV_FLAG_WIND)
			{
				spark->Flags = 256;
			}
			else
			{
				spark->Flags = 0;
			}
			spark->Gravity = -4 - (GetRandomControl() & 3);
			spark->MaxYvel = -4 - (GetRandomControl() & 3);
			spark->dSize = ((GetRandomControl() & 0x3F) + 64);
			spark->sSize = spark->dSize >> 3;
			spark->Size = spark->dSize >> 3;
			
			angle += deltaAngle;
		}
	}
}