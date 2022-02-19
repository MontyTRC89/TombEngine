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

#define QUAD_HIT_LEFT  11
#define QUAD_HIT_RIGHT 12
#define QUAD_HIT_FRONT 13
#define QUAD_HIT_BACK  14

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
	QUAD_STATE_WHEELIE = 21,	// Unused.
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

enum QuadEffectPosition
{
	EXHAUST_LEFT = 0,
	EXHAUST_RIGHT = 1,
	FRONT_LEFT_TYRE = 2,
	FRONT_RIGHT_TYRE = 3,
	BACK_LEFT_TYRE = 4,
	BACK_RIGHT_TYRE = 5
};

enum QuadFlags
{
	QUAD_FLAG_DEAD = 0x80,
	QUAD_FLAG_IS_FALLING = 0x40
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

void InitialiseQuadBike(short itemNumber)
{
	auto* quadItem = &g_Level.Items[itemNumber];
	quadItem->Data = QuadInfo();
	auto* quadInfo = (QuadInfo*)quadItem->Data;

	quadInfo->TurnRate = 0;
	quadInfo->MomentumAngle = quadItem->Position.yRot;
	quadInfo->ExtraRotation = 0;
	quadInfo->Velocity = 0;
	quadInfo->LeftVerticalVelocity = 0;
	quadInfo->RightVerticalVelocity = 0;
	quadInfo->Pitch = 0;
	quadInfo->Flags = 0;
}

static void QuadbikeExplode(ITEM_INFO* laraItem, ITEM_INFO* quadItem)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	if (TestEnvironment(ENV_FLAG_WATER, quadItem))
		TriggerUnderwaterExplosion(quadItem, 1);
	else
	{
		TriggerExplosionSparks(quadItem->Position.xPos, quadItem->Position.yPos, quadItem->Position.zPos, 3, -2, 0, quadItem->RoomNumber);

		for (int i = 0; i < 3; i++)
			TriggerExplosionSparks(quadItem->Position.xPos, quadItem->Position.yPos, quadItem->Position.zPos, 3, -1, 0, quadItem->RoomNumber);
	}

	auto pos = PHD_3DPOS(quadItem->Position.xPos, quadItem->Position.yPos - CLICK(0.5f), quadItem->Position.zPos, 0, quadItem->Position.yRot, 0);
	TriggerShockwave(&pos, 50, 180, 40, GenerateFloat(160, 200), 60, 60, 64, GenerateFloat(0, 359), 0);

	SoundEffect(SFX_TR4_EXPLOSION1, NULL, 0);
	SoundEffect(SFX_TR4_EXPLOSION2, NULL, 0);

	quadItem->Status = ITEM_DEACTIVATED;
	laraInfo->Vehicle = NO_ITEM;
}

static int CanQuadbikeGetOff(int direction)
{
	auto* item = &g_Level.Items[Lara.Vehicle];
	short angle;

	if (direction < 0)
		angle = item->Position.yRot - ANGLE(90.0f);
	else
		angle = item->Position.yRot + ANGLE(90.0f);

	int x = item->Position.xPos + CLICK(2) * phd_sin(angle);
	int y = item->Position.yPos;
	int z = item->Position.zPos + CLICK(2) * phd_cos(angle);

	auto collResult = GetCollisionResult(x, y, z, item->RoomNumber);

	if (collResult.Position.FloorSlope ||
		collResult.Position.Floor == NO_HEIGHT)
	{
		return false;
	}

	if (abs(collResult.Position.Floor - item->Position.yPos) > CLICK(2))
		return false;

	if ((collResult.Position.Ceiling - item->Position.yPos) > -LARA_HEIGHT ||
		(collResult.Position.Floor - collResult.Position.Ceiling) < LARA_HEIGHT)
	{
		return false;
	}

	return true;
}

static bool QuadCheckGetOff(ITEM_INFO* laraItem, ITEM_INFO* quadItem)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* quadInfo = (QuadInfo*)quadItem->Data;

	if (laraInfo->Vehicle == NO_ITEM)
		return true;

	if ((laraItem->ActiveState == QUAD_STATE_DISMOUNT_RIGHT || laraItem->ActiveState == QUAD_STATE_DISMOUNT_LEFT) &&
		TestLastFrame(laraItem))
	{
		if (laraItem->ActiveState == QUAD_STATE_DISMOUNT_LEFT)
			laraItem->Position.yRot += ANGLE(90.0f);
		else
			laraItem->Position.yRot -= ANGLE(90.0f);

		SetAnimation(laraItem, LA_STAND_IDLE);
		laraItem->Position.xPos -= DISMOUNT_DISTANCE * phd_sin(laraItem->Position.yRot);
		laraItem->Position.zPos -= DISMOUNT_DISTANCE * phd_cos(laraItem->Position.yRot);
		laraItem->Position.xRot = 0;
		laraItem->Position.zRot = 0;
		laraInfo->Vehicle = NO_ITEM;
		laraInfo->Control.HandStatus = HandStatus::Free;

		if (laraItem->ActiveState == QUAD_STATE_FALL_OFF)
		{
			PHD_VECTOR pos = { 0, 0, 0 };

			SetAnimation(laraItem, LA_FREEFALL);
			GetJointAbsPosition(laraItem, &pos, LM_HIPS);

			laraItem->Position.xPos = pos.x;
			laraItem->Position.yPos = pos.y;
			laraItem->Position.zPos = pos.z;
			laraItem->VerticalVelocity = quadItem->VerticalVelocity;
			laraItem->Airborne = true;
			laraItem->Position.xRot = 0;
			laraItem->Position.zRot = 0;
			laraItem->HitPoints = 0;
			laraInfo->Control.HandStatus = HandStatus::Free;
			quadItem->Flags |= ONESHOT;

			return false;
		}
		else if (laraItem->ActiveState == QUAD_STATE_FALL_DEATH)
		{
			laraItem->TargetState = LS_DEATH;
			laraItem->VerticalVelocity = DAMAGE_START + DAMAGE_LENGTH;
			laraItem->VerticalVelocity = 0;
			quadInfo->Flags |= QUAD_FLAG_DEAD;

			return false;
		}

		return true;
	}
	else
		return true;
}

