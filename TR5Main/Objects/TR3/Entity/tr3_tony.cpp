#include "framework.h"
#include "Objects/TR3/Entity/tr3_tony.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/lot.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/lara_fx.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Objects/TR3/boss.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Lara;

enum TonyFlameType
{
	T_NOFLAME = 0,
	T_ROCKZAPPL = 0,
	T_ROCKZAPPR,
	T_ZAPP,
	T_DROPPER,
	T_ROCKZAPPDEBRIS,
	T_ZAPPDEBRIS,
	T_DROPPERDEBRIS
};

struct TONY_FLAME
{
	bool on;
	PHD_VECTOR pos;
	int fallspeed;
	int speed;
	short yRot;
	short room_number;
	TonyFlameType type;
};

enum TONY_STATE
{
	TONYBOSS_WAIT,
	TONYBOSS_RISE,
	TONYBOSS_FLOAT,
	TONYBOSS_ZAPP,
	TONYBOSS_ROCKZAPP,
	TONYBOSS_BIGBOOM,
	TONYBOSS_DEATH
};

static BOSS_STRUCT BossData;
#define TONYBOSS_TURN ANGLE(2.0f)
#define TONYBOSS_HITS 1 //Tony Harder To Kill, was 100 (6 shotgun shots)
#define MAX_TONY_TRIGGER_RANGE 0x4000

static void TriggerTonyEffect(const TONY_FLAME flame)
{
	short fx_number = CreateNewEffect(flame.room_number);
	if (fx_number != -1)
	{
		FX_INFO* fx = &EffectList[fx_number];
		fx->pos.xPos = flame.pos.x;
		fx->pos.yPos = flame.pos.y;
		fx->pos.zPos = flame.pos.z;
		fx->fallspeed = flame.fallspeed;
		fx->pos.xRot = 0;
		fx->pos.yRot = flame.yRot;
		fx->pos.zRot = 0;
		fx->objectNumber = ID_TONY_BOSS_FLAME;
		fx->speed = flame.speed;
		fx->shade = 0;
		fx->flag1 = flame.type;
		fx->flag2 = (GetRandomControl() & 3) + 1;

		switch (flame.type)
		{
		case T_ZAPPDEBRIS:
			fx->flag2 *= 2;
			break;
		case T_ZAPP:
			fx->flag2 = 0;
			break;
		}
	}
}

static void TriggerTonyFlame(short itemNum, int hand)
{
	ITEM_INFO* item;
	SPARKS* sptr;
	int dx, dz;

	item = &g_Level.Items[itemNum];
	dx = LaraItem->Position.xPos - item->Position.xPos;
	dz = LaraItem->Position.zPos - item->Position.zPos;
	if (dx < -MAX_TONY_TRIGGER_RANGE || dx > MAX_TONY_TRIGGER_RANGE || dz < -MAX_TONY_TRIGGER_RANGE || dz > MAX_TONY_TRIGGER_RANGE)
		return;

	sptr = &Sparks[GetFreeSpark()];
	sptr->on = true;
	sptr->sR = 255;
	sptr->sG = 48 + (GetRandomControl() & 31);
	sptr->sB = 48;
	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;
	sptr->colFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->fadeToBlack = 8;
	sptr->sLife = sptr->life = (GetRandomControl() & 7) + 24;
	sptr->transType = TransTypeEnum::COLADD;
	sptr->extras = NULL;
	sptr->dynamic = -1;
	sptr->x = ((GetRandomControl() & 15) - 8);
	sptr->y = 0;
	sptr->z = ((GetRandomControl() & 15) - 8);
	sptr->xVel = ((GetRandomControl() & 255) - 128);
	sptr->yVel = -(GetRandomControl() & 15) - 16;
	sptr->zVel = ((GetRandomControl() & 255) - 128);
	sptr->friction = 5;

	if (GetRandomControl() & 1)
	{
		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_ITEM | SP_NODEATTACH;
		sptr->rotAng = GetRandomControl() & 4095;
		if (GetRandomControl() & 1)
			sptr->rotAdd = -(GetRandomControl() & 15) - 16;
		else
			sptr->rotAdd = (GetRandomControl() & 15) + 16;
	}
	else
	{
		sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTACH;
	}

	sptr->gravity = -(GetRandomControl() & 31) - 16;
	sptr->maxYvel = -(GetRandomControl() & 7) - 16;
	sptr->fxObj = itemNum;
	sptr->nodeNumber = hand;
	sptr->def = Objects[ID_DEFAULT_SPRITES].meshIndex;
	sptr->scalar = 1;
	unsigned char size = (GetRandomControl() & 31) + 32;
	sptr->size = size;
	sptr->sSize = size;
	sptr->dSize = size / 4;
}

