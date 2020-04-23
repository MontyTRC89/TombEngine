#include "../newobjects.h"
#include "../../Game/items.h"
#include "../../Game/effect2.h"
#include "../../Game/lot.h"
#include "../../Game/Box.h"
#include "../../Game/sphere.h"
#include "../../Game/people.h"
#include "../../Game/draw.h"
#include "../../Game/effects.h"
#include "../../Game/misc.h"
#include "../../Specific/setup.h"
#include "..\..\Specific\level.h"
#include "../../Game/lara.h"

#define MAX_TRIGGER_RANGE 0x4000
#define SMALL_FLASH 10
#define BIG_FLASH 16
#define BOLT_SPEED 384

enum { NOTHING, SUMMONING, KNOCKBACK };
enum { NORMAL_BOLT, LARGE_BOLT, SUMMON_BOLT };
enum { RIGHT_PRONG, ICONPOS, LEFT_PRONG };
enum { ATTACK_HEAD, ATTACK_HAND1, ATTACK_HAND2 };

enum londonboss_anims {
	LONDONBOSS_EMPTY,
	LONDONBOSS_STAND,
	LONDONBOSS_WALK,
	LONDONBOSS_RUN,
	LONDONBOSS_SUMMON,
	LONDONBOSS_BIGZAP,
	LONDONBOSS_DEATH,
	LONDONBOSS_LAUGH,
	LONDONBOSS_LILZAP,
	LONDONBOSS_VAULT2,
	LONDONBOSS_VAULT3,
	LONDONBOSS_VAULT4,
	LONDONBOSS_GODOWN
};

#define	LONDONBOSS_VAULT2_ANIM 9
#define	LONDONBOSS_VAULT3_ANIM 18
#define	LONDONBOSS_VAULT4_ANIM 15
#define	LONDONBOSS_GODOWN_ANIM 21
#define	LONDONBOSS_STND2SUM_ANIM 1
#define LONDONBOSS_SUMMON_ANIM 2
#define	LONDONBOSS_GODOWN_ANIM 21
#define LONDONBOSS_VAULT_SHIFT 96
#define LONDONBOSS_AWARE_DISTANCE SQUARE(WALL_SIZE)
#define LONDONBOSS_WALK_TURN ANGLE(4)
#define LONDONBOSS_RUN_TURN ANGLE(7)
#define LONDONBOSS_WALK_RANGE SQUARE(WALL_SIZE)
#define LONDONBOSS_WALK_CHANCE 0x100
#define LONDONBOSS_LAUGH_CHANCE 0x100
#define LONDONBOSS_TURN ANGLE(2)
#define LONDONBOSS_DIE_ANIM 17
#define LONDONBOSS_FINAL_HEIGHT -11776
#define BIGZAP_TIMER 600

static long death_radii[5];
static long death_heights[5];
static long radii[5] = { 200,400,500,500,475 };
static long heights[5] = { -1536,-1280,-832,-384,0 };
static long dradii[5] = { 100 << 4,350 << 4,400 << 4,350 << 4,100 << 4 };
static long dheights1[5] = { -1536 - (768 << 3),-1152 - (384 << 3),-768,-384 + (384 << 3),0 + (768 << 3) };
static long dheights2[5] = { -1536,-1152,-768,-384,0 };

static BITE_INFO londonboss_points[3] = {
	{16,56,356, 10},	// Right prong.
	{-28,48,304, 10},	// Icon.
	{-72,48,356, 10},	// Left prong.
};

static SHIELD_POINTS LondonBossShield[40];
static EXPLOSION_RING ExpRings[6];
static EXPLOSION_RING KBRings[3];
static BOSS_STRUCT bossdata;

static char	links[20][4] = {
	{	0,0,1,2		},
	{	0,0,2,3,	},
	{	0,0,3,4,	},
	{	0,0,4,1,	},
	{	1,2,5,6,	},
	{	2,3,6,7,	},
	{	3,4,7,8,	},
	{	4,1,8,5,	},
	{	1 + 4,2 + 4,5 + 4,6 + 4,	},
	{	2 + 4,3 + 4,6 + 4,7 + 4,	},
	{	3 + 4,4 + 4,7 + 4,8 + 4,	},
	{	4 + 4,1 + 4,8 + 4,5 + 4,	},
	{	1 + 8,2 + 8,5 + 8,6 + 8,	},
	{	2 + 8,3 + 8,6 + 8,7 + 8,	},
	{	3 + 8,4 + 8,7 + 8,8 + 8,	},
	{	4 + 8,1 + 8,8 + 8,5 + 8,	},
	{	1 + 12,2 + 12,5 + 12,5 + 12,	},
	{	2 + 12,3 + 12,5 + 12,5 + 12,	},
	{	3 + 12,4 + 12,5 + 12,5 + 12,	},
	{	4 + 12,1 + 12,5 + 12,5 + 12,	}
};

static char	sumlinks[8][4] = {
	{ 0,0,1,2 },
	{ 0,0,2,3 },
	{ 0,0,3,4 },
	{ 0,0,4,1 },
	{ 1,2,5,5 },
	{ 2,3,5,5 },
	{ 3,4,5,5 },
	{ 4,1,5,5 }
};

