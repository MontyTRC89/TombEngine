#include "framework.h"
#include "switch.h"
#include "door.h"
#include "items.h"
#include "lot.h"
#include "objects.h"
#include "Lara.h"
#ifdef NEW_INV
#include "newinv2.h"
#else
#include "inventory.h"
#endif
#include "draw.h"
#include "sphere.h"
#include "camera.h"
#include "setup.h"
#include "level.h"
#include "input.h"
#include "sound.h"

byte SequenceUsed[6];
byte SequenceResults[3][3][3];
byte Sequences[3];
byte CurrentSequence;
int PulleyItemNumber = NO_ITEM;

extern PHD_VECTOR OldPickupPos;
#ifndef NEW_INV
extern Inventory g_Inventory;
#endif

OBJECT_COLLISION_BOUNDS Switch2Bounds =
{
	0xFC00, 0x0400, 0xFC00, 0x0400, 0xFC00, 0x0200, 0xC720, 0x38E0, 0xC720, 0x38E0,
	0xC720, 0x38E0
};
PHD_VECTOR Switch2Position = { 0, 0, 108 };  
OBJECT_COLLISION_BOUNDS TurnSwitchBoundsA = // offset 0xA14D8
{
	0x0200, 0x0380, 0x0000, 0x0000, 0xFE00, 0x0000, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR TurnSwitchPos = { 650, 0, 138 }; // offset 0xA14F0
OBJECT_COLLISION_BOUNDS TurnSwitchBoundsC = // offset 0xA14FC
{
	0x0200, 0x0380, 0x0000, 0x0000, 0x0000, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR TurnSwitchPosA = { 650, 0, -138 }; // offset 0xA1514
PHD_VECTOR RailSwitchPos = { 0, 0, -550 }; // offset 0xA1544
OBJECT_COLLISION_BOUNDS RailSwitchBounds = // offset 0xA1550
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0xFD00, 0xFE00, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR RailSwitchPos2 = { 0, 0, 550 }; // offset 0xA1568
OBJECT_COLLISION_BOUNDS RailSwitchBounds2 = // offset 0xA1574
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0x0200, 0x0300, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
OBJECT_COLLISION_BOUNDS JumpSwitchBounds = // offset 0xA158C
{
	0xFF80, 0x0080, 0xFF00, 0x0100, 0x0180, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR JumpSwitchPos = { 0, -208, 256 }; // offset 0xA15A4
PHD_VECTOR CrowbarPos = { -89, 0, -328 }; // offset 0xA15B0
OBJECT_COLLISION_BOUNDS CrowbarBounds = // offset 0xA15BC
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0xFE00, 0xFF00, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR CrowbarPos2 = { 89, 0, 328 }; // offset 0xA15D4
OBJECT_COLLISION_BOUNDS CrowbarBounds2 = // offset 0xA15E0
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0x0100, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
OBJECT_COLLISION_BOUNDS FullBlockSwitchBounds = // offset 0xA15F8
{
	0xFE80, 0x0180, 0x0000, 0x0100, 0x0000, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR FullBlockSwitchPos = { 0, 0, 0 }; // offset 0xA1610
OBJECT_COLLISION_BOUNDS PulleyBounds = // offset 0xA161C
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0xFE00, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR PulleyPos = { 0, 0, -148 }; // offset 0xA1634
PHD_VECTOR CrowDovePos = { 0, 0, -400 }; // offset 0xA1640
OBJECT_COLLISION_BOUNDS UnderwaterSwitchBounds = // offset 0xA164C
{
	0xFF00, 0x0100, 0xFB00, 0xFE00, 0xFE00, 0x0000, 0xC720, 0x38E0, 0xC720, 0x38E0,
	0xC720, 0x38E0
};
OBJECT_COLLISION_BOUNDS UnderwaterSwitchBounds2 = // offset 0xA1664
{
	0xFF00, 0x0100, 0xFB00, 0xFE00, 0x0000, 0x0200, 0xC720, 0x38E0, 0xC720, 0x38E0,
	0xC720, 0x38E0
};
PHD_VECTOR UnderwaterSwitchPos = { 0, -736, -416 }; // offset 0xA167C
PHD_VECTOR UnderwaterSwitchPos2 = { 0, -736, 416 }; // offset 0xA1688
OBJECT_COLLISION_BOUNDS SwitchBounds = // offset 0xA1694
{
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR SwitchPos = { 0, 0, 0 }; // offset 0xA16AC

void ProcessExplodingSwitchType8(ITEM_INFO* item) 
{
	PHD_VECTOR pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;
	GetJointAbsPosition(item, &pos, 0);
	TestTriggersAtXYZ(pos.x, pos.y, pos.z, item->roomNumber, 1, 0);
	ExplodeItemNode(item, Objects[item->objectNumber].nmeshes - 1, 0, 64);
	item->meshBits |= 1 << ((Objects[item->objectNumber].nmeshes & 0xFF) - 2);
}

void CrowDoveSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (item->flags & 0x100
		|| !(item->meshBits & 4)
		|| (!(TrInput & IN_ACTION) 
			|| Lara.gunStatus 
			|| l->currentAnimState != LS_STOP 
			|| l->animNumber != LA_STAND_IDLE 
			|| l->gravityStatus)
		&& (!Lara.isMoving || Lara.generalPtr != (void*)itemNum))
	{
		if (l->currentAnimState != LS_DOVESWITCH)
			ObjectCollision(itemNum, l, coll);
	}
	else
	{
		int oldYrot = item->pos.yRot;
		item->pos.yRot = l->pos.yRot;
		if (TestLaraPosition(&PulleyBounds, item, l))
		{
			if (MoveLaraPosition(&CrowDovePos, item, l))
			{
				l->animNumber = LA_DOVESWITCH_TURN;
				l->currentAnimState = LS_DOVESWITCH;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				
				AddActiveItem(itemNum);

				item->itemFlags[0] = 0; // This enables the switch again (in TR5 this switch was always triggered by heavy triggers)
				item->status = ITEM_ACTIVE;
				item->pos.yRot = oldYrot;
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				Lara.generalPtr = (void*)item;
			}
			else
			{
				Lara.generalPtr = (void*)itemNum;
			}
			item->pos.yRot = oldYrot;
		}
		else
		{
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
			item->pos.yRot = oldYrot;
		}
	}
}

void CrowDoveSwitchControl(short itemNumber) 
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if ((item->meshBits & 2))
	{
		ExplodeItemNode(item, 1, 0, 0x100);
		SoundEffect(SFX_TR5_RAVENSWITCH_EXP, &item->pos, 0);
		item->meshBits = 5;
		RemoveActiveItem(itemNumber);
		item->itemFlags[0] = 1; // I use this for not making it activable again by trigger
	}
	else if (!item->itemFlags[0])
	{
		if (item->currentAnimState == 0)
			item->goalAnimState = 1;

		AnimateItem(item);

		if (item->currentAnimState == 0)
			item->pos.yRot += ANGLE(90);
	}
}

void FullBlockSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	
	if ((!(TrInput & IN_ACTION)
		|| item->status 
		|| item->flags & 0x100
		|| CurrentSequence >= 3u
		|| Lara.gunStatus
		|| l->currentAnimState != LS_STOP
		|| l->animNumber != LA_STAND_IDLE)
		&& (!Lara.isMoving || Lara.generalPtr != (void*)itemNum))
	{
		ObjectCollision(itemNum, l, coll);
		return;
	}
	
	if (TestLaraPosition(&FullBlockSwitchBounds, item, l))
	{
		if (MoveLaraPosition(&FullBlockSwitchPos, item, l))
		{
			if (item->currentAnimState == 1)
			{
				l->currentAnimState = LS_SWITCH_DOWN;
				l->animNumber = LA_BUTTON_GIANT_PUSH;
				item->goalAnimState = 0;
			}
			l->goalAnimState = LS_STOP;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			item->status = ITEM_ACTIVE;
			
			AddActiveItem(itemNum);
			AnimateItem(item);

			Lara.isMoving = false;
			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			Lara.gunStatus = LG_HANDS_BUSY;
		}
		else
		{
			Lara.generalPtr = (void*)itemNum;
		}
	}
	else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
	{
		Lara.isMoving = false;
		Lara.gunStatus = LG_NO_ARMS;
	}
}

void FullBlockSwitchControl(short itemNumber) 
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	if (item->animNumber != Objects[item->objectNumber].animIndex + 2
		|| CurrentSequence >= 3u
		|| item->itemFlags[0])
	{
		if (CurrentSequence >= 4u)
		{
			item->itemFlags[0] = 0;
			item->goalAnimState = 1;
			item->status = ITEM_NOT_ACTIVE;
			if (++CurrentSequence >= 7u)
				CurrentSequence = 0;
		}
	}
	else
	{
		item->itemFlags[0] = 1;
		Sequences[CurrentSequence++] = item->triggerFlags;
	}

	AnimateItem(item);
}

void CrowbarSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	int doSwitch = 0;
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if ((!(TrInput & IN_ACTION) && 
#ifdef NEW_INV
		GLOBAL_inventoryitemchosen != ID_CROWBAR_ITEM
#else
		g_Inventory.GetSelectedObject() != ID_CROWBAR_ITEM
#endif
		|| l->currentAnimState != LS_STOP
		|| l->animNumber != LA_STAND_IDLE
		|| Lara.gunStatus
		|| item->itemFlags[0])
		&& (!Lara.isMoving || Lara.generalPtr != (void*)itemNum))
	{
		ObjectCollision(itemNum, l, coll);
		return;
	}
	
	if (item->currentAnimState)
	{
		if (item->currentAnimState != 1)
		{
			ObjectCollision(itemNum, l, coll);
			return;
		}

		l->pos.yRot ^= (short)ANGLE(180);
		if (TestLaraPosition(&CrowbarBounds2, item, l))
		{
			if (Lara.isMoving || 
#ifdef NEW_INV
				GLOBAL_inventoryitemchosen == ID_CROWBAR_ITEM
#else
				g_Inventory.GetSelectedObject() == ID_CROWBAR_ITEM
#endif
				)
			{
				if (MoveLaraPosition(&CrowbarPos2, item, l))
				{
					l->animNumber = LA_CROWBAR_USE_ON_FLOOR;
					doSwitch = 1;
					l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
					item->goalAnimState = 0;
				}
				else
				{
					Lara.generalPtr = (void*)itemNum;
				}
#ifdef NEW_INV
				GLOBAL_inventoryitemchosen = NO_ITEM;
#else
				g_Inventory.SetSelectedObject(NO_ITEM);
#endif
			}
			else
			{
				doSwitch = -1;
			}
		}
		else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}
		l->pos.yRot ^= (short)ANGLE(180);
	}
	else
	{
		if (!TestLaraPosition(&CrowbarBounds, item, l))
		{
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
			ObjectCollision(itemNum, l, coll);
			return;
		}

		if (!(Lara.isMoving && 
#ifdef NEW_INV
			GLOBAL_inventoryitemchosen != ID_CROWBAR_ITEM)
#else
			g_Inventory.GetSelectedObject() != ID_CROWBAR_ITEM)
#endif
			)
		{
			if (Lara.Crowbar)
#ifdef NEW_INV
				GLOBAL_inventoryitemchosen = ID_CROWBAR_ITEM;
#else
				g_Inventory.SetEnterObject(ID_CROWBAR_ITEM);
#endif
			else
			{
				if (OldPickupPos.x != l->pos.xPos || OldPickupPos.y != l->pos.yPos || OldPickupPos.z != l->pos.zPos)
				{
					OldPickupPos.x = l->pos.xPos;
					OldPickupPos.y = l->pos.yPos;
					OldPickupPos.z = l->pos.zPos;
					SayNo();
				}
			}
			return;
		}

		if (MoveLaraPosition(&CrowbarPos, item, l))
		{
			l->animNumber = LA_CROWBAR_USE_ON_FLOOR;
			doSwitch = 1;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			item->goalAnimState = 1;
		}
		else
		{
			Lara.generalPtr = (void*)itemNum;
		}
