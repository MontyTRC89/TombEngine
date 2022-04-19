#include "framework.h"
#include "tr4_hammerhead.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Game/itemdata/creature_info.h"

#define STATE_HAMMERHEAD_STOP       0
#define STATE_HAMMERHEAD_SWIM_SLOW  1
#define STATE_HAMMERHEAD_SWIM_FAST  2
#define STATE_HAMMERHEAD_ATTACK     3
#define STATE_HAMMERHEAD_DEATH      5
#define STATE_HAMMERHEAD_KILL       6

BITE_INFO HammerheadAttack = { 0, 0, 0, 12 };

void InitialiseHammerhead(short itemNumber)
{
    ITEM_INFO* item = &g_Level.Items[itemNumber];

    ClearItem(itemNumber);

    item->animNumber = Objects[item->objectNumber].animIndex + 8;
    item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
    item->goalAnimState = STATE_HAMMERHEAD_STOP;
    item->currentAnimState = STATE_HAMMERHEAD_STOP;
}

void HammerheadControl(short itemNumber)
{
    if (CreatureActive(itemNumber))
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];
        CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

        if (item->hitPoints > 0)
        {
            if (item->aiBits)
                GetAITarget(creature);
            else if (creature->hurtByLara)
                creature->enemy = LaraItem;

            AI_INFO info;
            CreatureAIInfo(item, &info);

            if (creature->enemy != LaraItem)
            {
                phd_atan(LaraItem->pos.zPos - item->pos.zPos, LaraItem->pos.xPos - item->pos.xPos);
            }

            GetCreatureMood(item, &info, VIOLENT);
            CreatureMood(item, &info, VIOLENT);

            short angle = CreatureTurn(item, creature->maximumTurn);

            switch (item->currentAnimState)
            {
            case STATE_HAMMERHEAD_STOP:
                item->goalAnimState = STATE_HAMMERHEAD_SWIM_SLOW;
                creature->flags = 0;
                break;

            case STATE_HAMMERHEAD_SWIM_SLOW:
                creature->maximumTurn = ANGLE(7);
                if (info.distance <= SQUARE(1024))
                {
                    if (info.distance < SQUARE(682))
                    {
                        item->goalAnimState = STATE_HAMMERHEAD_ATTACK;
                    }
                }
                else
                {
                    item->goalAnimState = STATE_HAMMERHEAD_SWIM_FAST;
                }
                break;

            case STATE_HAMMERHEAD_SWIM_FAST:
                if (info.distance < SQUARE(1024))
                {
                    item->goalAnimState = STATE_HAMMERHEAD_SWIM_SLOW;
                }
                break;

            case STATE_HAMMERHEAD_ATTACK:
                if (!creature->flags)
                {
                    if (item->touchBits & 0x3400)
                    {
                        LaraItem->hitPoints -= 120;
                        LaraItem->hitStatus = true;
                        CreatureEffect(item, &HammerheadAttack, DoBloodSplat);
                        creature->flags = 1;
                    }
                }
                break;

            default:
                break;

            }

            CreatureTilt(item, 0);
            CreatureJoint(item, 0, -2 * angle);
            CreatureJoint(item, 1, -2 * angle);
            CreatureJoint(item, 2, -2 * angle);
            CreatureJoint(item, 3, 2 * angle);

            // NOTE: in TR2 shark there was a call to CreatureKill with special kill anim
            // Hammerhead seems to not have it in original code but this check is still there as a leftover
            if (item->currentAnimState == STATE_HAMMERHEAD_KILL)
            {
                AnimateItem(item);
            }
            else
            {
                CreatureAnimation(itemNumber, angle, 0);
                CreatureUnderwater(item, 341);
            }
        }
        else
        {
            item->hitPoints = 0;
            if (item->currentAnimState != STATE_HAMMERHEAD_DEATH)
            {
                item->animNumber = Objects[item->objectNumber].animIndex + 4;
                item->currentAnimState = STATE_HAMMERHEAD_DEATH;
                item->frameNumber = g_Level.Anims[item->frameNumber].frameBase;
            }

            CreatureFloat(itemNumber);
        }
    }
}