static void DrawLondonBossShield(ITEM_INFO* item)
{
	// PSX
}

static void DrawExplosionRings()
{
	// PSX
}

static void DrawSummonRings()
{
	// PSX
}

static void DrawKnockBackRings()
{
	// PSX
}

static void TriggerLaserBolt(PHD_VECTOR* pos, ITEM_INFO* item, long type, short yang)
{
	ITEM_INFO* bolt_item;
	short item_number;
	short angles[2];

	/* Create a laser bolt and launch it on its way */
	item_number = CreateItem();
	if (item_number != NO_ITEM)
	{
		bolt_item = &Items[item_number];

		bolt_item->objectNumber = ID_LASER_BOLT;
		bolt_item->roomNumber = item->roomNumber;
		bolt_item->pos.xPos = pos->x;
		bolt_item->pos.yPos = pos->y;
		bolt_item->pos.zPos = pos->z;

		InitialiseItem(item_number);

		if (type == SUMMON_BOLT)
		{
			bolt_item->pos.yPos += item->pos.yPos - 384;
			bolt_item->pos.xRot = -pos->y << 5;
			bolt_item->pos.yRot = GetRandomControl() << 1;
		}
		else
		{
			phd_GetVectorAngles(LaraItem->pos.xPos - pos->x, LaraItem->pos.yPos - STEP_SIZE - pos->y, LaraItem->pos.zPos - pos->z, angles);
			bolt_item->pos.xRot = angles[1];
			bolt_item->pos.yRot = yang;
			bolt_item->pos.zRot = 0;
		}

		if (type != LARGE_BOLT)
		{
			bolt_item->speed = 16;
			bolt_item->itemFlags[0] = -24;
			bolt_item->itemFlags[1] = 4;
			if (type == SUMMON_BOLT)
				bolt_item->itemFlags[2] = 1;	// Set the 'just draw front and back' flag.
		}
		else
		{
			bolt_item->speed = 24;
			bolt_item->itemFlags[0] = 31;
			bolt_item->itemFlags[1] = 16;
		}

		AddActiveItem(item_number);
	}
}

static void TriggerPlasmaBallFlame(short fx_number, long type, long xv, long yv, long zv)
{
	SPARKS* sptr;
	long size;
	long dx, dz;

	dx = LaraItem->pos.xPos - Effects[fx_number].pos.xPos;
	dz = LaraItem->pos.zPos - Effects[fx_number].pos.zPos;

	if (dx < -MAX_TRIGGER_RANGE || dx > MAX_TRIGGER_RANGE || dz < -MAX_TRIGGER_RANGE || dz > MAX_TRIGGER_RANGE)
		return;

	sptr = &Sparks[GetFreeSpark()];

	sptr->on = 1;
	sptr->sG = 255;
	sptr->sB = 48 + (GetRandomControl() & 31);
	sptr->sR = 48;

	sptr->dG = 192 + (GetRandomControl() & 63);
	sptr->dB = 128 + (GetRandomControl() & 63);
	sptr->dR = 32;

	sptr->colFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->fadeToBlack = 8;
	sptr->sLife = sptr->life = (GetRandomControl() & 7) + 24;

	sptr->transType = 2;

	sptr->extras = 0;
	sptr->dynamic = -1;

	sptr->x = ((GetRandomControl() & 15) - 8);
	sptr->y = 0;
	sptr->z = ((GetRandomControl() & 15) - 8);

	sptr->xVel = xv + (GetRandomControl() & 255) - 128;
	sptr->yVel = yv;
	sptr->zVel = zv + (GetRandomControl() & 255) - 128;
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

	sptr->fxObj = fx_number;
	sptr->scalar = 1;
	size = (GetRandomControl() & 31) + 64;
	sptr->size = sptr->sSize = size;
	sptr->dSize = size >> 2;
	sptr->gravity = sptr->maxYvel = 0;
}

static void TriggerPlasmaBall(ITEM_INFO* item, long type, PHD_VECTOR* pos1, short roomNumber, short angle)
{
	FX_INFO* fx;
	PHD_VECTOR pos;
	short fx_number, angles[2];
	long speed;

	pos.x = pos1->x;
	pos.y = pos1->y;
	pos.z = pos1->z;
	speed = (GetRandomControl() & 31) + 64;
	angles[0] = angle + GetRandomControl() + 0x4000;
	angles[1] = 0x2000;

	fx_number = CreateNewEffect(roomNumber);
	if (fx_number != NO_ITEM)
	{
		fx = &Effects[fx_number];
		fx->pos.xPos = pos.x;
		fx->pos.yPos = pos.y;
		fx->pos.zPos = pos.z;
		fx->pos.yRot = angles[0];
		fx->pos.xRot = angles[1];
		fx->objectNumber = ID_LASER_BOLT;
		fx->speed = speed;
		fx->fallspeed = 0;
		fx->flag1 = 1;
		if (type == 2)
			fx->flag2 = 1;
		else
			fx->flag2 = 0;
	}
}

