#include "../newobjects.h"
#include "../../Game/Box.h"

byte dogAnims[] = { 0x14, 0x15, 0x16, 0x14 };

void InitialiseDog(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    item->currentAnimState = 1;
    item->animNumber = Objects[item->objectNumber].animIndex + 8;
    if (!item->triggerFlags)
    {
        item->animNumber = Objects[item->objectNumber].animIndex + 1;
        // TODO: item->flags2 ^= (item->flags2 ^ ((item->flags2 & 0xFE) + 2)) & 6;
    }
    item->frameNumber = Anims[item->animNumber].frameBase;
}

void DogControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	short angle = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;

	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	if (item->hitPoints <= 0)
	{
		if (item->animNumber == obj->animIndex + 1)
		{
			item->hitPoints = obj->hitPoints;
		}
		else if (item->currentAnimState != 11)
		{
			item->animNumber = obj->animIndex+ dogAnims[GetRandomControl() & 3];
			item->currentAnimState = 11;
			item->frameNumber = Anims[item->animNumber].frameBase;
		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);
		else
			creature->enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item, &info);

		int distance;
		if (creature->enemy == LaraItem)
		{
			distance = info.distance;
		}
		else
		{
			int dx = LaraItem->pos.xPos - item->pos.xPos;
			int dz = LaraItem->pos.zPos - item->pos.zPos;
			ATAN(dz, dx);
			distance = SQUARE(dx) + SQUARE(dz);
		}

		if (info.ahead)
		{
			joint2 = info.xAngle; // Maybe swapped
			joint1 = info.angle;
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (!creature->mood)
			creature->maximumTurn >>= 1;

		angle = CreatureTurn(item, creature->maximumTurn);
		joint0 = 4 * angle;


		if (creature->hurtByLara || distance < 0x900000 && !(item->aiBits & MODIFY)) 
		{
			AlertAllGuards(itemNumber);
			item->aiBits &= ~MODIFY;
		}

#if OLD_CODE
		v14 = GetRandomControl();
		v15 = item->frameNumber - anims[item->animNumber].frame_base;
		switch (item->currentAnimState)
		{
		case 0:
		case 8:
			joint1 = 0;
			joint2 = 0;
			if (creature->mood && (item->_bf15ea & 0x3E00) != 4096)
			{
				item->goal_anim_state = 1;
			}
			else
			{
				v18 = __OFSUB__(++creature->flags, 300);
				v16 = creature->flags == 300;
				v17 = (creature->flags - 300) < 0;
				creature->maximumTurn = 0;
				if (!((v17 ^ v18) | v16) && v14 < 128)
					item->goal_anim_state = 1;
			}
			goto LABEL_100;
		case 1:
			goto LABEL_30;
		case 2:
			creature->maximumTurn = 546;
			v24 = item->_bf15ea;
			if (v24 & 0x800)
			{
				item->goal_anim_state = 2;
				goto LABEL_100;
			}
			if (!creature->mood && v14 < 256)
				goto LABEL_63;
			item->goal_anim_state = 5;
			goto LABEL_100;
		case 3:
			creature->maximumTurn = 1092;
			v25 = creature->mood;
			if (v25 == 2)
			{
				if (lara.target != item && v33)
					item->goal_anim_state = 9;
			}
			else if (v25)
			{
				if (v34 && v32 < 0x100000)
				{
					item->goal_anim_state = 6;
				}
				else if (v32 < 2359296)
				{
					item->required_anim_state = 5;
					item->goal_anim_state = 9;
				}
			}
			else
			{
				item->goal_anim_state = 9;
			}
			goto LABEL_100;
		case 5:
			creature->maximumTurn = 546;
			v26 = creature->mood;
			if (v26)
			{
				if (v26 == 2)
				{
					item->goal_anim_state = 3;
				}
				else if (v34 && v32 < 116281)
				{
					item->goal_anim_state = 12;
					item->required_anim_state = 5;
				}
				else if (v32 > 2359296 || item->_bf15ea & 0x10)
				{
					item->goal_anim_state = 3;
				}
			}
			else
			{
				item->goal_anim_state = 9;
			}
			goto LABEL_100;
		case 6:
			if (v34 && item->touch_bits & 0x6648 && v15 >= 4 && v15 <= 14)
			{
				CreatureEffectOld(item, &unk_508518, 2, -1, DoBloodSplat);
				LaraItem->hitPoints -= 20;
				LaraItem->_bf15ea |= 0x10u;
			}
			item->goal_anim_state = 3;
			goto LABEL_100;
		case 7:
			joint1 = 0;
			joint2 = 0;
			goto LABEL_100;
		case 9:
			v19 = item->required_anim_state;
			if (v19)
			{
				item->goal_anim_state = v19;
				goto LABEL_100;
			}
		LABEL_30:
			creature->maximumTurn = 0;
			v20 = item->_bf15ea;
			if (v20 & 0x200)
			{
				joint1 = AIGuard(creature);
				if (GetRandomControl())
					goto LABEL_100;
				if (item->currentAnimState == 1)
				{
					item->goal_anim_state = 9;
					goto LABEL_100;
				}
			}
			else
			{
				v21 = item->currentAnimState;
				if (v21 == 9 && v14 < 128)
				{
					item->goal_anim_state = 1;
					goto LABEL_100;
				}
				if (v20 & 0x800)
				{
					if (v21 == 1)
						item->goal_anim_state = 2;
					else
						LABEL_63:
					item->goal_anim_state = 1;
					goto LABEL_100;
				}
				v22 = creature->mood;
				if (v22 == 2)
				{
					if (lara.target == item || !v33 || v20 & 0x10)
					{
						item->required_anim_state = 3;
						item->goal_anim_state = 9;
					}
					else
					{
						item->goal_anim_state = 1;
					}
					goto LABEL_100;
				}
				if (v22)
				{
					item->required_anim_state = 3;
					if (v21 == 1)
						item->goal_anim_state = 9;
					goto LABEL_100;
				}
				creature->flags = 0;
				creature->maximumTurn = 182;
				if (v14 < 256)
				{
					v23 = item->_bf15ea;
					if (v23 & 0x1000)
					{
						if (item->currentAnimState == 1)
						{
							item->goal_anim_state = 8;
							creature->flags = 0;
							goto LABEL_100;
						}
					}
				}
				if (v14 >= 4096)
				{
					if (!(v14 & 0x1F))
						item->goal_anim_state = 7;
					goto LABEL_100;
				}
				if (item->currentAnimState == 1)
				{
					item->goal_anim_state = 2;
					goto LABEL_100;
				}
			}
			item->goal_anim_state = 1;
			goto LABEL_100;
		case 12:
			if (v34 && item->touch_bits & 0x48 && (v15 >= 9 && v15 <= 12 || v15 >= 22 && v15 <= 25))
			{
				CreatureEffectOld(item, &unk_508518, 2, -1, DoBloodSplat);
				LaraItem->hitPoints -= 10;
				LaraItem->_bf15ea |= 0x10u;
			}
			goto LABEL_100;
		default:
			goto LABEL_100;
		}

#endif
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);
}