static int GetOnQuadBike(ITEM_INFO* laraItem, ITEM_INFO* quadItem, COLL_INFO* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	if (!(TrInput & IN_ACTION) ||
		laraItem->Airborne ||
		laraInfo->Control.HandStatus != HandStatus::Free ||
		quadItem->Flags & ONESHOT ||
		abs(quadItem->Position.yPos - laraItem->Position.yPos) > CLICK(1))
	{
		return false;
	}

	auto dist = pow(laraItem->Position.xPos - quadItem->Position.xPos, 2) + pow(laraItem->Position.zPos - quadItem->Position.zPos, 2);
	if (dist > 170000)
		return false;

	auto probe = GetCollisionResult(quadItem);
	if (probe.Position.Floor < -32000)
		return false;
	else
	{
		short angle = phd_atan(quadItem->Position.zPos - laraItem->Position.zPos, quadItem->Position.xPos - laraItem->Position.xPos);
		angle -= quadItem->Position.yRot;

		if ((angle > -ANGLE(45.0f)) && (angle < ANGLE(135.0f)))
		{
			short tempAngle = laraItem->Position.yRot - quadItem->Position.yRot;
			if (tempAngle > ANGLE(45.0f) && tempAngle < ANGLE(135.0f))
				return true;
			else
				return false;
		}
		else
		{
			short tempAngle = laraItem->Position.yRot - quadItem->Position.yRot;
			if (tempAngle > ANGLE(225.0f) && tempAngle < ANGLE(315.0f))
				return true;
			else
				return false;
		}
	}

	return true;
}

static void QuadBaddieCollision(ITEM_INFO* laraItem, ITEM_INFO* quadItem)
{
	vector<short> roomsList;
	roomsList.push_back(quadItem->RoomNumber);

	auto* roomInfo = &g_Level.Rooms[quadItem->RoomNumber];
	for (int i = 0; i < roomInfo->doors.size(); i++)
		roomsList.push_back(roomInfo->doors[i].room);

	for (int i = 0; i < roomsList.size(); i++)
	{
		auto itemNum = g_Level.Rooms[roomsList[i]].itemNumber;

		while (itemNum != NO_ITEM)
		{
			auto* item = &g_Level.Items[itemNum];
			if (item->Collidable &&
				item->Status != ITEM_INVISIBLE &&
				item != laraItem && item != quadItem)
			{
				auto* object = &Objects[item->ObjectNumber];
				if (object->collision && object->intelligent)
				{
					int x = quadItem->Position.xPos - item->Position.xPos;
					int y = quadItem->Position.yPos - item->Position.yPos;
					int z = quadItem->Position.zPos - item->Position.zPos;

					if (x > -4096 && x < 4096 &&
						z > -4096 && z < 4096 &&
						y > -4096 && y < 4096)
					{
						if (TestBoundsCollide(item, quadItem, QUAD_RADIUS))
						{
							DoLotsOfBlood(item->Position.xPos, quadItem->Position.yPos - STEP_SIZE, item->Position.zPos, quadItem->VerticalVelocity, quadItem->Position.yRot, item->RoomNumber, 3);
							item->HitPoints = 0;
						}
					}
				}
			}

			itemNum = item->NextItem;
		}
	}
}