static long KnockBackCollision(EXPLOSION_RING* erptr)
{
	int radius, x, z, dist;
	short angle, diff;

	radius = erptr->radius;
	x = LaraItem->pos.xPos - erptr->x;
	z = LaraItem->pos.zPos - erptr->z;
	if (x > 16000 || x < -16000 || z > 16000 || z < -16000)
		dist = 0x7FFF;
	else
		dist = SQUARE(x) + SQUARE(z);

	if (dist < SQUARE(radius))
	{
		/* Whump! Been hit by sphere - throw Lara through air */
		LaraItem->hitPoints -= 200;
		LaraItem->hitStatus = true;
		angle = ATAN(z, x);
		diff = LaraItem->pos.yRot - angle;
		if (abs(diff) < 0x4000)
		{
			/* Facing away from blast */
			LaraItem->speed = 75;
			LaraItem->pos.yRot = angle;
		}
		else
		{
			/* Facing towards blast */
			LaraItem->pos.yRot = short(angle - 0x8000);
			LaraItem->speed = -75;
		}
		LaraItem->gravityStatus = true;
		LaraItem->fallspeed = -50;
		LaraItem->pos.xRot = LaraItem->pos.zRot = 0;
		LaraItem->animNumber = 34;
		LaraItem->frameNumber = GF(34, 0);
		LaraItem->currentAnimState = 3;
		LaraItem->goalAnimState = 3;

		TriggerExplosionSparks(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 3, -2, 2, LaraItem->roomNumber);	// -2 = Set off a dynamic light controller.
		for (x = 0; x < 3; x++)
			TriggerPlasmaBall(LaraItem, 2, (PHD_VECTOR*)&LaraItem->pos, LaraItem->roomNumber, GetRandomControl() << 1);

		return 1;
	}

	return 0;
}

static void ExplodeLondonBoss(ITEM_INFO* item)
{
	SHIELD_POINTS* shptr;
	long lp, lp1, angle, y, r, g, b;

	if (bossdata.explode_count == 1 ||
		bossdata.explode_count == 15 ||
		bossdata.explode_count == 25 ||
		bossdata.explode_count == 35 ||
		bossdata.explode_count == 45 ||
		bossdata.explode_count == 55)	// Trigger explosion & ring ?
	{
		long x, y, z;

		x = item->pos.xPos + ((GetRandomControl() & 1023) - 512);
		y = item->pos.yPos - (GetRandomControl() & 1023) - 256;
		z = item->pos.zPos + ((GetRandomControl() & 1023) - 512);

		ExpRings[bossdata.ring_count].x = x;
		ExpRings[bossdata.ring_count].y = y;
		ExpRings[bossdata.ring_count].z = z;
		ExpRings[bossdata.ring_count].on = true;
		bossdata.ring_count++;

		TriggerExplosionSparks(x, y, z, 3, -2, 2, 0);	// -2 = Set off a dynamic light controller.
		for (lp = 0; lp < 2; lp++)
			TriggerExplosionSparks(x, y, z, 3, -1, 2, 0);

		SoundEffect(SFX_TR3_BLAST_CIRCLE, &item->pos, PITCH_SHIFT | 0x800000);
	}

	// Adjust shield coordinates.
	for (lp = 0; lp < 5; lp++)
	{
		if (bossdata.explode_count < 128)	// Expand.
		{
			death_radii[lp] = (dradii[lp] >> 4) + (((dradii[lp]) * bossdata.explode_count) >> 7);
			death_heights[lp] = dheights2[lp] + (((dheights1[lp] - dheights2[lp]) * bossdata.explode_count) >> 7);
		}
	}

	// Setup shield coordinates.
	shptr = &LondonBossShield[0];	// x,y,z,rgb.

	for (lp = 0; lp < 5; lp++)
	{
		y = death_heights[lp];
		angle = (Wibble << 3) & 511;
		for (lp1 = 0; lp1 < 8; lp1++)
		{
			shptr->x = (rcossin_tbl[angle << 1] * death_radii[lp]) >> 11;
			shptr->y = y;
			shptr->z = (rcossin_tbl[(angle << 1) + 1] * death_radii[lp]) >> 11;

			if (lp != 0 && lp != 4 && bossdata.explode_count < 64)
			{
				g = (GetRandomControl() & 31) + 224;
				b = (g >> 2) + (GetRandomControl() & 63);
				r = (GetRandomControl() & 63);
				r = (r * (64 - bossdata.explode_count)) >> 6;
				g = (g * (64 - bossdata.explode_count)) >> 6;
				b = (b * (64 - bossdata.explode_count)) >> 6;
				shptr->rgb = r | (g << 8) | (b << 16);
			}
			else
				shptr->rgb = 0;
			angle += 512;
			angle &= 4095;
			shptr++;
		}
	}
}

static void LondonBossDie(short item_number)
{
	ITEM_INFO* item;
	item = &Items[item_number];
	item->collidable = false;
	item->hitPoints = -16384;

	KillItem(item_number);
	DisableBaddieAI(item_number);

	item->flags |= ONESHOT;	// Don't want bad guys triggered ever again
}

