#include "framework.h"
#include "tr5_larson.h"
#include "items.h"
#include "box.h"
#include "sphere.h"
#include "effect2.h"
#include "people.h"
#include "lot.h"
#include "lara.h"
#include "setup.h"
#include "level.h"

#define STATE_TR5_LARSON_STOP		1
#define STATE_TR5_LARSON_WALK		2
#define STATE_TR5_LARSON_RUN		3
#define STATE_TR5_LARSON_AIM		4
#define STATE_TR5_LARSON_DIE		5
#define STATE_TR5_LARSON_IDLE		6
#define STATE_TR5_LARSON_ATTACK		7

#define ANIMATION_TR5_PIERRE_DIE	12
#define ANIMATION_TR5_LARSON_DIE	15

#define TR5_LARSON_MIN_HP			40

BITE_INFO LarsonGun = { -55, 200, 5, 14 };
BITE_INFO PierreGun1 = { 60, 200, 0, 11 };
BITE_INFO PierreGun2 = { -57, 200, 0, 14 };

void InitialiseLarson(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = STATE_TR5_LARSON_STOP;
	item->currentAnimState = STATE_TR5_LARSON_STOP;

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

void LarsonControl(short itemNumber)
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
	
	// In Streets of Rome when Larson HP are below 40 he runs way
	/*if (item->hitPoints <= TR5_LARSON_MIN_HP && !(item->flags & ONESHOT))
	{
		item->hitPoints = TR5_LARSON_MIN_HP;
		creature->flags++;
	}*/

	// Fire weapon effects
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
			item->status = ITEM_DESACTIVATED;
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

		// FIXME: this should make Larson running away, but it's broken
		/*if (creature->flags)
		{
			item->hitPoints = 60;
			item->gravityStatus = false;
			item->hitStatus = false;
			item->collidable = false;
			item->status = ITEM_DESACTIVATED;
			creature->flags = 0;
		}*/

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
		case STATE_TR5_LARSON_STOP:
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
				item->goalAnimState = STATE_TR5_LARSON_RUN;
			}
			else if (Targetable(item, &info))
			{
				item->goalAnimState = STATE_TR5_LARSON_AIM;
			}
			else
			{
				if (item->aiBits & GUARD || CurrentLevel == 2 || item->itemFlags[3])
				{
					creature->maximumTurn = 0;
					item->goalAnimState = STATE_TR5_LARSON_STOP;
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
							item->goalAnimState = STATE_TR5_LARSON_RUN;
						else
							item->goalAnimState = STATE_TR5_LARSON_WALK;
					}
					else
					{
						item->goalAnimState = GetRandomControl() >= 96 ? 2 : 6;
					}
				}
			}
			break;

		case STATE_TR5_LARSON_WALK:
			if (info.ahead)
				joint2 = info.angle;
			
			creature->maximumTurn = ANGLE(7);
			if (!creature->mood && GetRandomControl() < 96)
			{
				item->requiredAnimState = STATE_TR5_LARSON_IDLE;
				item->goalAnimState = STATE_TR5_LARSON_STOP;
				break;
			}

			if (creature->mood == ESCAPE_MOOD || item->aiBits & AMBUSH)
			{
				item->requiredAnimState = STATE_TR5_LARSON_RUN;
				item->goalAnimState = STATE_TR5_LARSON_STOP;
			}
			else if (Targetable(item, &info))
			{
				item->requiredAnimState = STATE_TR5_LARSON_AIM;
				item->goalAnimState = STATE_TR5_LARSON_STOP;
			}
			else if (!info.ahead || info.distance > SQUARE(3072))
			{
				item->requiredAnimState = STATE_TR5_LARSON_RUN;
				item->goalAnimState = STATE_TR5_LARSON_STOP;
			}
			break;

		case STATE_TR5_LARSON_RUN:
			if (info.ahead)
				joint2 = info.angle;
			creature->maximumTurn = ANGLE(11);
			tilt = angle / 2;

			if (creature->reachedGoal)
			{
				item->goalAnimState = STATE_TR5_LARSON_STOP;
			}
			else if (item->aiBits & AMBUSH)
			{
				item->goalAnimState = STATE_TR5_LARSON_RUN;
			}
			else if (creature->mood || GetRandomControl() >= 96)
			{
				if (Targetable(item, &info))
				{
					item->requiredAnimState = STATE_TR5_LARSON_AIM;
					item->goalAnimState = STATE_TR5_LARSON_STOP;
				}
				else if (info.ahead)
				{
					if (info.distance <= SQUARE(3072))
					{
						item->requiredAnimState = STATE_TR5_LARSON_WALK;
						item->goalAnimState = STATE_TR5_LARSON_STOP;
					}
				}
			}
			else
			{
				item->requiredAnimState = STATE_TR5_LARSON_IDLE;
				item->goalAnimState = STATE_TR5_LARSON_STOP;
			}
			break;

		case STATE_TR5_LARSON_AIM:
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
				item->goalAnimState = STATE_TR5_LARSON_ATTACK;
			else
				item->goalAnimState = STATE_TR5_LARSON_STOP;
			break;

		case STATE_TR5_LARSON_IDLE:
			joint0 = info.angle >> 1;
			joint2 = info.angle >> 1;
			if (info.ahead)
				joint1 = info.xAngle;

			if (creature->mood)
			{
				item->goalAnimState = STATE_TR5_LARSON_STOP;
			}
			else
			{
				if (GetRandomControl() <= 96)
				{
					item->requiredAnimState = STATE_TR5_LARSON_WALK;
					item->goalAnimState = STATE_TR5_LARSON_STOP;
				}
			}
			break;

		case STATE_TR5_LARSON_ATTACK:
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
				if (item->objectNumber == ID_PIERRE)
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
			if (creature->mood == ESCAPE_MOOD && GetRandomControl() > 0x2000)
				item->requiredAnimState = STATE_TR5_LARSON_STOP;
			break;

		default:
			break;

		}
	}
	else if (item->currentAnimState == STATE_TR5_LARSON_DIE)
	{
		// When Larson dies, it activates trigger at start position
		if (item->objectNumber == ID_LARSON 
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
		// Die
		if (item->objectNumber == ID_PIERRE)
			item->animNumber = Objects[ID_PIERRE].animIndex + ANIMATION_TR5_PIERRE_DIE;
		else
			item->animNumber = Objects[ID_LARSON].animIndex + ANIMATION_TR5_LARSON_DIE;
		item->currentAnimState = STATE_TR5_LARSON_DIE;
		item->frameNumber = Anims[item->animNumber].frameBase;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);

	/*if (creature->reachedGoal)
	{
		if (CurrentLevel == 2)
		{
			item->goalAnimState = STATE_TR5_LARSON_STOP;
			item->requiredAnimState = STATE_TR5_LARSON_STOP;
			creature->reachedGoal = false;
			item->gravityStatus = false;
			item->hitStatus = false;
			item->collidable = false;
			item->status = ITEM_NOT_ACTIVE;
			item->triggerFlags = 0;
		}
		else
		{
			item->hitPoints = -16384;
			DisableBaddieAI(itemNumber);
			KillItem(itemNumber);
		}
	}*/
}