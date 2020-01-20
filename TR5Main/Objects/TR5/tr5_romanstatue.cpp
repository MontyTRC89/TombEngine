#include "../newobjects.h"
#include "../../Game/sphere.h"
#include "../../Game/items.h"
#include "../../Game/tomb4fx.h"
#include "../../Game/effect2.h"

// TODO: check dword_51CF98 name
void InitialiseRomanStatue(short itemNum)
{
    /*
    ITEM_INFO* v1; // esi
    __int16 v2; // ax
    unsigned int v3; // edi
    __int16 v4; // cx
    int v5; // eax
    int v6; // eax

    v1 = &Items[itemNum];
    ClearItem(itemNum);
    v2 = objects[ID_ROMAN_GOD].anim_index + 16;
    v3 = v1->flags2 & 0xFFFFFFF9;
    v1->anim_number = objects[ID_ROMAN_GOD].anim_index + 16;
    v4 = anims[v2].frame_base;
    v1->goal_anim_state = 13;
    v1->current_anim_state = 13;
    v5 = v1->pos.y_rot;
    v1->frame_number = v4;
    v1->flags2 = v3;
    v6 = ((v5 + 0x4000) >> 3) & 0x1FFE;
    v1->pos.x_pos += 1944 * rcossin_tbl[v6] >> 14;
    v1->pos.z_pos += 1944 * rcossin_tbl[v6 + 1] >> 14;
    memset(&dword_51CF98, 0, 0x2Cu);
    *(&dword_51CF98 + 44) = 0;
    */
}

void RomanStatueHitEffect(ITEM_INFO* item, PHD_VECTOR* pos, int joint)
{
	GetJointAbsPosition(item, pos, joint);

	if (!(GetRandomControl() & 0x1F))
	{
		short fxNumber = CreateNewEffect(item->roomNumber);
		if (fxNumber != -1)
		{
			FX_INFO* fx = &Effects[fxNumber];

			fx->pos.xPos = pos->x;
			fx->pos.yPos = pos->y;
			fx->pos.zPos = pos->z;
			fx->roomNumber = item->roomNumber;
			fx->pos.zRot = 0;
			fx->pos.xRot = 0;
			fx->pos.yRot = 2 * GetRandomControl();
			fx->speed = 1;
			fx->fallspeed = 0;
			fx->objectNumber = ID_BODY_PART;
			fx->shade = 16912;
			fx->flag2 = 9729;
			fx->frameNumber = Objects[ID_BUBBLES].meshIndex + 2 * (GetRandomControl() & 7);
			fx->counter = 0;
			fx->flag1 = 0;
		}
	}
	
	if (!(GetRandomControl() & 0xF))
	{
		SMOKE_SPARKS* spark = &SmokeSparks[GetFreeSmokeSpark()];

		spark->on = 1;
		spark->sShade = 0;
		spark->colFadeSpeed = 4;
		spark->fadeToBlack = 32;
		spark->dShade = (GetRandomControl() & 0xF) + 64;
		spark->transType = COLADD;
		spark->life = spark->sLife= (GetRandomControl() & 3) + 64;
		spark->x = (GetRandomControl() & 0x1F) + pos->x - 16;
		spark->y = (GetRandomControl() & 0x1F) + pos->y - 16;
		spark->z = (GetRandomControl() & 0x1F) + pos->z - 16;
		spark->yVel = 0;
		spark->xVel = (GetRandomControl() & 0x7F) - 64;
		spark->friction = 4;
		spark->flags = SP_ROTATE;
		spark->zVel = (GetRandomControl() & 0x7F) - 64;
		spark->rotAng = GetRandomControl() & 0xFFF;
		spark->rotAdd = (GetRandomControl() & 0x1F) - 16;
		spark->maxYvel = 0;
		spark->gravity = (GetRandomControl() & 7) + 8;
		spark->mirror = 0;
		spark->sSize = spark->size = (GetRandomControl() & 7) + 8;
		spark->dSize = spark->size * 2;
	}
}

