#include "framework.h"
#include "quad.h"
#include "quad_info.h"
#include "lara.h"
#include "effects/effects.h"
#include "items.h"
#include "collide.h"
#include "camera.h"
#include "effects/tomb4fx.h"
#include "lara_flare.h"
#include "lara_one_gun.h"
#include "misc.h"
#include "setup.h"
#include "level.h"
#include "input.h"
#include "animation.h"
#include "Sound/sound.h"
#include "Specific/prng.h"

using std::vector;
using namespace TEN::Math::Random;

#define MAX_VELOCITY	0xA000
#define MIN_DRIFT_SPEED	0x3000
#define BRAKE 0x0280
#define REVERSE_ACC -0x0300
#define MAX_BACK	-0x3000
#define MAX_REVS 0xa000
#define TERMINAL_FALLSPEED 240
#define QUAD_SLIP 100
#define QUAD_SLIP_SIDE 50

#define QUAD_FRONT	550
#define QUAD_BACK  -550
#define QUAD_SIDE	260
#define QUAD_RADIUS	500
#define QUAD_HEIGHT	512
#define QUAD_SNOW	500 // Unused.

#define QUAD_HIT_LEFT  11
#define QUAD_HIT_RIGHT 12
#define QUAD_HIT_FRONT 13
#define QUAD_HIT_BACK  14

#define SMAN_SHOT_DAMAGE 10
#define SMAN_LARA_DAMAGE 50

#define DAMAGE_START  140
#define DAMAGE_LENGTH 14

#define DISMOUNT_DISTANCE 385 // Root bone offset from final frame of animation.

#define QUAD_UNDO_TURN ANGLE(2.0f)
#define QUAD_TURN_RATE (ANGLE(0.5f) + QUAD_UNDO_TURN)
#define QUAD_TURN_MAX ANGLE(5.0f)
#define QUAD_DRIFT_TURN_RATE (ANGLE(0.75f) + QUAD_UNDO_TURN)
#define QUAD_DRIFT_TURN_MAX ANGLE(8.0f)

#define MIN_MOMENTUM_TURN ANGLE(3.0f)
#define MAX_MOMENTUM_TURN ANGLE(1.5f)
#define QUAD_MAX_MOM_TURN ANGLE(150.0f)

#define QUAD_MAX_HEIGHT STEP_SIZE
#define QUAD_MIN_BOUNCE (MAX_VELOCITY / 2) / 256

// TODO: Common controls for all vehicles + unique settings page to set them. @Sezz 2021.11.14
#define QUAD_IN_ACCELERATE	IN_ACTION
#define QUAD_IN_BRAKE		IN_JUMP
#define QUAD_IN_DRIFT		(IN_DUCK | IN_SPRINT)
#define QUAD_IN_DISMOUNT	IN_ROLL
#define QUAD_IN_LEFT		IN_LEFT
#define QUAD_IN_RIGHT		IN_RIGHT

enum QuadState {
	QUAD_STATE_DRIVE = 1,
	QUAD_STATE_TURN_LEFT = 2,
	QUAD_STATE_SLOW = 5,
	QUAD_STATE_BRAKE = 6,
	QUAD_STATE_BIKE_DEATH = 7,
	QUAD_STATE_FALL = 8,
	QUAD_STATE_MOUNT_RIGHT = 9,
	QUAD_STATE_DISMOUNT_RIGHT = 10,
	QUAD_STATE_HIT_BACK = 11,
	QUAD_STATE_HIT_FRONT = 12,
	QUAD_STATE_HIT_LEFT = 13,
	QUAD_STATE_HIT_RIGHT = 14,
	QUAD_STATE_IDLE = 15,
	QUAD_STATE_LAND = 17,
	QUAD_STATE_STOP_SLOWLY = 18,
	QUAD_STATE_FALL_DEATH = 19,
	QUAD_STATE_FALL_OFF = 20,
	QUAD_STATE_WHEELIE = 21, // Unused.
	QUAD_STATE_TURN_RIGHT = 22,
	QUAD_STATE_MOUNT_LEFT = 23,
	QUAD_STATE_DISMOUNT_LEFT = 24,
};

enum QuadAnim {
	QUAD_ANIM_IDLE_DEATH = 0,
	QUAD_ANIM_UNK_1 = 1,
	QUAD_ANIM_DRIVE_BACK = 2,
	QUAD_ANIM_TURN_LEFT_START = 3,
	QUAD_ANIM_TURN_LEFT_CONTINUE = 4,
	QUAD_ANIM_TURN_LEFT_END = 5,
	QUAD_ANIM_LEAP_START = 6,
	QUAD_ANIM_LEAP_CONTINUE = 7,
	QUAD_ANIM_LEAP_END = 8,
	QUAD_ANIM_MOUNT_RIGHT = 9,
	QUAD_ANIM_DISMOUNT_RIGHT = 10,
	QUAD_ANIM_HIT_FRONT = 11,
	QUAD_ANIM_HIT_BACK = 12,
	QUAD_ANIM_HIT_RIGHT = 13,
	QUAD_ANIM_HIT_LEFT = 14,
	QUAD_ANIM_UNK_2 = 15,
	QUAD_ANIM_UNK_3 = 16,
	QUAD_ANIM_UNK_4 = 17,
	QUAD_ANIM_IDLE = 18,
	QUAD_ANIM_FALL_OFF_DEATH = 19,
	QUAD_ANIM_TURN_RIGHT_START = 20,
	QUAD_ANIM_TURN_RIGHT_CONTINUE = 21,
	QUAD_ANIM_TURN_RIGHT_END = 22,
	QUAD_ANIM_MOUNT_LEFT = 23,
	QUAD_ANIM_DISMOUNT_LEFT = 24,
	QUAD_ANIM_LEAP_START2 = 25,
	QUAD_ANIM_LEAP_CONTINUE2 = 26,
	QUAD_ANIM_LEAP_END2 = 27,
	QUAD_ANIM_LEAP_TO_FREEFALL = 28
};

enum QuadFlags {
	QUAD_FLAG_DEAD = 0x80,
	QUAD_FLAG_IS_FALLING = 0x40
};

enum QuadEffectPosition {
	EXHAUST_LEFT = 0,
	EXHAUST_RIGHT,
	FRONT_LEFT_TYRE,
	FRONT_RIGHT_TYRE,
	BACK_LEFT_TYRE,
	BACK_RIGHT_TYRE
};

BITE_INFO quadEffectsPositions[6] = {
	{ -56, -32, -380, 0	},
	{ 56, -32, -380, 0 },
	{ -8, 180, -48, 3 },
	{ 8, 180, -48, 4 },
	{ 90, 180, -32, 6 },
	{ -90, 180, -32, 7 }
};

bool QuadDriftStarting;
bool QuadCanDriftStart;
int QuadSmokeStart;
bool QuadNoGetOff;

