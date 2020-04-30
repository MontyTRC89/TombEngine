#include "../newobjects.h"
#include "../../Game/effect2.h"
#include "../../Game/sphere.h"
#include "../../Game/items.h"
#include "../../Game/lot.h"
#include "../../Game/Box.h"
#include "../../Game/draw.h"
#include "../../Game/effects.h"
#include "..\..\Specific\level.h"
#include "../../Specific/setup.h"
#include "../../Game/lara.h"

// TODO: custom render for DrawExplosionRings() and DrawTonyBossShield()
// TODO: add flame effect for tony, check TriggerXX() function.

enum TONY_EFFECT
{
	ROCKZAPPL = 0,		// To ceiling from left hand
	ROCKZAPPR,			// To ceiling from right hand
	ZAPP,				// From right hand.
	DROPPER,			// From ceiling.
	ROCKZAPPDEBRIS,		// Small bits from ceiling explosions.
	ZAPPDEBRIS,			// Small bits from hand flame explosions.
	DROPPERDEBRIS,		// Small bits from droppers explosions.
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

static long death_radii[5];
static long death_heights[5];
static long radii[5] = { 200,400,500,500,475 };
static long heights[5] = { -1536,-1280,-832,-384,0 };
static long dradii[5] = { 100 << 4,350 << 4,400 << 4,350 << 4,100 << 4 };
static long dheights1[5] = { -1536 - (768 << 3),-1152 - (384 << 3),-768,-384 + (384 << 3),0 + (768 << 3) };
static long dheights2[5] = { -1536,-1152,-768,-384,0 };

static SHIELD_POINTS TonyBossShield[40]; // x,y,z,rgb.
static EXPLOSION_RING ExpRings[7];
static BOSS_STRUCT bossdata;             // exclusive for tony unlike TR3

#define TONYBOSS_TURN 			(ONE_DEGREE * 2)
#define TONYBOSS_HITS			100
#define MAX_TONY_TRIGGER_RANGE	0x4000
#define SPN_TONYHANDLFLAME		4  // {0, 64, 0, 10}
#define SPN_TONYHANDRFLAME		5  // {0, 64, 0, 13}

void TriggerTonyFlame(short itemNum, long hand)
{
	ITEM_INFO* item;
	long size;
	SPARKS* sptr;
	long dx, dz;

	item = &Items[itemNum];
	dx = LaraItem->pos.xPos - item->pos.xPos;
	dz = LaraItem->pos.zPos - item->pos.zPos;

	if (dx < -MAX_TONY_TRIGGER_RANGE || dx > MAX_TONY_TRIGGER_RANGE || dz < -MAX_TONY_TRIGGER_RANGE || dz > MAX_TONY_TRIGGER_RANGE)
		return;

	sptr = &Sparks[GetFreeSpark()];

	sptr->on = 1;
	sptr->sR = 255;
	sptr->sG = 48 + (GetRandomControl() & 31);
	sptr->sB = 48;

	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;

	sptr->colFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->fadeToBlack = 8;
	sptr->sLife = sptr->life = (GetRandomControl() & 7) + 24;

	sptr->transType = 2;

	sptr->extras = 0;
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
		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_ITEM | SP_NODEATTATCH;
		sptr->rotAng = GetRandomControl() & 4095;
		if (GetRandomControl() & 1)
			sptr->rotAdd = -(GetRandomControl() & 15) - 16;
		else
			sptr->rotAdd = (GetRandomControl() & 15) + 16;
	}
	else
		sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTATCH;

	sptr->gravity = -(GetRandomControl() & 31) - 16;
	sptr->maxYvel = -(GetRandomControl() & 7) - 16;
	sptr->fxObj = itemNum;
	sptr->nodeNumber = 0;

	//sptr->def = Objects[ID_DEFAULT_SPRITES].meshIndex;
	sptr->scalar = 1;
	size = (GetRandomControl() & 31) + 64;
	sptr->size = size;
	sptr->sSize = size;
	sptr->dSize = size >> 2;
}

