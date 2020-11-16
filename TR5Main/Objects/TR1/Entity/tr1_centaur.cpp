#include "framework.h"
#include "box.h"
#include "items.h"
#include "level.h"
#include "lara.h"
#include "people.h"
#include "sound.h"
#include "tomb4fx.h"
#include "tr1_centaur.h"

enum centaur_anims { 
	CENTAUR_EMPTY, 
	CENTAUR_STOP, 
	CENTAUR_SHOOT, 
	CENTAUR_RUN, 
	CENTAUR_AIM, 
	CENTAUR_DEATH, 
	CENTAUR_WARNING};

BITE_INFO centaur_rocket = { 11, 415, 41, 13 };
BITE_INFO centaur_rear = { 50, 30, 0, 5 };

#define CENTAUR_TOUCH 0x30199

#define CENTAUR_DIE_ANIM 8

#define CENTAUR_TURN ANGLE(4)

#define CENTAUR_REAR_CHANCE 0x60

#define CENTAUR_REAR_RANGE SQUARE(WALL_SIZE*3/2)

#define FLYER_PART_DAMAGE 100

#define CENTAUR_REAR_DAMAGE 200

short RocketGun(long long x, long long y, long long z, short speed, short yrot, short room_number)
{
	// MISSILE3
	short fx_number;
	FX_INFO *fx;

	fx_number = CreateItem();
	if (fx_number != NO_ITEM)
	{
		fx = &EffectList[fx_number];
		fx->pos.xPos = x;
		fx->pos.yPos = y;
		fx->pos.zPos = z;
		fx->roomNumber = room_number;
		fx->pos.xRot = fx->pos.zRot = 0;
		fx->pos.yRot = yrot;
		fx->speed = 220;
		fx->frameNumber = 0;
//		fx->objectNumber = MISSILE3;
		fx->objectNumber = ID_HARPOON; //whatever
		fx->shade = 16 * 256;
//		ShootAtLara(fx);
	}

	return (fx_number);
}

void CentaurControl(short itemNum)
{
	ITEM_INFO *item;
	CREATURE_INFO *centaur;
	short angle, head, fx_number;
	AI_INFO info;

	item = &g_Level.Items[itemNum];

	if (!CreatureActive(itemNum))
		return;

	centaur = (CREATURE_INFO *)item->data;
	head = angle = 0;

	/* Has centaur been killed? */
	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != CENTAUR_DEATH)
		{
			item->animNumber = Objects[ID_CENTAUR_MUTANT].animIndex + CENTAUR_DIE_ANIM;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = CENTAUR_DEATH;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, CENTAUR_TURN);

		switch (item->currentAnimState)
		{
		case CENTAUR_STOP:
			CreatureJoint(item, 17, 0);
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (info.bite && info.distance < CENTAUR_REAR_RANGE)
				item->goalAnimState = CENTAUR_RUN;
			else if (Targetable(item, &info))
				item->goalAnimState = CENTAUR_AIM;
			else
				item->goalAnimState = CENTAUR_RUN;
			break;

		case CENTAUR_RUN:
			if (info.bite && info.distance < CENTAUR_REAR_RANGE)
			{
				item->requiredAnimState = CENTAUR_WARNING;
				item->goalAnimState = CENTAUR_STOP;
			}
			else if (Targetable(item, &info))
			{
				item->requiredAnimState = CENTAUR_AIM;
				item->goalAnimState = CENTAUR_STOP;
			}
			else if (GetRandomControl() < CENTAUR_REAR_CHANCE)
			{
				item->requiredAnimState = CENTAUR_WARNING;
				item->goalAnimState = CENTAUR_STOP;
			}
			break;

		case CENTAUR_AIM:
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (Targetable(item, &info))
				item->goalAnimState = CENTAUR_SHOOT;
			else
				item->goalAnimState = CENTAUR_STOP;
			break;

		case CENTAUR_SHOOT:
			if (!item->requiredAnimState)
			{
				item->requiredAnimState = CENTAUR_AIM;

//				fx_number = CreatureEffect(item, &centaur_rocket, RocketGun);
				fx_number = CreatureEffect(item, &centaur_rocket, RocketGun);
				if (fx_number != NO_ITEM)
					CreatureJoint(item, 17, EffectList[fx_number].pos.xRot);
				/*shoot*/
			}
			break;

		case CENTAUR_WARNING:
			if (!item->requiredAnimState && (item->touchBits & CENTAUR_TOUCH))
			{
				CreatureEffect(item, &centaur_rear, DoBloodSplat);

				LaraItem->hitPoints -= CENTAUR_REAR_DAMAGE;
				LaraItem->hitStatus = 1;

				item->requiredAnimState = CENTAUR_STOP;
			}
			break;
		}
	}

	CreatureJoint(item, 0, head);

	/* Actually do animation allowing for collisions */
	CreatureAnimation(itemNum, angle, 0);

	/* Explode on death */
	if (item->status == ITEM_DEACTIVATED)
	{
		SoundEffect(171, &item->pos, NULL);
		ExplodingDeath(itemNum, 0xffffffff, FLYER_PART_DAMAGE);
		KillItem(itemNum);
		item->status = ITEM_DEACTIVATED;
	}
}