void InitialiseQuadBike(short itemNumber)
{
	ITEM_INFO* quad = &g_Level.Items[itemNumber];
	quad->data = QUAD_INFO();
	QUAD_INFO* quadInfo = quad->data;

	quadInfo->velocity = 0;
	quadInfo->turnRate = 0;
	quadInfo->leftFallspeed = 0;
	quadInfo->rightFallspeed = 0;
	quadInfo->momentumAngle = quad->pos.yRot;
	quadInfo->extraRotation = 0;
	quadInfo->trackMesh = 0;
	quadInfo->pitch = 0;
	quadInfo->flags = 0;
}

static void QuadbikeExplode(ITEM_INFO* lara, ITEM_INFO* quad)
{
	LaraInfo*& laraInfo = LaraItem->data;

	if (g_Level.Rooms[quad->roomNumber].flags & ENV_FLAG_WATER)
		TriggerUnderwaterExplosion(quad, 1);
	else
	{
		TriggerExplosionSparks(quad->pos.xPos, quad->pos.yPos, quad->pos.zPos, 3, -2, 0, quad->roomNumber);

		for (int i = 0; i < 3; i++)
			TriggerExplosionSparks(quad->pos.xPos, quad->pos.yPos, quad->pos.zPos, 3, -1, 0, quad->roomNumber);
	}

	auto pos = PHD_3DPOS(quad->pos.xPos, quad->pos.yPos - (STEP_SIZE / 2), quad->pos.zPos, 0, quad->pos.yRot, 0);
	TriggerShockwave(&pos, 50, 180, 40, GenerateFloat(160, 200), 60, 60, 64, GenerateFloat(0, 359), 0);
	SoundEffect(SFX_TR4_EXPLOSION1, NULL, 0);
	SoundEffect(SFX_TR4_EXPLOSION2, NULL, 0);

	quad->status = ITEM_DEACTIVATED;
	laraInfo->Vehicle = NO_ITEM;
}

static int CanQuadbikeGetOff(int direction)
{
	short angle;
	auto item = &g_Level.Items[Lara.Vehicle];

	if (direction < 0)
		angle = item->pos.yRot - ANGLE(90.0f);
	else
		angle = item->pos.yRot + ANGLE(90.0f);

	int x = item->pos.xPos + (STEP_SIZE * 2) * phd_sin(angle);
	int y = item->pos.yPos;
	int z = item->pos.zPos + (STEP_SIZE * 2) * phd_cos(angle);

	auto collResult = GetCollisionResult(x, y, z, item->roomNumber);

	if (collResult.Position.Slope ||
		collResult.Position.Floor == NO_HEIGHT)
	{
		return false;
	}

	if (abs(collResult.Position.Floor - item->pos.yPos) > (STEP_SIZE * 2))
		return false;

	if ((collResult.Position.Ceiling - item->pos.yPos) > -LARA_HEIGHT ||
		(collResult.Position.Floor - collResult.Position.Ceiling) < LARA_HEIGHT)
	{
		return false;
	}

	return true;
}

static bool QuadCheckGetOff(ITEM_INFO* lara, ITEM_INFO* quad)
{
	LaraInfo*& laraInfo = lara->data;
	auto quadInfo = (QUAD_INFO*)quad->data;

	if (laraInfo->Vehicle == NO_ITEM)
		return true;

	if ((lara->currentAnimState == QUAD_STATE_DISMOUNT_RIGHT || lara->currentAnimState == QUAD_STATE_DISMOUNT_LEFT) &&
		TestLastFrame(lara))
	{
		if (lara->currentAnimState == QUAD_STATE_DISMOUNT_LEFT)
			lara->pos.yRot += ANGLE(90.0f);
		else
			lara->pos.yRot -= ANGLE(90.0f);

		SetAnimation(lara, LA_STAND_IDLE);
		lara->pos.xPos -= DISMOUNT_DISTANCE * phd_sin(lara->pos.yRot);
		lara->pos.zPos -= DISMOUNT_DISTANCE * phd_cos(lara->pos.yRot);
		lara->pos.xRot = 0;
		lara->pos.zRot = 0;
		laraInfo->Vehicle = NO_ITEM;
		laraInfo->gunStatus = LG_NO_ARMS;

		if (lara->currentAnimState == QUAD_STATE_FALL_OFF)
		{
			PHD_VECTOR pos = { 0, 0, 0 };

			SetAnimation(lara, LA_FREEFALL);
			GetJointAbsPosition(lara, &pos, LM_HIPS);

			lara->pos.xPos = pos.x;
			lara->pos.yPos = pos.y;
			lara->pos.zPos = pos.z;
			lara->fallspeed = quad->fallspeed;
			lara->gravityStatus = true;
			lara->pos.xRot = 0;
			lara->pos.zRot = 0;
			lara->hitPoints = 0;
			laraInfo->gunStatus = LG_NO_ARMS;
			quad->flags |= ONESHOT;

			return false;
		}
		else if (lara->currentAnimState == QUAD_STATE_FALL_DEATH)
		{
			quadInfo->flags |= QUAD_FLAG_DEAD;
			lara->goalAnimState = LS_DEATH;
			lara->fallspeed = DAMAGE_START + DAMAGE_LENGTH;
			lara->speed = 0;

			return false;
		}

		return true;
	}
	else
		return true;
}

static int GetOnQuadBike(ITEM_INFO* lara, ITEM_INFO* quad, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = lara->data;

	if (!(TrInput & IN_ACTION) ||
		lara->gravityStatus ||
		laraInfo->gunStatus != LG_NO_ARMS ||
		quad->flags & ONESHOT ||
		abs(quad->pos.yPos - lara->pos.yPos) > STEP_SIZE)
	{
		return false;
	}

	auto dist = pow(lara->pos.xPos - quad->pos.xPos, 2) + pow(lara->pos.zPos - quad->pos.zPos, 2);
	if (dist > 170000)
		return false;

	auto probe = GetCollisionResult(quad);
	if (probe.Position.Floor < -32000)
		return false;
	else
	{
		short angle = phd_atan(quad->pos.zPos - lara->pos.zPos, quad->pos.xPos - lara->pos.xPos);
		angle -= quad->pos.yRot;

		if ((angle > -ANGLE(45.0f)) && (angle < ANGLE(135.0f)))
		{
			short tempAngle = lara->pos.yRot - quad->pos.yRot;
			if (tempAngle > ANGLE(45.0f) && tempAngle < ANGLE(135.0f))
				return true;
			else
				return false;
		}
		else
		{
			short tempAngle = lara->pos.yRot - quad->pos.yRot;
			if (tempAngle > ANGLE(225.0f) && tempAngle < ANGLE(315.0f))
				return true;
			else
				return false;
		}
	}

	return true;
}