static void TriggerFireBallFlame(short fxNumber, long type, long xv, long yv, long zv)
{
	SPARKS* sptr;
	int dx, dz;

	dx = LaraItem->Position.xPos - EffectList[fxNumber].pos.xPos;
	dz = LaraItem->Position.zPos - EffectList[fxNumber].pos.zPos;
	if (dx < -MAX_TONY_TRIGGER_RANGE || dx > MAX_TONY_TRIGGER_RANGE || dz < -MAX_TONY_TRIGGER_RANGE || dz > MAX_TONY_TRIGGER_RANGE)
		return;

	sptr = &Sparks[GetFreeSpark()];
	sptr->on = true;
	sptr->sR = 255;
	sptr->sG = 48 + (GetRandomControl() & 31);
	sptr->sB = 48;
	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;
	sptr->colFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->fadeToBlack = 8;
	sptr->sLife = sptr->life = (GetRandomControl() & 7) + 24;
	sptr->transType = TransTypeEnum::COLADD;
	sptr->extras = 0;
	sptr->dynamic = -1;
	sptr->x = ((GetRandomControl() & 15) - 8);
	sptr->y = 0;
	sptr->z = ((GetRandomControl() & 15) - 8);
	sptr->xVel = xv + ((GetRandomControl() & 255) - 128);
	sptr->yVel = yv;
	sptr->zVel = zv + ((GetRandomControl() & 255) - 128);
	sptr->friction = 5;

	if (GetRandomControl() & 1)
	{
		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX;
		sptr->rotAng = GetRandomControl() & 4095;
		if (GetRandomControl() & 1)
			sptr->rotAdd = -(GetRandomControl() & 15) - 16;
		else
			sptr->rotAdd = (GetRandomControl() & 15) + 16;
	}
	else
		sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;

	sptr->fxObj = (unsigned char)fxNumber;
	sptr->def = (unsigned char)Objects[ID_DEFAULT_SPRITES].meshIndex;
	sptr->scalar = 1;
	unsigned char size = (GetRandomControl() & 31) + 64;
	sptr->size = size;
	sptr->sSize = size;
	sptr->dSize = size / 4;

	if (type == T_ROCKZAPPL || type == T_ROCKZAPPR)
	{
		sptr->gravity = (GetRandomControl() & 31) + 16;
		sptr->maxYvel = (GetRandomControl() & 15) + 48;
		sptr->yVel = -sptr->yVel * 16;
		sptr->scalar = 2;
	}
	else if (type == T_ROCKZAPPDEBRIS || type == T_ZAPPDEBRIS || type == T_DROPPERDEBRIS)
	{
		sptr->gravity = 0;
		sptr->maxYvel = 0;
	}
	else if (type == T_DROPPER)
	{
		sptr->gravity = -(GetRandomControl() & 31) - 16;
		sptr->maxYvel = -(GetRandomControl() & 31) - 64;
		sptr->yVel = sptr->yVel * 16;
		sptr->scalar = 2;
	}
	else if (type == T_ZAPP)
	{
		sptr->gravity = sptr->maxYvel = 0;
		sptr->scalar = 2;
	}
}

