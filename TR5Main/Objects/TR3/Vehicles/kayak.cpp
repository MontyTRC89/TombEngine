#include "framework.h"
#include "Objects/TR3/Vehicles/kayak.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Objects/TR3/Vehicles/kayak_info.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Specific/setup.h"

using std::vector;

#define KAYAK_COLLIDE			CLICK(0.25f)
#define DISMOUNT_DISTANCE 		CLICK(3) // TODO: Find accurate distance.
#define KAYAK_TO_BADDIE_RADIUS	CLICK(1)

#define MAX_SPEED			0x380000
#define KAYAK_FRICTION		0x8000
#define KAYAK_ROT_FRIC		0x50000
#define KAYAK_DFLECT_ROT	0x80000
#define KAYAK_FWD_VEL		0x180000
#define KAYAK_FWD_ROT		0x800000
#define KAYAK_LR_VEL		0x100000
#define KAYAK_LR_ROT		0xc00000
#define KAYAK_MAX_LR		0xc00000
#define KAYAK_TURN_ROT		0x200000
#define KAYAK_MAX_TURN		0x1000000
#define KAYAK_TURN_BRAKE	0x8000
#define KAYAK_HARD_ROT		0x1000000
#define KAYAK_MAX_STAT		0x1000000
#define BOAT_SLIP			50
#define BOAT_SIDE_SLIP		50

#define HIT_BACK	1
#define HIT_FRONT	2
#define HIT_LEFT	3
#define HIT_RIGHT	4

#define KAYAK_MOUNT_LEFT_FRAME	GetFrameNumber(KAYAK_ANIM_MOUNT_RIGHT, 0)
#define KAYAK_IDLE_FRAME		GetFrameNumber(KAYAK_ANIM_IDLE, 0)
#define KAYAK_MOUNT_RIGHT_FRAME	GetFrameNumber(KAYAK_ANIM_MOUNT_LEFT, 0)

#define KAYAK_DRAW_SHIFT	32
#define LARA_LEG_BITS		((1 << LM_HIPS) | (1 << LM_LTHIGH) | (1 << LM_LSHIN) | (1 << LM_LFOOT) | (1 << LM_RTHIGH) | (1 << LM_RSHIN) | (1 << LM_RFOOT))
#define NUM_WAKE_SPRITES	32
#define WAKE_SIZE 			32
#define WAKE_SPEED 			4
#define KAYAK_X				128
#define KAYAK_Z				128
#define KAYAK_MAX_KICK		-80
#define KAYAK_MIN_BOUNCE	((MAX_SPEED / 2) / 256)

#define KAYAK_IN_FORWARD	IN_FORWARD
#define KAYAK_IN_BACK		IN_BACK
#define KAYAK_IN_LEFT		IN_LEFT
#define KAYAK_IN_RIGHT		IN_RIGHT
#define KAYAK_IN_HOLD_LEFT	IN_LSTEP
#define KAYAK_IN_HOLD_RIGHT	IN_RSTEP
#define KAYAK_IN_DISMOUNT	(IN_JUMP | IN_ROLL)

enum KayakState
{
	KAYAK_STATE_BACK,
	KAYAK_STATE_IDLE,
	KAYAK_STATE_TURN_LEFT,
	KAYAK_STATE_TURN_RIGHT,
	KAYAK_STATE_MOUNT_LEFT,
	KAYAK_STATE_IDLE_DEATH,
	KAYAK_STATE_FORWARD,
	KAYAK_STATE_CAPSIZE_RECOVER,	// Unused.
	KAYAK_STATE_CAPSIZE_DEATH,		// Unused.
	KAYAK_STATE_DISMOUNT,
	KAYAK_STATE_HOLD_LEFT,
	KAYAK_STATE_HOLD_RIGHT,
	KAYAK_STATE_MOUNT_RIGHT,
	KAYAK_STATE_DISMOUNT_LEFT,
	KAYAK_STATE_DISMOUNT_RIGHT,
};

enum KayakAnim
{
	KAYAK_ANIM_PADDLE_BACK_END = 0,
	KAYAK_ANIM_PADDLE_BACK_START = 1,
	KAYAK_ANIM_PADDLE_BACK = 2,
	KAYAK_ANIM_MOUNT_RIGHT = 3,
	KAYAK_ANIM_GET_PADDLE = 4,
	KAYAK_ANIM_IDLE_DEATH = 5,
	KAYAK_ANIM_CAPSIZE_DEATH = 6,			// Unused.
	KAYAK_ANIM_PADDLE_FORWARD_END = 7,		// Unused.
	KAYAK_ANIM_PADDLE_FORWARD = 8,			// Unused.
	KAYAK_ANIM_PADDLE_FORWARD_START = 9,	// Unused.
	KAYAK_ANIM_HIT_BACK = 10,
	KAYAK_ANIM_HIT_FRONT = 11,
	KAYAK_ANIM_HIT_RIGHT = 12,
	KAYAK_ANIM_CAPSIZE_LEFT = 13,			// Unused.
	KAYAK_ANIM_DISMOUNT_START = 14,
	KAYAK_ANIM_PADDLE_LEFT = 15,
	KAYAK_ANIM_IDLE = 16,
	KAYAK_ANIM_PADDLE_RIGHT = 17,
	KAYAK_ANIM_CAPSIZE_STRUGGLE = 18,		// Unused.
	KAYAK_ANIM_CAPSIZE_RECOVER_LEFT = 19,	// Unused.
	KAYAK_ANIM_HOLD_PADDLE_LEFT_START = 20,
	KAYAK_ANIM_HOLD_PADDLE_LEFT_END = 21,
	KAYAK_ANIM_HOLD_PADDLE_RIGHT_START = 22,
	KAYAK_ANIM_HOLD_PADDLE_RIGHT_END = 23,
	KAYAK_ANIM_DISMOUNT_LEFT = 24,
	KAYAK_ANIM_OVERBOARD_DEATH = 25,
	KAYAK_ANIM_HOLD_PADDLE_LEFT = 26,
	KAYAK_ANIM_HOLD_PADDLE_RIGHT = 27,
	KAYAK_ANIM_MOUNT_LEFT = 28,
	KAYAK_ANIM_HIT_LEFT = 29,
	KAYAK_ANIM_CAPSIZE_RIGHT = 30,			// Unused.
	KAYAK_ANIM_CAPSIZE_RECOVER_RIGHT = 31,	// Unused.
	KAYAK_ANIM_DISMOUNT_RIGHT = 32
};