void TriggerFireBallFlame(short fxNumber, long type, long xv, long yv, long zv)
{
	SPARKS* sptr;
	long		size;
	long		dx, dz;

	dx = LaraItem->pos.xPos - Effects[fxNumber].pos.xPos;
	dz = LaraItem->pos.zPos - Effects[fxNumber].pos.zPos;

	if (dx < -MAX_TONY_TRIGGER_RANGE || dx > MAX_TONY_TRIGGER_RANGE || dz < -MAX_TONY_TRIGGER_RANGE || dz > MAX_TONY_TRIGGER_RANGE)
		return;

	sptr = &Sparks[GetFreeSpark()];

	sptr->on = 1;
	sptr->sR = 255;
	sptr->sG = 48 + (GetRandomControl() & 31);
	sptr->sB = 48;

	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;

	sptr->colFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->fadeToBlack = 8;
	sptr->sLife = sptr->life = (GetRandomControl() & 7) + 24;

	sptr->transType = 2;

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
	{
		sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;
	}

	sptr->fxObj = fxNumber;

	//sptr->def = objects[EXPLOSION1].mesh_index;
	sptr->scalar = 1;
	size = (GetRandomControl() & 31) + 64;
	sptr->size = sptr->sSize = size;
	sptr->dSize = size >> 2;

	if (type == ROCKZAPPL || type == ROCKZAPPR)
	{
		sptr->gravity = (GetRandomControl() & 31) + 16;
		sptr->maxYvel = (GetRandomControl() & 15) + 48;
		sptr->yVel = -sptr->yVel << 4;
		sptr->scalar = 2;
	}
	else if (type == ROCKZAPPDEBRIS || type == ZAPPDEBRIS || type == DROPPERDEBRIS)
	{
		sptr->gravity = sptr->maxYvel = 0;
	}
	else if (type == DROPPER)
	{
		sptr->gravity = -(GetRandomControl() & 31) - 16;
		sptr->maxYvel = -(GetRandomControl() & 31) - 64;
		sptr->yVel = sptr->yVel << 4;
		sptr->scalar = 2;
	}
	else if (type == ZAPP)
	{
		sptr->gravity = sptr->maxYvel = 0;
		sptr->scalar = 2;
	}
}

void TriggerFireBall(ITEM_INFO* item, long type, PHD_VECTOR* pos1, short roomNumber, short angle, long zdspeed)
{
	FX_INFO* fx;
	PHD_VECTOR pos;
	short fxNumber;
	long speed, fallspeed;

	if (type == ROCKZAPPL)
	{
		pos.x = pos.y = pos.z = 0;
		GetJointAbsPosition(item, &pos, 10);
		angle = item->pos.yRot;
		speed = 0;
		fallspeed = -16;
	}
	else if (type == ROCKZAPPR)
	{
		pos.x = pos.y = pos.z = 0;
		GetJointAbsPosition(item, &pos, 13);
		angle = item->pos.yRot;
		speed = 0;
		fallspeed = -16;
	}
	else if (type == ZAPP)
	{
		pos.x = pos.y = pos.z = 0;
		GetJointAbsPosition(item, &pos, 13);
		speed = 160;
		fallspeed = -(GetRandomControl() & 7) - 32;
	}
	else if (type == ROCKZAPPDEBRIS)
	{
		pos.x = pos1->x;
		pos.y = pos1->y;
		pos.z = pos1->z;
		speed = zdspeed + (GetRandomControl() & 3);
		angle = GetRandomControl() << 1;
		fallspeed = (GetRandomControl() & 3) - 2;
	}
	else if (type == ZAPPDEBRIS)
	{
		pos.x = pos1->x;
		pos.y = pos1->y;
		pos.z = pos1->z;
		speed = (GetRandomControl() & 7) + 48;
		angle += (GetRandomControl() & 0x1fff) - 0x9000;
		fallspeed = -(GetRandomControl() & 15) - 16;
	}
	else if (type == DROPPER)
	{
		pos.x = pos1->x;
		pos.y = pos1->y;
		pos.z = pos1->z;
		speed = 0;
		fallspeed = (GetRandomControl() & 3) + 4;
	}
	else// if (type == DROPPERDEBRIS)
	{
		pos.x = pos1->x;
		pos.y = pos1->y;
		pos.z = pos1->z;
		speed = (GetRandomControl() & 31) + 32;
		angle = GetRandomControl() << 1;
		fallspeed = -(GetRandomControl() & 31) - 32;
	}

	fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != NO_ITEM)
	{
		fx = &Effects[fxNumber];
		fx->pos.xPos = pos.x;
		fx->pos.yPos = pos.y;
		fx->pos.zPos = pos.z;
		fx->pos.yRot = angle;
		fx->objectNumber = ID_GUARD1; // TONYFIREBALL
		fx->speed = speed;
		fx->fallspeed = fallspeed;
		fx->flag1 = type;
		fx->flag2 = (GetRandomControl() & 3) + 1;
		if (type == ZAPPDEBRIS)
			fx->flag2 <<= 1;
		else if (type == ZAPP)
			fx->flag2 = 0;
	}
}

