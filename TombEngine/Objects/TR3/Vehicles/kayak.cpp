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
#include "Game/Lara/lara_helpers.h"
#include "Objects/TR3/Vehicles/kayak_info.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Specific/setup.h"

using std::vector;

#define KAYAK_COLLIDE			CLICK(0.25f)
#define DISMOUNT_DISTANCE 		CLICK(3) // TODO: Find accurate distance.
#define KAYAK_TO_ENTITY_RADIUS	CLICK(1)

#define MAX_VELOCITY				0x380000
#define KAYAK_FRICTION				0x8000
#define KAYAK_ROTATE_FRICTION		0x50000
#define KAYAK_DEFLECT_ROTATION		0x80000
#define KAYAK_FORWARD_VELOCITY		0x180000
#define KAYAK_FORWARD_ROTATION		0x800000
#define KAYAK_LEFT_RIGHT_VELOCITY	0x100000
#define KAYAK_LEFT_RIGHT_ROTATION	0xc00000
#define KAYAK_MAX_LEFT_RIGHT		0xc00000
#define KAYAK_TURN_ROTATION			0x200000
#define KAYAK_MAX_TURN				0x1000000
#define KAYAK_TURN_BRAKE			0x8000
#define KAYAK_HARD_ROTATION			0x1000000
#define KAYAK_MAX_STAT				0x1000000

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
#define WAKE_VELOCITY 		4
#define KAYAK_X				128
#define KAYAK_Z				128
#define KAYAK_MAX_KICK		-80
#define KAYAK_MIN_BOUNCE	((MAX_VELOCITY / 2) / 256)

#define KAYAK_IN_FORWARD	IN_FORWARD
#define KAYAK_IN_BACK		IN_BACK
#define KAYAK_IN_LEFT		IN_LEFT
#define KAYAK_IN_RIGHT		IN_RIGHT
#define KAYAK_IN_HOLD		IN_WALK
#define KAYAK_IN_HOLD_LEFT	IN_LSTEP
#define KAYAK_IN_HOLD_RIGHT	IN_RSTEP
#define KAYAK_IN_DISMOUNT	(IN_JUMP | IN_ROLL)

