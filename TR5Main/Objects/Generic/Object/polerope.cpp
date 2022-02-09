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
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"

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
		auto isLara = (!item->Data.is<LaraInfo*>());

		if (isLara &&
			TrInput & IN_ACTION && 
			!Lara.gunStatus && 
			l->ActiveState == LS_IDLE && 
			l->AnimNumber == LA_STAND_IDLE || Lara.Control.IsMoving &&
			Lara.interactedItem == itemNumber)
		{
			short rot = item->Position.yRot;
			item->Position.yRot = l->Position.yRot;

			if (TestLaraPosition(&PoleBounds, item, l))
			{
				if (MoveLaraPosition(&PolePos, item, l))
				{
					l->AnimNumber = LA_STAND_TO_POLE;
					l->ActiveState = LS_POLE_IDLE;
					l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
					Lara.Control.IsMoving = false;
					Lara.gunStatus = LG_HANDS_BUSY;
				}
				else
				{
					Lara.interactedItem = itemNumber;
				}
				item->Position.yRot = rot;
			}
			else
			{
				if (Lara.Control.IsMoving && Lara.interactedItem == itemNumber)
				{
					Lara.Control.IsMoving = false;
					Lara.gunStatus = LG_HANDS_FREE;
				}
				item->Position.yRot = rot;
			}
		}
		else if (isLara && 
				 TrInput & IN_ACTION &&
			     !Lara.gunStatus && 
				 l->Airborne && 
				 l->VerticalVelocity > Lara.gunStatus && 
				 l->ActiveState == LS_REACH || l->ActiveState == LS_JUMP_UP)
		{
			if (TestBoundsCollide(item, l, 100) &&
				TestLaraPoleCollision(l, coll, true, -STEP_SIZE) &&
				TestLaraPoleCollision(l, coll, false))
			{
				if (TestCollision(item, l))
				{
					short rot = item->Position.yRot;
					item->Position.yRot = l->Position.yRot;
					if (l->ActiveState == LS_REACH)
					{
						PolePosR.y = l->Position.yPos - item->Position.yPos + 10;
						AlignLaraPosition(&PolePosR, item, l);
						l->AnimNumber = LA_REACH_TO_POLE;
						l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
					}
					else
					{
						PolePosR.y = l->Position.yPos - item->Position.yPos + 66;
						AlignLaraPosition(&PolePosR, item, l);
						l->AnimNumber = LA_JUMP_UP_TO_POLE;
						l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
					}
					l->Airborne = false;
					l->VerticalVelocity = false;
					l->ActiveState = LS_POLE_IDLE;
					Lara.gunStatus = LG_HANDS_BUSY;
					item->Position.yRot = rot;
				}
			}
		}
		else
		{
			if (!isLara || 
				((l->ActiveState < LS_POLE_IDLE || l->ActiveState > LS_POLE_TURN_COUNTER_CLOCKWISE) && l->ActiveState != LS_JUMP_BACK))
				ObjectCollision(itemNumber, l, coll);
		}
	}
}