struct WAKE_PTS 
{
	int 	x[2];
	int 	y;
	int		z[2];
	short	xvel[2];
	short	zvel[2];
	byte 	life;
	byte	pad[3];
};

enum class KayakMountType
{
	None,
	Left,
	Right
};

WAKE_PTS WakePts[NUM_WAKE_SPRITES][2];
byte CurrentStartWake = 0;
byte WakeShade = 0;

void InitialiseKayak(short itemNum)
{
	ITEM_INFO* kayakItem = &g_Level.Items[itemNum];
	KAYAK_INFO* kayakInfo;
	kayakItem->Data = KAYAK_INFO();
	kayakInfo = kayakItem->Data;

	kayakInfo->Vel = 0;
	kayakInfo->Rot = 0;
	kayakInfo->Flags = 0;
	kayakInfo->FallSpeedF = 0;
	kayakInfo->FallSpeedL = 0;
	kayakInfo->FallSpeedR = 0;
	kayakInfo->OldPos = kayakItem->Position;

	for (int i = 0; i < NUM_WAKE_SPRITES; i++)
	{
		WakePts[i][0].life = 0;
		WakePts[i][1].life = 0;
	}
}

void KayakDraw(ITEM_INFO* kayakItem)
{
	DrawAnimatingItem(kayakItem);
}

void KayakDoWake(ITEM_INFO* kayakItem, int xOffset, int zOffset, short rotate)
{
	if (WakePts[CurrentStartWake][rotate].life)
		return;

	float s = phd_sin(kayakItem->Position.yRot);
	float c = phd_cos(kayakItem->Position.yRot);

	int x = kayakItem->Position.xPos + zOffset * s + xOffset * c;
	int z = kayakItem->Position.zPos + zOffset * c - xOffset * s;

	int probedRoomNum = GetCollisionResult(x, kayakItem->Position.yPos, z, kayakItem->RoomNumber).RoomNumber;
	int waterHeight = GetWaterHeight(x, kayakItem->Position.yPos, z, probedRoomNum);

	if (waterHeight != NO_HEIGHT)
	{
		short angle1, angle2;
		if (kayakItem->Velocity < 0)
		{
			if (!rotate)
			{
				angle1 = kayakItem->Position.yRot - ANGLE(10.0f);
				angle2 = kayakItem->Position.yRot - ANGLE(30.0f);
			}
			else
			{
				angle1 = kayakItem->Position.yRot + ANGLE(10.0f);
				angle2 = kayakItem->Position.yRot + ANGLE(30.0f);
			}
		}
		else
		{
			if (!rotate)
			{
				angle1 = kayakItem->Position.yRot - ANGLE(170.0f);
				angle2 = kayakItem->Position.yRot - ANGLE(150.0f);
			}
			else
			{
				angle1 = kayakItem->Position.yRot + ANGLE(170.0f);
				angle2 = kayakItem->Position.yRot + ANGLE(150.0f);
			}
		}

		int xv[2], zv[2];
		xv[0] = WAKE_SPEED * phd_sin(angle1);
		zv[0] = WAKE_SPEED * phd_cos(angle1);
		xv[1] = (WAKE_SPEED + 2) * phd_sin(angle2);
		zv[1] = (WAKE_SPEED + 2) * phd_cos(angle2);

		WakePts[CurrentStartWake][rotate].y = kayakItem->Position.yPos + KAYAK_DRAW_SHIFT;
		WakePts[CurrentStartWake][rotate].life = 0x40;

		for (int i = 0; i < 2; i++)
		{
			WakePts[CurrentStartWake][rotate].x[i] = x;
			WakePts[CurrentStartWake][rotate].z[i] = z;
			WakePts[CurrentStartWake][rotate].xvel[i] = xv[i];
			WakePts[CurrentStartWake][rotate].zvel[i] = zv[i];
		}

		if (rotate == 1)
		{
			CurrentStartWake++;
			CurrentStartWake &= (NUM_WAKE_SPRITES - 1);
		}
	}
}

void KayakDoRipple(ITEM_INFO* kayakItem, int xOffset, int zOffset)
{
	float s = phd_sin(kayakItem->Position.yRot);
	float c = phd_cos(kayakItem->Position.yRot);

	int x = kayakItem->Position.xPos + zOffset * s + xOffset * c;
	int z = kayakItem->Position.zPos + zOffset * c - xOffset * s;

	int probedRoomNum = GetCollisionResult(x, kayakItem->Position.yPos, z, kayakItem->RoomNumber).RoomNumber;
	int waterHeight = GetWaterHeight(x, kayakItem->Position.yPos, z, probedRoomNum);

	if (waterHeight != NO_HEIGHT)
		SetupRipple(x, kayakItem->Position.yPos, z, -2 - (GetRandomControl() & 1), 0, Objects[ID_KAYAK_PADDLE_TRAIL_SPRITE].meshIndex,TO_RAD(kayakItem->Position.yRot));
}

void KayakUpdateWakeFX()
{
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < NUM_WAKE_SPRITES; j++)
		{
			if (WakePts[j][i].life)
			{
				WakePts[j][i].life--;
				WakePts[j][i].x[0] += WakePts[j][i].xvel[0];
				WakePts[j][i].z[0] += WakePts[j][i].zvel[0];
				WakePts[j][i].x[1] += WakePts[j][i].xvel[1];
				WakePts[j][i].z[1] += WakePts[j][i].zvel[1];
			}
		}
	}
}

KayakMountType KayakGetMountType(short itemNum, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	ITEM_INFO* kayakItem = &g_Level.Items[itemNum];
	LaraInfo*& laraInfo = laraItem->Data;

	if (!(TrInput & IN_ACTION) ||
		laraInfo->gunStatus != LG_HANDS_FREE ||
		laraItem->Airborne)
	{
		return KayakMountType::None;
	}

	int dist = pow(laraItem->Position.xPos - kayakItem->Position.xPos, 2) + pow(laraItem->Position.zPos - kayakItem->Position.zPos, 2);
	if (dist > pow(360, 2))
		return KayakMountType::None;

	auto probe = GetCollisionResult(kayakItem);
	if (probe.Position.Floor > -32000)
	{
		short angle = phd_atan(kayakItem->Position.zPos - laraItem->Position.zPos, kayakItem->Position.xPos - laraItem->Position.xPos);
		angle -= kayakItem->Position.yRot;

		int deltaAngle = laraItem->Position.yRot - kayakItem->Position.yRot;
		if (angle > -ANGLE(45.0f) && angle < ANGLE(135.0f))
		{
			deltaAngle = laraItem->Position.yRot - kayakItem->Position.yRot;
			if (deltaAngle > ANGLE(45.0f) && deltaAngle < ANGLE(135.0f))
				return KayakMountType::Left;
		}
		else
		{
			deltaAngle = laraItem->Position.yRot - kayakItem->Position.yRot;
			if (deltaAngle > ANGLE(225.0f) && deltaAngle < ANGLE(315.0f))
				return KayakMountType::Right;
		}
	}

	return KayakMountType::None;
}