static void QuadBaddieCollision(ITEM_INFO* lara, ITEM_INFO* quad)
{
	vector<short> roomsList;
	roomsList.push_back(quad->roomNumber);

	ROOM_INFO* room = &g_Level.Rooms[quad->roomNumber];
	for (int i = 0; i < room->doors.size(); i++)
		roomsList.push_back(room->doors[i].room);

	for (int i = 0; i < roomsList.size(); i++)
	{
		auto itemNum = g_Level.Rooms[roomsList[i]].itemNumber;

		while (itemNum != NO_ITEM)
		{
			ITEM_INFO* item = &g_Level.Items[itemNum];
			if (item->collidable &&
				item->status != ITEM_INVISIBLE &&
				item != lara &&
				item != quad)
			{
				OBJECT_INFO* object = &Objects[item->objectNumber];
				if (object->collision && object->intelligent)
				{
					auto x = quad->pos.xPos - item->pos.xPos;
					auto y = quad->pos.yPos - item->pos.yPos;
					auto z = quad->pos.zPos - item->pos.zPos;

					if (x > -4096 && x < 4096 && z > -4096 && z < 4096 && y > -4096 && y < 4096)
					{
						if (TestBoundsCollide(item, quad, QUAD_RADIUS))
						{
							DoLotsOfBlood(item->pos.xPos, quad->pos.yPos - STEP_SIZE, item->pos.zPos, quad->speed, quad->pos.yRot, item->roomNumber, 3);
							item->hitPoints = 0;
						}
					}
				}
			}

			itemNum = item->nextItem;
		}
	}
}

static int GetQuadCollisionAnim(ITEM_INFO* quad, PHD_VECTOR* p)
{
	p->x = quad->pos.xPos - p->x;
	p->z = quad->pos.zPos - p->z;

	if (p->x || p->z)
	{
		float c = phd_cos(quad->pos.yRot);
		float s = phd_sin(quad->pos.yRot);
		int front = p->z * c + p->x * s;
		int side = -p->z * s + p->x * c;

		if (abs(front) > abs(side))
		{
			if (front > 0)
				return QUAD_HIT_BACK;
			else
				return QUAD_HIT_FRONT;
		}
		else
		{
			if (side > 0)
				return QUAD_HIT_LEFT;
			else
				return QUAD_HIT_RIGHT;
		}
	}

	return 0;
}

static int TestQuadHeight(ITEM_INFO* quad, int dz, int dx, PHD_VECTOR* pos)
{
	pos->y = quad->pos.yPos - dz * phd_sin(quad->pos.xRot) + dx * phd_sin(quad->pos.zRot);

	float c = phd_cos(quad->pos.yRot);
	float s = phd_sin(quad->pos.yRot);

	pos->z = quad->pos.zPos + dz * c - dx * s;
	pos->x = quad->pos.xPos + dz * s + dx * c;

	auto probe = GetCollisionResult(pos->x, pos->y, pos->z, quad->roomNumber);
	if (probe.Position.Ceiling > pos->y ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		return NO_HEIGHT;
	}

	return probe.Position.Floor;
}

static int DoQuadShift(ITEM_INFO* quad, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	COLL_RESULT probe;
	auto x = pos->x / SECTOR(1);
	auto z = pos->z / SECTOR(1);
	auto oldX = old->x / SECTOR(1);
	auto oldZ = old->z / SECTOR(1);
	auto shiftX = pos->x & (WALL_SIZE - 1);
	auto shiftZ = pos->z & (WALL_SIZE - 1);

	if (x == oldX)
	{
		if (z == oldZ)
		{
			quad->pos.zPos += (old->z - pos->z);
			quad->pos.xPos += (old->x - pos->x);
		}
		else if (z > oldZ)
		{
			quad->pos.zPos -= shiftZ + 1;

			return (pos->x - quad->pos.xPos);
		}
		else
		{
			quad->pos.zPos += WALL_SIZE - shiftZ;

			return (quad->pos.xPos - pos->x);
		}
	}
	else if (z == oldZ)
	{
		if (x > oldX)
		{
			quad->pos.xPos -= shiftX + 1;

			return (quad->pos.zPos - pos->z);
		}
		else
		{
			quad->pos.xPos += WALL_SIZE - shiftX;

			return (pos->z - quad->pos.zPos);
		}
	}
	else
	{
		x = 0;
		z = 0;

		probe = GetCollisionResult(old->x, pos->y, pos->z, quad->roomNumber);
		if (probe.Position.Floor < (old->y - STEP_SIZE))
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = WALL_SIZE - shiftZ;
		}

		probe = GetCollisionResult(pos->x, pos->y, old->z, quad->roomNumber);
		if (probe.Position.Floor < (old->y - STEP_SIZE))
		{
			if (pos->x > old->x)
				x = -shiftX - 1;
			else
				x = WALL_SIZE - shiftX;
		}

		if (x && z)
		{
			quad->pos.zPos += z;
			quad->pos.xPos += x;
		}
		else if (z)
		{
			quad->pos.zPos += z;

			if (z > 0)
				return (quad->pos.xPos - pos->x);
			else
				return (pos->x - quad->pos.xPos);
		}
		else if (x)
		{
			quad->pos.xPos += x;

			if (x > 0)
				return (pos->z - quad->pos.zPos);
			else
				return (quad->pos.zPos - pos->z);
		}
		else
		{
			quad->pos.zPos += (old->z - pos->z);
			quad->pos.xPos += (old->x - pos->x);
		}
	}

	return 0;
}

static int DoQuadDynamics(int height, int fallspeed, int* y)
{
	if (height > *y)
	{
		*y += fallspeed;
		if (*y > height - QUAD_MIN_BOUNCE)
		{
			*y = height;
			fallspeed = 0;
		}
		else
			fallspeed += 6;
	}
	else
	{
		int kick = (height - *y) * 4;

		if (kick < -80)
			kick = -80;

		fallspeed += ((kick - fallspeed) / 8);

		if (*y > height)
			*y = height;
	}

	return fallspeed;
}