#ifdef NEW_INV
		GLOBAL_inventoryitemchosen = NO_ITEM;
#else
		g_Inventory.SetSelectedObject(NO_ITEM);
#endif
	}

	if (!doSwitch)
	{
		ObjectCollision(itemNum, l, coll);
		return;
	}

	if (doSwitch != -1)
	{
		l->goalAnimState = LS_SWITCH_DOWN;
		l->currentAnimState = LS_SWITCH_DOWN;
		Lara.isMoving = false;
		Lara.headYrot = 0;
		Lara.headXrot = 0;
		Lara.torsoYrot = 0;
		Lara.torsoXrot = 0;
		Lara.gunStatus = LG_HANDS_BUSY;
		item->status = ITEM_ACTIVE;

		AddActiveItem(itemNum);
		AnimateItem(item);

		return;
	}

	if (Lara.Crowbar)
#ifdef NEW_INV
		GLOBAL_enterinventory = ID_CROWBAR_ITEM;
#else
		g_Inventory.SetEnterObject(ID_CROWBAR_ITEM);
#endif
	else
	{
		if (OldPickupPos.x != l->pos.xPos || OldPickupPos.y != l->pos.yPos || OldPickupPos.z != l->pos.zPos)
		{
			OldPickupPos.x = l->pos.xPos;
			OldPickupPos.y = l->pos.yPos;
			OldPickupPos.z = l->pos.zPos;
			SayNo();
		}
	}
}

void JumpSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	
	if (TrInput & IN_ACTION)
	{
		if (!Lara.gunStatus)
		{
			if (l->currentAnimState == LS_REACH || l->currentAnimState == LS_JUMP_UP)
			{
				if (l->status || l->gravityStatus)
				{
					if (l->fallspeed > 0 && !item->currentAnimState)
					{
						if (TestLaraPosition(&JumpSwitchBounds, item, l))
						{
							AlignLaraPosition(&JumpSwitchPos, item, l);

							l->currentAnimState = LS_SWITCH_DOWN;
							l->animNumber = LA_JUMPSWITCH_PULL;
							l->fallspeed = 0;
							l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
							l->gravityStatus = false;
							Lara.gunStatus = LG_HANDS_BUSY;
							
							item->goalAnimState = 1;
							item->status = ITEM_ACTIVE;
							
							AddActiveItem(itemNum);
						}
					}
				}
			}
		}
	}
}

void RailSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	int flag = 0;
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if ((!(TrInput & IN_ACTION) 
		|| l->currentAnimState != LS_STOP 
		|| l->animNumber != LA_STAND_IDLE 
		|| Lara.gunStatus)
		&& (!Lara.isMoving 
			|| Lara.generalPtr != (void*)itemNum))
	{
		ObjectCollision(itemNum, l, coll);
		return;
	}

	if (item->currentAnimState)
	{
		if (item->currentAnimState == 1)
		{
			l->pos.yRot ^= (short)ANGLE(180);
			if (TestLaraPosition(&RailSwitchBounds2, item, l))
			{
				if (MoveLaraPosition(&RailSwitchPos2, item, l))
				{
					item->goalAnimState = 0;
					flag = 1;
				}
				else
				{
					Lara.generalPtr = (void*)itemNum;
				}
			}
			else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}

			l->pos.yRot ^= (short)ANGLE(180);

			if (flag)
			{
				l->animNumber = LA_LEVER_PUSH;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				l->goalAnimState = LS_LEVERSWITCH_PUSH;
				l->currentAnimState = LS_LEVERSWITCH_PUSH;
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;

				item->status = ITEM_ACTIVE;
				AddActiveItem(itemNum);
				AnimateItem(item);

				return;
			}
		}
		
		ObjectCollision(itemNum, l, coll);
	}
	else
	{
		if (TestLaraPosition(&RailSwitchBounds, item, l))
		{
			if (MoveLaraPosition(&RailSwitchPos, item, l))
			{
				item->goalAnimState = 1;
				l->animNumber = LA_LEVER_PUSH;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				l->goalAnimState = LS_LEVERSWITCH_PUSH;
				l->currentAnimState = LS_LEVERSWITCH_PUSH;
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;

				item->status = ITEM_ACTIVE;
				AddActiveItem(itemNum);
				AnimateItem(item);
			}
			else
			{
				Lara.generalPtr = (void*)itemNum;
			}
		}
		else if (Lara.isMoving)
		{
			if (Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
		}

		ObjectCollision(itemNum, l, coll);
	}
}

void TurnSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	int flag = 0;

	if (item->currentAnimState
		&& TrInput & IN_ACTION
		&& l->currentAnimState == LS_STOP
		&& l->animNumber == LA_STAND_IDLE
		&& !l->gravityStatus
		&& Lara.gunStatus == LG_NO_ARMS
		|| Lara.isMoving && Lara.generalPtr == (void*)itemNum)
	{
		short ItemNos[8];
		if (TestLaraPosition(&TurnSwitchBoundsA, item, l))
		{
			if (MoveLaraPosition(&TurnSwitchPosA, item, l))
			{
				l->animNumber = LA_TURNSWITCH_GRAB_COUNTER_CLOCKWISE;
				l->frameNumber = g_Level.Anims[LA_TURNSWITCH_GRAB_COUNTER_CLOCKWISE].frameBase;
				item->animNumber = Objects[item->objectNumber].animIndex + 4;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->itemFlags[0] = 1;
				ForcedFixedCamera.x = item->pos.xPos - 1024 * phd_sin(item->pos.yRot);
				ForcedFixedCamera.z = item->pos.zPos - 1024 * phd_cos(item->pos.yRot);
				Lara.isMoving = 0;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				l->currentAnimState = LA_REACH;

				UseForcedFixedCamera = true;
				ForcedFixedCamera.y = item->pos.yPos - 2048;
				ForcedFixedCamera.roomNumber = item->roomNumber;

				AddActiveItem(itemNum);

				item->status = ITEM_ACTIVE;
				item->itemFlags[1] = 0;

				if (GetSwitchTrigger(item, ItemNos, 0))
				{
					if (!TriggerActive(&g_Level.Items[ItemNos[0]]))
					{
						g_Level.Items[ItemNos[0]].animNumber = Objects[g_Level.Items[ItemNos[0]].objectNumber].animIndex;
						g_Level.Items[ItemNos[0]].frameNumber = g_Level.Anims[g_Level.Items[ItemNos[0]].animNumber].frameBase;
					}
				}
				return;
			}
			Lara.generalPtr = (void*)itemNum;
		}
		else
		{
			l->pos.yRot ^= (short)ANGLE(180);
			if (TestLaraPosition(&TurnSwitchBoundsC, item, l))
			{
				if (MoveLaraPosition(&TurnSwitchPos, item, l))
				{
					l->animNumber = 319;
					flag = 1;
					l->frameNumber = g_Level.Anims[319].frameBase;
					item->itemFlags[0] = 2;
					ForcedFixedCamera.x = item->pos.xPos + 1024 * phd_sin(item->pos.yRot);
					ForcedFixedCamera.z = item->pos.zPos + 1024 * phd_cos(item->pos.yRot);
				}
				else
				{
					Lara.generalPtr = (void*)itemNum;
				}
			}
			else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
			l->pos.yRot ^= (short)ANGLE(180);
			if (flag)
			{
				Lara.isMoving = 0;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				l->currentAnimState = LA_REACH;
				UseForcedFixedCamera = true;
				ForcedFixedCamera.y = item->pos.yPos - 2048;
				ForcedFixedCamera.roomNumber = item->roomNumber;
				AddActiveItem(itemNum);
				item->status = ITEM_ACTIVE;
				item->itemFlags[1] = 0;
				if (GetSwitchTrigger(item, ItemNos, 0))
				{
					if (!TriggerActive(&g_Level.Items[ItemNos[0]]))
					{
						g_Level.Items[ItemNos[0]].animNumber = Objects[g_Level.Items[ItemNos[0]].objectNumber].animIndex + 4;
						g_Level.Items[ItemNos[0]].frameNumber = g_Level.Anims[g_Level.Items[ItemNos[0]].animNumber].frameBase;
					}
				}
				return;
			}
		}
	}

	if (coll->enableBaddiePush && TestBoundsCollide(item, l, coll->radius))
	{
		GlobalCollisionBounds.X1 = -512;
		GlobalCollisionBounds.X2 = 512;
		GlobalCollisionBounds.Y1 = -512;
		GlobalCollisionBounds.Y2 = 0;
		GlobalCollisionBounds.Z1 = -512;
		GlobalCollisionBounds.Z2 = 512;

		ItemPushLara(item, l, coll, 0, 2);

		GlobalCollisionBounds.X1 = 256;
		GlobalCollisionBounds.X2 = 1024;
		GlobalCollisionBounds.Z1 = -128;
		GlobalCollisionBounds.Z2 = 128;

		ItemPushLara(item, l, coll, 0, 2);
	}
}