int KayakGetCollisionAnim(ITEM_INFO* kayakItem, int xDiff, int zDiff)
{
	xDiff = kayakItem->Position.xPos - xDiff;
	zDiff = kayakItem->Position.zPos - zDiff;

	if ((xDiff) || (zDiff))
	{
		float s = phd_sin(kayakItem->Position.yRot);
		float c = phd_cos(kayakItem->Position.yRot);

		int front = zDiff * c + xDiff * s;
		int side = -zDiff * s + xDiff * c;

		if (abs(front) > abs(side))
		{
			if (front > 0)
				return HIT_BACK;
			else
				return HIT_FRONT;
		}
		else
		{
			if (side > 0)
				return HIT_LEFT;
			else
				return HIT_RIGHT;
		}
	}

	return 0;
}

int KayakDoDynamics(int height, int fallSpeed, int* y)
{
	if (height > * y)
	{
		*y += fallSpeed;

		if (*y > height)
		{
			*y = height;
			fallSpeed = 0;
		}
		else
			fallSpeed += GRAVITY;
	}
	else
	{
		int kick = (height - *y) * 4;

		if (kick < KAYAK_MAX_KICK)
			kick = KAYAK_MAX_KICK;

		fallSpeed += (kick - fallSpeed) / 8;

		if (*y > height)
			*y = height;
	}

	return fallSpeed;
}

void KayakDoCurrent(ITEM_INFO* kayakItem, ITEM_INFO* laraItem)
{
	LaraInfo*& laraInfo = laraItem->Data;
	ROOM_INFO* room = &g_Level.Rooms[kayakItem->RoomNumber];

	if (!laraInfo->currentActive)
	{
		int absVel = abs(laraInfo->currentVel.x);
		int shift;

		if (absVel > 16)
			shift = 4;
		else if (absVel > 8)
			shift = 3;
		else
			shift = 2;

		laraInfo->currentVel.x -= laraInfo->currentVel.x >> shift;

		if (abs(laraInfo->currentVel.x) < 4)
			laraInfo->currentVel.x = 0;

		absVel = abs(laraInfo->currentVel.z);
		if (absVel > 16)
			shift = 4;
		else if (absVel > 8)
			shift = 3;
		else
			shift = 2;

		laraInfo->currentVel.z -= laraInfo->currentVel.z >> shift;
		if (abs(laraInfo->currentVel.z) < 4)
			laraInfo->currentVel.z = 0;

		if (laraInfo->currentVel.x == 0 && laraInfo->currentVel.z == 0)
			return;
	}
	else
	{
		int sinkval = laraInfo->currentActive - 1;
		
		PHD_VECTOR target;
		target.x = g_Level.Sinks[sinkval].x;
		target.y = g_Level.Sinks[sinkval].y;
		target.z = g_Level.Sinks[sinkval].z;
		
		int angle = (((mGetAngle(target.x, target.z, laraItem->Position.xPos, laraItem->Position.zPos) - ANGLE(90))) / 16) & 4095;

		int dx = target.x - laraItem->Position.xPos;
		int dz = target.z - laraItem->Position.zPos;

		int speed = g_Level.Sinks[sinkval].strength;
		dx = phd_sin(angle * 16) * speed * 1024;
		dz = phd_cos(angle * 16) * speed * 1024;

		laraInfo->currentVel.x += (dx - laraInfo->currentVel.x) / 16;
		laraInfo->currentVel.z += (dz - laraInfo->currentVel.z) / 16;
	}

	kayakItem->Position.xPos += laraInfo->currentVel.x / 256;
	kayakItem->Position.zPos += laraInfo->currentVel.z / 256;

	laraInfo->currentActive = 0;
}

int KayakTestHeight(ITEM_INFO* kayakItem, int x, int z, PHD_VECTOR* pos)
{
	Matrix world =
		Matrix::CreateFromYawPitchRoll(TO_RAD(kayakItem->Position.yRot), TO_RAD(kayakItem->Position.xRot), TO_RAD(kayakItem->Position.zRot)) *
		Matrix::CreateTranslation(kayakItem->Position.xPos, kayakItem->Position.yPos, kayakItem->Position.zPos);

	Vector3 vec = Vector3(x, 0, z);
	vec = Vector3::Transform(vec, world);
	
	pos->x = vec.x;
	pos->y = vec.y;
	pos->z = vec.z;

	auto probe = GetCollisionResult(pos->x, pos->y, pos->z, kayakItem->RoomNumber);
	int probedRoomNum = probe.RoomNumber;

	int height = GetWaterHeight(pos->x, pos->y, pos->z, probedRoomNum);
	if (height == NO_HEIGHT)
	{
		height = probe.Position.Floor;
		if (height == NO_HEIGHT)
			return height;
	}

	return (height - 5);
}

bool KayakCanGetOut(ITEM_INFO* kayakItem, int dir)
{
	PHD_VECTOR pos;
	int height = KayakTestHeight(kayakItem, (dir < 0) ? -DISMOUNT_DISTANCE : DISMOUNT_DISTANCE, 0, &pos);

	if ((kayakItem->Position.yPos - height) > 0)
		return false;

	return true;
}

