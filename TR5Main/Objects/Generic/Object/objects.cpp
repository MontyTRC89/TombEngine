#include "framework.h"
#include "Objects/Generic/Object/objects.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Sound/sound.h"
#include "Game/collision/collide_item.h"

OBJECT_TEXTURE* WaterfallTextures[6];
float WaterfallY[6];
int lastWaterfallY = 0;

PHD_VECTOR TightRopePos = { 0, 0, 0 };
OBJECT_COLLISION_BOUNDS TightRopeBounds =
{ -256, 256, 0, 0, -256, 256, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), ANGLE(-10), ANGLE(10) };

OBJECT_COLLISION_BOUNDS ParallelBarsBounds =
{ -640, 640, 704, 832, -96, 96, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), ANGLE(-10), ANGLE(10) };

void ControlAnimatingSlots(short itemNumber)
{
	// TODO: TR5 has here a series of hardcoded OCB codes, this function actually is just a placeholder
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
		AnimateItem(item);
}

void ControlTriggerTriggerer(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	FLOOR_INFO* floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &item->RoomNumber);

	if (floor->Flags.MarkTriggerer)
	{
		if (TriggerActive(item))
			floor->Flags.MarkTriggererActive = true;
		else
			floor->Flags.MarkTriggererActive = false;
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

	int dx = item->Position.xPos - LaraItem->Position.xPos;
	int dy = item->Position.yPos - LaraItem->Position.yPos;
	int dz = item->Position.zPos - LaraItem->Position.zPos;

	if (dx >= -16384 && dx <= 16384 && dy >= -16384 && dy <= 16384 && dz >= -16384 && dz <= 16384)
	{
		if (!(Wibble & 0xC))
		{
			TriggerWaterfallMist(
				item->Position.xPos + 68 * phd_sin(item->Position.yRot),
				item->Position.yPos,
				item->Position.zPos + 68 * phd_cos(item->Position.yRot),
				item->Position.yRot >> 4);
		}

		SoundEffect(SFX_TR4_WATERFALL_LOOP, &item->Position, 0);
	}
}

void TightRopeCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	
	if (((TrInput & IN_ACTION) == 0
		|| l->ActiveState != LS_IDLE
		|| l->AnimNumber != LA_STAND_IDLE
		|| l->Status == ITEM_INVISIBLE
		|| Lara.gunStatus)
		&& (!Lara.Control.IsMoving || Lara.interactedItem !=itemNum))
	{
#ifdef NEW_TIGHTROPE
		if(l->ActiveState == LS_TIGHTROPE_FORWARD &&
		   l->TargetState != LS_TIGHTROPE_EXIT &&
		   !Lara.Control.TightropeControl.CanDismount)
		{
			if(item->Position.yRot == l->Position.yRot)
			{
				if(abs(item->Position.xPos - l->Position.xPos) + abs(item->Position.zPos - l->Position.zPos) < 640)
					Lara.Control.TightropeControl.CanDismount = true;
			}
		}

#else // !NEW_TIGHTROPE
		if(l->ActiveState == LS_TIGHTROPE_FORWARD &&
		   l->TargetState != LS_TIGHTROPE_EXIT &&
		   !Lara.Control.TightropeControl.Off)
		{
			if(item->Position.yRot == l->Position.yRot)
			{
				if(abs(item->Position.xPos - l->Position.xPos) + abs(item->Position.zPos - l->Position.zPos) < 640)
					Lara.tightRopeOff = true;
			}
		}
#endif
	}
	else
	{
		item->Position.yRot += -ANGLE(180);

		if (TestLaraPosition(&TightRopeBounds, item, l))
		{
			if (MoveLaraPosition(&TightRopePos, item, l))
			{
				l->ActiveState = LS_TIGHTROPE_ENTER;
				l->AnimNumber = LA_TIGHTROPE_START;
				l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
				Lara.Control.IsMoving = false;
				ResetLaraFlex(l);
#ifdef NEW_TIGHTROPE
				Lara.Control.TightropeControl.Balance = 0;
				Lara.Control.TightropeControl.CanDismount = false;
				Lara.Control.TightropeControl.TightropeItem = itemNum;
				Lara.Control.TightropeControl.TimeOnTightrope = 0;
#else // !NEW_TIGHTROPE
				Lara.tightRopeOnCount = 60;
				Lara.tightRopeOff = 0;
				Lara.tightRopeFall = 0;
#endif
			}
			else
				Lara.interactedItem = itemNum;

			item->Position.yRot += -ANGLE(180);
		}
		else
		{
			if (Lara.Control.IsMoving && Lara.interactedItem == itemNum)
				Lara.Control.IsMoving = false;

			item->Position.yRot += -ANGLE(180);
		}
	}
}

void ParallelBarsCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (TrInput & IN_ACTION && l->ActiveState == LS_REACH && l->AnimNumber == LA_REACH)
	{
		int test1 = TestLaraPosition(&ParallelBarsBounds, item, l);
		int test2 = 0;
		if (!test1)
		{
			item->Position.yRot += -ANGLE(180);
			test2 = TestLaraPosition(&ParallelBarsBounds, item, l);
			item->Position.yRot += -ANGLE(180);
		}

		if (test1 || test2)
		{
			l->ActiveState = LS_MISC_CONTROL;
			l->AnimNumber = LA_SWINGBAR_GRAB;
			l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
			l->VerticalVelocity = false;
			l->Airborne = false;

			ResetLaraFlex(item);

			if (test1)
				l->Position.yRot = item->Position.yRot;
			else
				l->Position.yRot = item->Position.yRot + -ANGLE(180);

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
		
			if (l->Position.yRot & 0x4000)
				l->Position.xPos += item->Position.xPos - ((pos1.x + pos2.x) >> 1);
			else
				l->Position.zPos += item->Position.zPos - ((pos1.z + pos2.z) / 2);
			l->Position.yPos += item->Position.yPos - ((pos1.y + pos2.y) / 2);

			Lara.interactedItem = itemNumber;
		}
		else
		{
			ObjectCollision(itemNumber, l, coll);
		}
	}
	else if (l->ActiveState != LS_BARS_SWING)
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
	GetJointAbsPosition(&g_Level.Items[item->ItemFlags[2]], &pos1, 0);

	pos2.x = 830;
	pos2.z = -12;
	pos2.y = 0;
	GetJointAbsPosition(&g_Level.Items[item->ItemFlags[3]], &pos2, 0);

	item->Position.xPos = pos2.x;
	item->Position.yPos = pos2.y;
	item->Position.zPos = pos2.z;

	dx = (pos2.x - pos1.x) * (pos2.x - pos1.x);
	dy = (pos2.y - pos1.y) * (pos2.y - pos1.y);
	dz = (pos2.z - pos1.z) * (pos2.z - pos1.z);

	item->ItemFlags[1] = ((sqrt(dx + dy + dz) * 2) + sqrt(dx + dy + dz)) * 2;
	item->Position.xRot = -4869;
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

	if (item->Position.yRot > 0)
	{
		if (item->Position.yRot == ANGLE(90))
			item->Position.xPos -= 256;
	}
	else if (item->Position.yRot)
	{
		if (item->Position.yRot == -ANGLE(180))
		{
			item->Position.zPos += 256;
		}
		else if (item->Position.yRot == -ANGLE(90))
		{
			item->Position.xPos += 256;
		}
	}
	else
	{
		item->Position.zPos -= 256;
	}
}

void InitialiseAnimating(short itemNumber)
{
	/*ITEM_INFO* item = &g_Level.Items[itemNumber];
	item->ActiveState = 0;
	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;*/
}

void AnimatingControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
		return;

	item->Status = ITEM_ACTIVE;
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


	if (!item->ItemFlags[2])
	{
		int div = item->TriggerFlags % 10 << 10;
		int mod = item->TriggerFlags / 10 << 10;
		item->ItemFlags[0] = GetRandomControl() % div;
		item->ItemFlags[1] = GetRandomControl() % mod;
		item->ItemFlags[2] = (GetRandomControl() & 0xF) + 15;
	}

	if (--item->ItemFlags[2] < 15)
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
		spark->transType = TransTypeEnum::COLADD;
		spark->life = spark->sLife = (GetRandomControl() & 3) + 24;
		spark->x = item->ItemFlags[1] + (GetRandomControl() & 0x3F) + item->Position.xPos - 544;
		spark->y = item->Position.yPos;
		spark->z = item->ItemFlags[0] + (GetRandomControl() & 0x3F) + item->Position.zPos - 544;
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