static int QuadDynamics(ITEM_INFO* lara, ITEM_INFO* quad)
{
	LaraInfo*& laraInfo = lara->data;
	auto quadInfo = (QUAD_INFO*)quad->data;

	COLL_RESULT probe;
	PHD_VECTOR moved, fl, fr, br, bl, mtl, mbl, mtr, mbr, mml, mmr;
	PHD_VECTOR old, oldFrontLeft, oldFrontRight, oldBottomLeft, oldBottomRight, mtl_old, moldBottomLeft, mtr_old, moldBottomRight, mml_old, mmr_old;
	int heightFrontLeft, heightFrontRight, heightBackRight, heightBackLeft, hmtl, hmbl, hmtr, hmbr, hmml, hmmr;
	int holdFrontRight, holdFrontLeft, holdBottomRight, holdBottomLeft, hmtl_old, hmoldBottomLeft, hmtr_old, hmoldBottomRight, hmml_old, hmmr_old;
	int slip, collide;
	short rot, rotadd;
	int newspeed;

	QuadNoGetOff = false;

	holdFrontLeft = TestQuadHeight(quad, QUAD_FRONT, -QUAD_SIDE, &oldFrontLeft);
	holdFrontRight = TestQuadHeight(quad, QUAD_FRONT, QUAD_SIDE, &oldFrontRight);
	holdBottomLeft = TestQuadHeight(quad, -QUAD_FRONT, -QUAD_SIDE, &oldBottomLeft);
	holdBottomRight = TestQuadHeight(quad, -QUAD_FRONT, QUAD_SIDE, &oldBottomRight);
	hmml_old = TestQuadHeight(quad, 0, -QUAD_SIDE, &mml_old);
	hmmr_old = TestQuadHeight(quad, 0, QUAD_SIDE, &mmr_old);
	hmtl_old = TestQuadHeight(quad, QUAD_FRONT / 2, -QUAD_SIDE, &mtl_old);
	hmtr_old = TestQuadHeight(quad, QUAD_FRONT / 2, QUAD_SIDE, &mtr_old);
	hmoldBottomLeft = TestQuadHeight(quad, -QUAD_FRONT / 2, -QUAD_SIDE, &moldBottomLeft);
	hmoldBottomRight = TestQuadHeight(quad, -QUAD_FRONT / 2, QUAD_SIDE, &moldBottomRight);

	old.x = quad->pos.xPos;
	old.y = quad->pos.yPos;
	old.z = quad->pos.zPos;

	if (oldBottomLeft.y > holdBottomLeft)
		oldBottomLeft.y = holdBottomLeft;
	if (oldBottomRight.y > holdBottomRight)
		oldBottomRight.y = holdBottomRight;
	if (oldFrontLeft.y > holdFrontLeft)
		oldFrontLeft.y = holdFrontLeft;
	if (oldFrontRight.y > holdFrontRight)
		oldFrontRight.y = holdFrontRight;
	if (moldBottomLeft.y > hmoldBottomLeft)
		moldBottomLeft.y = hmoldBottomLeft;
	if (moldBottomRight.y > hmoldBottomRight)
		moldBottomRight.y = hmoldBottomRight;
	if (mtl_old.y > hmtl_old)
		mtl_old.y = hmtl_old;
	if (mtr_old.y > hmtr_old)
		mtr_old.y = hmtr_old;
	if (mml_old.y > hmml_old)
		mml_old.y = hmml_old;
	if (mmr_old.y > hmmr_old)
		mmr_old.y = hmmr_old;

	if (quad->pos.yPos > (quad->floor - STEP_SIZE))
	{
		short momentum;

		if (quadInfo->turnRate < -QUAD_UNDO_TURN)
			quadInfo->turnRate += QUAD_UNDO_TURN;

		else if (quadInfo->turnRate > QUAD_UNDO_TURN)
			quadInfo->turnRate -= QUAD_UNDO_TURN;

		else
			quadInfo->turnRate = 0;

		quad->pos.yRot += quadInfo->turnRate + quadInfo->extraRotation;

		rot = quad->pos.yRot - quadInfo->momentumAngle;

		momentum = MIN_MOMENTUM_TURN - (((((MIN_MOMENTUM_TURN - MAX_MOMENTUM_TURN) * 256) / MAX_VELOCITY) * quadInfo->velocity) / 256);
		if (!(TrInput & QUAD_IN_ACCELERATE) && quadInfo->velocity > 0)
			momentum += (momentum / 4);

		if (rot < -MAX_MOMENTUM_TURN)
		{
			if (rot < -QUAD_MAX_MOM_TURN)
			{
				rot = -QUAD_MAX_MOM_TURN;
				quadInfo->momentumAngle = quad->pos.yRot - rot;
			}
			else
				quadInfo->momentumAngle -= momentum;
		}
		else if (rot > MAX_MOMENTUM_TURN)
		{
			if (rot > QUAD_MAX_MOM_TURN)
			{
				rot = QUAD_MAX_MOM_TURN;
				quadInfo->momentumAngle = quad->pos.yRot - rot;
			}
			else
				quadInfo->momentumAngle += momentum;
		}
		else
			quadInfo->momentumAngle = quad->pos.yRot;
	}

	else
		quad->pos.yRot += quadInfo->turnRate + quadInfo->extraRotation;

	probe = GetCollisionResult(quad);
	int speed = 0;
	if (quad->pos.yPos >= probe.Position.Floor)
		speed = quad->speed * phd_cos(quad->pos.xRot);
	else
		speed = quad->speed;

	quad->pos.zPos += speed * phd_cos(quadInfo->momentumAngle);
	quad->pos.xPos += speed * phd_sin(quadInfo->momentumAngle);

	slip = QUAD_SLIP * phd_sin(quad->pos.xRot);
	if (abs(slip) > QUAD_SLIP / 2)
	{
		if (slip > 0)
			slip -= 10;
		else
			slip += 10;
		quad->pos.zPos -= slip * phd_cos(quad->pos.yRot);
		quad->pos.xPos -= slip * phd_sin(quad->pos.yRot);
	}

	slip = QUAD_SLIP_SIDE * phd_sin(quad->pos.zRot);
	if (abs(slip) > QUAD_SLIP_SIDE / 2)
	{
		quad->pos.zPos -= slip * phd_sin(quad->pos.yRot);
		quad->pos.xPos += slip * phd_cos(quad->pos.yRot);
	}

	moved.x = quad->pos.xPos;
	moved.z = quad->pos.zPos;

	if (!(quad->flags & ONESHOT))
		QuadBaddieCollision(lara, quad);

	rot = 0;

	heightFrontLeft = TestQuadHeight(quad, QUAD_FRONT, -QUAD_SIDE, &fl);
	if (heightFrontLeft < oldFrontLeft.y - STEP_SIZE)
		rot = DoQuadShift(quad, &fl, &oldFrontLeft);

	hmtl = TestQuadHeight(quad, QUAD_FRONT / 2, -QUAD_SIDE, &mtl);
	if (hmtl < mtl_old.y - STEP_SIZE)
		DoQuadShift(quad, &mtl, &mtl_old);

	hmml = TestQuadHeight(quad, 0, -QUAD_SIDE, &mml);
	if (hmml < mml_old.y - STEP_SIZE)
		DoQuadShift(quad, &mml, &mml_old);

	hmbl = TestQuadHeight(quad, -QUAD_FRONT / 2, -QUAD_SIDE, &mbl);
	if (hmbl < moldBottomLeft.y - STEP_SIZE)
		DoQuadShift(quad, &mbl, &moldBottomLeft);

	heightBackLeft = TestQuadHeight(quad, -QUAD_FRONT, -QUAD_SIDE, &bl);
	if (heightBackLeft < oldBottomLeft.y - STEP_SIZE)
	{
		rotadd = DoQuadShift(quad, &bl, &oldBottomLeft);
		if ((rotadd > 0 && rot >= 0) || (rotadd < 0 && rot <= 0))
			rot += rotadd;
	}

	heightFrontRight = TestQuadHeight(quad, QUAD_FRONT, QUAD_SIDE, &fr);
	if (heightFrontRight < oldFrontRight.y - STEP_SIZE)
	{
		rotadd = DoQuadShift(quad, &fr, &oldFrontRight);
		if ((rotadd > 0 && rot >= 0) || (rotadd < 0 && rot <= 0))
			rot += rotadd;
	}

	hmtr = TestQuadHeight(quad, QUAD_FRONT / 2, QUAD_SIDE, &mtr);
	if (hmtr < mtr_old.y - STEP_SIZE)
		DoQuadShift(quad, &mtr, &mtr_old);

	hmmr = TestQuadHeight(quad, 0, QUAD_SIDE, &mmr);
	if (hmmr < mmr_old.y - STEP_SIZE)
		DoQuadShift(quad, &mmr, &mmr_old);

	hmbr = TestQuadHeight(quad, -QUAD_FRONT / 2, QUAD_SIDE, &mbr);
	if (hmbr < moldBottomRight.y - STEP_SIZE)
		DoQuadShift(quad, &mbr, &moldBottomRight);

	heightBackRight = TestQuadHeight(quad, -QUAD_FRONT, QUAD_SIDE, &br);
	if (heightBackRight < oldBottomRight.y - STEP_SIZE)
	{
		rotadd = DoQuadShift(quad, &br, &oldBottomRight);
		if ((rotadd > 0 && rot >= 0) || (rotadd < 0 && rot <= 0))
			rot += rotadd;
	}

	probe = GetCollisionResult(quad);
	if (probe.Position.Floor < quad->pos.yPos - STEP_SIZE)
		DoQuadShift(quad, (PHD_VECTOR*)&quad->pos, &old);

	quadInfo->extraRotation = rot;

	collide = GetQuadCollisionAnim(quad, &moved);

	if (collide)
	{
		newspeed = (quad->pos.zPos - old.z) * phd_cos(quadInfo->momentumAngle) + (quad->pos.xPos - old.x) * phd_sin(quadInfo->momentumAngle);

		newspeed *= 256;

		if (&g_Level.Items[laraInfo->Vehicle] == quad &&
			quadInfo->velocity == MAX_VELOCITY &&
			newspeed < (quadInfo->velocity - 10))
		{
			lara->hitPoints -= (quadInfo->velocity - newspeed) / 128;
			lara->hitStatus = 1;
		}

		if (quadInfo->velocity > 0 && newspeed < quadInfo->velocity)
			quadInfo->velocity = (newspeed < 0) ? 0 : newspeed;

		else if (quadInfo->velocity < 0 && newspeed > quadInfo->velocity)
			quadInfo->velocity = (newspeed > 0) ? 0 : newspeed;

		if (quadInfo->velocity < MAX_BACK)
			quadInfo->velocity = MAX_BACK;
	}

	return collide;
}