int KayakDoShift(ITEM_INFO* kayakItem, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int x = pos->x / SECTOR(1);
	int z = pos->z / SECTOR(1);

	int xOld = old->x / SECTOR(1);
	int zOld = old->z / SECTOR(1);

	int xShift = pos->x & (WALL_SIZE - 1);
	int zShift = pos->z & (WALL_SIZE - 1);

	if (x == xOld)
	{
		old->x = 0;

		if (z == zOld)
		{
			kayakItem->Position.zPos += old->z - pos->z;
			kayakItem->Position.xPos += old->x - pos->x;
		}
		else if (z > zOld)
		{
			kayakItem->Position.zPos -= zShift + 1;
			return (pos->x - kayakItem->Position.xPos);
		}
		else
		{
			kayakItem->Position.zPos += WALL_SIZE - zShift;
			return (kayakItem->Position.xPos - pos->x);
		}
	}
	else if (z == zOld)
	{
		old->z = 0;

		if (x > xOld)
		{
			kayakItem->Position.xPos -= xShift + 1;
			return (kayakItem->Position.zPos - pos->z);
		}

		else
		{
			kayakItem->Position.xPos += WALL_SIZE - xShift;
			return (pos->z - kayakItem->Position.zPos);
		}
	}
	else
	{
		x = 0;
		z = 0;

		auto probe = GetCollisionResult(old->x, pos->y, pos->z, kayakItem->RoomNumber);
		if (probe.Position.Floor < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -zShift - 1;
			else
				z = WALL_SIZE - zShift;
		}

		probe = GetCollisionResult(pos->x, pos->y, old->z, kayakItem->RoomNumber);
		if (probe.Position.Floor < old->y - STEP_SIZE)
		{
			if (pos->x > old->x)
				x = -xShift - 1;
			else
				x = WALL_SIZE - xShift;
		}

		if (x && z)
		{
			kayakItem->Position.xPos += x;
			kayakItem->Position.zPos += z;
		}
		else if (x)
		{
			kayakItem->Position.xPos += x;

			if (x > 0)
				return (pos->z - kayakItem->Position.zPos);
			else
				return (kayakItem->Position.zPos - pos->z);
		}
		else if (z)
		{
			kayakItem->Position.zPos += z;

			if (z > 0)
				return (kayakItem->Position.xPos - pos->x);
			else
				return (pos->x - kayakItem->Position.xPos);
		}
		else
		{
			kayakItem->Position.xPos += (old->x - pos->x);
			kayakItem->Position.zPos += (old->z - pos->z);
		}
	}

	return 0;
}