void ControlLaserBolts(short item_number)
{
	FLOOR_INFO* floor;
	ITEM_INFO* item;
	PHD_VECTOR	oldpos;
	long g, b, rad;
	short roomNumber, oldroom, hitlara;
	int speed;

	item = &Items[item_number];

	oldpos.x = item->pos.xPos;
	oldpos.y = item->pos.yPos;
	oldpos.z = item->pos.zPos;
	oldroom = item->roomNumber;

	speed = (item->speed * COS(item->pos.xRot)) >> W2V_SHIFT;
	item->pos.zPos += (speed * COS(item->pos.yRot)) >> W2V_SHIFT;
	item->pos.xPos += (speed * SIN(item->pos.yRot)) >> W2V_SHIFT;
	item->pos.yPos += -((item->speed * SIN(item->pos.xRot)) >> W2V_SHIFT);
	if (item->speed < BOLT_SPEED)
		item->speed += (item->speed >> 3) + 2;

	if (item->itemFlags[2] && item->speed > (BOLT_SPEED >> 1))
	{
		item->itemFlags[3]++;
		if (item->itemFlags[3] >= 16)
		{
			KillItem(item_number);
			return;
		}
	}

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	if (item->roomNumber != roomNumber)
		ItemNewRoom(item_number, roomNumber);

	if (!item->itemFlags[2])
	{
		if (ItemNearLara(&item->pos, 400))
			hitlara = 1;
		else
			hitlara = 0;

		item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		if (hitlara || item->pos.yPos >= item->floor || item->pos.yPos <= GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos))
		{
			SoundEffect(SFX_EXPLOSION1, &item->pos, 0);
			if (item->itemFlags[0] < 0)
				rad = 2;
			else
				rad = 3;
			TriggerExplosionSparks(oldpos.x, oldpos.y, oldpos.z, rad, -2, 2, item->roomNumber);	// -2 = Set off a dynamic light controller.
			for (g = 0; g < rad; g++)
				TriggerExplosionSparks(oldpos.x, oldpos.y, oldpos.z, 2, -1, 2, item->roomNumber);
			rad++;
			for (g = 0; g < rad; g++)
				TriggerPlasmaBall(item, 1, &oldpos, oldroom, item->pos.yRot);

			if (hitlara)
			{
				LaraItem->hitPoints -= 30 + ((item->itemFlags[0] >= 0) << 9);
				LaraItem->hitStatus = true;
			}
			else
			{
				long	dx, dy, dz;
				dx = SQUARE(LaraItem->pos.xPos - item->pos.xPos);
				dy = SQUARE(LaraItem->pos.yPos - 256 - item->pos.yPos);
				dz = SQUARE(LaraItem->pos.zPos - item->pos.zPos);
				dx = sqrt(dx + dy + dz);
				if (dx < 0x400)
				{
					LaraItem->hitPoints -= (0x400 - dx) >> (6 - ((item->itemFlags[0] >= 0) << 1));
					LaraItem->hitStatus = true;
				}
			}

			KillItem(item_number);
			return;
		}
	}
	g = 31 - (GetRandomControl() & 7);
	b = g >> 1;

	if (item->itemFlags[0] < 0)	// Small bolt?
	{
		if (item->itemFlags[2])
		{
			g = (g * (16 - item->itemFlags[3])) >> 4;
			b = (b * (16 - item->itemFlags[3])) >> 4;
		}

		rad = -item->itemFlags[0];
		if (rad > SMALL_FLASH)
		{
			item->itemFlags[0]++;
			item->itemFlags[1] += 2;
		}
	}
	else
	{
		rad = item->itemFlags[0];
		if (rad > BIG_FLASH)
		{
			item->itemFlags[0]--;
			item->itemFlags[1] += 4;
		}
	}

	TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, rad, 0, g, b);
}

void ControlLondBossPlasmaBall(short fx_number)
{
	FX_INFO* fx;
	FLOOR_INFO* floor;
	long speed;
	long rnd, r, g, b, old_y;
	byte radtab[2] = { 13,7 };
	short roomNumber;

	fx = &Effects[fx_number];
	old_y = fx->pos.yPos;

	fx->fallspeed++;
	if (fx->speed > 8)
		fx->speed -= 2;
	if (fx->pos.xRot > -0x3c00)
		fx->pos.xRot -= 0x100;

	speed = (fx->speed * COS(fx->pos.xRot)) >> W2V_SHIFT;
	fx->pos.zPos += (speed * COS(fx->pos.yRot)) >> W2V_SHIFT;
	fx->pos.xPos += (speed * SIN(fx->pos.yRot)) >> W2V_SHIFT;
	fx->pos.yPos += -((fx->speed * SIN(fx->pos.xRot)) >> W2V_SHIFT) + fx->fallspeed;
	if ((Wibble & 15) == 0)
		TriggerPlasmaBallFlame(fx_number, 0, 0, abs(old_y - fx->pos.yPos) << 3, 0);

	roomNumber = fx->roomNumber;
	floor = GetFloor(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, &roomNumber);
	if (fx->pos.yPos >= GetFloorHeight(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos) ||
		fx->pos.yPos < GetCeiling(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos))
	{
		KillEffect(fx_number);
		return;
	}

	if (Rooms[roomNumber].flags & ENV_FLAG_WATER)
	{
		KillEffect(fx_number);
		return;
	}

	if (!fx->flag2 && ItemNearLara(&fx->pos, 200))
	{
		LaraItem->hitPoints -= 25;
		LaraItem->hitStatus = true;
		KillEffect(fx_number);
		return;
	}

	if (roomNumber != fx->roomNumber)
		EffectNewRoom(fx_number, LaraItem->roomNumber);

	rnd = GetRandomControl();
	g = 31 - ((rnd >> 4) & 3);
	b = 24 - ((rnd >> 6) & 3);
	r = rnd & 7;
	TriggerDynamicLight(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, radtab[fx->flag1], r, g, b);
}