void TonyFireBallControl(short fxNumber)
{
	FX_INFO* fx;
	FLOOR_INFO* floor;
	long old_x, old_y, old_z, x;
	long rnd, r, g, b;
	unsigned char radtab[7] = { 16,0,14,9,7,7,7 };
	short roomNumber;

	fx = &Effects[fxNumber];

	old_x = fx->pos.xPos;
	old_y = fx->pos.yPos;
	old_z = fx->pos.zPos;

	if (fx->flag1 == ROCKZAPPL || fx->flag1 == ROCKZAPPR)
	{
		fx->fallspeed += (fx->fallspeed >> 3) + 1;
		if (fx->fallspeed < -4096)
			fx->fallspeed = -4096;
		fx->pos.yPos += fx->fallspeed;
		if (Wibble & 4)
			TriggerFireBallFlame(fxNumber, fx->flag1, 0, 0, 0);
	}
	else if (fx->flag1 == DROPPER)
	{
		fx->fallspeed += 2;
		fx->pos.yPos += fx->fallspeed;
		if (Wibble & 4)
			TriggerFireBallFlame(fxNumber, fx->flag1, 0, 0, 0);
	}
	else
	{
		if (fx->flag1 != ZAPP)
		{
			if (fx->speed > 48)
				fx->speed--;
		}
		fx->fallspeed += fx->flag2;
		if (fx->fallspeed > 512)
			fx->fallspeed = 512;
		fx->pos.yPos += fx->fallspeed >> 1;
		fx->pos.zPos += (fx->speed * phd_cos(fx->pos.yRot) >> W2V_SHIFT);
		fx->pos.xPos += (fx->speed * phd_sin(fx->pos.yRot) >> W2V_SHIFT);
		if (Wibble & 4)
			TriggerFireBallFlame(fxNumber, fx->flag1, (old_x - fx->pos.xPos) << 3, (old_y - fx->pos.yPos) << 3, (old_z - fx->pos.zPos) << 3);
	}

	roomNumber = fx->roomNumber;
	floor = GetFloor(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, &roomNumber);
	if (fx->pos.yPos >= GetFloorHeight(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos) || fx->pos.yPos < GetCeiling(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos))
	{
		if (fx->flag1 == ROCKZAPPL || fx->flag1 == ROCKZAPPR || fx->flag1 == ZAPP || fx->flag1 == DROPPER)
		{
			PHD_VECTOR	pos;

			TriggerExplosionSparks(old_x, old_y, old_z, 3, -2, 0, fx->roomNumber);	// -2 = Set off a dynamic light controller.
			if (fx->flag1 == ROCKZAPPL || fx->flag1 == ROCKZAPPR)
			{
				for (x = 0; x < 2; x++)
					TriggerExplosionSparks(old_x, old_y, old_z, 3, -1, 0, fx->roomNumber);
			}
			pos.x = old_x;
			pos.y = old_y;
			pos.z = old_z;
			if (fx->flag1 == ZAPP)
				r = 7;
			else
				r = 3;
			if (fx->flag1 == ZAPP)
				g = ZAPPDEBRIS;
			else if (fx->flag1 == DROPPER)
				g = DROPPERDEBRIS;
			else
				g = ROCKZAPPDEBRIS;

			for (x = 0; x < r; x++)
				TriggerFireBall((ITEM_INFO*)NULL, g, &pos, fx->roomNumber, fx->pos.yRot, 32 + (x << 2));

			if (fx->flag1 == ROCKZAPPL || fx->flag1 == ROCKZAPPR)
			{
				roomNumber = LaraItem->roomNumber;
				floor = GetFloor(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, &roomNumber);
				pos.y = GetCeiling(floor, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos) + 256;
				pos.x = LaraItem->pos.xPos + (GetRandomControl() & 1023) - 512;
				pos.z = LaraItem->pos.zPos + (GetRandomControl() & 1023) - 512;
				TriggerExplosionSparks(pos.x, pos.y, pos.z, 3, -2, 0, roomNumber);	// -2 = Set off a dynamic light controller.
				TriggerFireBall((ITEM_INFO*)NULL, DROPPER, &pos, roomNumber, 0, 0);
			}
		}
		KillEffect(fxNumber);
		return;
	}

	if (Rooms[roomNumber].flags & ENV_FLAG_WATER)
	{
		KillEffect(fxNumber);
		return;
	}

	if (!Lara.burn)
	{
		if (ItemNearLara(&fx->pos, 200))
		{
			LaraItem->hitStatus = true;
			KillEffect(fxNumber);
			return;
		}
	}

	if (roomNumber != fx->roomNumber)
		EffectNewRoom(fxNumber, LaraItem->roomNumber);

	if (radtab[fx->flag1])
	{
		rnd = GetRandomControl();
		r = 31 - ((rnd >> 4) & 3);
		g = 24 - ((rnd >> 6) & 3);
		b = rnd & 7;
		//TriggerDynamic(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, radtab[fx->flag1], r, g, b);
	}
}