void KayakToBackground(ITEM_INFO* kayakItem, ITEM_INFO* laraItem)
{
	KAYAK_INFO* kayakInfo = kayakItem->Data;

	kayakInfo->OldPos = kayakItem->Position;

	PHD_VECTOR oldPos[9];
	int height[8];
	height[0] = KayakTestHeight(kayakItem, 0, 1024, &oldPos[0]);
	height[1] = KayakTestHeight(kayakItem, -96, 512, &oldPos[1]);
	height[2] = KayakTestHeight(kayakItem, 96, 512, &oldPos[2]);
	height[3] = KayakTestHeight(kayakItem, -128, 128, &oldPos[3]);
	height[4] = KayakTestHeight(kayakItem, 128, 128, &oldPos[4]);
	height[5] = KayakTestHeight(kayakItem, -128, -320, &oldPos[5]);
	height[6] = KayakTestHeight(kayakItem, 128, -320, &oldPos[6]);
	height[7] = KayakTestHeight(kayakItem, 0, -640, &oldPos[7]);

	for (int i = 0; i < 8; i++)
	{
		if (oldPos[i].y > height[i])
			oldPos[i].y = height[i];
	}

	oldPos[8].x = kayakItem->Position.xPos;
	oldPos[8].y = kayakItem->Position.yPos;
	oldPos[8].z = kayakItem->Position.zPos;
 
	PHD_VECTOR frontPos, leftPos, rightPos;
	int fh = KayakTestHeight(kayakItem, 0, 1024, &frontPos);
	int lh = KayakTestHeight(kayakItem, -KAYAK_X, KAYAK_Z, &leftPos);
	int rh = KayakTestHeight(kayakItem, KAYAK_X, KAYAK_Z, &rightPos);

	kayakItem->Position.yRot += (kayakInfo->Rot / 65536);

	kayakItem->Position.xPos += kayakItem->Velocity * phd_sin(kayakItem->Position.yRot);
	kayakItem->Position.zPos += kayakItem->Velocity * phd_cos(kayakItem->Position.yRot);

	KayakDoCurrent(kayakItem, laraItem);

	kayakInfo->FallSpeedL = KayakDoDynamics(lh, kayakInfo->FallSpeedL, &leftPos.y);
	kayakInfo->FallSpeedR = KayakDoDynamics(rh, kayakInfo->FallSpeedR, &rightPos.y);
	kayakInfo->FallSpeedF = KayakDoDynamics(fh, kayakInfo->FallSpeedF, &frontPos.y);

	kayakItem->VerticalVelocity = KayakDoDynamics(kayakInfo->Water, kayakItem->VerticalVelocity, &kayakItem->Position.yPos);

	int h = (leftPos.y + rightPos.y) / 2;
	int x = phd_atan(1024, kayakItem->Position.yPos - frontPos.y);
	int z = phd_atan(KAYAK_X, h - leftPos.y);

	kayakItem->Position.xRot = x;
	kayakItem->Position.zRot = z;

	int xOld = kayakItem->Position.xPos;
	int zOld = kayakItem->Position.zPos;

	int rot = 0;
	PHD_VECTOR pos;

	if ((h = KayakTestHeight(kayakItem, 0, -640, &pos)) < (oldPos[7].y - KAYAK_COLLIDE))
		rot = KayakDoShift(kayakItem, &pos, &oldPos[7]);

	if ((h = KayakTestHeight(kayakItem, 128, -320, &pos)) < (oldPos[6].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[6]);

	if ((h = KayakTestHeight(kayakItem, -128, -320, &pos)) < (oldPos[5].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[5]);

	if ((h = KayakTestHeight(kayakItem, 128, 128, &pos)) < (oldPos[4].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[4]);

	if ((h = KayakTestHeight(kayakItem, -128, 128, &pos)) < (oldPos[3].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[3]);

	if ((h = KayakTestHeight(kayakItem, 96, 512, &pos)) < (oldPos[2].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[2]);

	if ((h = KayakTestHeight(kayakItem, -96, 512, &pos)) < (oldPos[1].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[1]);

	if ((h = KayakTestHeight(kayakItem, 0, 1024, &pos)) < (oldPos[0].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[0]);

	kayakItem->Position.yRot += rot;

	auto probe = GetCollisionResult(kayakItem);
	int probedRoomNum = probe.RoomNumber;

	h = GetWaterHeight(kayakItem->Position.xPos, kayakItem->Position.yPos, kayakItem->Position.zPos, probedRoomNum);
	if (h == NO_HEIGHT)
		h = probe.Position.Floor;

	if (h < (kayakItem->Position.yPos - KAYAK_COLLIDE))
		KayakDoShift(kayakItem, (PHD_VECTOR*)&kayakItem->Position, &oldPos[8]);

	probe = GetCollisionResult(kayakItem);
	probedRoomNum = probe.RoomNumber;

	h = GetWaterHeight(kayakItem->Position.xPos, kayakItem->Position.yPos, kayakItem->Position.zPos, probedRoomNum);
	if (h == NO_HEIGHT)
		h = probe.Position.Floor;

	if (h == NO_HEIGHT)
	{
		GAME_VECTOR kpos;

		kpos.x = kayakInfo->OldPos.xPos;
		kpos.y = kayakInfo->OldPos.yPos;
		kpos.z = kayakInfo->OldPos.zPos;
		kpos.roomNumber = kayakItem->RoomNumber;

		CameraCollisionBounds(&kpos, 256, 0);
		{
			kayakItem->Position.xPos = kpos.x;
			kayakItem->Position.yPos = kpos.y;
			kayakItem->Position.zPos = kpos.z;
			kayakItem->RoomNumber = kpos.roomNumber;
		}
	}

	int collide = KayakGetCollisionAnim(kayakItem, xOld, zOld);

	int slip = 0;
	if (slip || collide)
	{
		int newspeed;

		newspeed = (kayakItem->Position.zPos - oldPos[8].z) * phd_cos(kayakItem->Position.yRot) + (kayakItem->Position.xPos - oldPos[8].x) * phd_sin(kayakItem->Position.yRot);

		newspeed *= 256;

		if (slip)
		{
			if (kayakInfo->Vel <= MAX_SPEED)
				kayakInfo->Vel = newspeed;
		}
		else
		{
			if (kayakInfo->Vel > 0 && newspeed < kayakInfo->Vel)
				kayakInfo->Vel = newspeed;

			else if (kayakInfo->Vel < 0 && newspeed > kayakInfo->Vel)
				kayakInfo->Vel = newspeed;
		}

		if (kayakInfo->Vel < -MAX_SPEED)
			kayakInfo->Vel = -MAX_SPEED;
	}
}

void KayakUserInput(ITEM_INFO* kayakItem, ITEM_INFO* laraItem)
{
	KAYAK_INFO* kayakInfo = kayakItem->Data;
	LaraInfo*& laraInfo = laraItem->Data;

	if (laraItem->HitPoints <= 0 &&
		laraItem->ActiveState != KAYAK_STATE_IDLE_DEATH)
	{
		laraItem->AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_IDLE_DEATH;
		laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
		laraItem->ActiveState = laraItem->TargetState = KAYAK_STATE_IDLE_DEATH;
	}

	int frame = laraItem->FrameNumber - g_Level.Anims[laraItem->AnimNumber].frameBase;

	switch (laraItem->ActiveState)
	{
		static char leftRight;

	case KAYAK_STATE_IDLE:
		if (TrInput & KAYAK_IN_DISMOUNT &&
			!laraInfo->currentActive &&
			!laraInfo->currentVel.x && !laraInfo->currentVel.z)
		{
			if (TrInput & KAYAK_IN_LEFT && KayakCanGetOut(kayakItem, -1))
			{
				laraItem->TargetState = KAYAK_STATE_DISMOUNT;
				laraItem->RequiredState = KAYAK_STATE_DISMOUNT_LEFT;
			}
			else if (TrInput & KAYAK_IN_RIGHT && KayakCanGetOut(kayakItem, 1))
			{
				laraItem->TargetState = KAYAK_STATE_DISMOUNT;
				laraItem->RequiredState = KAYAK_STATE_DISMOUNT_RIGHT;
			}
		}
		else if (TrInput & KAYAK_IN_FORWARD)
		{
			laraItem->TargetState = KAYAK_STATE_TURN_RIGHT;
			kayakInfo->Turn = false;
			kayakInfo->Forward = true;
		}
		else if (TrInput & KAYAK_IN_BACK)
			laraItem->TargetState = KAYAK_STATE_BACK;
		else if (TrInput & KAYAK_IN_LEFT)
		{
			laraItem->TargetState = KAYAK_STATE_TURN_LEFT;

			if (kayakInfo->Vel)
				kayakInfo->Turn = false;
			else
				kayakInfo->Turn = true;

			kayakInfo->Forward = false;
		}

		else if (TrInput & KAYAK_IN_RIGHT)
		{
			laraItem->TargetState = KAYAK_STATE_TURN_RIGHT;

			if (kayakInfo->Vel)
				kayakInfo->Turn = false;
			else
				kayakInfo->Turn = true;

			kayakInfo->Forward = false;
		}
		else if (TrInput & KAYAK_IN_HOLD_LEFT &&
			(kayakInfo->Vel ||
				laraInfo->currentVel.x ||
				laraInfo->currentVel.z))
		{
			laraItem->TargetState = KAYAK_STATE_HOLD_LEFT;
		}
		else if (TrInput & KAYAK_IN_HOLD_RIGHT &&
			(kayakInfo->Vel ||
				laraInfo->currentVel.x ||
				laraInfo->currentVel.z))
		{
			laraItem->TargetState = KAYAK_STATE_HOLD_RIGHT;
		}

		break;
		
	case KAYAK_STATE_TURN_LEFT:
		if (kayakInfo->Forward)
		{
			if (!frame)
				leftRight = 0;

			if (frame == 2 && !(leftRight & 0x80))
				leftRight++;

			else if (frame > 2)
				leftRight &= ~0x80;

			if (TrInput & KAYAK_IN_FORWARD)
			{
				if (TrInput & KAYAK_IN_LEFT)
				{
					if ((leftRight & ~0x80) >= 2)
						laraItem->TargetState = KAYAK_STATE_TURN_RIGHT;
				}
				else
					laraItem->TargetState = KAYAK_STATE_TURN_RIGHT;
			}
			else
				laraItem->TargetState = KAYAK_STATE_IDLE;
		}
		else if (!(TrInput & KAYAK_IN_LEFT))
			laraItem->TargetState = KAYAK_STATE_IDLE;

		if (frame == 7)
		{
			if (kayakInfo->Forward)
			{
				kayakInfo->Rot -= KAYAK_FWD_ROT;
				if (kayakInfo->Rot < -KAYAK_MAX_TURN)
					kayakInfo->Rot = -KAYAK_MAX_TURN;

				kayakInfo->Vel += KAYAK_FWD_VEL;
			}
			else if (kayakInfo->Turn)
			{
				kayakInfo->Rot -= KAYAK_HARD_ROT;
				if (kayakInfo->Rot < -KAYAK_MAX_STAT)
					kayakInfo->Rot = -KAYAK_MAX_STAT;
			}
			else
			{
				kayakInfo->Rot -= KAYAK_LR_ROT;
				if (kayakInfo->Rot < -KAYAK_MAX_LR)
					kayakInfo->Rot = -KAYAK_MAX_LR;

				kayakInfo->Vel += KAYAK_LR_VEL;
			}
		}

		if (frame > 6 && frame < 24 && frame & 1)
			KayakDoRipple(kayakItem, -CLICK(1.5f), -CLICK(0.25f));

		break;
		
	case KAYAK_STATE_TURN_RIGHT:	
		if (kayakInfo->Forward)
		{
			if (!frame)
				leftRight = 0;

			if (frame == 2 && !(leftRight & 0x80))
				leftRight++;

			else if (frame > 2)
				leftRight &= ~0x80;

			if (TrInput & KAYAK_IN_FORWARD)
			{
				if (TrInput & KAYAK_IN_RIGHT)
				{
					if ((leftRight & ~0x80) >= 2)
						laraItem->TargetState = KAYAK_STATE_TURN_LEFT;
				}
				else
					laraItem->TargetState = KAYAK_STATE_TURN_LEFT;
			}
			else
				laraItem->TargetState = KAYAK_STATE_IDLE;
		}

		else if (!(TrInput & KAYAK_IN_RIGHT))
			laraItem->TargetState = KAYAK_STATE_IDLE;

		if (frame == 7)
		{
			if (kayakInfo->Forward)
			{
				kayakInfo->Rot += KAYAK_FWD_ROT;
				if (kayakInfo->Rot > KAYAK_MAX_TURN)
					kayakInfo->Rot = KAYAK_MAX_TURN;

				kayakInfo->Vel += KAYAK_FWD_VEL;
			}
			else if (kayakInfo->Turn)
			{
				kayakInfo->Rot += KAYAK_HARD_ROT;
				if (kayakInfo->Rot > KAYAK_MAX_STAT)
					kayakInfo->Rot = KAYAK_MAX_STAT;
			}
			else
			{
				kayakInfo->Rot += KAYAK_LR_ROT;
				if (kayakInfo->Rot > KAYAK_MAX_LR)
					kayakInfo->Rot = KAYAK_MAX_LR;

				kayakInfo->Vel += KAYAK_LR_VEL;
			}
		}

		if (frame > 6 && frame < 24 && frame & 1)
			KayakDoRipple(kayakItem, CLICK(1.5f), -CLICK(0.25f));

		break;
		
	case KAYAK_STATE_BACK:
		if (!(TrInput & KAYAK_IN_BACK))
			laraItem->TargetState = KAYAK_STATE_IDLE;

		if ((laraItem->AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_ANIM_PADDLE_BACK)
		{
			if (frame == 8)
			{
				kayakInfo->Rot += KAYAK_FWD_ROT;
				kayakInfo->Vel -= KAYAK_FWD_VEL;
			}

			if (frame == 31)
			{
				kayakInfo->Rot -= KAYAK_FWD_ROT;
				kayakInfo->Vel -= KAYAK_FWD_VEL;
			}

			if (frame < 15 && frame & 1)
				KayakDoRipple(kayakItem, CLICK(1.5f), -CLICK(0.5f));

			else if (frame >= 20 && frame <= 34 && frame & 1)
				KayakDoRipple(kayakItem, -CLICK(1.5f), -CLICK(0.5f));
		}

		break;
		
	case KAYAK_STATE_HOLD_LEFT:
		if (!(TrInput & KAYAK_IN_HOLD_LEFT) ||
			!kayakInfo->Vel &&
				!laraInfo->currentVel.x &&
				!laraInfo->currentVel.z)
		{
			laraItem->TargetState = KAYAK_STATE_IDLE;
		}
		else if ((laraItem->AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_ANIM_HOLD_PADDLE_LEFT)
		{
			if (kayakInfo->Vel >= 0)
			{
				kayakInfo->Rot -= KAYAK_TURN_ROT;
				if (kayakInfo->Rot < -KAYAK_MAX_TURN)
					kayakInfo->Rot = -KAYAK_MAX_TURN;

				kayakInfo->Vel += -KAYAK_TURN_BRAKE;
				if (kayakInfo->Vel < 0)
					kayakInfo->Vel = 0;
			}

			if (kayakInfo->Vel < 0)
			{
				kayakInfo->Rot += KAYAK_TURN_ROT;

				kayakInfo->Vel += KAYAK_TURN_BRAKE;
				if (kayakInfo->Vel > 0)
					kayakInfo->Vel = 0;
			}

			if (!(Wibble & 3))
				KayakDoRipple(kayakItem, -CLICK(1), -CLICK(1));
		}

		break;
		
	case KAYAK_STATE_HOLD_RIGHT:
		if (!(TrInput & KAYAK_IN_HOLD_RIGHT) ||
			(!kayakInfo->Vel &&
				!laraInfo->currentVel.x &&
				!laraInfo->currentVel.z))
		{
			laraItem->TargetState = KAYAK_STATE_IDLE;
		}
		else if ((laraItem->AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_ANIM_HOLD_PADDLE_RIGHT)
		{
			if (kayakInfo->Vel >= 0)
			{
				kayakInfo->Rot += KAYAK_TURN_ROT;
				if (kayakInfo->Rot > KAYAK_MAX_TURN)
					kayakInfo->Rot = KAYAK_MAX_TURN;

				kayakInfo->Vel += -KAYAK_TURN_BRAKE;
				if (kayakInfo->Vel < 0)
					kayakInfo->Vel = 0;
			}

			if (kayakInfo->Vel < 0)
			{
				kayakInfo->Rot -= KAYAK_TURN_ROT;

				kayakInfo->Vel += KAYAK_TURN_BRAKE;
				if (kayakInfo->Vel > 0)
					kayakInfo->Vel = 0;
			}

			if (!(Wibble & 3))
				KayakDoRipple(kayakItem, CLICK(1), -CLICK(1));
		}

		break;
		
	case KAYAK_STATE_MOUNT_LEFT:
		if (laraItem->AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_GET_PADDLE &&
			frame == 24 &&
			!(kayakInfo->Flags & 0x80))
		{
			kayakInfo->Flags |= 0x80;
			laraInfo->meshPtrs[LM_RHAND] = Objects[ID_KAYAK_LARA_ANIMS].meshIndex + LM_RHAND;
			laraItem->MeshBits &= ~LARA_LEG_BITS;
		}

		break;
		
	case KAYAK_STATE_DISMOUNT:
		if (laraItem->AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_DISMOUNT_START &&
			frame == 27 &&
			kayakInfo->Flags & 0x80)
		{
			kayakInfo->Flags &= ~0x80;
			laraInfo->meshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
			laraItem->MeshBits |= LARA_LEG_BITS;
		}

		laraItem->TargetState = laraItem->RequiredState;
		break;
		
	case KAYAK_STATE_DISMOUNT_LEFT:
		if (laraItem->AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_DISMOUNT_LEFT &&
			frame == 83)
		{
			PHD_VECTOR vec = { 0, 350, 500 };
			GetLaraJointPosition(&vec, LM_HIPS);

			SetAnimation(laraItem, LA_JUMP_FORWARD);
			laraItem->Position.xPos = vec.x;
			laraItem->Position.yPos = vec.y;
			laraItem->Position.zPos = vec.z;
			laraItem->Position.xRot = 0;
			laraItem->Position.yRot = kayakItem->Position.yRot - ANGLE(90.0f);
			laraItem->Position.zRot = 0;
			laraItem->Airborne = true;
			laraItem->VerticalVelocity = -50;
			laraItem->Velocity = 40;
			laraItem->Airborne = true;
			laraInfo->gunStatus = LG_HANDS_FREE;
			laraInfo->Vehicle = NO_ITEM;
		}

		break;
		
	case KAYAK_STATE_DISMOUNT_RIGHT:
		if (laraItem->AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_DISMOUNT_RIGHT &&
			frame == 83)
		{
			PHD_VECTOR vec = { 0, 350, 500 };
			GetLaraJointPosition(&vec, LM_HIPS);

			SetAnimation(laraItem, LA_JUMP_FORWARD);
			laraItem->Position.xPos = vec.x;
			laraItem->Position.yPos = vec.y;
			laraItem->Position.zPos = vec.z;
			laraItem->Position.xRot = 0;
			laraItem->Position.yRot = kayakItem->Position.yRot + ANGLE(90.0f);
			laraItem->Position.zRot = 0;
			laraItem->Airborne = true;
			laraItem->VerticalVelocity = -50;
			laraItem->Velocity = 40;
			laraItem->Airborne = true;
			laraInfo->gunStatus = LG_HANDS_FREE;
			laraInfo->Vehicle = NO_ITEM;
		}
	}

	if (kayakInfo->Vel > 0)
	{
		kayakInfo->Vel -= KAYAK_FRICTION;
		if (kayakInfo->Vel < 0)
			kayakInfo->Vel = 0;
	}
	else if (kayakInfo->Vel < 0)
	{
		kayakInfo->Vel += KAYAK_FRICTION;
		if (kayakInfo->Vel > 0)
			kayakInfo->Vel = 0;
	}

	if (kayakInfo->Vel > MAX_SPEED)
		kayakInfo->Vel = MAX_SPEED;
	else if (kayakInfo->Vel < -MAX_SPEED)
		kayakInfo->Vel = -MAX_SPEED;

	kayakItem->Velocity = (kayakInfo->Vel / 65536);

	if (kayakInfo->Rot >= 0)
	{
		kayakInfo->Rot -= KAYAK_ROT_FRIC;
		if (kayakInfo->Rot < 0)
			kayakInfo->Rot = 0;
	}
	else if (kayakInfo->Rot < 0)
	{
		kayakInfo->Rot += KAYAK_ROT_FRIC;
		if (kayakInfo->Rot > 0)
			kayakInfo->Rot = 0;
	}
}

void KayakToItemCollision(ITEM_INFO* kayakItem, ITEM_INFO* laraItem)
{
	short roomsToCheck[128];
	short numRoomsToCheck = 0;
	roomsToCheck[numRoomsToCheck++] = kayakItem->RoomNumber;

	ROOM_INFO* room = &g_Level.Rooms[kayakItem->RoomNumber];
	for (int i = 0; i < room->doors.size(); i++)
		roomsToCheck[numRoomsToCheck++] = room->doors[i].room;

	for (int i = 0; i < numRoomsToCheck; i++)
	{
		short itemNum = g_Level.Rooms[roomsToCheck[i]].itemNumber;

		while (itemNum != NO_ITEM)
		{
			ITEM_INFO* item = &g_Level.Items[itemNum];
			short nextItem = item->NextItem;

			if (item->Collidable && item->Status != ITEM_INVISIBLE)
			{
				OBJECT_INFO* object = &Objects[item->ObjectNumber];

				if (object->collision &&
					(item->ObjectNumber == ID_TEETH_SPIKES ||
						item->ObjectNumber == ID_DARTS &&
						item->ActiveState != 1))
				{
					int x = kayakItem->Position.xPos - item->Position.xPos;
					int y = kayakItem->Position.yPos - item->Position.yPos;
					int z = kayakItem->Position.zPos - item->Position.zPos;

					if (x > -2048 && x < 2048 &&
						y > -2048 && y < 2048 &&
						z > -2048 && z < 2048)
					{
						if (TestBoundsCollide(item, kayakItem, KAYAK_TO_BADDIE_RADIUS))
						{
							DoLotsOfBlood(laraItem->Position.xPos, laraItem->Position.yPos - STEP_SIZE, laraItem->Position.zPos, kayakItem->Velocity, kayakItem->Position.yRot, laraItem->RoomNumber, 3);
							laraItem->HitPoints -= 5;
						}
					}
				}
			}

			itemNum = nextItem;
		}
	}
}

void KayakLaraRapidsDrown(ITEM_INFO* laraItem)
{
	LaraInfo*& laraInfo = laraItem->Data;

	laraItem->AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_OVERBOARD_DEATH;
	laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
	laraItem->ActiveState = 12;
	laraItem->TargetState = 12;
	laraItem->HitPoints = 0;
	laraItem->VerticalVelocity = 0;
	laraItem->Airborne = 0;
	laraItem->Velocity = 0;

	AnimateItem(laraItem);

	laraInfo->ExtraAnim = 1;
	laraInfo->gunStatus = LG_HANDS_BUSY;
	laraInfo->gunType = WEAPON_NONE;
	laraInfo->hitDirection = -1;
}

void KayakCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = laraItem->Data;

	if (laraItem->HitPoints < 0 || laraInfo->Vehicle != NO_ITEM)
		return;

	KayakMountType mountType = KayakGetMountType(itemNumber, laraItem, coll);
	if (mountType != KayakMountType::None)
	{
		ITEM_INFO* kayakItem = &g_Level.Items[itemNumber];

		laraInfo->Vehicle = itemNumber;

		if (laraInfo->gunType == WEAPON_FLARE)
		{
			CreateFlare(laraItem, ID_FLARE_ITEM, 0);
			UndrawFlareMeshes(laraItem);
			laraInfo->Flare.FlareControlLeft = 0;
			laraInfo->requestGunType = laraInfo->gunType = WEAPON_NONE;
		}

		if (mountType == KayakMountType::Right)
			laraItem->AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_MOUNT_RIGHT;
		else if (mountType == KayakMountType::Left)
			laraItem->AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_MOUNT_LEFT;

		laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
		laraItem->ActiveState = laraItem->TargetState = KAYAK_STATE_MOUNT_LEFT;

		laraInfo->waterStatus = LW_ABOVE_WATER;
		laraItem->Position.xPos = kayakItem->Position.xPos;
		laraItem->Position.yPos = kayakItem->Position.yPos;
		laraItem->Position.zPos = kayakItem->Position.zPos;
		laraItem->Position.yRot = kayakItem->Position.yRot;
		laraItem->Position.xRot = laraItem->Position.zRot = 0;
		laraItem->Airborne = false;
		laraItem->Velocity = 0;
		laraItem->VerticalVelocity = 0;

		if (laraItem->RoomNumber != kayakItem->RoomNumber)
			ItemNewRoom(laraInfo->itemNumber, kayakItem->RoomNumber);

		AnimateItem(laraItem);

		KAYAK_INFO* kayak = (KAYAK_INFO*)kayakItem->Data;
		kayak->Water = kayakItem->Position.yPos;
		kayak->Flags = 0;
	}
	else
	{
		coll->Setup.EnableObjectPush = true;
		ObjectCollision(itemNumber, laraItem, coll);
	}
}

int KayakControl(ITEM_INFO* laraItem)
{
	LaraInfo*& laraInfo = laraItem->Data;
	ITEM_INFO* kayakItem = &g_Level.Items[laraInfo->Vehicle];
	KAYAK_INFO* kayakInfo = (KAYAK_INFO*)kayakItem->Data;

	if (TrInput & IN_LOOK)
		LookUpDown();

	int ofs = kayakItem->VerticalVelocity;

	KayakUserInput(kayakItem, laraItem);
	KayakToBackground(kayakItem, laraItem);
	TestTriggers(kayakItem, false);

	auto probe = GetCollisionResult(kayakItem);
	int water = GetWaterHeight(kayakItem->Position.xPos, kayakItem->Position.yPos, kayakItem->Position.zPos, probe.RoomNumber);
	kayakInfo->Water = water;

	if (kayakInfo->Water == NO_HEIGHT)
	{
		water = probe.Position.Floor;
		kayakInfo->Water = water;
		kayakInfo->TrueWater = false;
	}
	else
	{
		kayakInfo->Water -= 5;
		kayakInfo->TrueWater = true;
	}

	if ((ofs - kayakItem->VerticalVelocity) > 128 &&
		kayakItem->VerticalVelocity == 0 &&
		water != NO_HEIGHT)
	{
		int damage = ofs - kayakItem->VerticalVelocity;
		if (damage > 160)
			laraItem->HitPoints -= (damage - 160) * 8;
	}

	if (laraInfo->Vehicle != NO_ITEM)
	{
		if (kayakItem->RoomNumber != probe.RoomNumber)
		{
			ItemNewRoom(laraInfo->Vehicle, probe.RoomNumber);
			ItemNewRoom(laraInfo->itemNumber, probe.RoomNumber);
		}

		laraItem->Position.xPos = kayakItem->Position.xPos;
		laraItem->Position.yPos = kayakItem->Position.yPos;
		laraItem->Position.zPos = kayakItem->Position.zPos;
		laraItem->Position.xRot = kayakItem->Position.xRot;
		laraItem->Position.yRot = kayakItem->Position.yRot;
		laraItem->Position.zRot = kayakItem->Position.zRot / 2;

		AnimateItem(laraItem);

		kayakItem->AnimNumber = Objects[ID_KAYAK].animIndex + (laraItem->AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex);
		kayakItem->FrameNumber = g_Level.Anims[kayakItem->AnimNumber].frameBase + (laraItem->FrameNumber - g_Level.Anims[laraItem->AnimNumber].frameBase);

		Camera.targetElevation = -ANGLE(30.0f);
		Camera.targetDistance = CLICK(8);
	}

	if (!(Wibble & 15) && kayakInfo->TrueWater)
	{
		KayakDoWake(kayakItem, -CLICK(0.5f), 0, 0);
		KayakDoWake(kayakItem, CLICK(0.5f), 0, 1);
	}

	if (Wibble & 7)
	{
		if (!kayakInfo->TrueWater && kayakItem->VerticalVelocity < 20)
		{
			PHD_VECTOR dest;
			char cnt = 0;
			short MistZPos[10] = { 900, 750, 600, 450, 300, 150, 0,  -150, -300, -450 };
			short MistXPos[10] = { 32,  96,  170, 220, 300, 400, 400, 300,  200,  64 };

			cnt ^= 1;

			for (int i = cnt; i < 10; i += 2)
			{
				if (GetRandomControl() & 1)
					dest.x = (MistXPos[i] / 2);
				else
					dest.x = -(MistXPos[i] / 2);
				dest.y = 50;
				dest.z = MistZPos[i];
			}
		}
	}

	if (!kayakItem->Velocity &&
		!laraInfo->currentVel.x &&
		!laraInfo->currentVel.z)
	{
		if (WakeShade)
			WakeShade--;
	}
	else
	{
		if (WakeShade < 16)
			WakeShade++;
	}

	KayakUpdateWakeFX();
	KayakToItemCollision(kayakItem, laraItem);

	return (laraInfo->Vehicle != NO_ITEM) ? 1 : 0;
}
