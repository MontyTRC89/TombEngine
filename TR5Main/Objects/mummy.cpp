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

BITE_INFO mummyBite1 = { 0, 0, 0, 11 };
BITE_INFO mummyBite2 = { 0, 0, 0, 14 };

void __cdecl InitialiseMummy(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	if (item->triggerFlags == 2)
	{
		item->animNumber = Objects[ID_MUMMY].animIndex + 12;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 8;
		item->currentAnimState = 8;
		item->status = ITEM_INVISIBLE;
	}
	else
	{
		item->animNumber = Objects[ID_MUMMY].animIndex + 19;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 0;
		item->currentAnimState = 0;
	}
}

void __cdecl MummyControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short tilt = 0;
	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	if (item->aiBits)
		GetAITarget(creature);
	else if (creature->hurtByLara)
		creature->enemy = LaraItem;

	AI_INFO info;
	CreatureAIInfo(item, &info);

	if (item->hitStatus)
	{
		if (info.distance < SQUARE(3072))
		{
			if (item->currentAnimState != 7 && item->currentAnimState != 5 && item->currentAnimState != 8)
			{
				if (GetRandomControl() & 3 || Lara.gunType != 4 && Lara.gunType != 5 && Lara.gunType != 2)
				{
					if (!(GetRandomControl() & 7) || Lara.gunType == 4 || Lara.gunType == 5 || Lara.gunType == 2)
					{
						if (item->currentAnimState == 3 || item->currentAnimState == 4)
						{
							item->currentAnimState = 6;
							item->animNumber = Objects[ID_MUMMY].animIndex + 20;
						}
						else
						{
							item->currentAnimState = 5;
							item->animNumber = Objects[ID_MUMMY].animIndex + 3;
						}
						item->frameNumber = Anims[item->animNumber].frameBase;
						item->pos.yRot += info.angle;
					}
				}
				else
				{
					item->animNumber = Objects[ID_MUMMY].animIndex + 10;
					item->frameNumber = Anims[item->animNumber].frameBase;
					item->currentAnimState = 7;
					item->pos.yRot += info.angle;
					creature->maximumTurn = 0;
				}
				
				CreatureTilt(item, 0);
				CreatureJoint(item, 0, joint0);
				CreatureJoint(item, 1, joint1);
				CreatureJoint(item, 2, joint2);

				CreatureAnimation(itemNum, angle, 0);

				return;
			}
		}
	}

	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	angle = CreatureTurn(item, creature->maximumTurn);

	if (info.ahead)
	{
		joint0 = info.angle >> 1;
		joint2 = info.angle >> 1;
		joint1 = info.xAngle;
	}

	switch (item->currentAnimState)
	{
	case 1:
		creature->flags = 0;
		creature->maximumTurn = 0;

		if (info.distance <= SQUARE(512) || info.distance >= SQUARE(7168))
		{
			if (info.distance - SQUARE(512) <= 0)
				item->goalAnimState = 10;
			else
			{
				item->goalAnimState = 1;
				joint0 = 0;
				joint1 = 0;
				joint2 = 0;
				if (item->triggerFlags > -100 && (item->triggerFlags & 0x8000) != 0)
					item->triggerFlags++;
			}
		}
		else
			item->goalAnimState = 2;
		break;

	case 2:
		if (item->triggerFlags == 1)
		{
			creature->maximumTurn = 0;
			if (item->frameNumber == Anims[item->animNumber].frameEnd)
				item->triggerFlags = 0;
		}
		else
		{
			creature->maximumTurn = ANGLE(7);
			if (info.distance >= SQUARE(3072))
			{
				if (info.distance > SQUARE(7168))
				{
					item->goalAnimState = 1;
				}
			}
			else
			{
				item->goalAnimState = 3;
			}
		}
		break;

	case 3:
		creature->flags = 0;
		creature->maximumTurn = ANGLE(7);
		if (info.distance < SQUARE(512))
		{
			item->goalAnimState = 1;
			break;
		}
		if (info.distance > SQUARE(3072) && info.distance < SQUARE(7168))
		{
			item->goalAnimState = 2;
			break;
		}
		if (info.distance <= SQUARE(682))
			item->goalAnimState = 4;
		else if (info.distance > SQUARE(7168))
			item->goalAnimState = 1;
		break;

	case 0:
		creature->maximumTurn = 0;
		if (info.distance < SQUARE(1024) || item->triggerFlags > -1)
			item->goalAnimState = 2;
		break;

	case 8:
		joint0 = 0;
		joint1 = 0;
		joint2 = 0;
		creature->maximumTurn = 0;
		item->hitPoints = 0;
		if (info.distance < SQUARE(1024) || !(GetRandomControl() & 0x7F))
		{
			item->goalAnimState = 9;
			item->hitPoints = Objects[ID_MUMMY].hitPoints;
		}
		break;

	case 4:
	case 10:
		creature->maximumTurn = 0;
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
		if (!creature->flags)
		{

			if (item->touchBits & 0x4800)
			{
				if (item->frameNumber > Anims[item->animNumber].frameEnd + 13 && item->frameNumber < Anims[item->animNumber].frameEnd + 22)
				{
					LaraItem->hitPoints -= 100;
					LaraItem->hitStatus = true;

					if (item->animNumber == Objects[ID_MUMMY].animIndex + 15)
					{
						CreatureEffect2(
							item,
							&mummyBite1,
							5,
							-1,
							DoBloodSplat);
					}
					else
					{
						CreatureEffect2(
							item,
							&mummyBite2,
							5,
							-1,
							DoBloodSplat);
					}
					creature->flags = 1;
				}
			}
		}
		break;
	default:
		break;
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);

	CreatureAnimation(itemNum, angle, 0);
}