static void TriggerFireBall(ITEM_INFO* item, TonyFlameType type, PHD_VECTOR* lara_pos, short roomNumber, short angle, int zdspeed)
{
	TONY_FLAME flame;
	memset(&flame, 0, sizeof(TONY_FLAME));

	switch (type)
	{
	case T_ROCKZAPPL:
		flame.on = true;
		flame.pos.x = 0;
		flame.pos.y = 0;
		flame.pos.z = 0;
		GetJointAbsPosition(item, &flame.pos, 10);
		flame.fallspeed = -16;
		flame.speed = 0;
		flame.yRot = item->Position.yRot;
		flame.room_number = roomNumber;
		flame.type = T_ROCKZAPPL;
		break;
	case T_ROCKZAPPR:
		flame.on = true;
		flame.pos.x = 0;
		flame.pos.y = 0;
		flame.pos.z = 0;
		GetJointAbsPosition(item, &flame.pos, 13);
		flame.fallspeed = -16;
		flame.speed = 0;
		flame.yRot = item->Position.yRot;
		flame.room_number = roomNumber;
		flame.type = T_ROCKZAPPR;
		break;
	case T_ZAPP:
		flame.on = true;
		flame.pos.x = 0;
		flame.pos.y = 0;
		flame.pos.z = 0;
		GetJointAbsPosition(item, &flame.pos, 13);
		flame.fallspeed = (GetRandomControl() & 7) + 10;
		flame.speed = 160;
		flame.yRot = item->Position.yRot;
		flame.room_number = roomNumber;
		flame.type = T_ZAPP;
		break;
	case T_DROPPER:
		flame.on = true;
		flame.pos.x = lara_pos->x;
		flame.pos.y = lara_pos->y + 64;
		flame.pos.z = lara_pos->z;
		flame.fallspeed = (GetRandomControl() & 3) + 4;
		flame.speed = 0;
		flame.yRot = angle;
		flame.room_number = roomNumber;
		flame.type = T_DROPPER;
		break;
	case T_ROCKZAPPDEBRIS:
		flame.on = true;
		flame.pos.x = lara_pos->x;
		flame.pos.y = lara_pos->y;
		flame.pos.z = lara_pos->z;
		flame.fallspeed = (GetRandomControl() & 3) - 2;
		flame.speed = zdspeed + (GetRandomControl() & 3);
		flame.yRot = GetRandomControl() * 2;
		flame.room_number = roomNumber;
		flame.type = T_ROCKZAPPDEBRIS;
		break;
	case T_ZAPPDEBRIS:
		flame.on = true;
		flame.pos.x = lara_pos->x;
		flame.pos.y = lara_pos->y;
		flame.pos.z = lara_pos->z;
		flame.fallspeed = -(GetRandomControl() & 15) - 16;
		flame.speed = (GetRandomControl() & 7) + 48;
		angle += (GetRandomControl() & 0x1fff) - 0x9000;
		flame.yRot = angle;
		flame.room_number = roomNumber;
		flame.type = T_ZAPPDEBRIS;
		break;
	case T_DROPPERDEBRIS:
		flame.on = true;
		flame.pos.x = lara_pos->x;
		flame.pos.y = lara_pos->y;
		flame.pos.z = lara_pos->z;
		flame.fallspeed = -(GetRandomControl() & 31) - 32;
		flame.speed = (GetRandomControl() & 31) + 32;
		flame.yRot = GetRandomControl() * 2;
		flame.room_number = roomNumber;
		flame.type = T_DROPPERDEBRIS;
		break;
	}

	if (flame.on)
		TriggerTonyEffect(flame);
}