enum KayakState
{
	KAYAK_STATE_BACK = 0,
	KAYAK_STATE_IDLE = 1,
	KAYAK_STATE_TURN_LEFT = 2,
	KAYAK_STATE_TURN_RIGHT = 3,
	KAYAK_STATE_MOUNT_LEFT = 4,
	KAYAK_STATE_IDLE_DEATH = 5,
	KAYAK_STATE_FORWARD = 6,
	KAYAK_STATE_CAPSIZE_RECOVER = 7,	// Unused.
	KAYAK_STATE_CAPSIZE_DEATH = 8,		// Unused.
	KAYAK_STATE_DISMOUNT = 9,
	KAYAK_STATE_HOLD_LEFT = 10,
	KAYAK_STATE_HOLD_RIGHT = 11,
	KAYAK_STATE_MOUNT_RIGHT = 12,
	KAYAK_STATE_DISMOUNT_LEFT = 13,
	KAYAK_STATE_DISMOUNT_RIGHT = 14,
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

enum class KayakMountType
{
	None,
	Left,
	Right
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

WAKE_PTS WakePts[NUM_WAKE_SPRITES][2];

void InitialiseKayak(short itemNumber)
{
	auto* kayakItem = &g_Level.Items[itemNumber];
	kayakItem->Data = KayakInfo();
	auto* kayak = (KayakInfo*)kayakItem->Data;

	kayak->TurnRate = 0;
	kayak->Velocity = 0;
	kayak->FrontVerticalVelocity = 0;
	kayak->LeftVerticalVelocity = 0;
	kayak->RightVerticalVelocity = 0;
	kayak->LeftRightCount = 0;
	kayak->OldPos = kayakItem->Pose;
	kayak->CurrentStartWake = 0;
	kayak->WakeShade = 0;
	kayak->Flags = 0;

	for (int i = 0; i < NUM_WAKE_SPRITES; i++)
	{
		WakePts[i][0].life = 0;
		WakePts[i][1].life = 0;
	}
}

void KayakDraw(ItemInfo* kayakItem)
{
	DrawAnimatingItem(kayakItem);
}

void KayakDoWake(ItemInfo* kayakItem, int xOffset, int zOffset, short rotate)
{
	auto* kayak = (KayakInfo*)kayakItem->Data;

	if (WakePts[kayak->CurrentStartWake][rotate].life)
		return;

	float s = phd_sin(kayakItem->Pose.Orientation.y);
	float c = phd_cos(kayakItem->Pose.Orientation.y);

	int x = kayakItem->Pose.Position.x + zOffset * s + xOffset * c;
	int z = kayakItem->Pose.Position.z + zOffset * c - xOffset * s;

	int probedRoomNum = GetCollision(x, kayakItem->Pose.Position.y, z, kayakItem->RoomNumber).RoomNumber;
	int waterHeight = GetWaterHeight(x, kayakItem->Pose.Position.y, z, probedRoomNum);

	if (waterHeight != NO_HEIGHT)
	{
		short angle1, angle2;
		if (kayakItem->Animation.Velocity < 0)
		{
			if (!rotate)
			{
				angle1 = kayakItem->Pose.Orientation.y - ANGLE(10.0f);
				angle2 = kayakItem->Pose.Orientation.y - ANGLE(30.0f);
			}
			else
			{
				angle1 = kayakItem->Pose.Orientation.y + ANGLE(10.0f);
				angle2 = kayakItem->Pose.Orientation.y + ANGLE(30.0f);
			}
		}
		else
		{
			if (!rotate)
			{
				angle1 = kayakItem->Pose.Orientation.y - ANGLE(170.0f);
				angle2 = kayakItem->Pose.Orientation.y - ANGLE(150.0f);
			}
			else
			{
				angle1 = kayakItem->Pose.Orientation.y + ANGLE(170.0f);
				angle2 = kayakItem->Pose.Orientation.y + ANGLE(150.0f);
			}
		}

		int xv[2], zv[2];
		xv[0] = WAKE_VELOCITY * phd_sin(angle1);
		zv[0] = WAKE_VELOCITY * phd_cos(angle1);
		xv[1] = (WAKE_VELOCITY + 2) * phd_sin(angle2);
		zv[1] = (WAKE_VELOCITY + 2) * phd_cos(angle2);

		WakePts[kayak->CurrentStartWake][rotate].y = kayakItem->Pose.Position.y + KAYAK_DRAW_SHIFT;
		WakePts[kayak->CurrentStartWake][rotate].life = 0x40;

		for (int i = 0; i < 2; i++)
		{
			WakePts[kayak->CurrentStartWake][rotate].x[i] = x;
			WakePts[kayak->CurrentStartWake][rotate].z[i] = z;
			WakePts[kayak->CurrentStartWake][rotate].xvel[i] = xv[i];
			WakePts[kayak->CurrentStartWake][rotate].zvel[i] = zv[i];
		}

		if (rotate == 1)
		{
			kayak->CurrentStartWake++;
			kayak->CurrentStartWake &= (NUM_WAKE_SPRITES - 1);
		}
	}
}

void KayakDoRipple(ItemInfo* kayakItem, int xOffset, int zOffset)
{
	float s = phd_sin(kayakItem->Pose.Orientation.y);
	float c = phd_cos(kayakItem->Pose.Orientation.y);

	int x = kayakItem->Pose.Position.x + zOffset * s + xOffset * c;
	int z = kayakItem->Pose.Position.z + zOffset * c - xOffset * s;

	int probedRoomNum = GetCollision(x, kayakItem->Pose.Position.y, z, kayakItem->RoomNumber).RoomNumber;
	int waterHeight = GetWaterHeight(x, kayakItem->Pose.Position.y, z, probedRoomNum);

	//if (waterHeight != NO_HEIGHT)
	//	SetupRipple(x, kayakItem->Pose.Position.y, z, -2 - (GetRandomControl() & 1), 0, Objects[ID_KAYAK_PADDLE_TRAIL_SPRITE].meshIndex,TO_RAD(kayakItem->Pose.Orientation.y));
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

KayakMountType KayakGetMountType(ItemInfo* laraItem, short itemNumber)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* kayakItem = &g_Level.Items[itemNumber];

	if (!(TrInput & IN_ACTION) ||
		lara->Control.HandStatus != HandStatus::Free ||
		laraItem->Animation.Airborne)
	{
		return KayakMountType::None;
	}

	int distance = pow(laraItem->Pose.Position.x - kayakItem->Pose.Position.x, 2) + pow(laraItem->Pose.Position.z - kayakItem->Pose.Position.z, 2);
	if (distance > pow(360, 2))
		return KayakMountType::None;

	auto probe = GetCollision(kayakItem);
	if (probe.Position.Floor > -32000)
	{
		short angle = phd_atan(kayakItem->Pose.Position.z - laraItem->Pose.Position.z, kayakItem->Pose.Position.x - laraItem->Pose.Position.x);
		angle -= kayakItem->Pose.Orientation.y;

		int deltaAngle = laraItem->Pose.Orientation.y - kayakItem->Pose.Orientation.y;
		if (angle > -ANGLE(45.0f) && angle < ANGLE(135.0f))
		{
			deltaAngle = laraItem->Pose.Orientation.y - kayakItem->Pose.Orientation.y;
			if (deltaAngle > ANGLE(45.0f) && deltaAngle < ANGLE(135.0f))
				return KayakMountType::Left;
		}
		else
		{
			deltaAngle = laraItem->Pose.Orientation.y - kayakItem->Pose.Orientation.y;
			if (deltaAngle > ANGLE(225.0f) && deltaAngle < ANGLE(315.0f))
				return KayakMountType::Right;
		}
	}

	return KayakMountType::None;
}

int KayakGetCollisionAnim(ItemInfo* kayakItem, int xDiff, int zDiff)
{
	xDiff = kayakItem->Pose.Position.x - xDiff;
	zDiff = kayakItem->Pose.Position.z - zDiff;

	if ((xDiff) || (zDiff))
	{
		float s = phd_sin(kayakItem->Pose.Orientation.y);
		float c = phd_cos(kayakItem->Pose.Orientation.y);

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

int KayakDoDynamics(int height, int verticalVelocity, int* y)
{
	if (height > * y)
	{
		*y += verticalVelocity;

		if (*y > height)
		{
			*y = height;
			verticalVelocity = 0;
		}
		else
			verticalVelocity += GRAVITY;
	}
	else
	{
		int kick = (height - *y) * 4;

		if (kick < KAYAK_MAX_KICK)
			kick = KAYAK_MAX_KICK;

		verticalVelocity += (kick - verticalVelocity) / 8;

		if (*y > height)
			*y = height;
	}

	return verticalVelocity;
}

void KayakDoCurrent(ItemInfo* laraItem, ItemInfo* kayakItem)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* room = &g_Level.Rooms[kayakItem->RoomNumber];

	if (!lara->WaterCurrentActive)
	{
		int absVelocity = abs(lara->WaterCurrentPull.x);
		int shift;

		if (absVelocity > 16)
			shift = 4;
		else if (absVelocity > 8)
			shift = 3;
		else
			shift = 2;

		lara->WaterCurrentPull.x -= lara->WaterCurrentPull.x >> shift;

		if (abs(lara->WaterCurrentPull.x) < 4)
			lara->WaterCurrentPull.x = 0;

		absVelocity = abs(lara->WaterCurrentPull.z);
		if (absVelocity > 16)
			shift = 4;
		else if (absVelocity > 8)
			shift = 3;
		else
			shift = 2;

		lara->WaterCurrentPull.z -= lara->WaterCurrentPull.z >> shift;
		if (abs(lara->WaterCurrentPull.z) < 4)
			lara->WaterCurrentPull.z = 0;

		if (lara->WaterCurrentPull.x == 0 && lara->WaterCurrentPull.z == 0)
			return;
	}
	else
	{
		int sinkval = lara->WaterCurrentActive - 1;
		
		auto target = Vector3Int(g_Level.Sinks[sinkval].x, g_Level.Sinks[sinkval].y, g_Level.Sinks[sinkval].z);
		int angle = (((mGetAngle(target.x, target.z, laraItem->Pose.Position.x, laraItem->Pose.Position.z) - ANGLE(90.0f))) / 16) & 4095;

		int dx = target.x - laraItem->Pose.Position.x;
		int dz = target.z - laraItem->Pose.Position.z;

		int velocity = g_Level.Sinks[sinkval].strength;
		dx = phd_sin(angle * 16) * velocity * 1024;
		dz = phd_cos(angle * 16) * velocity * 1024;

		lara->WaterCurrentPull.x += (dx - lara->WaterCurrentPull.x) / 16;
		lara->WaterCurrentPull.z += (dz - lara->WaterCurrentPull.z) / 16;
	}

	kayakItem->Pose.Position.x += lara->WaterCurrentPull.x / 256;
	kayakItem->Pose.Position.z += lara->WaterCurrentPull.z / 256;

	lara->WaterCurrentActive = 0;
}

int KayakTestHeight(ItemInfo* kayakItem, int x, int z, Vector3Int* pos)
{
	Matrix world =
		Matrix::CreateFromYawPitchRoll(TO_RAD(kayakItem->Pose.Orientation.y), TO_RAD(kayakItem->Pose.Orientation.x), TO_RAD(kayakItem->Pose.Orientation.z)) *
		Matrix::CreateTranslation(kayakItem->Pose.Position.x, kayakItem->Pose.Position.y, kayakItem->Pose.Position.z);

	Vector3 vec = Vector3(x, 0, z);
	vec = Vector3::Transform(vec, world);
	
	pos->x = vec.x;
	pos->y = vec.y;
	pos->z = vec.z;

	auto probe = GetCollision(pos->x, pos->y, pos->z, kayakItem->RoomNumber);
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

bool KayakCanGetOut(ItemInfo* kayakItem, int dir)
{
	Vector3Int pos;
	int height = KayakTestHeight(kayakItem, (dir < 0) ? -DISMOUNT_DISTANCE : DISMOUNT_DISTANCE, 0, &pos);

	if ((kayakItem->Pose.Position.y - height) > 0)
		return false;

	return true;
}

int KayakDoShift(ItemInfo* kayakItem, Vector3Int* pos, Vector3Int* old)
{
	int x = pos->x / SECTOR(1);
	int z = pos->z / SECTOR(1);

	int xOld = old->x / SECTOR(1);
	int zOld = old->z / SECTOR(1);

	int xShift = pos->x & (SECTOR(1) - 1);
	int zShift = pos->z & (SECTOR(1) - 1);

	if (x == xOld)
	{
		old->x = 0;

		if (z == zOld)
		{
			kayakItem->Pose.Position.z += old->z - pos->z;
			kayakItem->Pose.Position.x += old->x - pos->x;
		}
		else if (z > zOld)
		{
			kayakItem->Pose.Position.z -= zShift + 1;
			return (pos->x - kayakItem->Pose.Position.x);
		}
		else
		{
			kayakItem->Pose.Position.z += SECTOR(1) - zShift;
			return (kayakItem->Pose.Position.x - pos->x);
		}
	}
	else if (z == zOld)
	{
		old->z = 0;

		if (x > xOld)
		{
			kayakItem->Pose.Position.x -= xShift + 1;
			return (kayakItem->Pose.Position.z - pos->z);
		}

		else
		{
			kayakItem->Pose.Position.x += SECTOR(1) - xShift;
			return (pos->z - kayakItem->Pose.Position.z);
		}
	}
	else
	{
		x = 0;
		z = 0;

		auto probe = GetCollision(old->x, pos->y, pos->z, kayakItem->RoomNumber);
		if (probe.Position.Floor < (old->y - CLICK(1)))
		{
			if (pos->z > old->z)
				z = -zShift - 1;
			else
				z = SECTOR(1) - zShift;
		}

		probe = GetCollision(pos->x, pos->y, old->z, kayakItem->RoomNumber);
		if (probe.Position.Floor < (old->y - CLICK(1)))
		{
			if (pos->x > old->x)
				x = -xShift - 1;
			else
				x = SECTOR(1) - xShift;
		}

		if (x && z)
		{
			kayakItem->Pose.Position.x += x;
			kayakItem->Pose.Position.z += z;
		}
		else if (x)
		{
			kayakItem->Pose.Position.x += x;

			if (x > 0)
				return (pos->z - kayakItem->Pose.Position.z);
			else
				return (kayakItem->Pose.Position.z - pos->z);
		}
		else if (z)
		{
			kayakItem->Pose.Position.z += z;

			if (z > 0)
				return (kayakItem->Pose.Position.x - pos->x);
			else
				return (pos->x - kayakItem->Pose.Position.x);
		}
		else
		{
			kayakItem->Pose.Position.x += (old->x - pos->x);
			kayakItem->Pose.Position.z += (old->z - pos->z);
		}
	}

	return 0;
}

void KayakToBackground(ItemInfo* laraItem, ItemInfo* kayakItem)
{
	auto* kayak = (KayakInfo*)kayakItem->Data;

	kayak->OldPos = kayakItem->Pose;

	Vector3Int oldPos[9];
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

	oldPos[8].x = kayakItem->Pose.Position.x;
	oldPos[8].y = kayakItem->Pose.Position.y;
	oldPos[8].z = kayakItem->Pose.Position.z;
 
	Vector3Int frontPos, leftPos, rightPos;
	int frontHeight = KayakTestHeight(kayakItem, 0, 1024, &frontPos);
	int leftHeight = KayakTestHeight(kayakItem, -KAYAK_X, KAYAK_Z, &leftPos);
	int rightHeight = KayakTestHeight(kayakItem, KAYAK_X, KAYAK_Z, &rightPos);

	kayakItem->Pose.Orientation.y += kayak->TurnRate / (USHRT_MAX + 1);
	kayakItem->Pose.Position.x += kayakItem->Animation.Velocity * phd_sin(kayakItem->Pose.Orientation.y);
	kayakItem->Pose.Position.z += kayakItem->Animation.Velocity * phd_cos(kayakItem->Pose.Orientation.y);

	KayakDoCurrent(laraItem,kayakItem);

	kayak->LeftVerticalVelocity = KayakDoDynamics(leftHeight, kayak->LeftVerticalVelocity, &leftPos.y);
	kayak->RightVerticalVelocity = KayakDoDynamics(rightHeight, kayak->RightVerticalVelocity, &rightPos.y);
	kayak->FrontVerticalVelocity = KayakDoDynamics(frontHeight, kayak->FrontVerticalVelocity, &frontPos.y);

	kayakItem->Animation.VerticalVelocity = KayakDoDynamics(kayak->WaterHeight, kayakItem->Animation.VerticalVelocity, &kayakItem->Pose.Position.y);

	int height2 = (leftPos.y + rightPos.y) / 2;
	int x = phd_atan(1024, kayakItem->Pose.Position.y - frontPos.y);
	int z = phd_atan(KAYAK_X, height2 - leftPos.y);

	kayakItem->Pose.Orientation.x = x;
	kayakItem->Pose.Orientation.z = z;

	int xOld = kayakItem->Pose.Position.x;
	int zOld = kayakItem->Pose.Position.z;

	int rot = 0;
	Vector3Int pos;

	if ((height2 = KayakTestHeight(kayakItem, 0, -CLICK(2.5f), &pos)) < (oldPos[7].y - KAYAK_COLLIDE))
		rot = KayakDoShift(kayakItem, &pos, &oldPos[7]);

	if ((height2 = KayakTestHeight(kayakItem, CLICK(0.5f), -CLICK(1.25f), &pos)) < (oldPos[6].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[6]);

	if ((height2 = KayakTestHeight(kayakItem, -CLICK(0.5f), -CLICK(1.25f), &pos)) < (oldPos[5].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[5]);

	if ((height2 = KayakTestHeight(kayakItem, CLICK(0.5f), CLICK(0.5f), &pos)) < (oldPos[4].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[4]);

	if ((height2 = KayakTestHeight(kayakItem, -CLICK(0.5f), CLICK(0.5f), &pos)) < (oldPos[3].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[3]);

	if ((height2 = KayakTestHeight(kayakItem, 96, CLICK(2), &pos)) < (oldPos[2].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[2]);

	if ((height2 = KayakTestHeight(kayakItem, -96, CLICK(2), &pos)) < (oldPos[1].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[1]);

	if ((height2 = KayakTestHeight(kayakItem, 0, CLICK(4), &pos)) < (oldPos[0].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[0]);

	kayakItem->Pose.Orientation.y += rot;

	auto probe = GetCollision(kayakItem);
	int probedRoomNum = probe.RoomNumber;

	height2 = GetWaterHeight(kayakItem->Pose.Position.x, kayakItem->Pose.Position.y, kayakItem->Pose.Position.z, probedRoomNum);
	if (height2 == NO_HEIGHT)
		height2 = probe.Position.Floor;

	if (height2 < (kayakItem->Pose.Position.y - KAYAK_COLLIDE))
		KayakDoShift(kayakItem, (Vector3Int*)&kayakItem->Pose, &oldPos[8]);

	probe = GetCollision(kayakItem);
	probedRoomNum = probe.RoomNumber;

	height2 = GetWaterHeight(kayakItem->Pose.Position.x, kayakItem->Pose.Position.y, kayakItem->Pose.Position.z, probedRoomNum);
	if (height2 == NO_HEIGHT)
		height2 = probe.Position.Floor;

	if (height2 == NO_HEIGHT)
	{
		GameVector kayakPos;
		kayakPos.x = kayak->OldPos.Position.x;
		kayakPos.y = kayak->OldPos.Position.y;
		kayakPos.z = kayak->OldPos.Position.z;
		kayakPos.roomNumber = kayakItem->RoomNumber;

		CameraCollisionBounds(&kayakPos, 256, 0);
		{
			kayakItem->Pose.Position.x = kayakPos.x;
			kayakItem->Pose.Position.y = kayakPos.y;
			kayakItem->Pose.Position.z = kayakPos.z;
			kayakItem->RoomNumber = kayakPos.roomNumber;
		}
	}

	int collide = KayakGetCollisionAnim(kayakItem, xOld, zOld);

	int slip = 0; // Remnant?
	if (slip || collide)
	{
		int newVelocity;

		newVelocity = (kayakItem->Pose.Position.z - oldPos[8].z) * phd_cos(kayakItem->Pose.Orientation.y) + (kayakItem->Pose.Position.x - oldPos[8].x) * phd_sin(kayakItem->Pose.Orientation.y);
		newVelocity *= 256;

		if (slip)
		{
			if (kayak->Velocity <= MAX_VELOCITY)
				kayak->Velocity = newVelocity;
		}
		else
		{
			if (kayak->Velocity > 0 && newVelocity < kayak->Velocity)
				kayak->Velocity = newVelocity;

			else if (kayak->Velocity < 0 && newVelocity > kayak->Velocity)
				kayak->Velocity = newVelocity;
		}

		if (kayak->Velocity < -MAX_VELOCITY)
			kayak->Velocity = -MAX_VELOCITY;
	}
}

void KayakUserInput(ItemInfo* laraItem, ItemInfo* kayakItem)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* kayak = (KayakInfo*)kayakItem->Data;

	if (laraItem->HitPoints <= 0 &&
		laraItem->Animation.ActiveState != KAYAK_STATE_IDLE_DEATH)
	{
		laraItem->Animation.AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_IDLE_DEATH;
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		laraItem->Animation.ActiveState = laraItem->Animation.TargetState = KAYAK_STATE_IDLE_DEATH;
	}

	int frame = laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

	switch (laraItem->Animation.ActiveState)
	{
	case KAYAK_STATE_IDLE:
		if (TrInput & KAYAK_IN_DISMOUNT &&
			!lara->WaterCurrentActive &&
			!lara->WaterCurrentPull.x && !lara->WaterCurrentPull.z)
		{
			if (TrInput & KAYAK_IN_LEFT && !(TrInput & KAYAK_IN_HOLD) && KayakCanGetOut(kayakItem, -1))
			{
				laraItem->Animation.TargetState = KAYAK_STATE_DISMOUNT;
				laraItem->Animation.RequiredState = KAYAK_STATE_DISMOUNT_LEFT;
			}
			else if (TrInput & KAYAK_IN_RIGHT && !(TrInput & KAYAK_IN_HOLD) && KayakCanGetOut(kayakItem, 1))
			{
				laraItem->Animation.TargetState = KAYAK_STATE_DISMOUNT;
				laraItem->Animation.RequiredState = KAYAK_STATE_DISMOUNT_RIGHT;
			}
		}
		else if (TrInput & KAYAK_IN_FORWARD)
		{
			laraItem->Animation.TargetState = KAYAK_STATE_TURN_RIGHT;
			kayak->Turn = false;
			kayak->Forward = true;
		}
		else if (TrInput & KAYAK_IN_BACK)
			laraItem->Animation.TargetState = KAYAK_STATE_BACK;
		else if (TrInput & KAYAK_IN_LEFT && !(TrInput & KAYAK_IN_HOLD))
		{
			laraItem->Animation.TargetState = KAYAK_STATE_TURN_LEFT;

			if (kayak->Velocity)
				kayak->Turn = false;
			else
				kayak->Turn = true;

			kayak->Forward = false;
		}

		else if (TrInput & KAYAK_IN_RIGHT && !(TrInput & KAYAK_IN_HOLD))
		{
			laraItem->Animation.TargetState = KAYAK_STATE_TURN_RIGHT;

			if (kayak->Velocity)
				kayak->Turn = false;
			else
				kayak->Turn = true;

			kayak->Forward = false;
		}
		else if ((TrInput & KAYAK_IN_HOLD_LEFT || (TrInput & KAYAK_IN_HOLD && TrInput & KAYAK_IN_LEFT)) &&
			(kayak->Velocity ||
				lara->WaterCurrentPull.x || lara->WaterCurrentPull.z))
		{
			laraItem->Animation.TargetState = KAYAK_STATE_HOLD_LEFT;
		}
		else if ((TrInput & KAYAK_IN_HOLD_RIGHT || (TrInput & KAYAK_IN_HOLD && TrInput & KAYAK_IN_RIGHT)) &&
			(kayak->Velocity ||
				lara->WaterCurrentPull.x || lara->WaterCurrentPull.z))
		{
			laraItem->Animation.TargetState = KAYAK_STATE_HOLD_RIGHT;
		}

		break;
		
	case KAYAK_STATE_TURN_LEFT:
		if (kayak->Forward)
		{
			if (!frame)
				kayak->LeftRightCount = 0;

			// TODO: Sort out the bitwise operations.
			if (frame == 2 && !(kayak->LeftRightCount & 0x80))
				kayak->LeftRightCount++;

			else if (frame > 2)
				kayak->LeftRightCount &= ~0x80;

			if (TrInput & KAYAK_IN_FORWARD)
			{
				if (TrInput & KAYAK_IN_LEFT && !(TrInput & KAYAK_IN_HOLD))
				{
					if ((kayak->LeftRightCount & ~0x80) >= 2)
						laraItem->Animation.TargetState = KAYAK_STATE_TURN_RIGHT;
				}
				else
					laraItem->Animation.TargetState = KAYAK_STATE_TURN_RIGHT;
			}
			else
				laraItem->Animation.TargetState = KAYAK_STATE_IDLE;
		}
		else if (!(TrInput & KAYAK_IN_LEFT))
			laraItem->Animation.TargetState = KAYAK_STATE_IDLE;

		if (frame == 7)
		{
			if (kayak->Forward)
			{
				kayak->TurnRate -= KAYAK_FORWARD_ROTATION;
				if (kayak->TurnRate < -KAYAK_MAX_TURN)
					kayak->TurnRate = -KAYAK_MAX_TURN;

				kayak->Velocity += KAYAK_FORWARD_VELOCITY;
			}
			else if (kayak->Turn)
			{
				kayak->TurnRate -= KAYAK_HARD_ROTATION;
				if (kayak->TurnRate < -KAYAK_MAX_STAT)
					kayak->TurnRate = -KAYAK_MAX_STAT;
			}
			else
			{
				kayak->TurnRate -= KAYAK_LEFT_RIGHT_ROTATION;
				if (kayak->TurnRate < -KAYAK_MAX_LEFT_RIGHT)
					kayak->TurnRate = -KAYAK_MAX_LEFT_RIGHT;

				kayak->Velocity += KAYAK_LEFT_RIGHT_VELOCITY;
			}
		}

		if (frame > 6 && frame < 24 && frame & 1)
			KayakDoRipple(kayakItem, -CLICK(1.5f), -CLICK(0.25f));

		break;
		
	case KAYAK_STATE_TURN_RIGHT:	
		if (kayak->Forward)
		{
			if (!frame)
				kayak->LeftRightCount = 0;

			if (frame == 2 && !(kayak->LeftRightCount & 0x80))
				kayak->LeftRightCount++;

			else if (frame > 2)
				kayak->LeftRightCount &= ~0x80;

			if (TrInput & KAYAK_IN_FORWARD)
			{
				if (TrInput & KAYAK_IN_RIGHT && !(TrInput & KAYAK_IN_HOLD))
				{
					if ((kayak->LeftRightCount & ~0x80) >= 2)
						laraItem->Animation.TargetState = KAYAK_STATE_TURN_LEFT;
				}
				else
					laraItem->Animation.TargetState = KAYAK_STATE_TURN_LEFT;
			}
			else
				laraItem->Animation.TargetState = KAYAK_STATE_IDLE;
		}

		else if (!(TrInput & KAYAK_IN_RIGHT))
			laraItem->Animation.TargetState = KAYAK_STATE_IDLE;

		if (frame == 7)
		{
			if (kayak->Forward)
			{
				kayak->TurnRate += KAYAK_FORWARD_ROTATION;
				if (kayak->TurnRate > KAYAK_MAX_TURN)
					kayak->TurnRate = KAYAK_MAX_TURN;

				kayak->Velocity += KAYAK_FORWARD_VELOCITY;
			}
			else if (kayak->Turn)
			{
				kayak->TurnRate += KAYAK_HARD_ROTATION;
				if (kayak->TurnRate > KAYAK_MAX_STAT)
					kayak->TurnRate = KAYAK_MAX_STAT;
			}
			else
			{
				kayak->TurnRate += KAYAK_LEFT_RIGHT_ROTATION;
				if (kayak->TurnRate > KAYAK_MAX_LEFT_RIGHT)
					kayak->TurnRate = KAYAK_MAX_LEFT_RIGHT;

				kayak->Velocity += KAYAK_LEFT_RIGHT_VELOCITY;
			}
		}

		if (frame > 6 && frame < 24 && frame & 1)
			KayakDoRipple(kayakItem, CLICK(1.5f), -CLICK(0.25f));

		break;
		
	case KAYAK_STATE_BACK:
		if (!(TrInput & KAYAK_IN_BACK))
			laraItem->Animation.TargetState = KAYAK_STATE_IDLE;

		if ((laraItem->Animation.AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_ANIM_PADDLE_BACK)
		{
			if (frame == 8)
			{
				kayak->TurnRate += KAYAK_FORWARD_ROTATION;
				kayak->Velocity -= KAYAK_FORWARD_VELOCITY;
			}

			if (frame == 31)
			{
				kayak->TurnRate -= KAYAK_FORWARD_ROTATION;
				kayak->Velocity -= KAYAK_FORWARD_VELOCITY;
			}

			if (frame < 15 && frame & 1)
				KayakDoRipple(kayakItem, CLICK(1.5f), -CLICK(0.5f));

			else if (frame >= 20 && frame <= 34 && frame & 1)
				KayakDoRipple(kayakItem, -CLICK(1.5f), -CLICK(0.5f));
		}

		break;
		
	case KAYAK_STATE_HOLD_LEFT:
		if (!(TrInput & KAYAK_IN_HOLD_LEFT || (TrInput & KAYAK_IN_HOLD && TrInput & KAYAK_IN_LEFT)) ||
			(!kayak->Velocity &&
				!lara->WaterCurrentPull.x &&
				!lara->WaterCurrentPull.z))
		{
			laraItem->Animation.TargetState = KAYAK_STATE_IDLE;
		}
		else if ((laraItem->Animation.AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_ANIM_HOLD_PADDLE_LEFT)
		{
			if (kayak->Velocity >= 0)
			{
				kayak->TurnRate -= KAYAK_TURN_ROTATION;
				if (kayak->TurnRate < -KAYAK_MAX_TURN)
					kayak->TurnRate = -KAYAK_MAX_TURN;

				kayak->Velocity += -KAYAK_TURN_BRAKE;
				if (kayak->Velocity < 0)
					kayak->Velocity = 0;
			}

			if (kayak->Velocity < 0)
			{
				kayak->TurnRate += KAYAK_TURN_ROTATION;

				kayak->Velocity += KAYAK_TURN_BRAKE;
				if (kayak->Velocity > 0)
					kayak->Velocity = 0;
			}

			if (!(Wibble & 3))
				KayakDoRipple(kayakItem, -CLICK(1), -CLICK(1));
		}

		break;
		
	case KAYAK_STATE_HOLD_RIGHT:
		if (!(TrInput & KAYAK_IN_HOLD_RIGHT || (TrInput & KAYAK_IN_HOLD && TrInput & KAYAK_IN_RIGHT)) ||
			(!kayak->Velocity &&
				!lara->WaterCurrentPull.x &&
				!lara->WaterCurrentPull.z))
		{
			laraItem->Animation.TargetState = KAYAK_STATE_IDLE;
		}
		else if ((laraItem->Animation.AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_ANIM_HOLD_PADDLE_RIGHT)
		{
			if (kayak->Velocity >= 0)
			{
				kayak->TurnRate += KAYAK_TURN_ROTATION;
				if (kayak->TurnRate > KAYAK_MAX_TURN)
					kayak->TurnRate = KAYAK_MAX_TURN;

				kayak->Velocity += -KAYAK_TURN_BRAKE;
				if (kayak->Velocity < 0)
					kayak->Velocity = 0;
			}

			if (kayak->Velocity < 0)
			{
				kayak->TurnRate -= KAYAK_TURN_ROTATION;

				kayak->Velocity += KAYAK_TURN_BRAKE;
				if (kayak->Velocity > 0)
					kayak->Velocity = 0;
			}

			if (!(Wibble & 3))
				KayakDoRipple(kayakItem, CLICK(1), -CLICK(1));
		}

		break;
		
	case KAYAK_STATE_MOUNT_LEFT:
		if (laraItem->Animation.AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_GET_PADDLE &&
			frame == 24 &&
			!(kayak->Flags & 0x80))
		{
			kayak->Flags |= 0x80;
			lara->MeshPtrs[LM_RHAND] = Objects[ID_KAYAK_LARA_ANIMS].meshIndex + LM_RHAND;
			laraItem->MeshBits &= ~LARA_LEG_BITS;
		}

		break;
		
	case KAYAK_STATE_DISMOUNT:
		if (laraItem->Animation.AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_DISMOUNT_START &&
			frame == 27 &&
			kayak->Flags & 0x80)
		{
			kayak->Flags &= ~0x80;
			lara->MeshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
			laraItem->MeshBits |= LARA_LEG_BITS;
		}

		laraItem->Animation.TargetState = laraItem->Animation.RequiredState;
		break;
		
	case KAYAK_STATE_DISMOUNT_LEFT:
		if (laraItem->Animation.AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_DISMOUNT_LEFT &&
			frame == 83)
		{
			Vector3Int vec = { 0, 350, 500 };
			GetLaraJointPosition(&vec, LM_HIPS);

			SetAnimation(laraItem, LA_JUMP_FORWARD);
			laraItem->Pose.Position.x = vec.x;
			laraItem->Pose.Position.y = vec.y;
			laraItem->Pose.Position.z = vec.z;
			laraItem->Pose.Orientation.x = 0;
			laraItem->Pose.Orientation.y = kayakItem->Pose.Orientation.y - ANGLE(90.0f);
			laraItem->Pose.Orientation.z = 0;
			laraItem->Animation.Velocity = 40;
			laraItem->Animation.VerticalVelocity = -50;
			laraItem->Animation.Airborne = true;
			lara->Control.HandStatus = HandStatus::Free;
			lara->Vehicle = NO_ITEM;
			kayak->LeftRightCount = 0;
		}

		break;
		
	case KAYAK_STATE_DISMOUNT_RIGHT:
		if (laraItem->Animation.AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_DISMOUNT_RIGHT &&
			frame == 83)
		{
			Vector3Int vec = { 0, 350, 500 };
			GetLaraJointPosition(&vec, LM_HIPS);

			SetAnimation(laraItem, LA_JUMP_FORWARD);
			laraItem->Pose.Position.x = vec.x;
			laraItem->Pose.Position.y = vec.y;
			laraItem->Pose.Position.z = vec.z;
			laraItem->Pose.Orientation.x = 0;
			laraItem->Pose.Orientation.y = kayakItem->Pose.Orientation.y + ANGLE(90.0f);
			laraItem->Pose.Orientation.z = 0;
			laraItem->Animation.Velocity = 40;
			laraItem->Animation.VerticalVelocity = -50;
			laraItem->Animation.Airborne = true;
			lara->Control.HandStatus = HandStatus::Free;
			lara->Vehicle = NO_ITEM;
			kayak->LeftRightCount = 0;
		}
	}

	if (kayak->Velocity > 0)
	{
		kayak->Velocity -= KAYAK_FRICTION;
		if (kayak->Velocity < 0)
			kayak->Velocity = 0;
	}
	else if (kayak->Velocity < 0)
	{
		kayak->Velocity += KAYAK_FRICTION;
		if (kayak->Velocity > 0)
			kayak->Velocity = 0;
	}

	if (kayak->Velocity > MAX_VELOCITY)
		kayak->Velocity = MAX_VELOCITY;
	else if (kayak->Velocity < -MAX_VELOCITY)
		kayak->Velocity = -MAX_VELOCITY;

	kayakItem->Animation.Velocity = (kayak->Velocity / (USHRT_MAX + 1));
	;
	if (kayak->TurnRate >= 0)
	{
		kayak->TurnRate -= KAYAK_ROTATE_FRICTION;
		if (kayak->TurnRate < 0)
			kayak->TurnRate = 0;
	}
	else if (kayak->TurnRate < 0)
	{
		kayak->TurnRate += KAYAK_ROTATE_FRICTION;
		if (kayak->TurnRate > 0)
			kayak->TurnRate = 0;
	}
}

void KayakToItemCollision(ItemInfo* laraItem, ItemInfo* kayakItem)
{
	short roomsToCheck[128];
	short numRoomsToCheck = 0;
	roomsToCheck[numRoomsToCheck++] = kayakItem->RoomNumber;

	auto* room = &g_Level.Rooms[kayakItem->RoomNumber];
	for (int i = 0; i < room->doors.size(); i++)
		roomsToCheck[numRoomsToCheck++] = room->doors[i].room;

	for (int i = 0; i < numRoomsToCheck; i++)
	{
		for (short currentItemNumber : g_Level.Rooms[roomsToCheck[i]].Items)
		{
			auto* item = &g_Level.Items[currentItemNumber];

			if (item->Collidable && item->Status != ITEM_INVISIBLE)
			{
				auto* object = &Objects[item->ObjectNumber];

				if (object->collision &&
					(item->ObjectNumber == ID_TEETH_SPIKES ||
						item->ObjectNumber == ID_DARTS &&
						item->Animation.ActiveState != 1))
				{
					int x = kayakItem->Pose.Position.x - item->Pose.Position.x;
					int y = kayakItem->Pose.Position.y - item->Pose.Position.y;
					int z = kayakItem->Pose.Position.z - item->Pose.Position.z;

					if (x > -2048 && x < 2048 &&
						y > -2048 && y < 2048 &&
						z > -2048 && z < 2048)
					{
						if (TestBoundsCollide(item, kayakItem, KAYAK_TO_ENTITY_RADIUS))
						{
							DoLotsOfBlood(laraItem->Pose.Position.x, laraItem->Pose.Position.y - STEP_SIZE, laraItem->Pose.Position.z, kayakItem->Animation.Velocity, kayakItem->Pose.Orientation.y, laraItem->RoomNumber, 3);
							laraItem->HitPoints -= 5;
						}
					}
				}
			}
		}
	}
}

void KayakLaraRapidsDrown(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	laraItem->Animation.AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_OVERBOARD_DEATH;
	laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
	laraItem->Animation.ActiveState = 12; // TODO
	laraItem->Animation.TargetState = 12;
	laraItem->HitPoints = 0;
	laraItem->Animation.Velocity = 0;
	laraItem->Animation.VerticalVelocity = 0;
	laraItem->Animation.Airborne = false;

	AnimateItem(laraItem);

	lara->ExtraAnim = 1;
	lara->Control.HandStatus = HandStatus::Busy;
	lara->Control.Weapon.GunType = LaraWeaponType::None;
	lara->HitDirection = -1;
}

void KayakCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* kayakItem = &g_Level.Items[itemNumber];
	auto* kayak = (KayakInfo*)kayakItem->Data;

	if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
		return;

	KayakMountType mountType = KayakGetMountType(laraItem, itemNumber);
	if (mountType != KayakMountType::None)
	{
		lara->Vehicle = itemNumber;

		if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
		{
			CreateFlare(laraItem, ID_FLARE_ITEM, 0);
			UndrawFlareMeshes(laraItem);
			lara->Flare.ControlLeft = 0;
			lara->Control.Weapon.RequestGunType = lara->Control.Weapon.GunType = LaraWeaponType::None;
		}

		if (mountType == KayakMountType::Right)
			laraItem->Animation.AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_MOUNT_RIGHT;
		else if (mountType == KayakMountType::Left)
			laraItem->Animation.AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_MOUNT_LEFT;

		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		laraItem->Animation.ActiveState = laraItem->Animation.TargetState = KAYAK_STATE_MOUNT_LEFT;
		laraItem->Pose.Position.x = kayakItem->Pose.Position.x;
		laraItem->Pose.Position.y = kayakItem->Pose.Position.y;
		laraItem->Pose.Position.z = kayakItem->Pose.Position.z;
		laraItem->Pose.Orientation.x = 0;
		laraItem->Pose.Orientation.y = kayakItem->Pose.Orientation.y;
		laraItem->Pose.Orientation.z = 0;
		laraItem->Animation.Velocity = 0;
		laraItem->Animation.VerticalVelocity = 0;
		laraItem->Animation.Airborne = false;
		lara->Control.WaterStatus = WaterStatus::Dry;

		if (laraItem->RoomNumber != kayakItem->RoomNumber)
			ItemNewRoom(lara->ItemNumber, kayakItem->RoomNumber);

		AnimateItem(laraItem);

		kayak->WaterHeight = kayakItem->Pose.Position.y;
		kayak->Flags = 0;
	}
	else
	{
		coll->Setup.EnableObjectPush = true;
		ObjectCollision(itemNumber, laraItem, coll);
	}
}

bool KayakControl(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* kayakItem = &g_Level.Items[lara->Vehicle];
	auto* kayak = (KayakInfo*)kayakItem->Data;

	if (TrInput & IN_LOOK)
		LookUpDown(laraItem);

	int ofs = kayakItem->Animation.VerticalVelocity;

	KayakUserInput(laraItem, kayakItem);
	KayakToBackground(laraItem, kayakItem);
	TestTriggers(kayakItem, false);

	auto probe = GetCollision(kayakItem);
	int water = GetWaterHeight(kayakItem->Pose.Position.x, kayakItem->Pose.Position.y, kayakItem->Pose.Position.z, probe.RoomNumber);
	kayak->WaterHeight = water;

	if (kayak->WaterHeight == NO_HEIGHT)
	{
		water = probe.Position.Floor;
		kayak->WaterHeight = water;
		kayak->TrueWater = false;
	}
	else
	{
		kayak->WaterHeight -= 5;
		kayak->TrueWater = true;
	}

	if ((ofs - kayakItem->Animation.VerticalVelocity) > 128 &&
		kayakItem->Animation.VerticalVelocity == 0 &&
		water != NO_HEIGHT)
	{
		int damage = ofs - kayakItem->Animation.VerticalVelocity;
		if (damage > 160)
			laraItem->HitPoints -= (damage - 160) * 8;
	}

	if (lara->Vehicle != NO_ITEM)
	{
		if (kayakItem->RoomNumber != probe.RoomNumber)
		{
			ItemNewRoom(lara->Vehicle, probe.RoomNumber);
			ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
		}

		laraItem->Pose.Position.x = kayakItem->Pose.Position.x;
		laraItem->Pose.Position.y = kayakItem->Pose.Position.y;
		laraItem->Pose.Position.z = kayakItem->Pose.Position.z;
		laraItem->Pose.Orientation.x = kayakItem->Pose.Orientation.x;
		laraItem->Pose.Orientation.y = kayakItem->Pose.Orientation.y;
		laraItem->Pose.Orientation.z = kayakItem->Pose.Orientation.z / 2;

		AnimateItem(laraItem);

		kayakItem->Animation.AnimNumber = Objects[ID_KAYAK].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex);
		kayakItem->Animation.FrameNumber = g_Level.Anims[kayakItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);

		Camera.targetElevation = -ANGLE(30.0f);
		Camera.targetDistance = CLICK(8);
	}

	if (!(Wibble & 15) && kayak->TrueWater)
	{
		KayakDoWake(kayakItem, -CLICK(0.5f), 0, 0);
		KayakDoWake(kayakItem, CLICK(0.5f), 0, 1);
	}

	if (Wibble & 7)
	{
		if (!kayak->TrueWater && kayakItem->Animation.VerticalVelocity < 20)
		{
			Vector3Int dest;
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

	if (!kayakItem->Animation.Velocity &&
		!lara->WaterCurrentPull.x &&
		!lara->WaterCurrentPull.z)
	{
		if (kayak->WakeShade)
			kayak->WakeShade--;
	}
	else
	{
		if (kayak->WakeShade < 16)
			kayak->WakeShade++;
	}

	KayakUpdateWakeFX();
	KayakToItemCollision(laraItem, kayakItem);

	return (lara->Vehicle != NO_ITEM) ? true : false;
}