void TurnSwitchControl(short itemNum) 
{
	ITEM_INFO* l = LaraItem;
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (g_Level.Items[itemNum].itemFlags[0] == 2)
	{
		if (item->animNumber == Objects[ID_TURN_SWITCH].animIndex + 2)
		{
			item->pos.yRot += ANGLE(90);
			if (TrInput & IN_ACTION)
			{
				l->animNumber = LA_TURNSWITCH_PUSH_CLOCKWISE_START;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				item->animNumber = Objects[item->objectNumber].animIndex + 1;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
		}

		if (l->animNumber == LA_TURNSWITCH_PUSH_CLOCKWISE_END && l->frameNumber == g_Level.Anims[l->animNumber].frameEnd && !item->itemFlags[1])
			item->itemFlags[1] = 1;

		if (l->frameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase && 
			l->frameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 43
			|| 
			l->frameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 58 &&
			l->frameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 115)
		{
			SoundEffect(SFX_TR4_PUSHABLE_SOUND, &item->pos, 2);
		}
	}
	else
	{
		if (item->animNumber == Objects[ID_TURN_SWITCH].animIndex + 6)
		{
			item->pos.yRot -= ANGLE(90);
			if (TrInput & IN_ACTION)
			{
				l->animNumber = LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				item->animNumber = Objects[item->objectNumber].animIndex + 5;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
		}

		if (l->animNumber == LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END && l->frameNumber == g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END].frameEnd && 
			!item->itemFlags[1])
			item->itemFlags[1] = 1;

		if (l->frameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase &&
			l->frameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 43
			||
			l->frameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 58 &&
			l->frameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 115)
		{
			SoundEffect(SFX_TR4_PUSHABLE_SOUND, &item->pos, 2);
		}
	}

	AnimateItem(item);

	if (item->itemFlags[1] == 1)
	{
		l->animNumber = LA_STAND_IDLE;
		l->currentAnimState = LS_STOP;
		l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
		item->animNumber = Objects[item->objectNumber].animIndex;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->status = ITEM_NOT_ACTIVE;
		
		RemoveActiveItem(itemNum);

		Lara.gunStatus = LG_NO_ARMS;
		UseForcedFixedCamera = 0;
		item->itemFlags[1] = 2;
	}
}

void PulleyCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	
	if (item->flags & 0x100
		|| (!(TrInput & IN_ACTION) 
			|| Lara.gunStatus
			|| l->currentAnimState != LS_STOP
			|| l->animNumber != LA_STAND_IDLE 
			|| item->gravityStatus)
		&& (!Lara.isMoving 
			|| Lara.generalPtr != (void*)itemNum))
	{
		if (l->currentAnimState != LS_PULLEY)
			ObjectCollision(itemNum, l, coll);
	}
	else
	{
		short oldYrot = item->pos.yRot;
		item->pos.yRot = l->pos.yRot;
		if (TestLaraPosition(&PulleyBounds, item, l))
		{
			if (item->itemFlags[1])
			{
				if (OldPickupPos.x != l->pos.xPos || OldPickupPos.y != l->pos.yPos || OldPickupPos.z != l->pos.zPos)
				{
					OldPickupPos.x = l->pos.xPos;
					OldPickupPos.y = l->pos.yPos;
					OldPickupPos.z = l->pos.zPos;
					SayNo();
				}
			}
			else if (MoveLaraPosition(&PulleyPos, item, l))
			{
				l->animNumber = LA_PULLEY_GRAB;
				l->currentAnimState = LS_PULLEY;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				
				AddActiveItem(itemNum);
				
				item->pos.yRot = oldYrot;
				item->status = ITEM_ACTIVE;
				
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				Lara.generalPtr = item;
			}
			else
			{
				Lara.generalPtr = (void*)itemNum;
			}
			item->pos.yRot = oldYrot;
		}
		else
		{
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
			item->pos.yRot = oldYrot;
		}
	}
}

void UnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	int flag = 0;

	if (TrInput & IN_ACTION
		&& Lara.waterStatus == LW_UNDERWATER
		&& l->currentAnimState == LS_UNDERWATER_STOP
		&& l->animNumber == LA_UNDERWATER_IDLE
		&& !Lara.gunStatus
		&& !item->currentAnimState
		|| Lara.isMoving && Lara.generalPtr == (void*)itemNum)
	{
		flag = 0;
		if (TestLaraPosition(&UnderwaterSwitchBounds, item, l))
		{
			if (!MoveLaraPosition(&UnderwaterSwitchPos, item, l))
			{
				Lara.generalPtr = (void*)itemNum;
				return ;
			}
		LABEL_17:
			l->currentAnimState = LS_SWITCH_DOWN;
			l->animNumber = LA_UNDERWATER_CEILING_SWITCH_PULL;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			l->fallspeed = 0;
			Lara.isMoving = false;
			Lara.gunStatus = LG_HANDS_BUSY;
			item->goalAnimState = 1;
			item->status = ITEM_ACTIVE;

			AddActiveItem(itemNum);
			
			ForcedFixedCamera.x = item->pos.xPos - 1024 * phd_sin(item->pos.yRot + ANGLE(90));
			ForcedFixedCamera.y = item->pos.yPos - 1024;
			ForcedFixedCamera.z = item->pos.zPos - 1024 * phd_cos(item->pos.yRot + ANGLE(90));
			ForcedFixedCamera.roomNumber = item->roomNumber;

			return;
		}

		l->pos.yRot ^= (short)ANGLE(180);
		if (TestLaraPosition(&UnderwaterSwitchBounds2, item, l))
		{
			if (MoveLaraPosition(&UnderwaterSwitchPos2, item, l))
				flag = 1;
			else
				Lara.generalPtr = (void*)itemNum;
		}
		l->pos.yRot ^= (short)ANGLE(180);

		if (flag)
		{
			l->currentAnimState = LS_SWITCH_DOWN;
			l->animNumber = LA_UNDERWATER_CEILING_SWITCH_PULL;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			l->fallspeed = 0;
			Lara.isMoving = false;
			Lara.gunStatus = LG_HANDS_BUSY;
			item->goalAnimState = 1;
			item->status = ITEM_ACTIVE;

			AddActiveItem(itemNum);

			ForcedFixedCamera.x = item->pos.xPos - 1024 * phd_sin(item->pos.yRot + ANGLE(90));
			ForcedFixedCamera.y = item->pos.yPos - 1024;
			ForcedFixedCamera.z = item->pos.zPos - 1024 * phd_cos(item->pos.yRot + ANGLE(90));
			ForcedFixedCamera.roomNumber = item->roomNumber;
		}
	}
}

void SwitchCollision2(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (TrInput & IN_ACTION)
	{
		if (item->status == ITEM_NOT_ACTIVE && Lara.waterStatus == LW_UNDERWATER && !Lara.gunStatus && l->currentAnimState == LS_UNDERWATER_STOP)
		{
			if (TestLaraPosition(&Switch2Bounds, item, l))
			{
				if (item->currentAnimState == 1 || !item->currentAnimState)
				{
					if (MoveLaraPosition(&Switch2Position, item, l))
					{
						l->fallspeed = 0;
						l->goalAnimState = LS_SWITCH_DOWN;

						do
							AnimateLara(l);
						while (l->goalAnimState != LS_SWITCH_DOWN);

						l->goalAnimState = LS_UNDERWATER_STOP;
						Lara.gunStatus = LG_HANDS_BUSY;
						item->goalAnimState = item->currentAnimState != 1;
						item->status = ITEM_ACTIVE;						
						AddActiveItem(itemNum);
						AnimateItem(item);
					}
				}
			}
		}
	}
}

void SwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	if (TrInput & IN_ACTION
		&& l->currentAnimState == LS_STOP
		&& l->animNumber == LA_STAND_IDLE
		&& !Lara.gunStatus
		&& item->status == ITEM_NOT_ACTIVE
		&& !(item->flags & 0x100)
		&& item->triggerFlags >= 0
		|| Lara.isMoving && Lara.generalPtr == (void*)itemNum)
	{
		BOUNDING_BOX* bounds = GetBoundsAccurate(item);
		
		if (item->triggerFlags == 3 && item->currentAnimState == 1 || item->triggerFlags >= 5 && item->triggerFlags <= 7 && !item->currentAnimState)
			return;

		SwitchBounds.boundingBox.X1 = bounds->X1 - 256;
		SwitchBounds.boundingBox.X2 = bounds->X2 + 256;
		if (item->triggerFlags)
		{
			SwitchBounds.boundingBox.Z1 = bounds->Z1 - 512;
			SwitchBounds.boundingBox.Z2 = bounds->Z2 + 512;

			if (item->triggerFlags == 3)
			{
				SwitchPos.z = bounds->Z1 - 256;
			}
			else if (item->triggerFlags == 4)
			{
				SwitchPos.z = bounds->Z1 - 88;
			}
			else if (item->triggerFlags < 5 || item->triggerFlags > 7)
			{
				if (item->triggerFlags < 8)
					SwitchPos.z = bounds->Z1 - 128;
				else
					SwitchPos.z = bounds->Z1 - 96;
			} 
			else
			{
				SwitchPos.z = bounds->Z1 - 160;
			}
		}
		else
		{
			SwitchBounds.boundingBox.Z1 = bounds->Z1 - 200;
			SwitchBounds.boundingBox.Z2 = bounds->Z2 + 200;
			SwitchPos.z = bounds->Z1 - 64;
		}
		
		if (TestLaraPosition(&SwitchBounds, item, l))
		{
			if (MoveLaraPosition(&SwitchPos, item, l))
			{
				if (item->currentAnimState == 1) /* Switch down */
				{
					if (item->triggerFlags)
					{
						if (item->triggerFlags >= 3)
						{
							if (item->triggerFlags == 4)
							{
								l->currentAnimState = LS_SWITCH_UP;
								l->animNumber = LA_SWITCH_SMALL_DOWN;
								item->goalAnimState = 0;
							}
							else
							{
								if (item->triggerFlags >= 5 && item->triggerFlags <= 7)
								{
									if (item->triggerFlags == 6)
										DisableLaraControl = true;
									l->currentAnimState = LS_SWITCH_DOWN;
									l->animNumber = LA_BUTTON_SMALL_PUSH;
								}
								item->goalAnimState = 0;
							}
						}
						else
						{
							l->animNumber = LA_HOLESWITCH_ACTIVATE;
							l->currentAnimState = LS_HOLE;
							item->goalAnimState = 0;
						}
					}
					else
					{
						l->currentAnimState = LS_SWITCH_UP;
						l->animNumber = LA_WALLSWITCH_DOWN;
						item->goalAnimState = 0;
					}
				}
				else /* Switch up */
				{
					if (item->triggerFlags) 
					{
						if (item->triggerFlags == 3)
						{
							l->currentAnimState = LS_SWITCH_DOWN;
							l->animNumber = LA_BUTTON_LARGE_PUSH;
						}
						else if (item->triggerFlags == 4)
						{
							l->currentAnimState = LS_SWITCH_DOWN;
							l->animNumber = LA_SWITCH_SMALL_UP;
						}
						else if (item->triggerFlags < 8)
						{
							l->currentAnimState = LS_HOLE;
							l->animNumber = LA_HOLESWITCH_ACTIVATE;
						}
						else
						{
							l->currentAnimState = LS_SWITCH_DOWN;
							l->animNumber = LA_VALVE_TURN;
						}
					}
					else
					{
						l->currentAnimState = LS_SWITCH_DOWN;
						l->animNumber = LA_WALLSWITCH_UP;
					}

					item->goalAnimState = 1;
				}
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_BUSY;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;

				AddActiveItem(itemNum);
				item->status = ITEM_ACTIVE;
				AnimateItem(item);
			}
			else
			{
				Lara.generalPtr = (void*)itemNum;
			}
		}
		else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}

		return;
	}

	if (l->currentAnimState != LS_SWITCH_DOWN && l->currentAnimState != LS_SWITCH_UP)
		ObjectCollision(itemNum, l, coll);
}

