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
#include "creature.h"
#include "effect.h"
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
constexpr auto BAT_TARGET_YPOS = 896;
constexpr auto BAT_DAMAGE = 2;
static BITE_INFO batBite(0, 16, 45, 4);

static bool isBatCollideTarget(ITEM_INFO* item)
{
	return item->touchBits >= 0;
}

void InitialiseBat(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	InitialiseCreature(itemNumber);

	item->animNumber = Objects[item->objectNumber].animIndex + BAT_ANIM_IDLE;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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

	item = &g_Level.Items[itemNumber];
	bat = GetCreatureInfo(item);
	angle = 0;

    if (item->hitPoints > 0)
    {
        if (item->aiBits)
        {
            GetAITarget(bat);
        }
        else
        {
            bat->enemy = LaraItem;

            // NOTE: it seems weird, bat could target any enemy including dogs for example
            
            // check if voncroy are in range !
            // take in account that the bat will always target voncroy if he exist and triggered !
            // the bat will ignore lara completly !
            /*bestdistance = MAXINT;
            bat->enemy = LaraItem;

            slots = &BaddieSlots[0];
            for (int i = 0; i < NUM_SLOTS; i++, slots++)
            {
				if (slots->itemNum != NO_ITEM && slots->itemNum != itemNumber)
				{
					target = &g_Level.Items[slots->itemNum];
					if (target->objectNumber == ID_VON_CROY)
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
            }*/
        }

        // NOTE: chaned from TIMID to VIOLENT, otherwise the bat seems to ignore Lara. 
        // Personally, i feel fine with bat always VIOLENT, but I will inspect also GetCreatureMood and CreatureMood functions
        // for bugs.

        CreatureAIInfo(item, &info);
        GetCreatureMood(item, &info, VIOLENT);
        
        if (bat->flags)
            bat->mood = ESCAPE_MOOD;
       
        CreatureMood(item, &info, VIOLENT);
        
        angle = CreatureTurn(item, BAT_ANGLE);

        switch (item->currentAnimState)
        {
        case BAT_IDLE:
            if (info.distance < BAT_TARGETING_RANGE 
                || item->hitStatus 
                || bat->hurtByLara)
                item->goalAnimState = BAT_START;
            break;

        case BAT_FLY:
            if (info.distance < BAT_ATTACK_RANGE || !(GetRandomControl() & 0x3F))
                bat->flags = 0;

            if (!bat->flags)
            {
                if (item->touchBits
                    || bat->enemy != LaraItem
                    && info.distance < BAT_ATTACK_RANGE
                    && info.ahead
                    && abs(item->pos.yPos - bat->enemy->pos.yPos) < BAT_TARGET_YPOS)
                {
                    item->goalAnimState = BAT_ATK;
                }
            }
            break;

        case BAT_ATK:
            if (!bat->flags 
                && (item->touchBits 
                || bat->enemy != LaraItem) 
                && info.distance < BAT_ATTACK_RANGE
                && info.ahead && 
                abs(item->pos.yPos - bat->enemy->pos.yPos) < BAT_TARGET_YPOS)
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
    else if (item->currentAnimState == BAT_ATK)
    {
        item->animNumber = Objects[item->objectNumber].animIndex + 1;
        item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
        item->goalAnimState = BAT_FLY;
        item->currentAnimState = BAT_FLY;
    }
    else
    {
        if (item->pos.yPos >= item->floor)
        {
            item->goalAnimState = BAT_DEATH;
            item->pos.yPos = item->floor;
            item->gravityStatus = false;
        }
        else
        {
            item->gravityStatus = true;
            item->animNumber = Objects[item->objectNumber].animIndex + BAT_ANIM_FALLING;
            item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
            item->goalAnimState = BAT_FALLING;
            item->currentAnimState = BAT_FALLING;
            item->speed = 0;
        }
    }

    CreatureAnimation(itemNumber, angle, 0);
}