static void AnimateQuadBike(ITEM_INFO* lara, ITEM_INFO* quad, int collide, int dead)
{
	auto quadInfo = (QUAD_INFO*)quad->data;

	if (quad->pos.yPos != quad->floor &&
		lara->currentAnimState != QUAD_STATE_FALL &&
		lara->currentAnimState != QUAD_STATE_LAND &&
		lara->currentAnimState != QUAD_STATE_FALL_OFF &&
		!dead)
	{
		if (quadInfo->velocity < 0)
			lara->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_LEAP_START;
		else
			lara->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_LEAP_START2;

		lara->frameNumber = GetFrameNumber(lara, lara->animNumber);
		lara->currentAnimState = QUAD_STATE_FALL;
		lara->goalAnimState = QUAD_STATE_FALL;
	}
	else if (collide &&
		lara->currentAnimState != QUAD_STATE_HIT_FRONT &&
		lara->currentAnimState != QUAD_STATE_HIT_BACK &&
		lara->currentAnimState != QUAD_STATE_HIT_LEFT &&
		lara->currentAnimState != QUAD_STATE_HIT_RIGHT &&
		lara->currentAnimState != QUAD_STATE_FALL_OFF &&
		quadInfo->velocity > (MAX_VELOCITY / 3) &&
		!dead)
	{
		if (collide == QUAD_HIT_FRONT)
		{
			lara->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_BACK;
			lara->currentAnimState = QUAD_STATE_HIT_FRONT;
			lara->goalAnimState = QUAD_STATE_HIT_FRONT;
		}
		else if (collide == QUAD_HIT_BACK)
		{
			lara->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_FRONT;
			lara->currentAnimState = QUAD_STATE_HIT_BACK;
			lara->goalAnimState = QUAD_STATE_HIT_BACK;
		}
		else if (collide == QUAD_HIT_LEFT)
		{
			lara->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_RIGHT;
			lara->currentAnimState = QUAD_STATE_HIT_LEFT;
			lara->goalAnimState = QUAD_STATE_HIT_LEFT;
		}
		else
		{
			lara->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_LEFT;
			lara->currentAnimState = QUAD_STATE_HIT_RIGHT;
			lara->goalAnimState = QUAD_STATE_HIT_RIGHT;
		}

		lara->frameNumber = GetFrameNumber(lara, lara->animNumber);
		SoundEffect(SFX_TR3_QUAD_FRONT_IMPACT, &quad->pos, 0);
	}
	else
	{
		switch (lara->currentAnimState)
		{
		case QUAD_STATE_IDLE:
			if (dead)
				lara->goalAnimState = QUAD_STATE_BIKE_DEATH;
			else if (TrInput & QUAD_IN_DISMOUNT &&
				quadInfo->velocity == 0 &&
				!QuadNoGetOff)
			{
				if (TrInput & QUAD_IN_LEFT && CanQuadbikeGetOff(-1))
					lara->goalAnimState = QUAD_STATE_DISMOUNT_LEFT;
				else if (TrInput & QUAD_IN_RIGHT && CanQuadbikeGetOff(1))
					lara->goalAnimState = QUAD_STATE_DISMOUNT_RIGHT;
			}
			else if (TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE))
				lara->goalAnimState = QUAD_STATE_DRIVE;

			break;

		case QUAD_STATE_DRIVE:
			if (dead)
			{
				if (quadInfo->velocity > (MAX_VELOCITY / 2))
					lara->goalAnimState = QUAD_STATE_FALL_DEATH;
				else
					lara->goalAnimState = QUAD_STATE_BIKE_DEATH;
			}
			else if (!(TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE)) &&
				(quadInfo->velocity / 256) == 0)
			{
				lara->goalAnimState = QUAD_STATE_IDLE;
			}
			else if (TrInput & QUAD_IN_LEFT &&
				!QuadDriftStarting)
			{
				lara->goalAnimState = QUAD_STATE_TURN_LEFT;
			}
			else if (TrInput & QUAD_IN_RIGHT &&
				!QuadDriftStarting)
			{
				lara->goalAnimState = QUAD_STATE_TURN_RIGHT;
			}
			else if (TrInput & QUAD_IN_BRAKE)
			{
				if (quadInfo->velocity > (MAX_VELOCITY / 3 * 2))
					lara->goalAnimState = QUAD_STATE_BRAKE;
				else
					lara->goalAnimState = QUAD_STATE_SLOW;
			}

			break;

		case QUAD_STATE_BRAKE:
		case QUAD_STATE_SLOW:
		case QUAD_STATE_STOP_SLOWLY:
			if ((quadInfo->velocity / 256) == 0)
				lara->goalAnimState = QUAD_STATE_IDLE;
			else if (TrInput & QUAD_IN_LEFT)
				lara->goalAnimState = QUAD_STATE_TURN_LEFT;
			else if (TrInput & QUAD_IN_RIGHT)
				lara->goalAnimState = QUAD_STATE_TURN_RIGHT;

			break;

		case QUAD_STATE_TURN_LEFT:
			if ((quadInfo->velocity / 256) == 0)
				lara->goalAnimState = QUAD_STATE_IDLE;
			else if (TrInput & QUAD_IN_RIGHT)
			{
				lara->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_TURN_RIGHT_START;
				lara->frameNumber = GetFrameNumber(lara, lara->animNumber);
				lara->currentAnimState = QUAD_STATE_TURN_RIGHT;
				lara->goalAnimState = QUAD_STATE_TURN_RIGHT;
			}
			else if (!(TrInput & QUAD_IN_LEFT))
				lara->goalAnimState = QUAD_STATE_DRIVE;

			break;

		case QUAD_STATE_TURN_RIGHT:
			if ((quadInfo->velocity / 256) == 0)
				lara->goalAnimState = QUAD_STATE_IDLE;
			else if (TrInput & QUAD_IN_LEFT)
			{
				lara->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_TURN_LEFT_START;
				lara->frameNumber = GetFrameNumber(lara, lara->animNumber);
				lara->currentAnimState = QUAD_STATE_TURN_LEFT;
				lara->goalAnimState = QUAD_STATE_TURN_LEFT;
			}
			else if (!(TrInput & QUAD_IN_RIGHT))
				lara->goalAnimState = QUAD_STATE_DRIVE;

			break;

		case QUAD_STATE_FALL:
			if (quad->pos.yPos == quad->floor)
				lara->goalAnimState = QUAD_STATE_LAND;
			else if (quad->fallspeed > TERMINAL_FALLSPEED)
				quadInfo->flags |= QUAD_FLAG_IS_FALLING;

			break;

		case QUAD_STATE_FALL_OFF:
			break;

		case QUAD_STATE_HIT_FRONT:
		case QUAD_STATE_HIT_BACK:
		case QUAD_STATE_HIT_LEFT:
		case QUAD_STATE_HIT_RIGHT:
			if (TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE))
				lara->goalAnimState = QUAD_STATE_DRIVE;

			break;
		}

		if (g_Level.Rooms[quad->roomNumber].flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
		{
			lara->goalAnimState = QUAD_STATE_FALL_OFF;
			lara->pos.yPos = quad->pos.yPos + 700;
			lara->roomNumber = quad->roomNumber;
			lara->hitPoints = 0;
			QuadbikeExplode(lara, quad);
		}
	}
}