void ControlTonyFireBall(short fxNumber)
{
	FX_INFO* fx;
	FLOOR_INFO* floor;
	long old_x, old_y, old_z, x;
	long rnd, j;
	unsigned char radtab[7] = { 16,0,14,9,7,7,7 };
	TonyFlameType type;
	short roomNumber;

	fx = &EffectList[fxNumber];
	old_x = fx->pos.xPos;
	old_y = fx->pos.yPos;
	old_z = fx->pos.zPos;

	if (fx->flag1 == T_ROCKZAPPL || fx->flag1 == T_ROCKZAPPR)
	{
		fx->fallspeed += (fx->fallspeed / 8) + 1;
		if (fx->fallspeed < -4096)
			fx->fallspeed = -4096;
		fx->pos.yPos += fx->fallspeed;
		if (Wibble & 4)
			TriggerFireBallFlame(fxNumber, (TonyFlameType)fx->flag1, 0, 0, 0);
	}
	else if (fx->flag1 == T_DROPPER)
	{
		fx->fallspeed += 2;
		fx->pos.yPos += fx->fallspeed;
		if (Wibble & 4)
			TriggerFireBallFlame(fxNumber, (TonyFlameType)fx->flag1, 0, 0, 0);
	}
	else
	{
		if (fx->flag1 != T_ZAPP)
		{
			if (fx->speed > 48)
				fx->speed--;
		}
		fx->fallspeed += fx->flag2;
		if (fx->fallspeed > 512)
			fx->fallspeed = 512;
		fx->pos.yPos += fx->fallspeed / 2;
		fx->pos.zPos += fx->speed * phd_cos(fx->pos.yRot);
		fx->pos.xPos += fx->speed * phd_sin(fx->pos.yRot);
		if (Wibble & 4)
			TriggerFireBallFlame(fxNumber, (TonyFlameType)fx->flag1, (short)((old_x - fx->pos.xPos) * 8), (short)((old_y - fx->pos.yPos) * 8), (short)((old_z - fx->pos.zPos) * 4));
	}

	roomNumber = fx->roomNumber;
	floor = GetFloor(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, &roomNumber);
	if (fx->pos.yPos >= GetFloorHeight(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos) ||
		fx->pos.yPos < GetCeiling(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos))
	{
		if (fx->flag1 == T_ROCKZAPPL || fx->flag1 == T_ROCKZAPPR || fx->flag1 == T_ZAPP || fx->flag1 == T_DROPPER)
		{
			PHD_VECTOR pos;

			TriggerExplosionSparks(old_x, old_y, old_z, 3, -2, 0, fx->roomNumber);
			if (fx->flag1 == T_ROCKZAPPL || fx->flag1 == T_ROCKZAPPR)
			{
				for (x = 0; x < 2; x++)
					TriggerExplosionSparks(old_x, old_y, old_z, 3, -1, 0, fx->roomNumber);
			}
			pos.x = old_x;
			pos.y = old_y;
			pos.z = old_z;
			if (fx->flag1 == T_ZAPP)
				j = 7;
			else
				j = 3;
			if (fx->flag1 == T_ZAPP)
				type = T_ZAPPDEBRIS;
			else if (fx->flag1 == T_DROPPER)
				type = T_DROPPERDEBRIS;
			else
				type = T_ROCKZAPPDEBRIS;

			for (x = 0; x < j; x++)
				TriggerFireBall(NULL, type, &pos, fx->roomNumber, fx->pos.yRot, 32 + (x * 4));

			if (fx->flag1 == T_ROCKZAPPL || fx->flag1 == T_ROCKZAPPR)
			{
				roomNumber = LaraItem->RoomNumber;
				floor = GetFloor(LaraItem->Position.xPos, LaraItem->Position.yPos, LaraItem->Position.zPos, &roomNumber);
				pos.y = GetCeiling(floor, LaraItem->Position.xPos, LaraItem->Position.yPos, LaraItem->Position.zPos) + 256;
				pos.x = LaraItem->Position.xPos + (GetRandomControl() & 1023) - 512;
				pos.z = LaraItem->Position.zPos + (GetRandomControl() & 1023) - 512;
				TriggerExplosionSparks(pos.x, pos.y, pos.z, 3, -2, 0, roomNumber);
				TriggerFireBall(NULL, T_DROPPER, &pos, roomNumber, 0, 0);
			}
		}
		KillEffect(fxNumber);
		return;
	}

	
	if (TestEnvironment(ENV_FLAG_WATER, roomNumber))
	{
		KillEffect(fxNumber);
		return;
	}

	if (!Lara.burn)
	{
		if (ItemNearLara(&fx->pos, 200))
		{
			LaraItem->HitStatus = true;
			KillEffect(fxNumber);
			LaraItem->HitPoints -= 200;
			LaraBurn(LaraItem);
			return;
		}
	}

	if (roomNumber != fx->roomNumber)
		EffectNewRoom(fxNumber, LaraItem->RoomNumber);

	if (radtab[fx->flag1])
	{
		rnd = GetRandomControl();
		BYTE r3 = 31 - ((rnd / 16) & 3);
		BYTE g3 = 24 - ((rnd / 64) & 3);
		BYTE b3 = rnd & 7;
		TriggerDynamicLight(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, radtab[fx->flag1], r3, g3, b3);
	}
}