void InitialiseLondonBoss(short item_number)
{
	SHIELD_POINTS* shptr;
	long lp, lp1, angle, y;

	bossdata.explode_count = 0;
	bossdata.ring_count = 0;
	bossdata.dead = 0;
	bossdata.dropped_icon = 0;

	shptr = &LondonBossShield[0];

	for (lp = 0; lp < 5; lp++)
	{
		y = heights[lp];
		angle = 0;
		for (lp1 = 0; lp1 < 8; lp1++)
		{
			shptr->x = (rcossin_tbl[angle << 1] * radii[lp]) >> 11;
			shptr->y = y;
			shptr->z = (rcossin_tbl[(angle << 1) + 1] * radii[lp]) >> 11;
			shptr->rgb = 0;	// rgb.
			angle += 512;
			shptr++;
		}
	}
}

void LondonBossControl(short item_number)
{
	if (!CreatureActive(item_number))
		return;

	ITEM_INFO* item, *enemy;
	CREATURE_INFO* londonboss;
	short angle, head, tilt, lp, torso_y, torso_x;
	int lara_dx, lara_dz, lara_dy;
	AI_INFO info, lara_info;
	PHD_VECTOR	pos1, trident[3];

	item = &Items[item_number];
	londonboss = (CREATURE_INFO*)item->data;
	head = angle = tilt = torso_x = torso_y = 0;

	if (item->itemFlags[2])
	{
		if (item->itemFlags[2] == 1)
			item->hitPoints = 0;

		if (item->itemFlags[2] >= 12)
			TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, (GetRandomControl() & 3) + 8, 0, (GetRandomControl() & 7) + 8, (GetRandomControl() & 7) + 16);
		else
			TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, 25 - (item->itemFlags[2] << 1) + (GetRandomControl() & 1), (16 - item->itemFlags[2]) + (GetRandomControl() & 7), 32 - item->itemFlags[2], 31);
	}

	for (lp = 0; lp < 3; lp++)
	{
		trident[lp].x = londonboss_points[lp].x;
		trident[lp].y = londonboss_points[lp].y;
		trident[lp].z = londonboss_points[lp].z;
		GetJointAbsPosition(item, &trident[lp], londonboss_points[lp].meshNum);
	}

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != LONDONBOSS_DEATH)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + LONDONBOSS_DIE_ANIM;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = LONDONBOSS_DEATH;
		}

		if (Anims[item->animNumber].frameEnd - item->frameNumber == 1)
		{
			item->frameNumber = Anims[item->animNumber].frameEnd - 1;
			item->meshBits = 0;	// Don't draw any of the boss while I'm killing it.
			if (bossdata.explode_count == 0)
			{
				bossdata.ring_count = 0;
				for (lp = 0; lp < 6; lp++)
				{
					ExpRings[lp].on = false;
					ExpRings[lp].life = 32;
					ExpRings[lp].radius = 512;
					ExpRings[lp].speed = 128 + (lp << 5);
					ExpRings[lp].xrot = ((GetRandomControl() & 511) - 256) << 4;
					ExpRings[lp].zrot = ((GetRandomControl() & 511) - 256) << 4;
				}

				if (bossdata.dropped_icon == false)
				{
					//BossDropIcon(item_number);
					bossdata.dropped_icon = true;
				}
			}

			if (bossdata.explode_count < 256)
				bossdata.explode_count++;

			if (bossdata.explode_count > 128 && bossdata.ring_count == 6 && ExpRings[5].life == 0)
			{
				LondonBossDie(item_number);
				bossdata.dead = true;
			}
			else
			{
				ExplodeLondonBoss(item);
			}
			return;
		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(londonboss);

		CreatureAIInfo(item, &info);

		if (londonboss->enemy == LaraItem)
		{
			lara_info.angle = info.angle;
			lara_info.distance = info.distance;
			lara_info.xAngle = info.xAngle;
		}
		else
		{
			lara_dz = LaraItem->pos.zPos - item->pos.zPos;
			lara_dx = LaraItem->pos.xPos - item->pos.xPos;
			lara_dy = item->pos.yPos - LaraItem->pos.yPos;

			lara_info.angle = ATAN(lara_dz, lara_dx) - item->pos.yRot; //only need to fill out the bits of lara_info that will be needed by TargetVisible
			lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
			if (abs(lara_dx) > abs(lara_dz))
				lara_info.xAngle = ATAN(abs(lara_dx) + (abs(lara_dz) >> 1), lara_dy);
			else
				lara_info.xAngle = ATAN(abs(lara_dz) + (abs(lara_dx) >> 1), lara_dy);

		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, londonboss->maximumTurn);

		enemy = londonboss->enemy; //TargetVisible uses enemy, so need to fill this in as lara if we're doing other things
		londonboss->enemy = LaraItem;
		if (item->hitStatus || lara_info.distance < LONDONBOSS_AWARE_DISTANCE || TargetVisible(item, &lara_info) || LaraItem->pos.yPos < item->pos.yPos)
			AlertAllGuards(item_number);
		londonboss->enemy = enemy;

		if (LaraItem->pos.yPos < item->pos.yPos)
			londonboss->hurtByLara = true;

		if (item->timer > 0)
			item->timer--;

		item->hitPoints = 300;

		switch (item->currentAnimState)
		{
		case LONDONBOSS_LAUGH:
			if (abs(lara_info.angle) < LONDONBOSS_WALK_TURN)
				item->pos.yRot += lara_info.angle;
			else if (lara_info.angle < 0)
				item->pos.yRot -= LONDONBOSS_WALK_TURN;
			else
				item->pos.yRot += LONDONBOSS_WALK_TURN;

			if (londonboss->alerted)
			{
				item->goalAnimState = LONDONBOSS_STAND;
				break;
			}
			break;

		case LONDONBOSS_STAND:
			londonboss->flags = 0;
			londonboss->maximumTurn = 0;

			if (londonboss->reachedGoal)	//She's found this ambush point, so set up the next one
			{
				londonboss->reachedGoal = false;
				item->aiBits |= AMBUSH;
				item->itemFlags[3] += 0x2000;
			}

			head = lara_info.angle;

			if (item->aiBits & GUARD)
			{
				if ((head < -0x3000 || head > 0x3000) && item->pos.yPos > LONDONBOSS_FINAL_HEIGHT)
				{
					item->goalAnimState = LONDONBOSS_WALK;
					londonboss->maximumTurn = LONDONBOSS_WALK_TURN;
				}
				break;

				head = AIGuard(londonboss);
				if (!(GetRandomControl() & 0xF))
					item->goalAnimState = LONDONBOSS_LAUGH;
				break;
			}
			else if ((item->pos.yPos <= LONDONBOSS_FINAL_HEIGHT || item->pos.yPos < LaraItem->pos.yPos) && !(GetRandomControl() & 0xF) && !bossdata.charged && item->timer)
			{
				item->goalAnimState = LONDONBOSS_LAUGH;
			}
			else if (londonboss->reachedGoal || LaraItem->pos.yPos > item->pos.yPos || item->pos.yPos <= LONDONBOSS_FINAL_HEIGHT)
			{
				if (bossdata.charged)
					item->goalAnimState = LONDONBOSS_BIGZAP;
				else if (!item->timer)
					item->goalAnimState = LONDONBOSS_SUMMON;
				else
					item->goalAnimState = LONDONBOSS_LILZAP;
			}
			else if (item->aiBits & PATROL1)
			{
				item->goalAnimState = LONDONBOSS_WALK;
			}
			else if (londonboss->mood == ESCAPE_MOOD || item->pos.yPos > LaraItem->pos.yPos)
			{
				item->goalAnimState = LONDONBOSS_RUN;
			}
			else if (londonboss->mood == BORED_MOOD || ((item->aiBits & FOLLOW) && (londonboss->reachedGoal || lara_info.distance > SQUARE(WALL_SIZE * 2))))
			{
				if (item->requiredAnimState)
					item->goalAnimState = item->requiredAnimState;
				else if (info.ahead)
					item->goalAnimState = LONDONBOSS_STAND;
				else
					item->goalAnimState = LONDONBOSS_RUN;
			}
			else if (info.bite && info.distance < LONDONBOSS_WALK_RANGE)
				item->goalAnimState = LONDONBOSS_WALK;
			else
				item->goalAnimState = LONDONBOSS_RUN;
			break;

		case LONDONBOSS_WALK:
			head = lara_info.angle;
			londonboss->flags = 0;
			londonboss->maximumTurn = LONDONBOSS_WALK_TURN;

			if (item->aiBits & GUARD || (londonboss->reachedGoal && !(item->aiBits & FOLLOW)))
			{

			}
			else if (item->aiBits & PATROL1)
			{
				item->goalAnimState = LONDONBOSS_WALK;
				head = 0;
			}
			else if (londonboss->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = LONDONBOSS_RUN;
			}
			else if (londonboss->mood == BORED_MOOD)
			{
				if (GetRandomControl() < LONDONBOSS_LAUGH_CHANCE)
				{
					item->requiredAnimState = LONDONBOSS_LAUGH;
					item->goalAnimState = LONDONBOSS_STAND;
				}
			}
			else if (info.distance > LONDONBOSS_WALK_RANGE)
			{
				item->goalAnimState = LONDONBOSS_RUN;
			}
			break;

		case LONDONBOSS_RUN:
			if (info.ahead)
				head = info.angle;

			londonboss->maximumTurn = LONDONBOSS_RUN_TURN;
			tilt = angle / 2;

			if (item->aiBits & GUARD || (londonboss->reachedGoal && !(item->aiBits & FOLLOW)))
				item->goalAnimState = LONDONBOSS_STAND;
			else if (londonboss->mood == ESCAPE_MOOD)
				break;
			else if ((item->aiBits & FOLLOW) && (londonboss->reachedGoal || lara_info.distance > SQUARE(WALL_SIZE * 2)))
				item->goalAnimState = LONDONBOSS_STAND;
			else if (londonboss->mood == BORED_MOOD)
				item->goalAnimState = LONDONBOSS_WALK;
			else if (info.ahead && info.distance < LONDONBOSS_WALK_RANGE)
				item->goalAnimState = LONDONBOSS_WALK;
			break;

		case LONDONBOSS_SUMMON:
			head = lara_info.angle;

			if (londonboss->reachedGoal)	//She's found this ambush point, so set up the next one
			{
				londonboss->reachedGoal = false;
				item->aiBits = AMBUSH;
				item->itemFlags[3] += 0x2000;
			}

			if (item->animNumber == Objects[item->objectNumber].animIndex + LONDONBOSS_STND2SUM_ANIM)
			{
				if (item->frameNumber == Anims[item->animNumber].frameBase)
				{
					bossdata.hp_counter = item->hitPoints;
					item->timer = BIGZAP_TIMER;
				}
				else if (item->hitStatus && item->goalAnimState != LONDONBOSS_STAND) //if she takes *any* damage during the stand2summon anim, break off the summoning
				{
					StopSoundEffect(352);
					SoundEffect(353, &item->pos, 0); // Take Hit
					SoundEffect(355, &item->pos, 0); // Summon wind-down noise

					item->goalAnimState = LONDONBOSS_STAND; //Put a condition in here so she can do more than one attack per waypoint?
				}
			}
			else if (item->animNumber == Objects[item->objectNumber].animIndex + LONDONBOSS_SUMMON_ANIM && item->frameNumber == Anims[item->animNumber].frameEnd)
				bossdata.charged = true;

			if (abs(lara_info.angle) < LONDONBOSS_WALK_TURN)
				item->pos.yRot += lara_info.angle;
			else if (lara_info.angle < 0)
				item->pos.yRot -= LONDONBOSS_WALK_TURN;
			else
				item->pos.yRot += LONDONBOSS_WALK_TURN;

			// Summon.
			if ((Wibble & 7) == 0)
			{
				pos1.x = item->pos.xPos;
				pos1.y = (GetRandomControl() & 511) - 256;
				pos1.z = item->pos.zPos;
				TriggerLaserBolt(&pos1, item, SUMMON_BOLT, 0);

				for (lp = 0; lp < 6; lp++)
				{
					long rnd;

					if (!ExpRings[lp].on)
					{
						ExpRings[lp].on = true;
						ExpRings[lp].life = 64;
						ExpRings[lp].speed = (GetRandomControl() & 15) + 16;
						ExpRings[lp].x = item->pos.xPos;
						ExpRings[lp].y = item->pos.yPos + 128 - (rnd = (GetRandomControl() & 1023));
						ExpRings[lp].z = item->pos.zPos;
						ExpRings[lp].xrot = ((GetRandomControl() & 511) - 256) << 4;
						ExpRings[lp].zrot = ((GetRandomControl() & 511) - 256) << 4;
						ExpRings[lp].radius = (1024 + (1024 - (abs(rnd - 512))));
						break;
					}
				}
			}

			londonboss->maximumTurn = 0;
			break;

		case LONDONBOSS_BIGZAP:
			if (londonboss->reachedGoal)	//She's found this ambush point, so set up the next one
			{
				londonboss->reachedGoal = false;
				item->aiBits = AMBUSH;
				item->itemFlags[3] += 0x2000;
			}
			bossdata.charged = false;
			if (abs(lara_info.angle) < LONDONBOSS_WALK_TURN)
				item->pos.yRot += lara_info.angle;
			else if (lara_info.angle < 0)
				item->pos.yRot -= LONDONBOSS_WALK_TURN;
			else
				item->pos.yRot += LONDONBOSS_WALK_TURN;

			torso_y = lara_info.angle;
			torso_x = lara_info.xAngle;

			londonboss->maximumTurn = 0;

			if (item->frameNumber == Anims[item->animNumber].frameBase + 36)
			{
				TriggerLaserBolt(&trident[RIGHT_PRONG], item, NORMAL_BOLT, item->pos.yRot + 0x200);
				TriggerLaserBolt(&trident[ICONPOS], item, LARGE_BOLT, item->pos.yRot);
				TriggerLaserBolt(&trident[LEFT_PRONG], item, NORMAL_BOLT, item->pos.yRot - 0x200);
			}

			break;

		case LONDONBOSS_LILZAP:
			if (londonboss->reachedGoal) //She's found this ambush point, so set up the next one
			{
				londonboss->reachedGoal = false;
				item->aiBits = AMBUSH;
				item->itemFlags[3] += 0x2000;
			}

			if (abs(lara_info.angle) < LONDONBOSS_WALK_TURN)
				item->pos.yRot += lara_info.angle;
			else if (lara_info.angle < 0)
				item->pos.yRot -= LONDONBOSS_WALK_TURN;
			else
				item->pos.yRot += LONDONBOSS_WALK_TURN;

			torso_y = lara_info.angle;
			torso_x = lara_info.xAngle;

			londonboss->maximumTurn = 0;

			if (item->frameNumber == Anims[item->animNumber].frameBase + 14)
			{
				TriggerLaserBolt(&trident[RIGHT_PRONG], item, NORMAL_BOLT, item->pos.yRot + 0x200);
				TriggerLaserBolt(&trident[LEFT_PRONG], item, NORMAL_BOLT, item->pos.yRot - 0x200);
			}

			break;

		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	if ((item->currentAnimState < LONDONBOSS_VAULT2 || item->currentAnimState > LONDONBOSS_GODOWN) && item->currentAnimState != LONDONBOSS_DEATH)
	{
		switch (CreatureVault(item_number, angle, 2, LONDONBOSS_VAULT_SHIFT))
		{
		case 2:
			/* Half block jump */
			londonboss->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + LONDONBOSS_VAULT2_ANIM;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = LONDONBOSS_VAULT2;
			break;

		case 3:
			/* 3/4 block jump */
			londonboss->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + LONDONBOSS_VAULT3_ANIM;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = LONDONBOSS_VAULT3;
			break;

		case 4:
			/* Full block jump */
			londonboss->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + LONDONBOSS_VAULT4_ANIM;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = LONDONBOSS_VAULT4;
			break;
		case -4:
			/* Full block fall */
			londonboss->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + LONDONBOSS_GODOWN_ANIM;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = LONDONBOSS_GODOWN;
			break;
		}
	}
	else
	{
		londonboss->maximumTurn = 0;
		CreatureAnimation(item_number, angle, 0);
	}

	// Do light.
	{
		long g, b;

		g = abs(rcossin_tbl[item->itemFlags[1] << 7] >> 7);
		g += (GetRandomControl() & 7);
		if (g > 31)
			g = 31;
		b = g >> 1;
		TriggerDynamicLight(trident[ICONPOS].x, trident[ICONPOS].y, trident[ICONPOS].z, 10, 0, g >> 1, b >> 1);
		item->itemFlags[1]++;
		item->itemFlags[1] &= 63;
	}

	// Knockback.
	if (item->hitPoints > 0 && item->itemFlags[0] != KNOCKBACK && LaraItem->hitPoints > 0)
	{
		long dx, dy, dz;

		dx = LaraItem->pos.xPos - item->pos.xPos;
		dy = LaraItem->pos.yPos - 256 - item->pos.yPos;
		dz = LaraItem->pos.zPos - item->pos.zPos;

		if (dx > 8000 || dx < -8000 || dy > 8000 || dy < -8000 || dz > 8000 || dz < -8000)
		{
			dx = 0xFFF;
		}
		else
		{
			dx = SQUARE(dx);
			dy = SQUARE(dy);
			dz = SQUARE(dz);
			dx = sqrt(dx + dy + dz);
		}

		if (dx < 0xB00)
		{
			item->itemFlags[0] = KNOCKBACK;
			for (lp = 0; lp < 3; lp++)
			{
				KBRings[lp].on = true;
				KBRings[lp].life = 32;
				KBRings[lp].speed = 64 + ((lp == 1) << 4);
				KBRings[lp].x = item->pos.xPos;
				KBRings[lp].y = item->pos.yPos - 384 - 128 + (lp << 7);
				KBRings[lp].z = item->pos.zPos;
				KBRings[lp].xrot = KBRings[lp].zrot = 0;
				KBRings[lp].radius = 512 + ((lp == 1) << 8);
			}
		}
	}
	else if (KBRings[0].on == false && KBRings[1].on == false && KBRings[2].on == false)
	{
		item->itemFlags[0] = 0;
	}
}

void S_DrawLondonBoss(ITEM_INFO* item)
{
	//DrawAnimatingItem(item);

	if (bossdata.explode_count)
	{
		DrawLondonBossShield(item);
		DrawExplosionRings();
	}
	else
	{
		DrawSummonRings();
		DrawKnockBackRings();
	}

	if (item->hitPoints <= 0 && bossdata.explode_count == 0)
	{
		//UpdateElectricityPoints();
		//LaraElectricDeath(0, item);
		//LaraElectricDeath(1, item);
	}
}