static int QuadUserControl(ITEM_INFO* quad, int height, int* pitch)
{
	auto quadInfo = (QUAD_INFO*)quad->data;
	bool drive = false; // Never changes?

	if (!(TrInput & QUAD_IN_DRIFT) &&
		!quadInfo->velocity &&
		!QuadCanDriftStart)
	{
		QuadCanDriftStart = true;
	}
	else if (quadInfo->velocity)
		QuadCanDriftStart = false;

	if (!(TrInput & QUAD_IN_DRIFT))
		QuadDriftStarting = false;

	if (!QuadDriftStarting)
	{
		if (quadInfo->revs > 0x10)
		{
			quadInfo->velocity += (quadInfo->revs / 16);
			quadInfo->revs -= (quadInfo->revs / 8);
		}
		else
			quadInfo->revs = 0;
	}

	if (quad->pos.yPos >= (height - STEP_SIZE))
	{
		if (TrInput & IN_LOOK &&
			!quadInfo->velocity)
		{
			LookUpDown();
		}

		// Driving forward.
		if (quadInfo->velocity > 0)
		{
			if (TrInput & QUAD_IN_DRIFT &&
				!QuadDriftStarting &&
				quadInfo->velocity > MIN_DRIFT_SPEED)
			{
				if (TrInput & QUAD_IN_LEFT)
				{
					quadInfo->turnRate -= QUAD_DRIFT_TURN_RATE;
					if (quadInfo->turnRate < -QUAD_DRIFT_TURN_MAX)
						quadInfo->turnRate = -QUAD_DRIFT_TURN_MAX;
				}
				else if (TrInput & QUAD_IN_RIGHT)
				{
					quadInfo->turnRate += QUAD_DRIFT_TURN_RATE;
					if (quadInfo->turnRate > QUAD_DRIFT_TURN_MAX)
						quadInfo->turnRate = QUAD_DRIFT_TURN_MAX;
				}
			}
			else
			{
				if (TrInput & QUAD_IN_LEFT)
				{
					quadInfo->turnRate -= QUAD_TURN_RATE;
					if (quadInfo->turnRate < -QUAD_TURN_MAX)
						quadInfo->turnRate = -QUAD_TURN_MAX;
				}
				else if (TrInput & QUAD_IN_RIGHT)
				{
					quadInfo->turnRate += QUAD_TURN_RATE;
					if (quadInfo->turnRate > QUAD_TURN_MAX)
						quadInfo->turnRate = QUAD_TURN_MAX;
				}
			}
		}
		// Driving back.
		else if (quadInfo->velocity < 0)
		{
			if (TrInput & QUAD_IN_DRIFT &&
				!QuadDriftStarting &&
				quadInfo->velocity < (-MIN_DRIFT_SPEED + 0x800))
			{
				if (TrInput & QUAD_IN_LEFT)
				{
					quadInfo->turnRate -= QUAD_DRIFT_TURN_RATE;
					if (quadInfo->turnRate < -QUAD_DRIFT_TURN_MAX)
						quadInfo->turnRate = -QUAD_DRIFT_TURN_MAX;
				}
				else if (TrInput & QUAD_IN_RIGHT)
				{
					quadInfo->turnRate += QUAD_DRIFT_TURN_RATE;
					if (quadInfo->turnRate > QUAD_DRIFT_TURN_MAX)
						quadInfo->turnRate = QUAD_DRIFT_TURN_MAX;
				}
			}
			else
			{
				if (TrInput & QUAD_IN_RIGHT)
				{
					quadInfo->turnRate -= QUAD_TURN_RATE;
					if (quadInfo->turnRate < -QUAD_TURN_MAX)
						quadInfo->turnRate = -QUAD_TURN_MAX;
				}
				else if (TrInput & QUAD_IN_LEFT)
				{
					quadInfo->turnRate += QUAD_TURN_RATE;
					if (quadInfo->turnRate > QUAD_TURN_MAX)
						quadInfo->turnRate = QUAD_TURN_MAX;
				}
			}
		}

		// Driving back / braking.
		if (TrInput & QUAD_IN_BRAKE)
		{
			if (TrInput & QUAD_IN_DRIFT &&
				(QuadCanDriftStart || QuadDriftStarting))
			{
				QuadDriftStarting = true;
				quadInfo->revs -= 0x200;
				if (quadInfo->revs < MAX_BACK)
					quadInfo->revs = MAX_BACK;
			}
			else if (quadInfo->velocity > 0)
				quadInfo->velocity -= BRAKE;
			else
			{
				if (quadInfo->velocity > MAX_BACK)
					quadInfo->velocity += REVERSE_ACC;
			}
		}
		else if (TrInput & QUAD_IN_ACCELERATE)
		{
			if (TrInput & QUAD_IN_DRIFT &&
				(QuadCanDriftStart || QuadDriftStarting))
			{
				QuadDriftStarting = true;
				quadInfo->revs += 0x200;
				if (quadInfo->revs >= MAX_VELOCITY)
					quadInfo->revs = MAX_VELOCITY;
			}
			else if (quadInfo->velocity < MAX_VELOCITY)
			{
				if (quadInfo->velocity < 0x4000)
					quadInfo->velocity += (8 + (0x4000 + 0x800 - quadInfo->velocity) / 8);
				else if (quadInfo->velocity < 0x7000)
					quadInfo->velocity += (4 + (0x7000 + 0x800 - quadInfo->velocity) / 16);
				else if (quadInfo->velocity < MAX_VELOCITY)
					quadInfo->velocity += (2 + (MAX_VELOCITY - quadInfo->velocity) / 8);
			}
			else
				quadInfo->velocity = MAX_VELOCITY;

			quadInfo->velocity -= abs(quad->pos.yRot - quadInfo->momentumAngle) / 64;
		}

		else if (quadInfo->velocity > 0x0100)
			quadInfo->velocity -= 0x0100;
		else if (quadInfo->velocity < -0x0100)
			quadInfo->velocity += 0x0100;
		else
			quadInfo->velocity = 0;

		if (!(TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE)) &&
			QuadDriftStarting &&
			quadInfo->revs)
		{
			if (quadInfo->revs > 0x8)
				quadInfo->revs -= quadInfo->revs / 8;
			else
				quadInfo->revs = 0;
		}

		quad->speed = quadInfo->velocity / 256;

		if (quadInfo->engineRevs > 0x7000)
			quadInfo->engineRevs = -0x2000;

		int revs = 0;
		if (quadInfo->velocity < 0)
			revs = abs(quadInfo->velocity / 2);
		else if (quadInfo->velocity < 0x7000)
			revs = -0x2000 + (quadInfo->velocity * (0x6800 - -0x2000)) / 0x7000;
		else if (quadInfo->velocity <= MAX_VELOCITY)
			revs = -0x2800 + ((quadInfo->velocity - 0x7000) * (0x7000 - -0x2800)) / (MAX_VELOCITY - 0x7000);

		revs += abs(quadInfo->revs);
		quadInfo->engineRevs += (revs - quadInfo->engineRevs) / 8;
	}
	else
	{
		if (quadInfo->engineRevs < 0xA000)
			quadInfo->engineRevs += (0xA000 - quadInfo->engineRevs) / 8;
	}

	*pitch = quadInfo->engineRevs;

	return drive;
}

