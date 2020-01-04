#include "../newobjects.h"
#include "../../Game/items.h"
#include "../../Game/Box.h"
#include "../../Game/sphere.h"
#include "../../Game/effect2.h"
#include "../../Game/people.h"
#include "../../Game/lot.h"

BITE_INFO LarsonGun = { -55, 200, 5, 14 };
BITE_INFO PierreGun1 = { 60, 200, 0, 11 };
BITE_INFO PierreGun2 = { -57, 200, 0, 14 };

void InitialiseLarson(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;

	if (!item->triggerFlags)
		return;

	item->itemFlags[3] = item->triggerFlags;
	short rotY = item->pos.yRot;

	if (rotY > 4096 && rotY < 28672)
	{
		item->pos.xPos += STEPUP_HEIGHT;
	}
	else if (rotY < -4096 && rotY > -28672)
	{
		item->pos.xPos -= STEPUP_HEIGHT;
	}
	else if (rotY < -20480 || rotY > 20480)
	{
		item->pos.zPos -= STEPUP_HEIGHT;
	}
	else if (rotY > -8192 || rotY < 8192)
	{
		item->pos.zPos += STEPUP_HEIGHT;
	}
}

void ControlLarson(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;
	
	short joint0 = 0;
	short tilt = 0;
	short angle = 0;
	short joint2 = 0;
	short joint1 = 0;
	
	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	
	if (item->hitPoints <= 40 && !(item->flags & 0x100))
	{
		item->hitPoints = 40;
		creature->flags++;
	}

	if (item->firedWeapon)
	{
		PHD_VECTOR pos;

		pos.x = LarsonGun.x;
		pos.y = LarsonGun.y;
		pos.z = LarsonGun.z;

		GetJointAbsPosition(item, &pos, LarsonGun.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * item->firedWeapon + 10, 192, 128, 32);
		
		item->firedWeapon--;
	}

	if (item->triggerFlags)
	{
		if (CurrentLevel == 2)
		{
			item->itemFlags[3] = 1;
			item->gravityStatus = false;
			item->hitStatus = false;
			item->collidable = false;
			item->status = ITEM_DEACTIVATED;
		}
		else
		{
			item->gravityStatus = false;
			item->hitStatus = false;
			item->collidable = false;
			item->status = ITEM_ACTIVE;
		}
		item->triggerFlags = 0;
	}

	if (item->hitPoints > 0)
	{
		if (item->aiBits)
			GetAITarget(creature);
		else
			creature->enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			joint2 = info.angle;

		if (creature->flags)
		{
			item->hitPoints = 60;
			item->gravityStatus = false;
			item->hitStatus = false;
			item->collidable = false;
			item->status = ITEM_DEACTIVATED;
			creature->flags = 0;
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (info.distance < SQUARE(2048) 
			&& LaraItem->speed > 20
			|| item->hitStatus
			|| TargetVisible(item, &info) != 0)
		{
			item->status &= ~ITEM_ACTIVE;
			creature->alerted = true;
		}

		angle = CreatureTurn(item, creature->maximumTurn);
		
		switch (item->currentAnimState)
		{
		case 1:
			joint0 = info.angle >> 1;
			joint2 = info.angle >> 1;
			if (info.ahead)
				joint1 = info.xAngle;

			if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else if (item->aiBits & AMBUSH)
			{
				item->goalAnimState = 3;
			}
			else if (Targetable(item, &info))
			{
				item->goalAnimState = 4;
			}
			else
			{
				if (item->aiBits & GUARD || CurrentLevel == 2 || item->itemFlags[3])
				{
					creature->maximumTurn = 0;
					item->goalAnimState = 1;
					if (abs(info.angle) >= ANGLE(2))
					{
						if (info.angle > 0)
							item->pos.yRot += ANGLE(2);
						else
							item->pos.yRot -= ANGLE(2);
					}
					else
					{
						item->pos.yRot += info.angle;
					}
				}
				else
				{
					if (creature->mood)
					{
						if (creature->mood == ESCAPE_MOOD)
							item->goalAnimState = 3;
						else
							item->goalAnimState = 2;
					}
					else
					{
						item->goalAnimState = GetRandomControl() >= 96 ? 2 : 6;
					}
				}
			}
			break;

		case 2:
			if (info.ahead)
				joint2 = info.angle;
			
			creature->maximumTurn = ANGLE(7);
			if (!creature->mood && GetRandomControl() < 96)
			{
				item->requiredAnimState = 6;
				item->goalAnimState = 1;
				break;
			}

			if (creature->mood == ESCAPE_MOOD || item->aiBits & AMBUSH)
			{
				item->requiredAnimState = 3;
				item->goalAnimState = 1;
			}
			else if (Targetable(item, &info))
			{
				item->requiredAnimState = 4;
				item->goalAnimState = 1;
			}
			else if (!info.ahead || info.distance > SQUARE(3072))
			{
				item->requiredAnimState = 3;
				item->goalAnimState = 1;
			}
			break;

		case 3:
			if (info.ahead)
				joint2 = info.angle;
			creature->maximumTurn = ANGLE(11);
			tilt = angle / 2;

			if (creature->reachedGoal)
			{
				item->goalAnimState = 1;
			}
			else if (item->aiBits & AMBUSH)
			{
				item->goalAnimState = 3;
			}
			else if (creature->mood || GetRandomControl() >= 96)
			{
				if (Targetable(item, &info))
				{
					item->requiredAnimState = 4;
					item->goalAnimState = 1;
				}
				else if (info.ahead)
				{
					if (info.distance <= SQUARE(3072))
					{
						item->requiredAnimState = 2;
						item->goalAnimState = 1;
					}
				}
			}
			else
			{
				item->requiredAnimState = 6;
				item->goalAnimState = 1;
			}
			break;

		case 4:
			joint0 = info.angle >> 1;
			joint2 = info.angle >> 1;
			if (info.ahead)
				joint1 = info.xAngle;
			creature->maximumTurn = 0;
			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle > 0)
					item->pos.yRot += ANGLE(2);
				else
					item->pos.yRot -= ANGLE(2);
			}
			else
			{
				item->pos.yRot += info.angle;
			}

			if (Targetable(item, &info))
				item->goalAnimState = 7;
			else
				item->goalAnimState = 1;
			break;

		case 6:
			joint0 = info.angle >> 1;
			joint2 = info.angle >> 1;
			if (info.ahead)
				joint1 =info.xAngle;

			if (creature->mood)
			{
				item->goalAnimState = 1;
			}
			else
			{
				if (GetRandomControl() <= 96)
				{
					item->requiredAnimState = 2;
					item->goalAnimState = 1;
				}
			}
			break;

		case 7:
			joint0 = info.angle >> 1;
			joint2 = info.angle >> 1;
			if (info.ahead)
				joint1 = info.xAngle;
			creature->maximumTurn = 0;
			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle > 0)
					item->pos.yRot += ANGLE(2);
				else
					item->pos.yRot -= ANGLE(2);
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (item->frameNumber == Anims[item->animNumber].frameBase)
			{
				if (item->objectNumber == ID_TR5_PIERRE)
				{
					ShotLara(item, &info, &PierreGun1, joint0, 20);
					ShotLara(item, &info, &PierreGun2, joint0, 20);
				}
				else
				{
					ShotLara(item, &info, &LarsonGun, joint0, 20);
				}
				item->firedWeapon = 2;
			}
			if (creature->mood == 2 && GetRandomControl() > 0x2000)
				item->requiredAnimState = 1;
			break;

		default:
			break;

		}
	}
	else if (item->currentAnimState == 5)
	{
		if (item->objectNumber == ID_TR5_LARSON 
			&& item->frameNumber == Anims[item->animNumber].frameEnd)
		{
			short roomNumber= item->itemFlags[2] & 0xFF;
			short floorHeight = item->itemFlags[2] & 0xFF00;
			ROOM_INFO* r = &Rooms[roomNumber];
			
			int x = r->x + (((item->TOSSPAD >> 8) & 0xFF) << WALL_SHIFT) + 512;
			int y = r->minfloor + floorHeight;
			int z = r->z + ((item->TOSSPAD & 0xFF) << WALL_SHIFT) + 512;

			FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
			GetFloorHeight(floor, x, y, z);
			TestTriggers(TriggerIndex, 1, 0);

			joint0 = 0;
		}
	}
	else
	{
		if (item->objectNumber == ID_TR5_PIERRE)
			item->animNumber = Objects[ID_TR5_PIERRE].animIndex + 12;
		else
			item->animNumber = Objects[ID_TR5_LARSON].animIndex + 15;
		item->currentAnimState = 5;
		item->frameNumber = Anims[item->animNumber].frameBase;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);

	if (creature->reachedGoal)
	{
		if (CurrentLevel == 2)
		{
			item->goalAnimState = 1;
			item->requiredAnimState = 1;
			creature->reachedGoal = false;
			item->gravityStatus = false;
			item->hitStatus = false;
			item->collidable = false;
			item->status = ITEM_INACTIVE;
			item->triggerFlags = 0;
		}
		else
		{
			item->hitPoints = -16384;
			DisableBaddieAI(itemNumber);
			KillItem(itemNumber);
		}
	}
}