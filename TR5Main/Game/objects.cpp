#include "framework.h"
#include "objects.h"
#include "items.h"
#include "effect.h"
#include "effect2.h"
#include "draw.h"
#include "Lara.h"
#include "sphere.h"
#include "debris.h"
#include "control.h"
#include "switch.h"
#include "box.h"
#include "setup.h"
#include "tomb4fx.h"
#include "level.h"
#include "input.h"
#include "sound.h"

OBJECT_TEXTURE* WaterfallTextures[6];
float WaterfallY[6];
int lastWaterfallY = 0;
OBJECT_COLLISION_BOUNDS TightRopeBounds =
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0xFF00, 0x0100, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR TightRopePos = { 0, 0, 0 };

OBJECT_COLLISION_BOUNDS ParallelBarsBounds =
{
	0xFD80, 0x0280, 0x02C0, 0x0340, 0xFFA0, 0x0060, 0xF8E4, 0x071C, 0xEAAC, 0x1554, 0xF8E4, 0x071C
};

PHD_VECTOR PolePos = { 0, 0, -208 }; 
PHD_VECTOR PolePosR = { 0, 0, 0 }; 
OBJECT_COLLISION_BOUNDS PoleBounds = // offset 0xA1250
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0xFE00, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};

void ControlAnimatingSlots(short itemNumber)
{
	// TODO: TR5 has here a series of hardcoded OCB codes, this function actually is just a placeholder
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
		AnimateItem(item);
}

void PoleCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if ((TrInput & IN_ACTION) && !Lara.gunStatus && l->currentAnimState == LS_STOP && 
		l->animNumber == LA_STAND_IDLE
		|| Lara.isMoving && Lara.generalPtr == (void*)itemNumber)
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
				Lara.generalPtr = (void*)itemNumber;
			}
			item->pos.yRot = rot;
		}
		else
		{
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNumber)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
			item->pos.yRot = rot;
		}
	}
	else if (TrInput & IN_ACTION
		&& !Lara.gunStatus
		&& l->gravityStatus
		&& l->fallspeed > Lara.gunStatus
		&& (l->currentAnimState == LS_REACH || l->currentAnimState == LS_JUMP_UP))
	{
		if (TestBoundsCollide(item, l, 100))
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
		if ((l->currentAnimState < LS_POLE_IDLE || l->currentAnimState > LS_POLE_TURN_COUNTER_CLOCKWISE) && 
			l->currentAnimState != LS_JUMP_BACK)
			ObjectCollision(itemNumber, l, coll);
	}
}

void ControlTriggerTriggerer(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
	int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	
	if (TriggerIndex)
	{
		short* trigger = TriggerIndex;
		
		if ((*trigger & 0x1F) == 5)
		{
			if ((*trigger & 0x8000) != 0)
				return;
			trigger++;
		}
		
		if ((*trigger & 0x1F) == 6)
		{
			if ((*trigger & 0x8000) != 0)
				return;
			trigger++;
		}
		
		if ((*trigger & 0x1F) == 19)
		{
			if ((*trigger & 0x8000) != 0)
				return;
			trigger++;
		}
		
		if ((*trigger & 0x1F) == 20)
		{
			if (TriggerActive(item))
				*trigger |= 0x20u;
			else
				*trigger &= 0xDFu;
		}
	}
}

void AnimateWaterfalls()
{
	return;

	lastWaterfallY = (lastWaterfallY - 7) & 0x3F;
	float y = lastWaterfallY * 0.00390625f;
	float theY;

	for (int i = 0; i < 6; i++)
	{
		if (Objects[ID_WATERFALL1 + i].loaded)
		{
			OBJECT_TEXTURE* texture = WaterfallTextures[i];

			texture->vertices[0].y = y + WaterfallY[i];
			texture->vertices[1].y = y + WaterfallY[i];
			texture->vertices[2].y = y + WaterfallY[i] + 0.24609375f;
			texture->vertices[3].y = y + WaterfallY[i] + 0.24609375f;

			if (i < 5)
			{
				texture++;

				texture->vertices[0].y = y + WaterfallY[i];
				texture->vertices[1].y = y + WaterfallY[i];
				texture->vertices[2].y = y + WaterfallY[i] + 0.24609375f;
				texture->vertices[3].y = y + WaterfallY[i] + 0.24609375f;
			}
		}
	}
}