void TonyDie(short itemNum)
{
	ITEM_INFO* item;

	item = &Items[itemNum];
	item->collidable = false;
	item->hitPoints = -16384; // NOT_TARGETABLE
	
	KillItem(itemNum);
	DisableBaddieAI(itemNum);

	item->flags |= ONESHOT;
}

void InitialiseTony(short itemNum)
{
	SHIELD_POINTS* shptr;
	ITEM_INFO* item;
	int lp, lp1, y;
	short angle;

	item = &Items[itemNum];
	item->itemFlags[3] = 0;
	bossdata.explode_count = 0;
	bossdata.ring_count = 0;
	bossdata.dropped_icon = false;
	bossdata.dead = false;

	// initialise shield coordinate:
	shptr = &TonyBossShield[0];
	for (lp = 0; lp < 5; lp++)
	{
		y = heights[lp];
		angle = 0;
		for (lp1 = 0; lp1 < 8; lp1++)
		{
			shptr->x = (rcossin_tbl[angle << 1] * radii[lp]) >> 11;
			shptr->y = y;
			shptr->z = (rcossin_tbl[(angle << 1) + 1] * radii[lp]) >> 11;
			shptr->rgb = 0;
			angle += 512;
			shptr++;
		}
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

	item = &Items[itemNum];
	tonyboss = (CREATURE_INFO*)item->data;
	head = torso_y = torso_x = angle = tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 6)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 6;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 6;
		}
	}
	else
	{
		if (item->itemFlags[3] != 2)	        // Still invincible ?
			item->hitPoints = TONYBOSS_HITS;

		CreatureAIInfo(item, &info);

		if (!item->itemFlags[3])	            // Is she close enough yet ?
		{
			long	dx, dz;
			dx = item->pos.xPos - LaraItem->pos.xPos;
			dz = item->pos.zPos - LaraItem->pos.zPos;
			if ((SQUARE(dx) + SQUARE(dz)) < SQUARE(5120))
				item->itemFlags[3] = 1;
			angle = 0;
		}
		else
		{
			tonyboss->target.x = LaraItem->pos.xPos;
			tonyboss->target.z = LaraItem->pos.zPos;
			angle = CreatureTurn(item, tonyboss->maximumTurn);
		}

		if (info.ahead)
			head = info.angle;

		switch (item->currentAnimState)
		{
			case TONYBOSS_WAIT:	// Waiting.
				tonyboss->maximumTurn = 0;

				if (item->goalAnimState != TONYBOSS_RISE && item->itemFlags[3])
					item->goalAnimState = TONYBOSS_RISE;
				break;

			case TONYBOSS_RISE:	// Rising.
				if (item->frameNumber - Anims[item->animNumber].frameBase > 16)
					tonyboss->maximumTurn = TONYBOSS_TURN;
				else
					tonyboss->maximumTurn = 0;
				break;

			case TONYBOSS_FLOAT:	// Rising.
				torso_y = info.angle;
				torso_x = info.xAngle;

				tonyboss->maximumTurn = TONYBOSS_TURN;

				if (!bossdata.explode_count)
				{
					if (item->goalAnimState != TONYBOSS_BIGBOOM && item->itemFlags[3] != 2)
					{
						item->goalAnimState = TONYBOSS_BIGBOOM;
						tonyboss->maximumTurn = 0;
					}

					if (item->goalAnimState != TONYBOSS_ROCKZAPP && item->itemFlags[3] == 2)
					{
						if (!(Wibble & 255) && item->itemFlags[0] == 0)
						{
							item->goalAnimState = TONYBOSS_ROCKZAPP;
							item->itemFlags[0] = 1;
						}
					}

					if (item->goalAnimState != TONYBOSS_ZAPP && item->goalAnimState != TONYBOSS_ROCKZAPP && item->itemFlags[3] == 2)
					{
						if (!(Wibble & 255) && item->itemFlags[0] == 1)
						{
							item->goalAnimState = TONYBOSS_ZAPP;
							item->itemFlags[0] = 0;
						}
					}
				}

				break;

			case TONYBOSS_ROCKZAPP:	// Shooting at ceiling.
				torso_y = info.angle;
				torso_x = info.xAngle;

				tonyboss->maximumTurn = 0;

				if (item->frameNumber - Anims[item->animNumber].frameBase == 40)
				{
					//TriggerFireBall(item, ROCKZAPPL, (PHD_VECTOR*)NULL, item->roomNumber, 0, 0);
					//TriggerFireBall(item, ROCKZAPPR, (PHD_VECTOR*)NULL, item->roomNumber, 0, 0);
				}
				break;

			case TONYBOSS_ZAPP:	// Shooting at ceiling.
				torso_y = info.angle;
				torso_x = info.xAngle;

				tonyboss->maximumTurn = TONYBOSS_TURN >> 1;

				//if (item->frameNumber - Anims[item->animNumber].frameBase == 28)
				//	TriggerFireBall(item, ZAPP, (PHD_VECTOR*)NULL, item->roomNumber, item->pos.yRot, 0);
				break;

			case TONYBOSS_BIGBOOM:	// Changing room.
				tonyboss->maximumTurn = 0;
				if (item->frameNumber - Anims[item->animNumber].frameBase == 56)
				{
					item->itemFlags[3] = 2;
					bossdata.explode_count = 1;
				}
				break;

			default:
				break;
		}
	}

	if (item->currentAnimState == TONYBOSS_ROCKZAPP || item->currentAnimState == TONYBOSS_ZAPP || item->currentAnimState == TONYBOSS_BIGBOOM)
	{
		long bright, r, g, b;

		bright = item->frameNumber - Anims[item->animNumber].frameBase;
		if (bright > 16)
		{
			bright = Anims[item->animNumber].frameEnd - item->frameNumber;
			if (bright > 16)
				bright = 16;
		}

		rnd = GetRandomControl();
		r = 31 - ((rnd >> 4) & 3);
		g = 24 - ((rnd >> 6) & 3);
		b = rnd & 7;
		r = (r * bright) >> 4;
		g = (g * bright) >> 4;
		b = (b * bright) >> 4;

		pos1.x = pos1.y = pos1.z = 0;
		GetJointAbsPosition(item, &pos1, 10);
		//TriggerDynamic(pos1.x, pos1.y, pos1.z, 12, r, g, b);
		//TriggerTonyFlame(itemNum, SPN_TONYHANDRFLAME);

		if (item->currentAnimState == TONYBOSS_ROCKZAPP || item->currentAnimState == TONYBOSS_BIGBOOM)
		{
			pos1.x = pos1.y = pos1.z = 0;
			GetJointAbsPosition(item, &pos1, 13);
			//TriggerDynamic(pos1.x, pos1.y, pos1.z, 12, r, g, b);
			//TriggerTonyFlame(itemNum, SPN_TONYHANDLFLAME);
		}
	}

	if (bossdata.explode_count && item->hitPoints > 0)
	{
		//ExplodeTonyBoss(item);
		bossdata.explode_count++;
		//if (bossdata.explode_count == 32)
		//	FlipMap[];
		if (bossdata.explode_count > 64)
			bossdata.explode_count = bossdata.ring_count = 0;
	}

	/* Actually do animation allowing for collisions */
	CreatureJoint(item, 0, torso_y >> 1);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, torso_y >> 1);
	CreatureAnimation(itemNum, angle, 0);
}

void DrawTony(ITEM_INFO* item)
{
	//DrawAnimatingItem(item);

	// TODO: psx draw (need to be rewrited !)
	//if (bossdata.explode_count && item->hitPoints <= 0)
	//	DrawExplosionRings();

	//if (bossdata.explode_count && bossdata.explode_count <= 64)
	//	DrawTonyBossShield(item);
}