void QuadBikeCollision(short itemNumber, ITEM_INFO* lara, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = lara->data;
	ITEM_INFO* quad = &g_Level.Items[itemNumber];
	auto quadInfo = (QUAD_INFO*)quad->data;

	if (lara->hitPoints < 0 || laraInfo->Vehicle != NO_ITEM)
		return;

	if (GetOnQuadBike(lara, &g_Level.Items[itemNumber], coll))
	{
		short ang;

		laraInfo->Vehicle = itemNumber;

		if (laraInfo->gunType == WEAPON_FLARE)
		{
			CreateFlare(lara, ID_FLARE_ITEM, 0);
			UndrawFlareMeshes(lara);
			laraInfo->flareControlLeft = 0;
			laraInfo->requestGunType = laraInfo->gunType = WEAPON_NONE;
		}

		laraInfo->gunStatus = LG_HANDS_BUSY;

		ang = phd_atan(quad->pos.zPos - lara->pos.zPos, quad->pos.xPos - lara->pos.xPos);
		ang -= quad->pos.yRot;

		if (ang > -ANGLE(45.0f) && ang < ANGLE(135.0f))
		{
			lara->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_MOUNT_LEFT;
			lara->currentAnimState = lara->goalAnimState = QUAD_STATE_MOUNT_LEFT;
		}
		else
		{
			lara->animNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_MOUNT_RIGHT;
			lara->currentAnimState = lara->goalAnimState = QUAD_STATE_MOUNT_RIGHT;
		}

		lara->frameNumber = g_Level.Anims[lara->animNumber].frameBase;

		quad->hitPoints = 1;
		lara->pos.xPos = quad->pos.xPos;
		lara->pos.yPos = quad->pos.yPos;
		lara->pos.zPos = quad->pos.zPos;
		lara->pos.yRot = quad->pos.yRot;
		laraInfo->headXrot = laraInfo->headYrot = 0;
		laraInfo->torsoXrot = laraInfo->torsoYrot = 0;
		laraInfo->hitDirection = -1;

		AnimateItem(lara);

		quadInfo->revs = 0;
	}
	else
		ObjectCollision(itemNumber, lara, coll);
}

static void TriggerQuadExhaustSmoke(int x, int y, int z, short angle, int speed, int moving)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];

	spark->on = true;
	spark->sR = 0;
	spark->sG = 0;
	spark->sB = 0;

	spark->dR = 96;
	spark->dG = 96;
	spark->dB = 128;

	if (moving)
	{
		spark->dR = (spark->dR * speed) / 32;
		spark->dG = (spark->dG * speed) / 32;
		spark->dB = (spark->dB * speed) / 32;
	}

	spark->sLife = spark->life = (GetRandomControl() & 3) + 20 - (speed / 4096);
	if (spark->sLife < 9)
		spark->sLife = spark->life = 9;

	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 4;
	spark->extras = 0;
	spark->dynamic = -1;
	spark->x = x + ((GetRandomControl() & 15) - 8);
	spark->y = y + ((GetRandomControl() & 15) - 8);
	spark->z = z + ((GetRandomControl() & 15) - 8);
	int zv = speed * phd_cos(angle) / 4;
	int xv = speed * phd_sin(angle) / 4;
	spark->xVel = xv + ((GetRandomControl() & 255) - 128);
	spark->yVel = -(GetRandomControl() & 7) - 8;
	spark->zVel = zv + ((GetRandomControl() & 255) - 128);
	spark->friction = 4;

	if (GetRandomControl() & 1)
	{
		spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		spark->rotAng = GetRandomControl() & 4095;
		if (GetRandomControl() & 1)
			spark->rotAdd = -(GetRandomControl() & 7) - 24;
		else
			spark->rotAdd = (GetRandomControl() & 7) + 24;
	}
	else
		spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex;
	spark->scalar = 2;
	spark->gravity = -(GetRandomControl() & 3) - 4;
	spark->maxYvel = -(GetRandomControl() & 7) - 8;
	int size = (GetRandomControl() & 7) + 64 + (speed / 128);
	spark->dSize = size;
	spark->size = spark->sSize = size / 2;
}

