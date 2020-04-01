#include "objects.h"
#include "..\Global\global.h"
#include "items.h"
#include "effects.h"
#include "effect2.h"
#include "collide.h"
#include "draw.h"
#include "Lara.h"
#include "sphere.h"
#include "debris.h"
#include "control.h"
#include "switch.h"
#include "box.h"
#include "../Specific/setup.h"

int lastWaterfallY = 0;
short TightRopeBounds[12] = 
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0xFF00, 0x0100, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR TightRopePos = { 0, 0, 0 };

short ParallelBarsBounds[12] =  
{
	0xFD80, 0x0280, 0x02C0, 0x0340, 0xFFA0, 0x0060, 0xF8E4, 0x071C, 0xEAAC, 0x1554, 0xF8E4, 0x071C
};

PHD_VECTOR PolePos = { 0, 0, -208 }; 
PHD_VECTOR PolePosR = { 0, 0, 0 }; 
short PoleBounds[12] = // offset 0xA1250
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0xFE00, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};

extern LaraExtraInfo g_LaraExtra;

void InitialiseSmashObject(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	item->flags = 0;
	item->meshBits = 1;

	ROOM_INFO* r = &Rooms[item->roomNumber];
	FLOOR_INFO* floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);

	if (Boxes[floor->box].overlapIndex & END_BIT)
		Boxes[floor->box].overlapIndex |= BLOCKED;
}

void SmashObject(short itemNumber) 
{
	ITEM_INFO* item = &Items[itemNumber];
	ROOM_INFO* r = &Rooms[item->roomNumber];
	int sector = ((item->pos.zPos - r->z) >> 10) + r->xSize * ((item->pos.xPos - r->x) >> 10);
	
	BOX_INFO* box = &Boxes[r->floor[sector].box];
	if (box->overlapIndex & BOX_LAST)
		box->overlapIndex &= ~BOX_BLOCKED;

	SoundEffect(SFX_SMASH_GLASS, &item->pos, 0);

	item->collidable = 0;
	item->meshBits = 0xFFFE;

	ExplodingDeath(itemNumber, -1, 257); //ExplodingDeath2

	item->flags |= IFLAG_INVISIBLE;

	if (item->status == ITEM_ACTIVE)
		RemoveActiveItem(itemNumber);
	item->status = ITEM_DEACTIVATED;
}

void SmashObjectControl(short itemNumber) 
{
	SmashObject(itemNumber << 16);
}

void BridgeFlatFloor(ITEM_INFO* item, int x, int y, int z, int* height) 
{
	if (item->pos.yPos >= y)
	{
		*height = item->pos.yPos;
		HeightType = WALL;
		OnObject = 1;
	}
}

void BridgeFlatCeiling(ITEM_INFO* item, int x, int y, int z, int* height) 
{
	if (item->pos.yPos < y)
	{
		*height = item->pos.yPos + 256;
	}
}

int GetOffset(ITEM_INFO* item, int x, int z)
{
	if (item->pos.yRot == 0)
	{
		return (-x) & 0x3FF;
	}
	else if (item->pos.yRot == ANGLE(180))
	{
		return x & 0x3FF;
	}
	else if (item->pos.yRot == ANGLE(90))
	{
		return z & 0x3FF;
	}
	else
	{
		return (-z) & 0x3FF;
	}
}

void BridgeTilt1Floor(ITEM_INFO* item, int x, int y, int z, int* height) 
{
	int level = item->pos.yPos + (GetOffset(item, x, z) >> 2);

	if (level >= y)
	{
		*height = level;
		HeightType = WALL;
		OnObject = 1;
	}
}

void BridgeTilt1Ceiling(ITEM_INFO* item, int x, int y, int z, int* height) 
{
	int level = item->pos.yPos + (GetOffset(item, x, z) >> 2);

	if (level < y)
	{
		*height = level + 256;
	}
}