static int GetQuadCollisionAnim(ITEM_INFO* quadItem, PHD_VECTOR* p)
{
	p->x = quadItem->Position.xPos - p->x;
	p->z = quadItem->Position.zPos - p->z;

	if (p->x || p->z)
	{
		float c = phd_cos(quadItem->Position.yRot);
		float s = phd_sin(quadItem->Position.yRot);
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

static int TestQuadHeight(ITEM_INFO* quadItem, int dz, int dx, PHD_VECTOR* pos)
{
	pos->y = quadItem->Position.yPos - dz * phd_sin(quadItem->Position.xRot) + dx * phd_sin(quadItem->Position.zRot);

	float c = phd_cos(quadItem->Position.yRot);
	float s = phd_sin(quadItem->Position.yRot);

	pos->z = quadItem->Position.zPos + dz * c - dx * s;
	pos->x = quadItem->Position.xPos + dz * s + dx * c;

	auto probe = GetCollisionResult(pos->x, pos->y, pos->z, quadItem->RoomNumber);
	if (probe.Position.Ceiling > pos->y ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		return NO_HEIGHT;
	}

	return probe.Position.Floor;
}

static int DoQuadShift(ITEM_INFO* quadItem, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	COLL_RESULT probe;
	int x = pos->x / SECTOR(1);
	int z = pos->z / SECTOR(1);
	int oldX = old->x / SECTOR(1);
	int oldZ = old->z / SECTOR(1);
	int shiftX = pos->x & (WALL_SIZE - 1);
	int shiftZ = pos->z & (WALL_SIZE - 1);

	if (x == oldX)
	{
		if (z == oldZ)
		{
			quadItem->Position.zPos += (old->z - pos->z);
			quadItem->Position.xPos += (old->x - pos->x);
		}
		else if (z > oldZ)
		{
			quadItem->Position.zPos -= shiftZ + 1;
			return (pos->x - quadItem->Position.xPos);
		}
		else
		{
			quadItem->Position.zPos += WALL_SIZE - shiftZ;
			return (quadItem->Position.xPos - pos->x);
		}
	}
	else if (z == oldZ)
	{
		if (x > oldX)
		{
			quadItem->Position.xPos -= shiftX + 1;
			return (quadItem->Position.zPos - pos->z);
		}
		else
		{
			quadItem->Position.xPos += WALL_SIZE - shiftX;
			return (pos->z - quadItem->Position.zPos);
		}
	}
	else
	{
		x = 0;
		z = 0;

		probe = GetCollisionResult(old->x, pos->y, pos->z, quadItem->RoomNumber);
		if (probe.Position.Floor < (old->y - STEP_SIZE))
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = WALL_SIZE - shiftZ;
		}

		probe = GetCollisionResult(pos->x, pos->y, old->z, quadItem->RoomNumber);
		if (probe.Position.Floor < (old->y - STEP_SIZE))
		{
			if (pos->x > old->x)
				x = -shiftX - 1;
			else
				x = WALL_SIZE - shiftX;
		}

		if (x && z)
		{
			quadItem->Position.zPos += z;
			quadItem->Position.xPos += x;
		}
		else if (z)
		{
			quadItem->Position.zPos += z;

			if (z > 0)
				return (quadItem->Position.xPos - pos->x);
			else
				return (pos->x - quadItem->Position.xPos);
		}
		else if (x)
		{
			quadItem->Position.xPos += x;

			if (x > 0)
				return (pos->z - quadItem->Position.zPos);
			else
				return (quadItem->Position.zPos - pos->z);
		}
		else
		{
			quadItem->Position.zPos += (old->z - pos->z);
			quadItem->Position.xPos += (old->x - pos->x);
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

static int QuadDynamics(ITEM_INFO* laraItem, ITEM_INFO* quadItem)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* quadInfo = (QuadInfo*)quadItem->Data;

	quadInfo->NoDismount = false;

	PHD_VECTOR oldFrontLeft, oldFrontRight, oldBottomLeft, oldBottomRight;
	int holdFrontLeft = TestQuadHeight(quadItem, QUAD_FRONT, -QUAD_SIDE, &oldFrontLeft);
	int holdFrontRight = TestQuadHeight(quadItem, QUAD_FRONT, QUAD_SIDE, &oldFrontRight);
	int holdBottomLeft = TestQuadHeight(quadItem, -QUAD_FRONT, -QUAD_SIDE, &oldBottomLeft);
	int holdBottomRight = TestQuadHeight(quadItem, -QUAD_FRONT, QUAD_SIDE, &oldBottomRight);

	PHD_VECTOR mtlOld, mtrOld, mmlOld, mmrOld;
	int hmml_old = TestQuadHeight(quadItem, 0, -QUAD_SIDE, &mmlOld);
	int hmmr_old = TestQuadHeight(quadItem, 0, QUAD_SIDE, &mmrOld);
	int hmtl_old = TestQuadHeight(quadItem, QUAD_FRONT / 2, -QUAD_SIDE, &mtlOld);
	int hmtr_old = TestQuadHeight(quadItem, QUAD_FRONT / 2, QUAD_SIDE, &mtrOld);

	PHD_VECTOR moldBottomLeft, moldBottomRight;
	int hmoldBottomLeft = TestQuadHeight(quadItem, -QUAD_FRONT / 2, -QUAD_SIDE, &moldBottomLeft);
	int hmoldBottomRight = TestQuadHeight(quadItem, -QUAD_FRONT / 2, QUAD_SIDE, &moldBottomRight);

	PHD_VECTOR old;
	old.x = quadItem->Position.xPos;
	old.y = quadItem->Position.yPos;
	old.z = quadItem->Position.zPos;

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

	if (mtlOld.y > hmtl_old)
		mtlOld.y = hmtl_old;

	if (mtrOld.y > hmtr_old)
		mtrOld.y = hmtr_old;

	if (mmlOld.y > hmml_old)
		mmlOld.y = hmml_old;

	if (mmrOld.y > hmmr_old)
		mmrOld.y = hmmr_old;

	if (quadItem->Position.yPos > (quadItem->Floor - CLICK(1)))
	{
		if (quadInfo->TurnRate < -QUAD_UNDO_TURN)
			quadInfo->TurnRate += QUAD_UNDO_TURN;
		else if (quadInfo->TurnRate > QUAD_UNDO_TURN)
			quadInfo->TurnRate -= QUAD_UNDO_TURN;
		else
			quadInfo->TurnRate = 0;

		quadItem->Position.yRot += quadInfo->TurnRate + quadInfo->ExtraRotation;


		short momentum;
		momentum = MIN_MOMENTUM_TURN - (((((MIN_MOMENTUM_TURN - MAX_MOMENTUM_TURN) * 256) / MAX_VELOCITY) * quadInfo->Velocity) / 256);
		if (!(TrInput & QUAD_IN_ACCELERATE) && quadInfo->Velocity > 0)
			momentum += (momentum / 4);

		short rot = quadItem->Position.yRot - quadInfo->MomentumAngle;
		if (rot < -MAX_MOMENTUM_TURN)
		{
			if (rot < -QUAD_MAX_MOM_TURN)
			{
				rot = -QUAD_MAX_MOM_TURN;
				quadInfo->MomentumAngle = quadItem->Position.yRot - rot;
			}
			else
				quadInfo->MomentumAngle -= momentum;
		}
		else if (rot > MAX_MOMENTUM_TURN)
		{
			if (rot > QUAD_MAX_MOM_TURN)
			{
				rot = QUAD_MAX_MOM_TURN;
				quadInfo->MomentumAngle = quadItem->Position.yRot - rot;
			}
			else
				quadInfo->MomentumAngle += momentum;
		}
		else
			quadInfo->MomentumAngle = quadItem->Position.yRot;
	}

	else
		quadItem->Position.yRot += quadInfo->TurnRate + quadInfo->ExtraRotation;

	auto probe = GetCollisionResult(quadItem);
	int speed = 0;
	if (quadItem->Position.yPos >= probe.Position.Floor)
		speed = quadItem->VerticalVelocity * phd_cos(quadItem->Position.xRot);
	else
		speed = quadItem->VerticalVelocity;

	quadItem->Position.zPos += speed * phd_cos(quadInfo->MomentumAngle);
	quadItem->Position.xPos += speed * phd_sin(quadInfo->MomentumAngle);

	int slip = QUAD_SLIP * phd_sin(quadItem->Position.xRot);
	if (abs(slip) > QUAD_SLIP / 2)
	{
		if (slip > 0)
			slip -= 10;
		else
			slip += 10;
		quadItem->Position.zPos -= slip * phd_cos(quadItem->Position.yRot);
		quadItem->Position.xPos -= slip * phd_sin(quadItem->Position.yRot);
	}

	slip = QUAD_SLIP_SIDE * phd_sin(quadItem->Position.zRot);
	if (abs(slip) > QUAD_SLIP_SIDE / 2)
	{
		quadItem->Position.zPos -= slip * phd_sin(quadItem->Position.yRot);
		quadItem->Position.xPos += slip * phd_cos(quadItem->Position.yRot);
	}

	PHD_VECTOR moved;
	moved.x = quadItem->Position.xPos;
	moved.z = quadItem->Position.zPos;

	if (!(quadItem->Flags & ONESHOT))
		QuadBaddieCollision(laraItem, quadItem);

	short rot = 0;
	short rotAdd = 0;

	PHD_VECTOR fl;
	int heightFrontLeft = TestQuadHeight(quadItem, QUAD_FRONT, -QUAD_SIDE, &fl);
	if (heightFrontLeft < oldFrontLeft.y - STEP_SIZE)
		rot = DoQuadShift(quadItem, &fl, &oldFrontLeft);

	PHD_VECTOR mtl;
	int hmtl = TestQuadHeight(quadItem, QUAD_FRONT / 2, -QUAD_SIDE, &mtl);
	if (hmtl < mtlOld.y - STEP_SIZE)
		DoQuadShift(quadItem, &mtl, &mtlOld);

	PHD_VECTOR mml;
	int hmml = TestQuadHeight(quadItem, 0, -QUAD_SIDE, &mml);
	if (hmml < mmlOld.y - STEP_SIZE)
		DoQuadShift(quadItem, &mml, &mmlOld);

	PHD_VECTOR mbl;
	int hmbl = TestQuadHeight(quadItem, -QUAD_FRONT / 2, -QUAD_SIDE, &mbl);
	if (hmbl < moldBottomLeft.y - STEP_SIZE)
		DoQuadShift(quadItem, &mbl, &moldBottomLeft);

	PHD_VECTOR bl;
	int heightBackLeft = TestQuadHeight(quadItem, -QUAD_FRONT, -QUAD_SIDE, &bl);
	if (heightBackLeft < oldBottomLeft.y - STEP_SIZE)
	{
		rotAdd = DoQuadShift(quadItem, &bl, &oldBottomLeft);
		if ((rotAdd > 0 && rot >= 0) || (rotAdd < 0 && rot <= 0))
			rot += rotAdd;
	}

	PHD_VECTOR fr;
	int heightFrontRight = TestQuadHeight(quadItem, QUAD_FRONT, QUAD_SIDE, &fr);
	if (heightFrontRight < oldFrontRight.y - STEP_SIZE)
	{
		rotAdd = DoQuadShift(quadItem, &fr, &oldFrontRight);
		if ((rotAdd > 0 && rot >= 0) || (rotAdd < 0 && rot <= 0))
			rot += rotAdd;
	}

	PHD_VECTOR mtr;
	int hmtr = TestQuadHeight(quadItem, QUAD_FRONT / 2, QUAD_SIDE, &mtr);
	if (hmtr < (mtrOld.y - STEP_SIZE))
		DoQuadShift(quadItem, &mtr, &mtrOld);

	PHD_VECTOR mmr;
	int hmmr = TestQuadHeight(quadItem, 0, QUAD_SIDE, &mmr);
	if (hmmr < (mmrOld.y - STEP_SIZE))
		DoQuadShift(quadItem, &mmr, &mmrOld);

	PHD_VECTOR mbr;
	int hmbr = TestQuadHeight(quadItem, -QUAD_FRONT / 2, QUAD_SIDE, &mbr);
	if (hmbr < (moldBottomRight.y - STEP_SIZE))
		DoQuadShift(quadItem, &mbr, &moldBottomRight);

	PHD_VECTOR br;
	int heightBackRight = TestQuadHeight(quadItem, -QUAD_FRONT, QUAD_SIDE, &br);
	if (heightBackRight < (oldBottomRight.y - STEP_SIZE))
	{
		rotAdd = DoQuadShift(quadItem, &br, &oldBottomRight);
		if ((rotAdd > 0 && rot >= 0) || (rotAdd < 0 && rot <= 0))
			rot += rotAdd;
	}

	probe = GetCollisionResult(quadItem);
	if (probe.Position.Floor < quadItem->Position.yPos - STEP_SIZE)
		DoQuadShift(quadItem, (PHD_VECTOR*)&quadItem->Position, &old);

	quadInfo->ExtraRotation = rot;

	int collide = GetQuadCollisionAnim(quadItem, &moved);

	int newVelocity = 0;
	if (collide)
	{
		newVelocity = (quadItem->Position.zPos - old.z) * phd_cos(quadInfo->MomentumAngle) + (quadItem->Position.xPos - old.x) * phd_sin(quadInfo->MomentumAngle);
		newVelocity *= 256;

		if (&g_Level.Items[laraInfo->Vehicle] == quadItem &&
			quadInfo->Velocity == MAX_VELOCITY &&
			newVelocity < (quadInfo->Velocity - 10))
		{
			laraItem->HitPoints -= (quadInfo->Velocity - newVelocity) / 128;
			laraItem->HitStatus = 1;
		}

		if (quadInfo->Velocity > 0 && newVelocity < quadInfo->Velocity)
			quadInfo->Velocity = (newVelocity < 0) ? 0 : newVelocity;

		else if (quadInfo->Velocity < 0 && newVelocity > quadInfo->Velocity)
			quadInfo->Velocity = (newVelocity > 0) ? 0 : newVelocity;

		if (quadInfo->Velocity < MAX_BACK)
			quadInfo->Velocity = MAX_BACK;
	}

	return collide;
}

static void AnimateQuadBike(ITEM_INFO* laraItem, ITEM_INFO* quadItem, int collide, bool dead)
{
	auto* quadInfo = (QuadInfo*)quadItem->Data;

	if (quadItem->Position.yPos != quadItem->Floor &&
		laraItem->ActiveState != QUAD_STATE_FALL &&
		laraItem->ActiveState != QUAD_STATE_LAND &&
		laraItem->ActiveState != QUAD_STATE_FALL_OFF &&
		!dead)
	{
		if (quadInfo->Velocity < 0)
			laraItem->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_LEAP_START;
		else
			laraItem->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_LEAP_START2;

		laraItem->FrameNumber = GetFrameNumber(laraItem, laraItem->AnimNumber);
		laraItem->ActiveState = QUAD_STATE_FALL;
		laraItem->TargetState = QUAD_STATE_FALL;
	}
	else if (collide &&
		laraItem->ActiveState != QUAD_STATE_HIT_FRONT &&
		laraItem->ActiveState != QUAD_STATE_HIT_BACK &&
		laraItem->ActiveState != QUAD_STATE_HIT_LEFT &&
		laraItem->ActiveState != QUAD_STATE_HIT_RIGHT &&
		laraItem->ActiveState != QUAD_STATE_FALL_OFF &&
		quadInfo->Velocity > (MAX_VELOCITY / 3) &&
		!dead)
	{
		if (collide == QUAD_HIT_FRONT)
		{
			laraItem->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_BACK;
			laraItem->ActiveState = QUAD_STATE_HIT_FRONT;
			laraItem->TargetState = QUAD_STATE_HIT_FRONT;
		}
		else if (collide == QUAD_HIT_BACK)
		{
			laraItem->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_FRONT;
			laraItem->ActiveState = QUAD_STATE_HIT_BACK;
			laraItem->TargetState = QUAD_STATE_HIT_BACK;
		}
		else if (collide == QUAD_HIT_LEFT)
		{
			laraItem->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_RIGHT;
			laraItem->ActiveState = QUAD_STATE_HIT_LEFT;
			laraItem->TargetState = QUAD_STATE_HIT_LEFT;
		}
		else
		{
			laraItem->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_HIT_LEFT;
			laraItem->ActiveState = QUAD_STATE_HIT_RIGHT;
			laraItem->TargetState = QUAD_STATE_HIT_RIGHT;
		}

		laraItem->FrameNumber = GetFrameNumber(laraItem, laraItem->AnimNumber);
		SoundEffect(SFX_TR3_QUAD_FRONT_IMPACT, &quadItem->Position, 0);
	}
	else
	{
		switch (laraItem->ActiveState)
		{
		case QUAD_STATE_IDLE:
			if (dead)
				laraItem->TargetState = QUAD_STATE_BIKE_DEATH;
			else if (TrInput & QUAD_IN_DISMOUNT &&
				quadInfo->Velocity == 0 &&
				!quadInfo->NoDismount)
			{
				if (TrInput & QUAD_IN_LEFT && CanQuadbikeGetOff(-1))
					laraItem->TargetState = QUAD_STATE_DISMOUNT_LEFT;
				else if (TrInput & QUAD_IN_RIGHT && CanQuadbikeGetOff(1))
					laraItem->TargetState = QUAD_STATE_DISMOUNT_RIGHT;
			}
			else if (TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE))
				laraItem->TargetState = QUAD_STATE_DRIVE;

			break;

		case QUAD_STATE_DRIVE:
			if (dead)
			{
				if (quadInfo->Velocity > (MAX_VELOCITY / 2))
					laraItem->TargetState = QUAD_STATE_FALL_DEATH;
				else
					laraItem->TargetState = QUAD_STATE_BIKE_DEATH;
			}
			else if (!(TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE)) &&
				(quadInfo->Velocity / 256) == 0)
			{
				laraItem->TargetState = QUAD_STATE_IDLE;
			}
			else if (TrInput & QUAD_IN_LEFT &&
				!quadInfo->DriftStarting)
			{
				laraItem->TargetState = QUAD_STATE_TURN_LEFT;
			}
			else if (TrInput & QUAD_IN_RIGHT &&
				!quadInfo->DriftStarting)
			{
				laraItem->TargetState = QUAD_STATE_TURN_RIGHT;
			}
			else if (TrInput & QUAD_IN_BRAKE)
			{
				if (quadInfo->Velocity > (MAX_VELOCITY / 3 * 2))
					laraItem->TargetState = QUAD_STATE_BRAKE;
				else
					laraItem->TargetState = QUAD_STATE_SLOW;
			}

			break;

		case QUAD_STATE_BRAKE:
		case QUAD_STATE_SLOW:
		case QUAD_STATE_STOP_SLOWLY:
			if ((quadInfo->Velocity / 256) == 0)
				laraItem->TargetState = QUAD_STATE_IDLE;
			else if (TrInput & QUAD_IN_LEFT)
				laraItem->TargetState = QUAD_STATE_TURN_LEFT;
			else if (TrInput & QUAD_IN_RIGHT)
				laraItem->TargetState = QUAD_STATE_TURN_RIGHT;

			break;

		case QUAD_STATE_TURN_LEFT:
			if ((quadInfo->Velocity / 256) == 0)
				laraItem->TargetState = QUAD_STATE_IDLE;
			else if (TrInput & QUAD_IN_RIGHT)
			{
				laraItem->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_TURN_RIGHT_START;
				laraItem->FrameNumber = GetFrameNumber(laraItem, laraItem->AnimNumber);
				laraItem->ActiveState = QUAD_STATE_TURN_RIGHT;
				laraItem->TargetState = QUAD_STATE_TURN_RIGHT;
			}
			else if (!(TrInput & QUAD_IN_LEFT))
				laraItem->TargetState = QUAD_STATE_DRIVE;

			break;

		case QUAD_STATE_TURN_RIGHT:
			if ((quadInfo->Velocity / 256) == 0)
				laraItem->TargetState = QUAD_STATE_IDLE;
			else if (TrInput & QUAD_IN_LEFT)
			{
				laraItem->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_TURN_LEFT_START;
				laraItem->FrameNumber = GetFrameNumber(laraItem, laraItem->AnimNumber);
				laraItem->ActiveState = QUAD_STATE_TURN_LEFT;
				laraItem->TargetState = QUAD_STATE_TURN_LEFT;
			}
			else if (!(TrInput & QUAD_IN_RIGHT))
				laraItem->TargetState = QUAD_STATE_DRIVE;

			break;

		case QUAD_STATE_FALL:
			if (quadItem->Position.yPos == quadItem->Floor)
				laraItem->TargetState = QUAD_STATE_LAND;
			else if (quadItem->VerticalVelocity > TERMINAL_FALLSPEED)
				quadInfo->Flags |= QUAD_FLAG_IS_FALLING;

			break;

		case QUAD_STATE_FALL_OFF:
			break;

		case QUAD_STATE_HIT_FRONT:
		case QUAD_STATE_HIT_BACK:
		case QUAD_STATE_HIT_LEFT:
		case QUAD_STATE_HIT_RIGHT:
			if (TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE))
				laraItem->TargetState = QUAD_STATE_DRIVE;

			break;
		}

		if (TestEnvironment(ENV_FLAG_WATER, quadItem) ||
			TestEnvironment(ENV_FLAG_SWAMP, quadItem))
		{
			laraItem->TargetState = QUAD_STATE_FALL_OFF;
			laraItem->Position.yPos = quadItem->Position.yPos + 700;
			laraItem->RoomNumber = quadItem->RoomNumber;
			laraItem->HitPoints = 0;
			QuadbikeExplode(laraItem, quadItem);
		}
	}
}