void SwitchControl(short itemNumber) 
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (item->objectNumber != ID_AIRLOCK_SWITCH || item->triggerFlags < 8)
	{
		item->flags |= 0x3E00;
		if (!TriggerActive(item) && !(item->flags & 0x100))
		{
			if (item->objectNumber == ID_JUMP_SWITCH)
			{
				item->goalAnimState = 0;
				item->timer = 0;
				AnimateItem(item);
			}
			else
			{
				item->goalAnimState = 1;
				item->timer = 0;
			}
		}
	}
	else
	{	
		if (item->triggerFlags == 8)
		{
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 0;

			GetJointAbsPosition(item, &pos, 0);
			
			short roomNumber = item->roomNumber;
			GetFloor(pos.x, pos.y, pos.z, &roomNumber);
			if (roomNumber != item->roomNumber)
			{
				ItemNewRoom(itemNumber, roomNumber);
				AnimateItem(item);
			}
		}
	}
	
	AnimateItem(item);
}

int GetKeyTrigger(ITEM_INFO* item) 
{
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
	GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	if (TriggerIndex)
	{
		short* trigger = TriggerIndex;
		for (short i = *TriggerIndex; (i & 0x1F) != 4; trigger++)
		{
			if (i < 0)
				break;
			i = trigger[1];
		}
		if (*trigger & 4)
		{
			for (short* j = &trigger[2]; (*j / 256) & 0x3C || item != &g_Level.Items[*j & 0x3FF]; j++)
			{
				if (*j & 0x8000)
					return 0;
			}
			return 1;
		}
	}

	return 0;
}

int GetSwitchTrigger(ITEM_INFO* item, short* itemNos, int AttatchedToSwitch)
{
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
	GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	if (TriggerIndex)
	{
		short* trigger;
		for (trigger = TriggerIndex; (*trigger & DATA_TYPE) != TRIGGER_TYPE; trigger++)
		{
			if (*trigger & END_BIT)
				break;
		}

		if (*trigger & 4)
		{
			trigger += 2;
			short* current = itemNos;
			int k = 0;
			do
			{
				if (TRIG_BITS(*trigger) == TO_OBJECT && item != &g_Level.Items[*trigger & VALUE_BITS])
				{
					current[k] = *trigger & VALUE_BITS;
					++k;
				}
				if (*trigger & END_BIT)
					break;
				++trigger;
			}
			while (true);

			return k;
		}
	}

	return 0;
}

int SwitchTrigger(short itemNum, short timer) 
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	if (item->status == ITEM_DEACTIVATED)
	{
		if ((!item->currentAnimState && item->objectNumber != ID_JUMP_SWITCH || item->currentAnimState == 1 && item->objectNumber == ID_JUMP_SWITCH) && timer > 0)
		{
			item->timer = timer;
			item->status = ITEM_ACTIVE;
			if (timer != 1)
				item->timer = 30 * timer;
			return 1;
		}
		if (item->triggerFlags != 6 || item->currentAnimState)
		{
			RemoveActiveItem(itemNum);
			
			item->status = ITEM_NOT_ACTIVE;
			if (!item->itemFlags[0] == 0)
				item->flags |= 0x100;
			if (item->currentAnimState != 1)
				return 1;
			if (item->triggerFlags != 5 && item->triggerFlags != 6)
				return 1;
		}
		else
		{
			item->status = ITEM_ACTIVE;
			return 1;
		}
	}
	else if (item->status)
	{
		return ((item->flags & 0x100u) / 256);
	}
	else
	{
		return 0;
	}

	return 0;
}

void InitialiseSwitch(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (item->triggerFlags >= 1000)
	{
		item->itemFlags[3] = ((item->triggerFlags - 1000) % 10) | 16 * ((item->triggerFlags - 1000) / 10);
		item->triggerFlags = 6;
	}
}

void InitialiseShootSwitch(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (item->triggerFlags == 444)
		item->meshBits &= ~(1 << (Objects[item->objectNumber].nmeshes - 2));
}

void InitialisePulleySwitch(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->itemFlags[3] = item->triggerFlags;
	item->triggerFlags = abs(item->triggerFlags);

	if (item->status == ITEM_INVISIBLE)
	{
		item->itemFlags[1] = 1;
		item->status = ITEM_NOT_ACTIVE;
	}
}

void InitialiseCrowDoveSwitch(short itemNumber)
{
	g_Level.Items[itemNumber].meshBits = 3;
}

void ShootSwitchCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->objectNumber == ID_SHOOT_SWITCH1 && !(item->meshBits & 1))
		item->status = ITEM_INVISIBLE;
}