void BridgeTilt2Floor(ITEM_INFO* item, int x, int y, int z, int* height) 
{
	int level = item->pos.yPos + (GetOffset(item, x, z) >> 1);

	if (level >= y)
	{
		*height = level;
		HeightType = WALL;
		OnObject = 1;
	}
}

void BridgeTilt2Ceiling(ITEM_INFO* item, int x, int y, int z, int* height) 
{
	int level = item->pos.yPos + (GetOffset(item, x, z) >> 1);

	if (level < y)
	{
		*height = level + 256;
	}
}

void ControlAnimatingSlots(short itemNumber)
{
	// TODO: TR5 has here a series of hardcoded OCB codes, this function actually is just a placeholder
	ITEM_INFO* item = &Items[itemNumber];

	if (TriggerActive(item))
		AnimateItem(item);
}

void PoleCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNumber];

	if ((TrInput & IN_ACTION) && !Lara.gunStatus && l->currentAnimState == STATE_LARA_STOP && 
		l->animNumber == ANIMATION_LARA_STAY_IDLE
		|| Lara.isMoving && Lara.generalPtr == (void*)itemNumber)
	{
		short rot = item->pos.yRot;
		item->pos.yRot = l->pos.yRot;
		if (TestLaraPosition(PoleBounds, item, l))
		{
			if (MoveLaraPosition(&PolePos, item, l))
			{
				l->animNumber = ANIMATION_LARA_STAY_TO_POLE_GRAB;
				l->currentAnimState = STATE_LARA_POLE_IDLE;
				l->frameNumber = Anims[l->animNumber].frameBase;
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
		&& (l->currentAnimState == STATE_LARA_REACH || l->currentAnimState == STATE_LARA_JUMP_UP))
	{
		if (TestBoundsCollide(item, l, 100))
		{
			if (TestCollision(item, l))
			{
				short rot = item->pos.yRot;
				item->pos.yRot = l->pos.yRot;
				if (l->currentAnimState == STATE_LARA_REACH)
				{
					PolePosR.y = l->pos.yPos - item->pos.yPos + 10;
					AlignLaraPosition(&PolePosR, item, l);
					l->animNumber = ANIMATION_LARA_JUMP_FORWARD_TO_POLE_GRAB;
					l->frameNumber = Anims[l->animNumber].frameBase;
				}
				else
				{
					PolePosR.y = l->pos.yPos - item->pos.yPos + 66;
					AlignLaraPosition(&PolePosR, item, l);
					l->animNumber = ANIMATION_LARA_JUMP_UP_TO_POLE_GRAB;
					l->frameNumber = Anims[l->animNumber].frameBase;
				}
				l->gravityStatus = false;
				l->fallspeed = false;
				l->currentAnimState = STATE_LARA_POLE_IDLE;
				Lara.gunStatus = LG_HANDS_BUSY;
				item->pos.yRot = rot;
			}
		}
	}
	else
	{
		if ((l->currentAnimState < STATE_LARA_POLE_IDLE || l->currentAnimState > STATE_LARA_POLE_TURN_RIGHT) && 
			l->currentAnimState != STATE_LARA_JUMP_BACK)
			ObjectCollision(itemNumber, l, coll);
	}
}

void ControlTriggerTriggerer(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
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
	ITEM_INFO* item = &Items[itemNumber];
	TriggerActive(item);

	if (itemNumber != 0)
	{
		item->status = ITEM_ACTIVE;

		if (item->triggerFlags == 0x29C)
		{
			SoundEffect(SFX_D_METAL_KICKOPEN, &item->pos, 0);
		}
		else if (item->triggerFlags == 0x309)
		{
			SoundEffect(SFX_WATERFALL_LOOP, &item->pos, 0);
		}
	}
	else
	{
		if (item->triggerFlags == 2 || item->triggerFlags == 0x29C)
		{
			item->status = ITEM_INVISIBLE;
		}
	}
}

void TightRopeCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];
	
	if (((TrInput & IN_ACTION) == 0
		|| l->currentAnimState != STATE_LARA_STOP
		|| l->animNumber != ANIMATION_LARA_STAY_IDLE
		|| l->status == ITEM_INVISIBLE
		|| Lara.gunStatus)
		&& (!Lara.isMoving || Lara.generalPtr != (void*)itemNum))
	{
		if (l->currentAnimState == STATE_LARA_TIGHTROPE_FORWARD && 
			l->goalAnimState != STATE_LARA_TIGHTROPE_EXIT && 
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
		if (TestLaraPosition(TightRopeBounds, item, l))
		{
			if (MoveLaraPosition(&TightRopePos, item, l))
			{
				l->currentAnimState = STATE_LARA_TIGHTROPE_ENTER;
				l->animNumber = ANIMATION_LARA_TIGHTROPE_START;
				l->frameNumber = Anims[l->animNumber].frameBase;
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
	ITEM_INFO* item = &Items[itemNumber];
	if (TrInput & IN_ACTION && l->currentAnimState == STATE_LARA_REACH && l->animNumber == ANIMATION_LARA_TRY_HANG_SOLID)
	{
		int test1 = TestLaraPosition(ParallelBarsBounds, item, l);
		int test2 = 0;
		if (!test1)
		{
			item->pos.yRot += -ANGLE(180);
			test2 = TestLaraPosition(ParallelBarsBounds, item, l);
			item->pos.yRot += -ANGLE(180);
		}

		if (test1 || test2)
		{
			l->currentAnimState = STATE_LARA_MISC_CONTROL;
			l->animNumber = ANIMATION_LARA_BARS_GRAB;
			l->frameNumber = Anims[l->animNumber].frameBase;
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

			GetLaraJointPosition(&pos1, LJ_LHAND);
			GetLaraJointPosition(&pos2, LJ_RHAND);
		
			if (l->pos.yRot & 0x4000)
				l->pos.xPos += item->pos.xPos - ((pos1.x + pos2.x) >> 1);
			else
				l->pos.zPos += item->pos.zPos - ((pos1.z + pos2.z) >> 1);
			l->pos.yPos += item->pos.yPos - ((pos1.y + pos2.y) >> 1);

			Lara.generalPtr = item;
		}
		else
		{
			ObjectCollision(itemNumber, l, coll);
		}
	}
	else if (l->currentAnimState != STATE_LARA_BARS_SWING)
	{
		ObjectCollision(itemNumber, l, coll);
	}
}

void ControlXRayMachine(short itemNumber) 
{
	ITEM_INFO* item = &Items[itemNumber];

	if (!TriggerActive(item))
		return;

	/*if (item->triggerFlags == 0)
	{
		if (item->itemFlags[0] == 666)
		{
			if (item->itemFlags[1] != 0)
			{
				item->itemFlags[1]--;
			}
			else
			{
				item->itemFlags[1] = 30;
				SoundEffect(SFX_ALARM, &item->pos, 0);
			}
		}

		if (Lara.skelebob)
		{
			if (g_LaraExtra.Weapons[WEAPON_HK].Present)
			{
				TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 1, 0);
				item->itemFlags[0] = 666;
			}
		}

		return;
	}

	switch (item->triggerFlags)
	{
	case 111:
		if (item->itemFlags[0] != 0)
		{
			item->itemFlags[0]--;

			if (item->itemFlags[0] == 0)
			{
				TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 1, 0);
				RemoveActiveItem(itemNumber);
				item->flags |= IFLAG_INVISIBLE;
			}

			return;
		}

		if (Lara.fired)
			item->itemFlags[0] = 15;
		break;

	case 222:
		if (item->itemFlags[1] >= 144)
		{
			TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 1, 0);
			RemoveActiveItem(itemNumber);
			item->flags |= IFLAG_INVISIBLE;
			return;
		}

		if (item->itemFlags[1] < 128)
		{
			SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
			TriggerFontFire(&Items[item->itemFlags[0]], item->itemFlags[1], item->itemFlags[1] == 0 ? 16 : 1);
		}

		++item->itemFlags[1];
		break;

	case 333:
	{
		ROOM_INFO* r = &Rooms[item->roomNumber];
		MESH_INFO* mesh = r->mesh;
		int j;

		for (j = 0; j < r->numMeshes; j++, mesh++)
		{
			if (mesh->Flags & 1)
			{
				if (item->pos.xPos == mesh->x &&
					item->pos.yPos == mesh->y &&
					item->pos.zPos == mesh->z)
				{
					ShatterObject(NULL, mesh, 128, item->roomNumber, 0);
					mesh->Flags &= ~1;
					SoundEffect(ShatterSounds[gfCurrentLevel - 5][mesh->staticNumber], (PHD_3DPOS*) & mesh->x, 0);
				}
			}
		}

		RemoveActiveItem(itemNumber);
		item->flags |= IFLAG_INVISIBLE;
		break;
	}

	default:
		TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 1, 0);
		RemoveActiveItem(itemNumber);
		break;
	}*/
}

void CutsceneRopeControl(short itemNumber) 
{
	ITEM_INFO* item;
	PHD_VECTOR pos1;
	PHD_VECTOR pos2;
	int dx;
	int dy;
	int dz;

	item = &Items[itemNumber];

	pos1.x = -128;
	pos1.y = -72;
	pos1.z = -16;
	GetJointAbsPosition(&Items[item->itemFlags[2]], &pos1, 0);

	pos2.x = 830;
	pos2.z = -12;
	pos2.y = 0;
	GetJointAbsPosition(&Items[item->itemFlags[3]], &pos2, 0);

	item->pos.xPos = pos2.x;
	item->pos.yPos = pos2.y;
	item->pos.zPos = pos2.z;

	dx = (pos2.x - pos1.x) * (pos2.x - pos1.x);
	dy = (pos2.y - pos1.y) * (pos2.y - pos1.y);
	dz = (pos2.z - pos1.z) * (pos2.z - pos1.z);

	item->itemFlags[1] = ((SQRT_ASM(dx + dy + dz) << 1) + SQRT_ASM(dx + dy + dz)) << 1;
	item->pos.xRot = -4869;
}

void HybridCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll) 
{
	ITEM_INFO* item;

	item = &Items[itemNum];

	/*if (gfCurrentLevel == LVL5_SINKING_SUBMARINE)
	{
		if (item->frameNumber < Anims[item->animNumber].frame_end)
		{
			ObjectCollision(itemNum, laraitem, coll);
		}
	}*/
}

void InitialiseTightRope(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

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

void Inject_Objects()
{
	INJECT(0x00465FE0, TightRopeCollision);
	INJECT(0x00465200, SmashObject);

	/*INJECT(0x00465200, SmashObject);
	INJECT(0x00465330, SmashObjectControl);
	INJECT(0x00465350, BridgeFlatFloor);
	INJECT(0x00465390, BridgeFlatCeiling);
	INJECT(0x00465410, BridgeTilt1Floor);
	INJECT(0x00465480, BridgeTilt1Ceiling);
	INJECT(0x004654D0, BridgeTilt2Floor);
	INJECT(0x00465540, BridgeTilt2Ceiling);
	//INJECT(0x00465590, AnimatingControl);
	INJECT(0x00465A30, PoleCollision);
	INJECT(0x00465D00, ControlTriggerTriggerer);
	INJECT(0x00465DF0, AnimateWaterfalls);
	INJECT(0x00465F10, ControlWaterfall);
	INJECT(0x004661C0, ParallelBarsCollision);
	INJECT(0x00466420, ControlXRayMachine);
	INJECT(0x00466720, CutsceneRopeControl);
	INJECT(0x00466AA0, HybridCollision);*/
}