static int QuadUserControl(ITEM_INFO* quadItem, int height, int* pitch)
{
	auto* quadInfo = (QuadInfo*)quadItem->Data;
	bool drive = false; // Never changes?

	if (!(TrInput & QUAD_IN_DRIFT) &&
		!quadInfo->Velocity && !quadInfo->CanStartDrift)
	{
		quadInfo->CanStartDrift = true;
	}
	else if (quadInfo->Velocity)
		quadInfo->CanStartDrift = false;

	if (!(TrInput & QUAD_IN_DRIFT))
		quadInfo->DriftStarting = false;

	if (!quadInfo->DriftStarting)
	{
		if (quadInfo->Revs > 0x10)
		{
			quadInfo->Velocity += (quadInfo->Revs / 16);
			quadInfo->Revs -= (quadInfo->Revs / 8);
		}
		else
			quadInfo->Revs = 0;
	}

	if (quadItem->Position.yPos >= (height - STEP_SIZE))
	{
		if (TrInput & IN_LOOK && !quadInfo->Velocity)
			LookUpDown(LaraItem);

		// Driving forward.
		if (quadInfo->Velocity > 0)
		{
			if (TrInput & QUAD_IN_DRIFT &&
				!quadInfo->DriftStarting &&
				quadInfo->Velocity > MIN_DRIFT_SPEED)
			{
				if (TrInput & QUAD_IN_LEFT)
				{
					quadInfo->TurnRate -= QUAD_DRIFT_TURN_RATE;
					if (quadInfo->TurnRate < -QUAD_DRIFT_TURN_MAX)
						quadInfo->TurnRate = -QUAD_DRIFT_TURN_MAX;
				}
				else if (TrInput & QUAD_IN_RIGHT)
				{
					quadInfo->TurnRate += QUAD_DRIFT_TURN_RATE;
					if (quadInfo->TurnRate > QUAD_DRIFT_TURN_MAX)
						quadInfo->TurnRate = QUAD_DRIFT_TURN_MAX;
				}
			}
			else
			{
				if (TrInput & QUAD_IN_LEFT)
				{
					quadInfo->TurnRate -= QUAD_TURN_RATE;
					if (quadInfo->TurnRate < -QUAD_TURN_MAX)
						quadInfo->TurnRate = -QUAD_TURN_MAX;
				}
				else if (TrInput & QUAD_IN_RIGHT)
				{
					quadInfo->TurnRate += QUAD_TURN_RATE;
					if (quadInfo->TurnRate > QUAD_TURN_MAX)
						quadInfo->TurnRate = QUAD_TURN_MAX;
				}
			}
		}
		// Driving back.
		else if (quadInfo->Velocity < 0)
		{
			if (TrInput & QUAD_IN_DRIFT &&
				!quadInfo->DriftStarting &&
				quadInfo->Velocity < (-MIN_DRIFT_SPEED + 0x800))
			{
				if (TrInput & QUAD_IN_LEFT)
				{
					quadInfo->TurnRate -= QUAD_DRIFT_TURN_RATE;
					if (quadInfo->TurnRate < -QUAD_DRIFT_TURN_MAX)
						quadInfo->TurnRate = -QUAD_DRIFT_TURN_MAX;
				}
				else if (TrInput & QUAD_IN_RIGHT)
				{
					quadInfo->TurnRate += QUAD_DRIFT_TURN_RATE;
					if (quadInfo->TurnRate > QUAD_DRIFT_TURN_MAX)
						quadInfo->TurnRate = QUAD_DRIFT_TURN_MAX;
				}
			}
			else
			{
				if (TrInput & QUAD_IN_RIGHT)
				{
					quadInfo->TurnRate -= QUAD_TURN_RATE;
					if (quadInfo->TurnRate < -QUAD_TURN_MAX)
						quadInfo->TurnRate = -QUAD_TURN_MAX;
				}
				else if (TrInput & QUAD_IN_LEFT)
				{
					quadInfo->TurnRate += QUAD_TURN_RATE;
					if (quadInfo->TurnRate > QUAD_TURN_MAX)
						quadInfo->TurnRate = QUAD_TURN_MAX;
				}
			}
		}

		// Driving back / braking.
		if (TrInput & QUAD_IN_BRAKE)
		{
			if (TrInput & QUAD_IN_DRIFT &&
				(quadInfo->CanStartDrift || quadInfo->DriftStarting))
			{
				quadInfo->DriftStarting = true;
				quadInfo->Revs -= 0x200;
				if (quadInfo->Revs < MAX_BACK)
					quadInfo->Revs = MAX_BACK;
			}
			else if (quadInfo->Velocity > 0)
				quadInfo->Velocity -= BRAKE;
			else
			{
				if (quadInfo->Velocity > MAX_BACK)
					quadInfo->Velocity += REVERSE_ACC;
			}
		}
		else if (TrInput & QUAD_IN_ACCELERATE)
		{
			if (TrInput & QUAD_IN_DRIFT &&
				(quadInfo->CanStartDrift || quadInfo->DriftStarting))
			{
				quadInfo->DriftStarting = true;
				quadInfo->Revs += 0x200;
				if (quadInfo->Revs >= MAX_VELOCITY)
					quadInfo->Revs = MAX_VELOCITY;
			}
			else if (quadInfo->Velocity < MAX_VELOCITY)
			{
				if (quadInfo->Velocity < 0x4000)
					quadInfo->Velocity += (8 + (0x4000 + 0x800 - quadInfo->Velocity) / 8);
				else if (quadInfo->Velocity < 0x7000)
					quadInfo->Velocity += (4 + (0x7000 + 0x800 - quadInfo->Velocity) / 16);
				else if (quadInfo->Velocity < MAX_VELOCITY)
					quadInfo->Velocity += (2 + (MAX_VELOCITY - quadInfo->Velocity) / 8);
			}
			else
				quadInfo->Velocity = MAX_VELOCITY;

			quadInfo->Velocity -= abs(quadItem->Position.yRot - quadInfo->MomentumAngle) / 64;
		}

		else if (quadInfo->Velocity > 0x0100)
			quadInfo->Velocity -= 0x0100;
		else if (quadInfo->Velocity < -0x0100)
			quadInfo->Velocity += 0x0100;
		else
			quadInfo->Velocity = 0;

		if (!(TrInput & (QUAD_IN_ACCELERATE | QUAD_IN_BRAKE)) &&
			quadInfo->DriftStarting &&
			quadInfo->Revs)
		{
			if (quadInfo->Revs > 0x8)
				quadInfo->Revs -= quadInfo->Revs / 8;
			else
				quadInfo->Revs = 0;
		}

		quadItem->VerticalVelocity = quadInfo->Velocity / 256;

		if (quadInfo->EngineRevs > 0x7000)
			quadInfo->EngineRevs = -0x2000;

		int revs = 0;
		if (quadInfo->Velocity < 0)
			revs = abs(quadInfo->Velocity / 2);
		else if (quadInfo->Velocity < 0x7000)
			revs = -0x2000 + (quadInfo->Velocity * (0x6800 - -0x2000)) / 0x7000;
		else if (quadInfo->Velocity <= MAX_VELOCITY)
			revs = -0x2800 + ((quadInfo->Velocity - 0x7000) * (0x7000 - -0x2800)) / (MAX_VELOCITY - 0x7000);

		revs += abs(quadInfo->Revs);
		quadInfo->EngineRevs += (revs - quadInfo->EngineRevs) / 8;
	}
	else
	{
		if (quadInfo->EngineRevs < 0xA000)
			quadInfo->EngineRevs += (0xA000 - quadInfo->EngineRevs) / 8;
	}

	*pitch = quadInfo->EngineRevs;

	return drive;
}

void QuadBikeCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* quadItem = &g_Level.Items[itemNumber];
	auto* quadInfo = (QuadInfo*)quadItem->Data;

	if (laraItem->HitPoints < 0 || laraInfo->Vehicle != NO_ITEM)
		return;

	if (GetOnQuadBike(laraItem, &g_Level.Items[itemNumber], coll))
	{
		laraInfo->Vehicle = itemNumber;

		if (laraInfo->Control.WeaponControl.GunType == WEAPON_FLARE)
		{
			CreateFlare(laraItem, ID_FLARE_ITEM, 0);
			UndrawFlareMeshes(laraItem);
			laraInfo->Flare.ControlLeft = false;
			laraInfo->Control.WeaponControl.RequestGunType = laraInfo->Control.WeaponControl.GunType = WEAPON_NONE;
		}

		laraInfo->Control.HandStatus = HandStatus::Busy;

		short angle = phd_atan(quadItem->Position.zPos - laraItem->Position.zPos, quadItem->Position.xPos - laraItem->Position.xPos);
		angle -= quadItem->Position.yRot;

		if (angle > -ANGLE(45.0f) && angle < ANGLE(135.0f))
		{
			laraItem->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_MOUNT_LEFT;
			laraItem->ActiveState = laraItem->TargetState = QUAD_STATE_MOUNT_LEFT;
		}
		else
		{
			laraItem->AnimNumber = Objects[ID_QUAD_LARA_ANIMS].animIndex + QUAD_ANIM_MOUNT_RIGHT;
			laraItem->ActiveState = laraItem->TargetState = QUAD_STATE_MOUNT_RIGHT;
		}

		laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
		laraItem->Position.xPos = quadItem->Position.xPos;
		laraItem->Position.yPos = quadItem->Position.yPos;
		laraItem->Position.zPos = quadItem->Position.zPos;
		laraItem->Position.yRot = quadItem->Position.yRot;
		ResetLaraFlex(laraItem);
		laraInfo->hitDirection = -1;
		quadItem->HitPoints = 1;

		AnimateItem(laraItem);

		quadInfo->Revs = 0;
	}
	else
		ObjectCollision(itemNumber, laraItem, coll);
}

