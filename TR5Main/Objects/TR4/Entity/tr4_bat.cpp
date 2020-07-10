#include "framework.h"
#include "tr4_bat.h"
#include "box.h"
#include "control.h"
#include "effect2.h"
#include "misc.h"
#include "lara.h"
#include "lot.h"
#include "setup.h"
#include "trmath.h"

enum BAT_STATE
{
	BAT_EMPTY,
	BAT_START,
	BAT_FLY,
	BAT_ATK,
	BAT_FALLING,
	BAT_DEATH,
	BAT_IDLE
};

#define BAT_ANGLE ANGLE(20.0f)
constexpr auto BAT_ANIM_FALLING = 3;
constexpr auto BAT_ANIM_IDLE = 5;
constexpr auto BAT_ATTACK_RANGE = SQUARE(CLICK(1));
constexpr auto BAT_TARGETING_RANGE = SQUARE(SECTOR(5));
constexpr auto BAT_TARGET_YPOS = SQUARE(CLICK(2) / 18);
constexpr auto BAT_DAMAGE = 2;
static BITE_INFO batBite(0, 16, 45, 4);

static bool isBatCollideTarget(ITEM_INFO* item)
{
	return item->touchBits >= 0;
}

void InitialiseBat(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	InitialiseCreature(itemNumber);
	item->animNumber = Objects[item->objectNumber].animIndex + BAT_ANIM_IDLE;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = BAT_IDLE;
	item->currentAnimState = BAT_IDLE;
}

void BatControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item, *target;
	CREATURE_INFO* bat, *slots;
	AI_INFO info;
	int distance, bestdistance;
	short angle;

	item = &Items[itemNumber];
	bat = GetCreatureInfo(item);
	angle = 0;

    if (item->hitPoints <= 0)
    {
        if (item->pos.yPos >= item->floor)
        {
            item->goalAnimState = BAT_DEATH;
            item->pos.yPos = item->floor;
            item->gravityStatus = false;
        }
        else
        {
            item->animNumber = Objects[item->objectNumber].animIndex + BAT_ANIM_FALLING;
            item->frameNumber = Anims[item->animNumber].frameBase;
            item->goalAnimState = BAT_FALLING;
            item->currentAnimState = BAT_FALLING;
            item->speed = 0;
        }
    }
    else
    {
        if (item->aiBits & ALL_AIOBJ)
        {
            GetAITarget(bat);
        }
        else
        {
            // check if voncroy are in range !
            // take in account that the bat will always target voncroy if he exist and triggered !
            // the bat will ignore lara completly !
            bestdistance = MAXINT;
            bat->enemy = LaraItem;

            slots = &BaddieSlots[0];
            for (int i = 0; i < NUM_SLOTS; i++, slots++)
            {
                target = &Items[slots->itemNum];
                if (target->objectNumber == ID_VON_CROY && target->status != ITEM_INVISIBLE)
                {
                    int x, z;
                    x = target->pos.xPos - item->pos.xPos;
                    z = target->pos.zPos - item->pos.zPos;
                    distance = SQUARE(x) + SQUARE(z);
                    if (distance < bestdistance)
                    {
                        bat->enemy = target;
                        bestdistance = distance;
                    }
                }
            }
        }

        CreatureAIInfo(item, &info);
        GetCreatureMood(item, &info, TIMID);
        // note: this random dont exist in TR4 !
        // this part set the bat in escape mood, but for too long !
        // lara can escape and shot it easy ....
        if (bat->flags && !(GetRandomControl() & 16))
            bat->mood = ESCAPE_MOOD;
        else if (bat->flags && GetRandomControl() & 24) // fine maybe ? the bat react more with "16" but this reaction is too fast !
            bat->mood = STALK_MOOD;
        CreatureMood(item, &info, TIMID);
        angle = CreatureTurn(item, BAT_ANGLE);

        switch (item->currentAnimState)
        {
        case BAT_IDLE:
            if (info.distance < BAT_TARGETING_RANGE || item->hitStatus || bat->hurtByLara)
                item->goalAnimState = BAT_START;
            break;
        case BAT_FLY:
            if (info.distance < BAT_ATTACK_RANGE || !(GetRandomControl() & 0x3F))
                bat->flags = 0;

            if (!bat->flags)
            {
                if (isBatCollideTarget(item) || bat->enemy != LaraItem)
                {
                    if (info.distance < BAT_ATTACK_RANGE
                        && info.ahead && abs(item->pos.yPos - bat->enemy->pos.yPos) < BAT_TARGET_YPOS)
                    {
                        item->goalAnimState = BAT_ATK;
                    }
                }
            }
            break;
        case BAT_ATK:
            if (!bat->flags && (isBatCollideTarget(item) || bat->enemy != LaraItem) && info.distance < BAT_ATTACK_RANGE && info.ahead && abs(item->pos.yPos - bat->enemy->pos.yPos) < BAT_TARGET_YPOS)
            {
                CreatureEffect(item, &batBite, DoBloodSplat);
                if (bat->enemy == LaraItem)
                {
                    LaraItem->hitPoints -= BAT_DAMAGE;
                    LaraItem->hitStatus = true;
                }
                bat->flags = 1;
            }
            else
            {
                item->goalAnimState = BAT_FLY;
                bat->mood = BORED_MOOD;
            }
            break;
        }
    }

    CreatureAnimation(itemNumber, angle, 0);
}