void ControlWaterfall(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	int dx = item->pos.xPos - LaraItem->pos.xPos;
	int dy = item->pos.yPos - LaraItem->pos.yPos;
	int dz = item->pos.zPos - LaraItem->pos.zPos;

	if (dx >= -16384 && dx <= 16384 && dy >= -16384 && dy <= 16384 && dz >= -16384 && dz <= 16384)
	{
		if (!(Wibble & 0xC))
		{
			TriggerWaterfallMist(
				item->pos.xPos + 68 * phd_sin(item->pos.yRot),
				item->pos.yPos,
				item->pos.zPos + 68 * phd_cos(item->pos.yRot),
				item->pos.yRot >> 4);
		}

		SoundEffect(SFX_TR4_WATERFALL_LOOP, &item->pos, 0);
	}
}

void TightRopeCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	
	if (((TrInput & IN_ACTION) == 0
		|| l->currentAnimState != LS_STOP
		|| l->animNumber != LA_STAND_IDLE
		|| l->status == ITEM_INVISIBLE
		|| Lara.gunStatus)
		&& (!Lara.isMoving || Lara.generalPtr != (void*)itemNum))
	{
		if (l->currentAnimState == LS_TIGHTROPE_FORWARD && 
			l->goalAnimState != LS_TIGHTROPE_EXIT && 
			!Lara.tightRopeOff)
		{
			if (item->pos.yRot == l->pos.yRot)
			{
				if (abs(item->pos.xPos - l->pos.xPos) + abs(item->pos.zPos - l->pos.zPos) < 640)
					Lara.tightRopeOff = 1;
			}
		}
	}
	else
	{
		item->pos.yRot += -ANGLE(180);
		if (TestLaraPosition(&TightRopeBounds, item, l))
		{
			if (MoveLaraPosition(&TightRopePos, item, l))
			{
				l->currentAnimState = LS_TIGHTROPE_ENTER;
				l->animNumber = LA_TIGHTROPE_START;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.tightRopeOnCount = 60;
				Lara.tightRopeOff = 0;
				Lara.tightRopeFall = 0;
			}
			else
			{
				Lara.generalPtr = (void*)itemNum;
			}
			item->pos.yRot += -ANGLE(180);
		}
		else
		{
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
				Lara.isMoving = false;
			item->pos.yRot += -ANGLE(180);
		}
	}
}

void ParallelBarsCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (TrInput & IN_ACTION && l->currentAnimState == LS_REACH && l->animNumber == LA_REACH)
	{
		int test1 = TestLaraPosition(&ParallelBarsBounds, item, l);
		int test2 = 0;
		if (!test1)
		{
			item->pos.yRot += -ANGLE(180);
			test2 = TestLaraPosition(&ParallelBarsBounds, item, l);
			item->pos.yRot += -ANGLE(180);
		}

		if (test1 || test2)
		{
			l->currentAnimState = LS_MISC_CONTROL;
			l->animNumber = LA_SWINGBAR_GRAB;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			l->fallspeed = false;
			l->gravityStatus = false;

			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;

			if (test1)
				l->pos.yRot = item->pos.yRot;
			else
				l->pos.yRot = item->pos.yRot + -ANGLE(180);

			PHD_VECTOR pos1;
			pos1.x = 0;
			pos1.y = -128;
			pos1.z = 512;

			PHD_VECTOR pos2;
			pos2.x = 0;
			pos2.y = -128;
			pos2.z = 512;

			GetLaraJointPosition(&pos1, LM_LHAND);
			GetLaraJointPosition(&pos2, LM_RHAND);
		
			if (l->pos.yRot & 0x4000)
				l->pos.xPos += item->pos.xPos - ((pos1.x + pos2.x) / 2);
			else
				l->pos.zPos += item->pos.zPos - ((pos1.z + pos2.z) / 2);
			l->pos.yPos += item->pos.yPos - ((pos1.y + pos2.y) / 2);

			Lara.generalPtr = item;
		}
		else
		{
			ObjectCollision(itemNumber, l, coll);
		}
	}
	else if (l->currentAnimState != LS_BARS_SWING)
	{
		ObjectCollision(itemNumber, l, coll);
	}
}