static void TriggerQuadExhaustSmoke(int x, int y, int z, short angle, int speed, int moving)
{
	auto* spark = &Sparks[GetFreeSpark()];

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

bool QuadBikeControl(ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* quadItem = &g_Level.Items[laraInfo->Vehicle];
	auto* quadInfo = (QuadInfo*)quadItem->Data;

	GAME_VECTOR	oldPos;
	oldPos.x = quadItem->Position.xPos;
	oldPos.y = quadItem->Position.yPos;
	oldPos.z = quadItem->Position.zPos;
	oldPos.roomNumber = quadItem->RoomNumber;

	bool collide = QuadDynamics(laraItem, quadItem);

	auto probe = GetCollisionResult(quadItem);

	PHD_VECTOR frontLeft, frontRight;
	auto floorHeightLeft = TestQuadHeight(quadItem, QUAD_FRONT, -QUAD_SIDE, &frontLeft);
	auto floorHeightRight = TestQuadHeight(quadItem, QUAD_FRONT, QUAD_SIDE, &frontRight);

	TestTriggers(quadItem, false);

	bool dead = false;
	if (laraItem->HitPoints <= 0)
	{
		TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		dead = true;
	}

	int drive = -1;
	int pitch = 0;
	if (quadInfo->Flags)
		collide = false;
	else
	{
		switch (laraItem->ActiveState)
		{
		case QUAD_STATE_MOUNT_LEFT:
		case QUAD_STATE_MOUNT_RIGHT:
		case QUAD_STATE_DISMOUNT_LEFT:
		case QUAD_STATE_DISMOUNT_RIGHT:
			drive = -1;
			collide = false;
			break;

		default:
			drive = QuadUserControl(quadItem, probe.Position.Floor, &pitch);
			break;
		}
	}

	if (quadInfo->Velocity || quadInfo->Revs)
	{
		quadInfo->Pitch = pitch;
		if (quadInfo->Pitch < -0x8000)
			quadInfo->Pitch = -0x8000;
		else if (quadInfo->Pitch > 0xA000)
			quadInfo->Pitch = 0xA000;

		SoundEffect(SFX_TR3_QUAD_MOVE, &quadItem->Position, 0, 0.5f + (float)abs(quadInfo->Pitch) / (float)MAX_VELOCITY);
	}
	else
	{
		if (drive != -1)
			SoundEffect(SFX_TR3_QUAD_IDLE, &quadItem->Position, 0);

		quadInfo->Pitch = 0;
	}

	quadItem->Floor = probe.Position.Floor;

	short rotAdd = quadInfo->Velocity / 4;
	quadInfo->RearRot -= rotAdd;
	quadInfo->RearRot -= (quadInfo->Revs / 8);
	quadInfo->FrontRot -= rotAdd;

	quadInfo->LeftVerticalVelocity = DoQuadDynamics(floorHeightLeft, quadInfo->LeftVerticalVelocity, (int*)&frontLeft.y);
	quadInfo->RightVerticalVelocity = DoQuadDynamics(floorHeightRight, quadInfo->RightVerticalVelocity, (int*)&frontRight.y);
	quadItem->VerticalVelocity = DoQuadDynamics(probe.Position.Floor, quadItem->VerticalVelocity, (int*)&quadItem->Position.yPos);

	probe.Position.Floor = (frontLeft.y + frontRight.y) / 2;
	short xRot = phd_atan(QUAD_FRONT, quadItem->Position.yPos - probe.Position.Floor);
	short zRot = phd_atan(QUAD_SIDE, probe.Position.Floor - frontLeft.y);

	quadItem->Position.xRot += ((xRot - quadItem->Position.xRot) / 2);
	quadItem->Position.zRot += ((zRot - quadItem->Position.zRot) / 2);

	if (!(quadInfo->Flags & QUAD_FLAG_DEAD))
	{
		if (probe.RoomNumber != quadItem->RoomNumber)
		{
			ItemNewRoom(laraInfo->Vehicle, probe.RoomNumber);
			ItemNewRoom(laraInfo->ItemNumber, probe.RoomNumber);
		}

		laraItem->Position = quadItem->Position;

		AnimateQuadBike(laraItem, quadItem, collide, dead);
		AnimateItem(laraItem);

		quadItem->AnimNumber = Objects[ID_QUAD].animIndex + (laraItem->AnimNumber - Objects[ID_QUAD_LARA_ANIMS].animIndex);
		quadItem->FrameNumber = g_Level.Anims[quadItem->AnimNumber].frameBase + (laraItem->FrameNumber - g_Level.Anims[laraItem->AnimNumber].frameBase);

		Camera.targetElevation = -ANGLE(30.0f);

		if (quadInfo->Flags & QUAD_FLAG_IS_FALLING)
		{
			if (quadItem->Position.yPos == quadItem->Floor)
			{
				ExplodingDeath(laraInfo->ItemNumber, 0xffffffff, 1);
				laraItem->HitPoints = 0;
				laraItem->Flags |= ONESHOT;
				QuadbikeExplode(laraItem, quadItem);

				return false;
			}
		}
	}

	if (laraItem->ActiveState != QUAD_STATE_MOUNT_RIGHT &&
		laraItem->ActiveState != QUAD_STATE_MOUNT_LEFT &&
		laraItem->ActiveState != QUAD_STATE_DISMOUNT_RIGHT &&
		laraItem->ActiveState != QUAD_STATE_DISMOUNT_LEFT)
	{
		PHD_VECTOR pos;
		int speed = 0;
		short angle = 0;

		for (int i = 0; i < 2; i++)
		{
			pos.x = quadEffectsPositions[i].x;
			pos.y = quadEffectsPositions[i].y;
			pos.z = quadEffectsPositions[i].z;
			GetJointAbsPosition(quadItem, &pos, quadEffectsPositions[i].meshNum);
			angle = quadItem->Position.yRot + ((i == 0) ? 0x9000 : 0x7000);
			if (quadItem->VerticalVelocity > 32)
			{
				if (quadItem->VerticalVelocity < 64)
				{
					speed = 64 - quadItem->VerticalVelocity;
					TriggerQuadExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 1);
				}
			}
			else
			{
				if (quadInfo->SmokeStart < 16)
				{
					speed = ((quadInfo->SmokeStart * 2) + (GetRandomControl() & 7) + (GetRandomControl() & 16)) * 128;
					quadInfo->SmokeStart++;
				}
				else if (quadInfo->DriftStarting)
					speed = (abs(quadInfo->Revs) * 2) + ((GetRandomControl() & 7) * 128);
				else if ((GetRandomControl() & 3) == 0)
					speed = ((GetRandomControl() & 15) + (GetRandomControl() & 16)) * 128;
				else
					speed = 0;

				TriggerQuadExhaustSmoke(pos.x, pos.y, pos.z, angle, speed, 0);
			}
		}
	}
	else
		quadInfo->SmokeStart = 0;

	return QuadCheckGetOff(laraItem, quadItem);
}