int QuadBikeControl(ITEM_INFO* lara, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = lara->data;
	ITEM_INFO* quad = &g_Level.Items[laraInfo->Vehicle];
	auto quadInfo = (QUAD_INFO*)quad->data;

	short xRot, zRot, rotadd;
	int pitch, dead = 0;

	GAME_VECTOR	oldpos;
	oldpos.x = quad->pos.xPos;
	oldpos.y = quad->pos.yPos;
	oldpos.z = quad->pos.zPos;
	oldpos.roomNumber = quad->roomNumber;

	bool collide = QuadDynamics(lara, quad);

	short roomNumber = quad->roomNumber;
	FLOOR_INFO* floor = GetFloor(quad->pos.xPos, quad->pos.yPos, quad->pos.zPos, &roomNumber);
	auto height = GetFloorHeight(floor, quad->pos.xPos, quad->pos.yPos, quad->pos.zPos);
	auto ceiling = GetCeiling(floor, quad->pos.xPos, quad->pos.yPos, quad->pos.zPos);

	PHD_VECTOR frontLeft;
	PHD_VECTOR frontRight;
	auto floorHeightLeft = TestQuadHeight(quad, QUAD_FRONT, -QUAD_SIDE, &frontLeft);
	auto floorHeightRight = TestQuadHeight(quad, QUAD_FRONT, QUAD_SIDE, &frontRight);

	TestTriggers(quad, false);

	if (lara->hitPoints <= 0)
	{
		TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		dead = 1;
	}

	int drive = -1;

	if (quadInfo->flags)
		collide = false;
	else
	{
		switch (lara->currentAnimState)
		{
		case QUAD_STATE_MOUNT_LEFT:
		case QUAD_STATE_MOUNT_RIGHT:
		case QUAD_STATE_DISMOUNT_LEFT:
		case QUAD_STATE_DISMOUNT_RIGHT:
			drive = -1;
			collide = false;

			break;

		default:
			drive = QuadUserControl(quad, height, &pitch);

			break;
		}
	}

	if (quadInfo->velocity || quadInfo->revs)
	{
		int absvel = abs(quadInfo->velocity) + 1; // unused?
		quadInfo->pitch = pitch;
		if (quadInfo->pitch < -0x8000)
			quadInfo->pitch = -0x8000;
		else if (quadInfo->pitch > 0xA000)
			quadInfo->pitch = 0xA000;

		SoundEffect(SFX_TR3_QUAD_MOVE, &quad->pos, 0, 0.5f + (float)abs(quadInfo->pitch) / (float)MAX_VELOCITY);
	}
	else
	{
		if (drive != -1)
			SoundEffect(SFX_TR3_QUAD_IDLE, &quad->pos, 0);

		quadInfo->pitch = 0;
	}

	quad->floor = height;

	rotadd = quadInfo->velocity / 4;
	quadInfo->rearRot -= rotadd;
	quadInfo->rearRot -= (quadInfo->revs / 8);
	quadInfo->frontRot -= rotadd;

	quadInfo->leftFallspeed = DoQuadDynamics(floorHeightLeft, quadInfo->leftFallspeed, (int*)&frontLeft.y);
	quadInfo->rightFallspeed = DoQuadDynamics(floorHeightRight, quadInfo->rightFallspeed, (int*)&frontRight.y);
	quad->fallspeed = DoQuadDynamics(height, quad->fallspeed, (int*)&quad->pos.yPos);

	height = (frontLeft.y + frontRight.y) / 2;
	xRot = phd_atan(QUAD_FRONT, quad->pos.yPos - height);
	zRot = phd_atan(QUAD_SIDE, height - frontLeft.y);

	quad->pos.xRot += ((xRot - quad->pos.xRot) / 2);
	quad->pos.zRot += ((zRot - quad->pos.zRot) / 2);

	if (!(quadInfo->flags & QUAD_FLAG_DEAD))
	{
		if (roomNumber != quad->roomNumber)
		{
			ItemNewRoom(laraInfo->Vehicle, roomNumber);
			ItemNewRoom(laraInfo->itemNumber, roomNumber);
		}

		lara->pos.xPos = quad->pos.xPos;
		lara->pos.yPos = quad->pos.yPos;
		lara->pos.zPos = quad->pos.zPos;
		lara->pos.xRot = quad->pos.xRot;
		lara->pos.yRot = quad->pos.yRot;
		lara->pos.zRot = quad->pos.zRot;

		AnimateQuadBike(lara, quad, collide, dead);
		AnimateItem(lara);

		quad->animNumber = Objects[ID_QUAD].animIndex + (lara->animNumber - Objects[ID_QUAD_LARA_ANIMS].animIndex);
		quad->frameNumber = g_Level.Anims[quad->animNumber].frameBase + (lara->frameNumber - g_Level.Anims[lara->animNumber].frameBase);

		Camera.targetElevation = -ANGLE(30.0f);

		if (quadInfo->flags & QUAD_FLAG_IS_FALLING)
		{
			if (quad->pos.yPos == quad->floor)
			{
				ExplodingDeath(laraInfo->itemNumber, 0xffffffff, 1);
				lara->hitPoints = 0;
				lara->flags |= ONESHOT;
				QuadbikeExplode(lara, quad);

				return 0;
			}
		}
	}

	if (lara->currentAnimState != QUAD_STATE_MOUNT_RIGHT &&
		lara->currentAnimState != QUAD_STATE_MOUNT_LEFT &&
		lara->currentAnimState != QUAD_STATE_DISMOUNT_RIGHT &&
		lara->currentAnimState != QUAD_STATE_DISMOUNT_LEFT)
	{
		PHD_VECTOR pos;
		int speed = 0;
		short angle = 0;

		for (int i = 0; i < 2; i++)
		{
			pos.x = quadEffectsPositions[i].x;
			pos.y = quadEffectsPositions[i].y;
			pos.z = quadEffectsPositions[i].z;
			GetJointAbsPosition(quad, &pos, quadEffectsPositions[i].meshNum);
			angle = quad->pos.yRot + ((i == 0) ? 0x9000 : 0x7000);
			if (quad->speed > 32)
			{
				if (quad->speed < 64)
				{
					speed = 64 - quad->speed;
					TriggerQuadExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 1);
				}
			}
			else
			{
				if (QuadSmokeStart < 16)
				{
					speed = ((QuadSmokeStart * 2) + (GetRandomControl() & 7) + (GetRandomControl() & 16)) * 128;
					QuadSmokeStart++;
				}
				else if (QuadDriftStarting)
					speed = (abs(quadInfo->revs) * 2) + ((GetRandomControl() & 7) * 128);
				else if ((GetRandomControl() & 3) == 0)
					speed = ((GetRandomControl() & 15) + (GetRandomControl() & 16)) * 128;
				else
					speed = 0;

				TriggerQuadExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 0);
			}
		}
	}
	else
		QuadSmokeStart = 0;

	return QuadCheckGetOff(lara, quad);
}