static void TonyBossDie(short itemNum)
{
	ITEM_INFO* item;
	item = &g_Level.Items[itemNum];
	item->Collidable = false;
	item->HitPoints = -16384;
	
	KillItem(itemNum);
	DisableEntityAI(itemNum);

	item->Flags |= ONESHOT;
}

void InitialiseTony(short itemNum)
{
	ITEM_INFO* item;
	item = &g_Level.Items[itemNum];
	item->ItemFlags[3] = 0;
	BossData.ExplodeCount = 0;
	BossData.RingCount = 0;
	BossData.DroppedIcon = false;
	BossData.DrawExplode = false;
	BossData.Dead = false;
}

static bool TonyIsDying()
{
	return	BossData.ExplodeCount == 01 ||
			BossData.ExplodeCount == 15 ||
			BossData.ExplodeCount == 25 ||
			BossData.ExplodeCount == 35 ||
			BossData.ExplodeCount == 45 ||
			BossData.ExplodeCount == 55;
}

static void ExplodeTonyBoss(ITEM_INFO* item)
{
	if (item->HitPoints <= 0 && TonyIsDying())
	{
		int x, y, z;
		x = item->Position.xPos + (GetRandomDraw() & 0x3FF) - 512;
		y = item->Position.yPos - (GetRandomDraw() & 0x3FF) - 256;
		z = item->Position.zPos + (GetRandomDraw() & 0x3FF) - 512;
		BossData.DrawExplode = true;

		TriggerExplosionSparks(x, y, z, 3, -2, 0, item->RoomNumber);
		for (int i = 0; i < 2; i++)
			TriggerExplosionSparks(x, y, z, 3, -1, 0, item->RoomNumber);
	}

	if (BossData.DrawExplode)
	{
		BossData.DrawExplode = false;
	}
}

