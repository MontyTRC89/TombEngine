#include "framework.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/control/lot.h"
#include "Specific/input.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara.h"
#include "Specific/trmath.h"
#include "Game/collide.h"
#include "Game/sphere.h"

namespace TEN::Entities::Generic
{
	PHD_VECTOR PolePos = { 0, 0, -208 };
	PHD_VECTOR PolePosR = { 0, 0, 0 };

	OBJECT_COLLISION_BOUNDS PoleBounds = 
	{
		-256, 256, 
		0, 0, 
		-512, 512, 
		-ANGLE(10), ANGLE(10), 
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	void PoleCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
	{
		auto item = &g_Level.Items[itemNumber];
		auto isLara = (!item->data.is<LaraInfo*>());

		if (isLara &&
			(TrInput & IN_ACTION) && 
			!Lara.gunStatus && 
			l->currentAnimState == LS_STOP && 
			l->animNumber == LA_STAND_IDLE || Lara.isMoving &&
			Lara.interactedItem == itemNumber)
		{
			short rot = item->pos.yRot;
			item->pos.yRot = l->pos.yRot;

			if (TestLaraPosition(&PoleBounds, item, l))
			{
				if (MoveLaraPosition(&PolePos, item, l))
				{
					l->animNumber = LA_STAND_TO_POLE;
					l->currentAnimState = LS_POLE_IDLE;
					l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
					Lara.isMoving = false;
					Lara.gunStatus = LG_HANDS_BUSY;
				}
				else
				{
					Lara.interactedItem = itemNumber;
				}
				item->pos.yRot = rot;
			}
			else
			{
				if (Lara.isMoving && Lara.interactedItem == itemNumber)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_NO_ARMS;
				}
				item->pos.yRot = rot;
			}
		}
		else if (isLara && 
				 TrInput & IN_ACTION &&
			     !Lara.gunStatus && 
				 l->gravityStatus && 
				 l->fallspeed > Lara.gunStatus && 
				 l->currentAnimState == LS_REACH || l->currentAnimState == LS_JUMP_UP)
		{
			if (TestBoundsCollide(item, l, 100) &&
				TestLaraPoleCollision(l, coll, true, -STEP_SIZE) &&
				TestLaraPoleCollision(l, coll, false))
			{
				if (TestCollision(item, l))
				{
					short rot = item->pos.yRot;
					item->pos.yRot = l->pos.yRot;
					if (l->currentAnimState == LS_REACH)
					{
						PolePosR.y = l->pos.yPos - item->pos.yPos + 10;
						AlignLaraPosition(&PolePosR, item, l);
						l->animNumber = LA_REACH_TO_POLE;
						l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
					}
					else
					{
						PolePosR.y = l->pos.yPos - item->pos.yPos + 66;
						AlignLaraPosition(&PolePosR, item, l);
						l->animNumber = LA_JUMP_UP_TO_POLE;
						l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
					}
					l->gravityStatus = false;
					l->fallspeed = false;
					l->currentAnimState = LS_POLE_IDLE;
					Lara.gunStatus = LG_HANDS_BUSY;
					item->pos.yRot = rot;
				}
			}
		}
		else
		{
			if (!isLara || 
				((l->currentAnimState < LS_POLE_IDLE || l->currentAnimState > LS_POLE_TURN_COUNTER_CLOCKWISE) && l->currentAnimState != LS_JUMP_BACK))
				ObjectCollision(itemNumber, l, coll);
		}
	}
}