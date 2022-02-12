#include "framework.h"
#include "Objects/TR3/Vehicles/quad.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/misc.h"
#include "Objects/TR3/Vehicles/quad_info.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Specific/setup.h"
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
#define QUAD_IN_DRIFT		(IN_CROUCH | IN_SPRINT)
#define QUAD_IN_DISMOUNT	IN_ROLL
#define QUAD_IN_LEFT		IN_LEFT
#define QUAD_IN_RIGHT		IN_RIGHT

enum QuadState
{
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

enum QuadAnim
{
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

enum QuadFlags
{
	QUAD_FLAG_DEAD = 0x80,
	QUAD_FLAG_IS_FALLING = 0x40
};

enum QuadEffectPosition
{
	EXHAUST_LEFT = 0,
	EXHAUST_RIGHT,
	FRONT_LEFT_TYRE,
	FRONT_RIGHT_TYRE,
	BACK_LEFT_TYRE,
	BACK_RIGHT_TYRE
};

BITE_INFO quadEffectsPositions[6] =
{
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

void InitialiseQuadBike(short itemNum)
{
	ITEM_INFO* quad = &g_Level.Items[itemNum];
	quad->Data = QUAD_INFO();
	QUAD_INFO* quadInfo = quad->Data;

	quadInfo->velocity = 0;
	quadInfo->turnRate = 0;
	quadInfo->leftFallspeed = 0;
	quadInfo->rightFallspeed = 0;
	quadInfo->momentumAngle = quad->Position.yRot;
	quadInfo->extraRotation = 0;
	quadInfo->trackMesh = 0;
	quadInfo->pitch = 0;
	quadInfo->flags = 0;
}

static void QuadbikeExplode(ITEM_INFO* lara, ITEM_INFO* quad)
{
	LaraInfo*& laraInfo = LaraItem->Data;

	if (TestEnvironment(ENV_FLAG_WATER, quad))
		TriggerUnderwaterExplosion(quad, 1);
	else
	{
		TriggerExplosionSparks(quad->Position.xPos, quad->Position.yPos, quad->Position.zPos, 3, -2, 0, quad->RoomNumber);

		for (int i = 0; i < 3; i++)
			TriggerExplosionSparks(quad->Position.xPos, quad->Position.yPos, quad->Position.zPos, 3, -1, 0, quad->RoomNumber);
	}

	auto pos = PHD_3DPOS(quad->Position.xPos, quad->Position.yPos - (STEP_SIZE / 2), quad->Position.zPos, 0, quad->Position.yRot, 0);
	TriggerShockwave(&pos, 50, 180, 40, GenerateFloat(160, 200), 60, 60, 64, GenerateFloat(0, 359), 0);
	SoundEffect(SFX_TR4_EXPLOSION1, NULL, 0);
	SoundEffect(SFX_TR4_EXPLOSION2, NULL, 0);

	quad->Status = ITEM_DEACTIVATED;
	laraInfo->Vehicle = NO_ITEM;
}

static int CanQuadbikeGetOff(int direction)
{
	short angle;
	auto item = &g_Level.Items[Lara.Vehicle];

	if (direction < 0)
		angle = item->Position.yRot - ANGLE(90.0f);
	else
		angle = item->Position.yRot + ANGLE(90.0f);

	int x = item->Position.xPos + (STEP_SIZE * 2) * phd_sin(angle);
	int y = item->Position.yPos;
	int z = item->Position.zPos + (STEP_SIZE * 2) * phd_cos(angle);

	auto collResult = GetCollisionResult(x, y, z, item->RoomNumber);

	if (collResult.Position.FloorSlope ||
		collResult.Position.Floor == NO_HEIGHT)
	{
		return false;
	}

	if (abs(collResult.Position.Floor - item->Position.yPos) > (STEP_SIZE * 2))
		return false;

	if ((collResult.Position.Ceiling - item->Position.yPos) > -LARA_HEIGHT ||
		(collResult.Position.Floor - collResult.Position.Ceiling) < LARA_HEIGHT)
	{
		return false;
	}

	return true;
}

static bool QuadCheckGetOff(ITEM_INFO* lara, ITEM_INFO* quad)
{
	LaraInfo*& laraInfo = lara->Data;
	auto quadInfo = (QUAD_INFO*)quad->Data;

	if (laraInfo->Vehicle == NO_ITEM)
		return true;

	if ((lara->ActiveState == QUAD_STATE_DISMOUNT_RIGHT || lara->ActiveState == QUAD_STATE_DISMOUNT_LEFT) &&
		TestLastFrame(lara))
	{
		if (lara->ActiveState == QUAD_STATE_DISMOUNT_LEFT)
			lara->Position.yRot += ANGLE(90.0f);
		else
			lara->Position.yRot -= ANGLE(90.0f);

		SetAnimation(lara, LA_STAND_IDLE);
		lara->Position.xPos -= DISMOUNT_DISTANCE * phd_sin(lara->Position.yRot);
		lara->Position.zPos -= DISMOUNT_DISTANCE * phd_cos(lara->Position.yRot);
		lara->Position.xRot = 0;
		lara->Position.zRot = 0;
		laraInfo->Vehicle = NO_ITEM;
		laraInfo->Control.HandStatus = HandStatus::Free;

		if (lara->ActiveState == QUAD_STATE_FALL_OFF)
		{
			PHD_VECTOR pos = { 0, 0, 0 };

			SetAnimation(lara, LA_FREEFALL);
			GetJointAbsPosition(lara, &pos, LM_HIPS);

			lara->Position.xPos = pos.x;
			lara->Position.yPos = pos.y;
			lara->Position.zPos = pos.z;
			lara->VerticalVelocity = quad->VerticalVelocity;
			lara->Airborne = true;
			lara->Position.xRot = 0;
			lara->Position.zRot = 0;
			lara->HitPoints = 0;
			laraInfo->Control.HandStatus = HandStatus::Free;
			quad->Flags |= ONESHOT;

			return false;
		}
		else if (lara->ActiveState == QUAD_STATE_FALL_DEATH)
		{
			quadInfo->flags |= QUAD_FLAG_DEAD;
			lara->TargetState = LS_DEATH;
			lara->VerticalVelocity = DAMAGE_START + DAMAGE_LENGTH;
			lara->VerticalVelocity = 0;

			return false;
		}

		return true;
	}
	else
		return true;
}

static int GetOnQuadBike(ITEM_INFO* lara, ITEM_INFO* quad, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = lara->Data;

	if (!(TrInput & IN_ACTION) ||
		lara->Airborne ||
		laraInfo->Control.HandStatus != HandStatus::Free ||
		quad->Flags & ONESHOT ||
		abs(quad->Position.yPos - lara->Position.yPos) > STEP_SIZE)
	{
		return false;
	}

	auto dist = pow(lara->Position.xPos - quad->Position.xPos, 2) + pow(lara->Position.zPos - quad->Position.zPos, 2);
	if (dist > 170000)
		return false;

	auto probe = GetCollisionResult(quad);
	if (probe.Position.Floor < -32000)
		return false;
	else
	{
		short angle = phd_atan(quad->Position.zPos - lara->Position.zPos, quad->Position.xPos - lara->Position.xPos);
		angle -= quad->Position.yRot;

		if ((angle > -ANGLE(45.0f)) && (angle < ANGLE(135.0f)))
		{
			short tempAngle = lara->Position.yRot - quad->Position.yRot;
			if (tempAngle > ANGLE(45.0f) && tempAngle < ANGLE(135.0f))
				return true;
			else
				return false;
		}
		else
		{
			short tempAngle = lara->Position.yRot - quad->Position.yRot;
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
	roomsList.push_back(quad->RoomNumber);

	ROOM_INFO* room = &g_Level.Rooms[quad->RoomNumber];
	for (int i = 0; i < room->doors.size(); i++)
		roomsList.push_back(room->doors[i].room);

	for (int i = 0; i < roomsList.size(); i++)
	{
		auto itemNum = g_Level.Rooms[roomsList[i]].itemNumber;

		while (itemNum != NO_ITEM)
		{
			ITEM_INFO* item = &g_Level.Items[itemNum];
			if (item->Collidable &&
				item->Status != ITEM_INVISIBLE &&
				item != lara &&
				item != quad)
			{
				OBJECT_INFO* object = &Objects[item->ObjectNumber];
				if (object->collision && object->intelligent)
				{
					auto x = quad->Position.xPos - item->Position.xPos;
					auto y = quad->Position.yPos - item->Position.yPos;
					auto z = quad->Position.zPos - item->Position.zPos;

					if (x > -4096 && x < 4096 && z > -4096 && z < 4096 && y > -4096 && y < 4096)
					{
						if (TestBoundsCollide(item, quad, QUAD_RADIUS))
						{
							DoLotsOfBlood(item->Position.xPos, quad->Position.yPos - STEP_SIZE, item->Position.zPos, quad->VerticalVelocity, quad->Position.yRot, item->RoomNumber, 3);
							item->HitPoints = 0;
						}
					}
				}
			}

			itemNum = item->NextItem;
		}
	}
}

static int GetQuadCollisionAnim(ITEM_INFO* quad, PHD_VECTOR* p)
{
	p->x = quad->Position.xPos - p->x;
	p->z = quad->Position.zPos - p->z;

	if (p->x || p->z)
	{
		float c = phd_cos(quad->Position.yRot);
		float s = phd_sin(quad->Position.yRot);
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
	pos->y = quad->Position.yPos - dz * phd_sin(quad->Position.xRot) + dx * phd_sin(quad->Position.zRot);

	float c = phd_cos(quad->Position.yRot);
	float s = phd_sin(quad->Position.yRot);

	pos->z = quad->Position.zPos + dz * c - dx * s;
	pos->x = quad->Position.xPos + dz * s + dx * c;

	auto probe = GetCollisionResult(pos->x, pos->y, pos->z, quad->RoomNumber);
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
			quad->Position.zPos += (old->z - pos->z);
			quad->Position.xPos += (old->x - pos->x);
		}
		else if (z > oldZ)
		{
			quad->Position.zPos -= shiftZ + 1;

			return (pos->x - quad->Position.xPos);
		}
		else
		{
			quad->Position.zPos += WALL_SIZE - shiftZ;

			return (quad->Position.xPos - pos->x);
		}
	}
	else if (z == oldZ)
	{
		if (x > oldX)
		{
			quad->Position.xPos -= shiftX + 1;

			return (quad->Position.zPos - pos->z);
		}
		else
		{
			quad->Position.xPos += WALL_SIZE - shiftX;

			return (pos->z - quad->Position.zPos);
		}
	}
	else
	{
		x = 0;
		z = 0;

		probe = GetCollisionResult(old->x, pos->y, pos->z, quad->RoomNumber);
		if (probe.Position.Floor < (old->y - STEP_SIZE))
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = WALL_SIZE - shiftZ;
		}

		probe = GetCollisionResult(pos->x, pos->y, old->z, quad->RoomNumber);
		if (probe.Position.Floor < (old->y - STEP_SIZE))
		{
			if (pos->x > old->x)
				x = -shiftX - 1;
			else
				x = WALL_SIZE - shiftX;
		}

		if (x && z)
		{
			quad->Position.zPos += z;
			quad->Position.xPos += x;
		}
		else if (z)
		{
			quad->Position.zPos += z;

			if (z > 0)
				return (quad->Position.xPos - pos->x);
			else
				return (pos->x - quad->Position.xPos);
		}
		else if (x)
		{
			quad->Position.xPos += x;

			if (x > 0)
				return (pos->z - quad->Position.zPos);
			else
				return (quad->Position.zPos - pos->z);
		}
		else
		{
			quad->Position.zPos += (old->z - pos->z);
			quad->Position.xPos += (old->x - pos->x);
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
	LaraInfo*& laraInfo = lara->Data;
	auto quadInfo = (QUAD_INFO*)quad->Data;

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

	old.x = quad->Position.xPos;
	old.y = quad->Position.yPos;
	old.z = quad->Position.zPos;

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

	if (quad->Position.yPos > (quad->Floor - STEP_SIZE))
	{
		short momentum;

		if (quadInfo->turnRate < -QUAD_UNDO_TURN)
			quadInfo->turnRate += QUAD_UNDO_TURN;

		else if (quadInfo->turnRate > QUAD_UNDO_TURN)
			quadInfo->turnRate -= QUAD_UNDO_TURN;

		else
			quadInfo->turnRate = 0;

		quad->Position.yRot += quadInfo->turnRate + quadInfo->extraRotation;

		rot = quad->Position.yRot - quadInfo->momentumAngle;

		momentum = MIN_MOMENTUM_TURN - (((((MIN_MOMENTUM_TURN - MAX_MOMENTUM_TURN) * 256) / MAX_VELOCITY) * quadInfo->velocity) / 256);
		if (!(TrInput & QUAD_IN_ACCELERATE) && quadInfo->velocity > 0)
			momentum += (momentum / 4);

		if (rot < -MAX_MOMENTUM_TURN)
		{
			if (rot < -QUAD_MAX_MOM_TURN)
			{
				rot = -QUAD_MAX_MOM_TURN;
				quadInfo->momentumAngle = quad->Position.yRot - rot;
			}
			else
				quadInfo->momentumAngle -= momentum;
		}
		else if (rot > MAX_MOMENTUM_TURN)
		{
			if (rot > QUAD_MAX_MOM_TURN)
			{
				rot = QUAD_MAX_MOM_TURN;
				quadInfo->momentumAngle = quad->Position.yRot - rot;
			}
			else
				quadInfo->momentumAngle += momentum;
		}
		else
			quadInfo->momentumAngle = quad->Position.yRot;
	}

	else
		quad->Position.yRot += quadInfo->turnRate + quadInfo->extraRotation;

	probe = GetCollisionResult(quad);
	int speed = 0;
	if (quad->Position.yPos >= probe.Position.Floor)
		speed = quad->VerticalVelocity * phd_cos(quad->Position.xRot);
	else
		speed = quad->VerticalVelocity;

	quad->Position.zPos += speed * phd_cos(quadInfo->momentumAngle);
	quad->Position.xPos += speed * phd_sin(quadInfo->momentumAngle);

	slip = QUAD_SLIP * phd_sin(quad->Position.xRot);
	if (abs(slip) > QUAD_SLIP / 2)
	{
		if (slip > 0)
			slip -= 10;
		else
			slip += 10;
		quad->Position.zPos -= slip * phd_cos(quad->Position.yRot);
		quad->Position.xPos -= slip * phd_sin(quad->Position.yRot);
	}

	slip = QUAD_SLIP_SIDE * phd_sin(quad->Position.zRot);
	if (abs(slip) > QUAD_SLIP_SIDE / 2)
	{
		quad->Position.zPos -= slip * phd_sin(quad->Position.yRot);
		quad->Position.xPos += slip * phd_cos(quad->Position.yRot);
	}

	moved.x = quad->Position.xPos;
	moved.z = quad->Position.zPos;

	if (!(quad->Flags & ONESHOT))
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
	if (probe.Position.Floor < quad->Position.yPos - STEP_SIZE)
		DoQuadShift(quad, (PHD_VECTOR*)&quad->Position, &old);

	quadInfo->extraRotation = rot;

	collide = GetQuadCollisionAnim(quad, &moved);

	if (collide)
	{
		newspeed = (quad->Position.zPos - old.z) * phd_cos(quadInfo->momentumAngle) + (quad->Position.xPos - old.x) * phd_sin(quadInfo->momentumAngle);

		newspeed *= 256;

		if (&g_Level.Items[laraInfo->Vehicle] == quad &&
			quadInfo->velocity == MAX_VELOCITY &&
			newspeed < (quadInfo->velocity - 10))
		{
			lara->HitPoints -= (quadInfo->velocity - newspeed) / 128;
			lara->HitStatus = 1;
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
	auto quadInfo = (QUAD_INFO*)quad->Data;

	if (quad->Position.yPos != quad->Floor &&
		lara->ActiveState != QUAD_STATE_FALL &&
		lara->ActiveState != QUAD_STATE_LAND &&
		lara->ActiveState != QUAD_STATE_FALL_OFF &&
		!dead)
	{
		if (quadInfo->velocity < 0)
			lara->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_LEAP_START;
		else
			lara->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_LEAP_START2;

		lara->FrameNumber = GetFrameNumber(lara, lara->AnimNumber);
		lara->ActiveState = QUAD_STATE_FALL;
		lara->TargetState = QUAD_STATE_FALL;
	}
	else if (collide &&
		lara->ActiveState != QUAD_STATE_HIT_FRONT &&
		lara->ActiveState != QUAD_STATE_HIT_BACK &&
		lara->ActiveState != QUAD_STATE_HIT_LEFT &&
		lara->ActiveState != QUAD_STATE_HIT_RIGHT &&
		lara->ActiveState != QUAD_STATE_FALL_OFF &&
		quadInfo->velocity > (MAX_VELOCITY / 3) &&
		!dead)
	{
		if (collide == QUAD_HIT_FRONT)
		{
			lara->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_BACK;
			lara->ActiveState = QUAD_STATE_HIT_FRONT;
			lara->TargetState = QUAD_STATE_HIT_FRONT;
		}
		else if (collide == QUAD_HIT_BACK)
		{
			lara->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_FRONT;
			lara->ActiveState = QUAD_STATE_HIT_BACK;
			lara->TargetState = QUAD_STATE_HIT_BACK;
		}
		else if (collide == QUAD_HIT_LEFT)
		{
			lara->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_RIGHT;
			lara->ActiveState = QUAD_STATE_HIT_LEFT;
			lara->TargetState = QUAD_STATE_HIT_LEFT;
		}
		else
		{
			lara->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_LEFT;
			lara->ActiveState = QUAD_STATE_HIT_RIGHT;
			lara->TargetState = QUAD_STATE_HIT_RIGHT;
		}

		lara->FrameNumber = GetFrameNumber(lara, lara->AnimNumber);
		SoundEffect(SFX_TR3_QUAD_FRONT_IMPACT, &quad->Position, 0);
	}
	else
	{
		switch (lara->ActiveState)
		{
		case QUAD_STATE_IDLE:
			if (dead)
				lara->TargetState = QUAD_STATE_BIKE_DEATH;
			else if (TrInput & QUAD_IN_DISMOUNT &&
				quadInfo->velocity == 0 &&
				!QuadNoGetOff)
			{
				if (TrInput & QUAD_IN_LEFT && CanQuadbikeGetOff(-1))
					lara->TargetState = QUAD_STATE_DISMOUNT_LEFT;
				else if (TrInput & QUAD_IN_RIGHT && CanQuadbikeGetOff(1))
					lara->TargetState = QUAD_STATE_DISMOUNT_RIGHT;
			}
			else if (TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE))
				lara->TargetState = QUAD_STATE_DRIVE;

			break;

		case QUAD_STATE_DRIVE:
			if (dead)
			{
				if (quadInfo->velocity > (MAX_VELOCITY / 2))
					lara->TargetState = QUAD_STATE_FALL_DEATH;
				else
					lara->TargetState = QUAD_STATE_BIKE_DEATH;
			}
			else if (!(TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE)) &&
				(quadInfo->velocity / 256) == 0)
			{
				lara->TargetState = QUAD_STATE_IDLE;
			}
			else if (TrInput & QUAD_IN_LEFT &&
				!QuadDriftStarting)
			{
				lara->TargetState = QUAD_STATE_TURN_LEFT;
			}
			else if (TrInput & QUAD_IN_RIGHT &&
				!QuadDriftStarting)
			{
				lara->TargetState = QUAD_STATE_TURN_RIGHT;
			}
			else if (TrInput & QUAD_IN_BRAKE)
			{
				if (quadInfo->velocity > (MAX_VELOCITY / 3 * 2))
					lara->TargetState = QUAD_STATE_BRAKE;
				else
					lara->TargetState = QUAD_STATE_SLOW;
			}

			break;

		case QUAD_STATE_BRAKE:
		case QUAD_STATE_SLOW:
		case QUAD_STATE_STOP_SLOWLY:
			if ((quadInfo->velocity / 256) == 0)
				lara->TargetState = QUAD_STATE_IDLE;
			else if (TrInput & QUAD_IN_LEFT)
				lara->TargetState = QUAD_STATE_TURN_LEFT;
			else if (TrInput & QUAD_IN_RIGHT)
				lara->TargetState = QUAD_STATE_TURN_RIGHT;

			break;

		case QUAD_STATE_TURN_LEFT:
			if ((quadInfo->velocity / 256) == 0)
				lara->TargetState = QUAD_STATE_IDLE;
			else if (TrInput & QUAD_IN_RIGHT)
			{
				lara->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_TURN_RIGHT_START;
				lara->FrameNumber = GetFrameNumber(lara, lara->AnimNumber);
				lara->ActiveState = QUAD_STATE_TURN_RIGHT;
				lara->TargetState = QUAD_STATE_TURN_RIGHT;
			}
			else if (!(TrInput & QUAD_IN_LEFT))
				lara->TargetState = QUAD_STATE_DRIVE;

			break;

		case QUAD_STATE_TURN_RIGHT:
			if ((quadInfo->velocity / 256) == 0)
				lara->TargetState = QUAD_STATE_IDLE;
			else if (TrInput & QUAD_IN_LEFT)
			{
				lara->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_TURN_LEFT_START;
				lara->FrameNumber = GetFrameNumber(lara, lara->AnimNumber);
				lara->ActiveState = QUAD_STATE_TURN_LEFT;
				lara->TargetState = QUAD_STATE_TURN_LEFT;
			}
			else if (!(TrInput & QUAD_IN_RIGHT))
				lara->TargetState = QUAD_STATE_DRIVE;

			break;

		case QUAD_STATE_FALL:
			if (quad->Position.yPos == quad->Floor)
				lara->TargetState = QUAD_STATE_LAND;
			else if (quad->VerticalVelocity > TERMINAL_FALLSPEED)
				quadInfo->flags |= QUAD_FLAG_IS_FALLING;

			break;

		case QUAD_STATE_FALL_OFF:
			break;

		case QUAD_STATE_HIT_FRONT:
		case QUAD_STATE_HIT_BACK:
		case QUAD_STATE_HIT_LEFT:
		case QUAD_STATE_HIT_RIGHT:
			if (TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE))
				lara->TargetState = QUAD_STATE_DRIVE;

			break;
		}

		if (TestEnvironment(ENV_FLAG_WATER, quad) ||
			TestEnvironment(ENV_FLAG_SWAMP, quad))
		{
			lara->TargetState = QUAD_STATE_FALL_OFF;
			lara->Position.yPos = quad->Position.yPos + 700;
			lara->RoomNumber = quad->RoomNumber;
			lara->HitPoints = 0;
			QuadbikeExplode(lara, quad);
		}
	}
}

static int QuadUserControl(ITEM_INFO* quad, int height, int* pitch)
{
	auto quadInfo = (QUAD_INFO*)quad->Data;
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

	if (quad->Position.yPos >= (height - STEP_SIZE))
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

			quadInfo->velocity -= abs(quad->Position.yRot - quadInfo->momentumAngle) / 64;
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

		quad->VerticalVelocity = quadInfo->velocity / 256;

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
	LaraInfo*& laraInfo = lara->Data;
	ITEM_INFO* quad = &g_Level.Items[itemNumber];
	auto quadInfo = (QUAD_INFO*)quad->Data;

	if (lara->HitPoints < 0 || laraInfo->Vehicle != NO_ITEM)
		return;

	if (GetOnQuadBike(lara, &g_Level.Items[itemNumber], coll))
	{
		short ang;

		laraInfo->Vehicle = itemNumber;

		if (laraInfo->Control.WeaponControl.GunType == WEAPON_FLARE)
		{
			CreateFlare(lara, ID_FLARE_ITEM, 0);
			UndrawFlareMeshes(lara);
			laraInfo->Flare.ControlLeft = 0;
			laraInfo->Control.WeaponControl.RequestGunType = laraInfo->Control.WeaponControl.GunType = WEAPON_NONE;
		}

		laraInfo->Control.HandStatus = HandStatus::Busy;

		ang = phd_atan(quad->Position.zPos - lara->Position.zPos, quad->Position.xPos - lara->Position.xPos);
		ang -= quad->Position.yRot;

		if (ang > -ANGLE(45.0f) && ang < ANGLE(135.0f))
		{
			lara->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_MOUNT_LEFT;
			lara->ActiveState = lara->TargetState = QUAD_STATE_MOUNT_LEFT;
		}
		else
		{
			lara->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_MOUNT_RIGHT;
			lara->ActiveState = lara->TargetState = QUAD_STATE_MOUNT_RIGHT;
		}

		lara->FrameNumber = g_Level.Anims[lara->AnimNumber].frameBase;

		quad->HitPoints = 1;
		lara->Position.xPos = quad->Position.xPos;
		lara->Position.yPos = quad->Position.yPos;
		lara->Position.zPos = quad->Position.zPos;
		lara->Position.yRot = quad->Position.yRot;
		ResetLaraFlex(lara);
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
	LaraInfo*& laraInfo = lara->Data;
	ITEM_INFO* quad = &g_Level.Items[laraInfo->Vehicle];
	auto quadInfo = (QUAD_INFO*)quad->Data;

	short xRot, zRot, rotadd;
	int pitch, dead = 0;

	GAME_VECTOR	oldpos;
	oldpos.x = quad->Position.xPos;
	oldpos.y = quad->Position.yPos;
	oldpos.z = quad->Position.zPos;
	oldpos.roomNumber = quad->RoomNumber;

	bool collide = QuadDynamics(lara, quad);

	auto probe = GetCollisionResult(quad);

	PHD_VECTOR frontLeft;
	PHD_VECTOR frontRight;
	auto floorHeightLeft = TestQuadHeight(quad, QUAD_FRONT, -QUAD_SIDE, &frontLeft);
	auto floorHeightRight = TestQuadHeight(quad, QUAD_FRONT, QUAD_SIDE, &frontRight);

	TestTriggers(quad, false);

	if (lara->HitPoints <= 0)
	{
		TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		dead = 1;
	}

	int drive = -1;

	if (quadInfo->flags)
		collide = false;
	else
	{
		switch (lara->ActiveState)
		{
		case QUAD_STATE_MOUNT_LEFT:
		case QUAD_STATE_MOUNT_RIGHT:
		case QUAD_STATE_DISMOUNT_LEFT:
		case QUAD_STATE_DISMOUNT_RIGHT:
			drive = -1;
			collide = false;

			break;

		default:
			drive = QuadUserControl(quad, probe.Position.Floor, &pitch);

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

		SoundEffect(SFX_TR3_QUAD_MOVE, &quad->Position, 0, 0.5f + (float)abs(quadInfo->pitch) / (float)MAX_VELOCITY);
	}
	else
	{
		if (drive != -1)
			SoundEffect(SFX_TR3_QUAD_IDLE, &quad->Position, 0);

		quadInfo->pitch = 0;
	}

	quad->Floor = probe.Position.Floor;

	rotadd = quadInfo->velocity / 4;
	quadInfo->rearRot -= rotadd;
	quadInfo->rearRot -= (quadInfo->revs / 8);
	quadInfo->frontRot -= rotadd;

	quadInfo->leftFallspeed = DoQuadDynamics(floorHeightLeft, quadInfo->leftFallspeed, (int*)&frontLeft.y);
	quadInfo->rightFallspeed = DoQuadDynamics(floorHeightRight, quadInfo->rightFallspeed, (int*)&frontRight.y);
	quad->VerticalVelocity = DoQuadDynamics(probe.Position.Floor, quad->VerticalVelocity, (int*)&quad->Position.yPos);

	probe.Position.Floor = (frontLeft.y + frontRight.y) / 2;
	xRot = phd_atan(QUAD_FRONT, quad->Position.yPos - probe.Position.Floor);
	zRot = phd_atan(QUAD_SIDE, probe.Position.Floor - frontLeft.y);

	quad->Position.xRot += ((xRot - quad->Position.xRot) / 2);
	quad->Position.zRot += ((zRot - quad->Position.zRot) / 2);

	if (!(quadInfo->flags & QUAD_FLAG_DEAD))
	{
		if (probe.RoomNumber != quad->RoomNumber)
		{
			ItemNewRoom(laraInfo->Vehicle, probe.RoomNumber);
			ItemNewRoom(laraInfo->ItemNumber, probe.RoomNumber);
		}

		lara->Position = quad->Position;

		AnimateQuadBike(lara, quad, collide, dead);
		AnimateItem(lara);

		quad->AnimNumber = Objects[ID_QUAD].animIndex + (lara->AnimNumber - Objects[ID_QUAD_LARA_ANIMS].animIndex);
		quad->FrameNumber = g_Level.Anims[quad->AnimNumber].frameBase + (lara->FrameNumber - g_Level.Anims[lara->AnimNumber].frameBase);

		Camera.targetElevation = -ANGLE(30.0f);

		if (quadInfo->flags & QUAD_FLAG_IS_FALLING)
		{
			if (quad->Position.yPos == quad->Floor)
			{
				ExplodingDeath(laraInfo->ItemNumber, 0xffffffff, 1);
				lara->HitPoints = 0;
				lara->Flags |= ONESHOT;
				QuadbikeExplode(lara, quad);

				return 0;
			}
		}
	}

	if (lara->ActiveState != QUAD_STATE_MOUNT_RIGHT &&
		lara->ActiveState != QUAD_STATE_MOUNT_LEFT &&
		lara->ActiveState != QUAD_STATE_DISMOUNT_RIGHT &&
		lara->ActiveState != QUAD_STATE_DISMOUNT_LEFT)
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
			angle = quad->Position.yRot + ((i == 0) ? 0x9000 : 0x7000);
			if (quad->VerticalVelocity > 32)
			{
				if (quad->VerticalVelocity < 64)
				{
					speed = 64 - quad->VerticalVelocity;
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
