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
{
	-256, 256,
	0, 0,
	-256, 256,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	ANGLE(-10.0f), ANGLE(10.0f)
};

OBJECT_COLLISION_BOUNDS ParallelBarsBounds =
{
	-640, 640,
	704, 832,
	-96, 96,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	ANGLE(-10.0f), ANGLE(10.0f)
};

void ControlAnimatingSlots(short itemNumber)
{
	// TODO: TR5 has here a series of hardcoded OCB codes, this function actually is just a placeholder
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
		AnimateItem(item);
}

void ControlTriggerTriggerer(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
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
	auto* item = &g_Level.Items[itemNumber];

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

void TightropeCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* tightropeItem = &g_Level.Items[itemNumber];
	
	if ((!(TrInput & IN_ACTION) ||
		laraItem->Animation.ActiveState != LS_IDLE ||
		laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
		laraItem->Status == ITEM_INVISIBLE ||
		laraInfo->Control.HandStatus != HandStatus::Free) &&
		(!laraInfo->Control.IsMoving || laraInfo->InteractedItem !=itemNumber))
	{
#ifdef NEW_TIGHTROPE
		if (laraItem->Animation.ActiveState == LS_TIGHTROPE_WALK &&
		   laraItem->Animation.TargetState != LS_TIGHTROPE_DISMOUNT &&
		   !laraInfo->Control.Tightrope.CanDismount)
		{
			if (tightropeItem->Position.yRot == laraItem->Position.yRot)
			{
				if (abs(tightropeItem->Position.xPos - laraItem->Position.xPos) + abs(tightropeItem->Position.zPos - laraItem->Position.zPos) < 640)
					laraInfo->Control.Tightrope.CanDismount = true;
			}
		}

#else // !NEW_TIGHTROPE
		if (laraItem->Animation.ActiveState == LS_TIGHTROPE_WALK &&
		   laraItem->Animation.TargetState != LS_TIGHTROPE_DISMOUNT &&
		   !laraInfo->Control.Tightrope.Off)
		{
			if (item->Position.yRot == laraItem->Position.yRot)
			{
				if (abs(item->Position.xPos - laraItem->Position.xPos) + abs(item->Position.zPos - laraItem->Position.zPos) < 640)
					laraInfo->tightRopeOff = true;
			}
		}
#endif
	}
	else
	{
		tightropeItem->Position.yRot += -ANGLE(180.0f);

		if (TestLaraPosition(&TightRopeBounds, tightropeItem, laraItem))
		{
			if (MoveLaraPosition(&TightRopePos, tightropeItem, laraItem))
			{
				laraItem->Animation.ActiveState = LS_TIGHTROPE_ENTER;
				laraItem->Animation.AnimNumber = LA_TIGHTROPE_START;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraInfo->Control.IsMoving = false;
				ResetLaraFlex(laraItem);
#ifdef NEW_TIGHTROPE
				laraInfo->Control.Tightrope.Balance = 0;
				laraInfo->Control.Tightrope.CanDismount = false;
				laraInfo->Control.Tightrope.TightropeItem = itemNumber;
				laraInfo->Control.Tightrope.TimeOnTightrope = 0;
#else // !NEW_TIGHTROPE
				laraInfo->Control.Tightrope.OnCount = 60;
				laraInfo->Control.Tightrope.Off = 0;
				laraInfo->Control.Tightrope.Fall = 0;
#endif
			}
			else
				laraInfo->InteractedItem = itemNumber;

			tightropeItem->Position.yRot += -ANGLE(180.0f);
		}
		else
		{
			if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
				laraInfo->Control.IsMoving = false;

			tightropeItem->Position.yRot += -ANGLE(180.0f);
		}
	}
}

void HorizontalBarCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* barItem = &g_Level.Items[itemNumber];

	if (TrInput & IN_ACTION &&
		laraItem->Animation.ActiveState == LS_REACH &&
		laraItem->Animation.AnimNumber == LA_REACH)
	{
		int test1 = TestLaraPosition(&ParallelBarsBounds, barItem, laraItem);
		int test2 = 0;
		if (!test1)
		{
			barItem->Position.yRot += -ANGLE(180.0f);
			test2 = TestLaraPosition(&ParallelBarsBounds, barItem, laraItem);
			barItem->Position.yRot += -ANGLE(180);
		}

		if (test1 || test2)
		{
			laraItem->Animation.ActiveState = LS_MISC_CONTROL;
			laraItem->Animation.AnimNumber = LA_SWINGBAR_GRAB;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraItem->Animation.VerticalVelocity = false;
			laraItem->Animation.Airborne = false;

			ResetLaraFlex(barItem);

			if (test1)
				laraItem->Position.yRot = barItem->Position.yRot;
			else
				laraItem->Position.yRot = barItem->Position.yRot + -ANGLE(180.0f);

			PHD_VECTOR pos1 = { 0, -128, 512 };
			GetLaraJointPosition(&pos1, LM_LHAND);

			PHD_VECTOR pos2 = { 0, -128, 512 };
			GetLaraJointPosition(&pos2, LM_RHAND);
		
			if (laraItem->Position.yRot & 0x4000)
				laraItem->Position.xPos += barItem->Position.xPos - ((pos1.x + pos2.x) >> 1);
			else
				laraItem->Position.zPos += barItem->Position.zPos - ((pos1.z + pos2.z) / 2);
			laraItem->Position.yPos += barItem->Position.yPos - ((pos1.y + pos2.y) / 2);

			laraInfo->InteractedItem = itemNumber;
		}
		else
			ObjectCollision(itemNumber, laraItem, coll);
	}
	else if (laraItem->Animation.ActiveState != LS_HORIZONTAL_BAR_SWING)
		ObjectCollision(itemNumber, laraItem, coll);
}

void CutsceneRopeControl(short itemNumber) 
{
	auto* ropeItem = &g_Level.Items[itemNumber];

	PHD_VECTOR pos1 = { -128, -72, -16 };
	GetJointAbsPosition(&g_Level.Items[ropeItem->ItemFlags[2]], &pos1, 0);

	PHD_VECTOR pos2 = { 830, -12, 0 };
	GetJointAbsPosition(&g_Level.Items[ropeItem->ItemFlags[3]], &pos2, 0);

	ropeItem->Position.xPos = pos2.x;
	ropeItem->Position.yPos = pos2.y;
	ropeItem->Position.zPos = pos2.z;

	int dx = (pos2.x - pos1.x) * (pos2.x - pos1.x);
	int dy = (pos2.y - pos1.y) * (pos2.y - pos1.y);
	int dz = (pos2.z - pos1.z) * (pos2.z - pos1.z);

	ropeItem->ItemFlags[1] = ((sqrt(dx + dy + dz) * 2) + sqrt(dx + dy + dz)) * 2;
	ropeItem->Position.xRot = -4869;
}

void HybridCollision(short itemNumber, ITEM_INFO* laraitem, CollisionInfo* coll) 
{
	auto* item = &g_Level.Items[itemNumber];

	/*if (gfCurrentLevel == LVL5_SINKING_SUBMARINE)
	{
		if (item->frameNumber < g_Level.Anims[item->animNumber].frame_end)
		{
			ObjectCollision(itemNumber, laraitem, coll);
		}
	}*/
}

void InitialiseTightrope(short itemNumber)
{
	auto* tightropeItem = &g_Level.Items[itemNumber];

	if (tightropeItem->Position.yRot > 0)
	{
		if (tightropeItem->Position.yRot == ANGLE(90.0f))
			tightropeItem->Position.xPos -= 256;
	}
	else if (tightropeItem->Position.yRot)
	{
		if (tightropeItem->Position.yRot == -ANGLE(180.0f))
			tightropeItem->Position.zPos += CLICK(1);
		else if (tightropeItem->Position.yRot == -ANGLE(90.0f))
			tightropeItem->Position.xPos += CLICK(1);
	}
	else
		tightropeItem->Position.zPos -= CLICK(1);
}

void InitialiseAnimating(short itemNumber)
{
	/*auto* item = &g_Level.Items[itemNumber];
	item->ActiveState = 0;
	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;*/
}

void AnimatingControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

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
	auto* item = &g_Level.Items[itemNumber];

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
		auto* spark = &Sparks[GetFreeSpark()];
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