void TriggerRomanStatueShockwaveAttackSparks(int x, int y, int z, int color)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];

	spark->dG = (color >> 16) & 0xFF;
	spark->sG = (color >> 16) & 0xFF;
	spark->colFadeSpeed = 2;
	spark->dR = (color >> 8) & 0xFF;
	spark->sR = (color >> 8) & 0xFF;
	spark->transType = COLADD;
	spark->life = 16;
	spark->sLife = 16;
	spark->x = x;
	spark->on = 1;
	spark->dB = (color >> 24) & 0xFF;
	spark->sB = (color >> 24) & 0xFF;
	spark->fadeToBlack = 4;
	spark->y = y;
	spark->z = z;
	spark->zVel = 0;
	spark->yVel = 0;
	spark->xVel = 0;
	spark->flags = SP_SCALE | SP_DEF;
	spark->scalar = 3;
	spark->maxYvel = 0;
	spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 11;
	spark->gravity = 0;
	spark->dSize = spark->sSize = spark->size = (color >> 24) + (GetRandomControl() & 3);
}

void TriggerRomanStatueScreamingSparks(int x, int y, int z, short xv, short yv, short zv, int flags)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];
	
	spark->on = 1;
	spark->sR = 0;
	spark->sG = 0;
	spark->sB = 0;
	spark->dR = 64;
	if (flags)
	{
		spark->dG = (GetRandomControl() & 0x3F) - 64;
		spark->dB = spark->dG >> 1;
	}
	else
	{
		spark->dB = (GetRandomControl() & 0x3F) - 64;
		spark->dG = spark->dB >> 1;
	}
	spark->y = y;
	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 4;
	spark->life = 16;
	spark->sLife = 16;
	spark->z = z;
	spark->x = x;
	spark->transType = COLADD;
	spark->xVel = xv;
	spark->yVel = yv;
	spark->zVel = zv;
	spark->friction = 34;
	spark->maxYvel = 0;
	spark->gravity = 0;
	spark->flags = 0;
}

void TriggerRomanStatueAttackEffect1(short itemNum, int factor)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];
	
	spark->on = 1;
	spark->sR = 0;
	spark->sB = (GetRandomControl() & 0x3F) - 96;
	spark->dB = (GetRandomControl() & 0x3F) - 96;
	spark->dR = 0;
	if (factor < 16)
	{
		spark->sB = factor * spark->sB >> 4;
		spark->dB = factor * spark->dB >> 4;
	}
	spark->sG = spark->sB >> 1;
	spark->dG = spark->dB >> 1;
	spark->fadeToBlack = 4;
	spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
	spark->transType = COLADD;
	spark->dynamic = -1;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 32;
	spark->y = 0;
	spark->x = (GetRandomControl() & 0x1F) - 16;
	spark->z = (GetRandomControl() & 0x1F) - 16;
	spark->yVel = 0;
	spark->xVel = (byte)GetRandomControl() - 128;
	spark->friction = 4;
	spark->zVel = (byte)GetRandomControl() - 128;
	spark->flags = SP_NODEATTATCH | SP_EXPDEF | SP_ITEM | SP_ROTATE | SP_DEF | SP_SCALE; // 4762;
	spark->fxObj = itemNum;
	spark->nodeNumber = 6;
	spark->rotAng = GetRandomControl() & 0xFFF;
	spark->rotAdd = (GetRandomControl() & 0x3F) - 32;
	spark->maxYvel = 0;
	spark->gravity = -8 - (GetRandomControl() & 7);
	spark->scalar = 2;
	spark->dSize = 4;
	spark->sSize = spark->size = factor * ((GetRandomControl() & 0x1F) + 64) >> 4;
}

void RomanStatueAttack(PHD_3DPOS* pos, short roomNumber, short count)
{
	short fxNum = CreateNewEffect(roomNumber);

	if (fxNum != NO_ITEM)
	{
		FX_INFO* fx = &Effects[fxNum];

		fx->pos.xPos = pos->xPos;
		fx->pos.yPos = pos->yPos;
		fx->pos.zPos = pos->zPos;
		fx->pos.xRot = pos->xRot;
		fx->pos.yRot = pos->yRot;
		fx->pos.zRot = 0;
		fx->roomNumber = roomNumber;
		fx->counter = 16 * count + 15;
		fx->flag1 = 1;
		fx->objectNumber = ID_BUBBLES;
		fx->speed = (GetRandomControl() & 0x1F) + 64;
		fx->frameNumber = Objects[ID_BUBBLES].meshIndex + 16;
	}
}