void CutsceneRopeControl(short itemNumber) 
{
	ITEM_INFO* item;
	PHD_VECTOR pos1;
	PHD_VECTOR pos2;
	int dx;
	int dy;
	int dz;

	item = &g_Level.Items[itemNumber];

	pos1.x = -128;
	pos1.y = -72;
	pos1.z = -16;
	GetJointAbsPosition(&g_Level.Items[item->itemFlags[2]], &pos1, 0);

	pos2.x = 830;
	pos2.z = -12;
	pos2.y = 0;
	GetJointAbsPosition(&g_Level.Items[item->itemFlags[3]], &pos2, 0);

	item->pos.xPos = pos2.x;
	item->pos.yPos = pos2.y;
	item->pos.zPos = pos2.z;

	dx = (pos2.x - pos1.x) * (pos2.x - pos1.x);
	dy = (pos2.y - pos1.y) * (pos2.y - pos1.y);
	dz = (pos2.z - pos1.z) * (pos2.z - pos1.z);

	item->itemFlags[1] = ((sqrt(dx + dy + dz) * 2) + sqrt(dx + dy + dz)) * 2;
	item->pos.xRot = -4869;
}

void HybridCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll) 
{
	ITEM_INFO* item;

	item = &g_Level.Items[itemNum];

	/*if (gfCurrentLevel == LVL5_SINKING_SUBMARINE)
	{
		if (item->frameNumber < g_Level.Anims[item->animNumber].frame_end)
		{
			ObjectCollision(itemNum, laraitem, coll);
		}
	}*/
}

void InitialiseTightRope(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->pos.yRot > 0)
	{
		if (item->pos.yRot == ANGLE(90))
			item->pos.xPos -= 256;
	}
	else if (item->pos.yRot)
	{
		if (item->pos.yRot == -ANGLE(180))
		{
			item->pos.zPos += 256;
		}
		else if (item->pos.yRot == -ANGLE(90))
		{
			item->pos.xPos += 256;
		}
	}
	else
	{
		item->pos.zPos -= 256;
	}
}

void InitialiseAnimating(short itemNumber)
{
	/*ITEM_INFO* item = &g_Level.Items[itemNumber];
	item->currentAnimState = 0;
	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;*/
}

void AnimatingControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
		return;

	item->status = ITEM_ACTIVE;
	AnimateItem(item);

	// TODO: ID_SHOOT_SWITCH2 probably the bell in Trajan Markets, use LUA for that
	/*if (item->frameNumber >= g_Level.Anims[item->animNumber].frameEnd)
	{
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		RemoveActiveItem(itemNumber);
		item->aiBits = 0;
		item->status = ITEM_NOT_ACTIVE;
	}*/
}

void HighObject2Control(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
		return;


	if (!item->itemFlags[2])
	{
		int div = (item->triggerFlags % 10) * 1024;
		int mod = (item->triggerFlags / 10) * 1024;
		item->itemFlags[0] = GetRandomControl() % div;
		item->itemFlags[1] = GetRandomControl() % mod;
		item->itemFlags[2] = (GetRandomControl() & 0xF) + 15;
	}

	if (--item->itemFlags[2] < 15)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];
		spark->on = 1;
		spark->sR = -1;
		spark->sB = 16;
		spark->sG = (GetRandomControl() & 0x1F) + 48;
		spark->dR = (GetRandomControl() & 0x3F) - 64;
		spark->dB = 0;
		spark->dG = (GetRandomControl() & 0x3F) + -128;
		spark->fadeToBlack = 4;
		spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
		spark->transType = COLADD;
		spark->life = spark->sLife = (GetRandomControl() & 3) + 24;
		spark->x = item->itemFlags[1] + (GetRandomControl() & 0x3F) + item->pos.xPos - 544;
		spark->y = item->pos.yPos;
		spark->z = item->itemFlags[0] + (GetRandomControl() & 0x3F) + item->pos.zPos - 544;
		spark->xVel = (GetRandomControl() & 0x1FF) - 256;
		spark->friction = 6;
		spark->zVel = (GetRandomControl() & 0x1FF) - 256;
		spark->rotAng = GetRandomControl() & 0xFFF;
		spark->rotAdd = (GetRandomControl() & 0x3F) - 32;
		spark->maxYvel = 0;
		spark->yVel = -512 - (GetRandomControl() & 0x3FF);
		spark->sSize = spark->size = (GetRandomControl() & 0xF) + 32;
		spark->dSize = spark->size / 4;

		if (GetRandomControl() & 3)
		{
			spark->flags = SP_ROTATE | SP_DEF | SP_SCALE | SP_EXPDEF;
			spark->scalar = 3;
			spark->gravity = (GetRandomControl() & 0x3F) + 32;
		}
		else
		{
			spark->flags = SP_ROTATE | SP_DEF | SP_SCALE;
			spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_UNDERWATERDUST;
			spark->scalar = 1;
			spark->gravity = (GetRandomControl() & 0xF) + 64;
		}
	}
}