void TonyControl(short itemNum)
{
	ITEM_INFO* item;
	CREATURE_INFO* tonyboss;
	AI_INFO info;
	PHD_VECTOR pos1;
	short angle, head, torso_x, torso_y, tilt, lp;
	int rnd;

	if (!CreatureActive(itemNum))
		return;

	item = &g_Level.Items[itemNum];
	tonyboss = (CREATURE_INFO*)item->Data;
	head = torso_y = torso_x = angle = tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 6)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 6;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 6;
		}

		if ((item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase) > 110)
		{
			item->MeshBits = 0;
			if (!BossData.DroppedIcon)
				BossData.DroppedIcon = true;
		}

		if (BossData.ExplodeCount < 256)
			BossData.ExplodeCount++;

		if (BossData.ExplodeCount <= 128 || BossData.RingCount != 6)
		{
			ExplodeTonyBoss(item);
		}
		else
		{
			TonyBossDie(itemNum);
			BossData.Dead = true;
		}
	}
	else
	{
		if (item->ItemFlags[3] != 2)
			item->HitPoints = TONYBOSS_HITS;

		CreatureAIInfo(item, &info);

		if (!item->ItemFlags[3])
		{
			int dx, dz;
			dx = item->Position.xPos - LaraItem->Position.xPos;
			dz = item->Position.zPos - LaraItem->Position.zPos;
			if ((SQUARE(dx) + SQUARE(dz)) < SQUARE(5120))
				item->ItemFlags[3] = 1;
			angle = 0;
		}
		else
		{
			tonyboss->target.x = LaraItem->Position.xPos;
			tonyboss->target.z = LaraItem->Position.zPos;
			angle = CreatureTurn(item, tonyboss->maximumTurn);
		}

		if (info.ahead)
			head = info.angle;

		switch (item->ActiveState)
		{
			case TONYBOSS_WAIT:
				tonyboss->maximumTurn = 0;
				if (item->TargetState != TONYBOSS_RISE && item->ItemFlags[3])
					item->TargetState = TONYBOSS_RISE;
				break;

			case TONYBOSS_RISE:
				if ((item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase) > 16)
					tonyboss->maximumTurn = TONYBOSS_TURN;
				else
					tonyboss->maximumTurn = 0;
				break;

			case TONYBOSS_FLOAT:
				torso_y = info.angle;
				torso_x = info.xAngle;
				tonyboss->maximumTurn = TONYBOSS_TURN;

				if (!BossData.ExplodeCount)
				{
					if (item->TargetState != TONYBOSS_BIGBOOM && item->ItemFlags[3] != 2)
					{
						item->TargetState = TONYBOSS_BIGBOOM;
						tonyboss->maximumTurn = 0;
					}

					if (item->TargetState != TONYBOSS_ROCKZAPP && item->ItemFlags[3] == 2)
					{
						if (!(Wibble & 255) && item->ItemFlags[0] == 0)
						{
							item->TargetState = TONYBOSS_ROCKZAPP;
							item->ItemFlags[0] = 1;
						}
					}

					if (item->TargetState != TONYBOSS_ZAPP && item->TargetState != TONYBOSS_ROCKZAPP && item->ItemFlags[3] == 2)
					{
						if (!(Wibble & 255) && item->ItemFlags[0] == 1)
						{
							item->TargetState = TONYBOSS_ZAPP;
							item->ItemFlags[0] = 0;
						}
					}
				}
				break;

			case TONYBOSS_ROCKZAPP:
				torso_y = info.angle;
				torso_x = info.xAngle;
				tonyboss->maximumTurn = 0;

				if (item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase == 40)
				{
					TriggerFireBall(item, T_ROCKZAPPL, NULL, item->RoomNumber, 0, 0);
					TriggerFireBall(item, T_ROCKZAPPR, NULL, item->RoomNumber, 0, 0);
				}
				break;

			case TONYBOSS_ZAPP:
				torso_y = info.angle;
				torso_x = info.xAngle;
				tonyboss->maximumTurn = TONYBOSS_TURN / 2;

				if ((item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase) == 28)
					TriggerFireBall(item, T_ZAPP, NULL, item->RoomNumber, item->Position.yRot, 0);
				break;

			case TONYBOSS_BIGBOOM:
				tonyboss->maximumTurn = 0;
				if ((item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase) == 56)
				{
					item->ItemFlags[3] = 2;
					BossData.DrawExplode = true;
				}
				break;

			default:
				break;
		}
	}

	if (item->ActiveState == TONYBOSS_ROCKZAPP || item->ActiveState == TONYBOSS_ZAPP || item->ActiveState == TONYBOSS_BIGBOOM)
	{
		byte r, g, b;
		int bright;

		bright = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;
		if (bright > 16)
		{
			bright = g_Level.Anims[item->AnimNumber].frameEnd - item->FrameNumber;
			if (bright > 16)
				bright = 16;
		}

		rnd = GetRandomControl();
		r = 31 - ((rnd / 16) & 3);
		g = 24 - ((rnd / 64) & 3);
		b = rnd & 7;
		r = (r * bright) / 16;
		g = (g * bright) / 16;
		b = (b * bright) / 16;

		pos1.x = pos1.y = pos1.z = 0;
		GetJointAbsPosition(item, &pos1, 10);
		TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 12, r, g, b);
		TriggerTonyFlame(itemNum, 14);

		if (item->ActiveState == TONYBOSS_ROCKZAPP || item->ActiveState == TONYBOSS_BIGBOOM)
		{
			pos1.x = pos1.y = pos1.z = 0;
			GetJointAbsPosition(item, &pos1, 13);
			TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 12, r, g, b);
			TriggerTonyFlame(itemNum, 13);
		}
	}

	if (BossData.ExplodeCount && item->HitPoints > 0)
	{
		ExplodeTonyBoss(item);
		BossData.ExplodeCount++;

		if (BossData.ExplodeCount > 64)
		{
			BossData.ExplodeCount = 0;
			BossData.RingCount = 0;
		}
	}

	CreatureJoint(item, 0, torso_y >> 1);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, torso_y >> 1);
	CreatureAnimation(itemNum, angle, 0);
}

void S_DrawTonyBoss(ITEM_INFO* item)
{
	DrawAnimatingItem(item);
}