#include "framework.h"
#include "Objects/TR2/Vehicles/boat.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/particle/SimpleParticle.h"
#include "Objects/TR2/Vehicles/boat_info.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#define BOAT_UNDO_TURN		ANGLE(0.25f)
#define BOAT_TURN			(ANGLE(0.25f) / 2)
#define BOAT_MAX_TURN		ANGLE(4.0f)
#define BOAT_MAX_VELOCITY	110
#define BOAT_SLOW_SPEED		(BOAT_MAX_VELOCITY / 3)
#define BOAT_FAST_SPEED		(BOAT_MAX_VELOCITY + 75)
#define BOAT_MIN_SPEED		20
#define BOAT_ACCELERATION	5
#define BOAT_BRAKE			5
#define BOAT_SLOWDOWN		1
#define BOAT_REVERSE		-2	// -5
#define BOAT_MAX_BACK		-20
#define BOAT_MAX_KICK		-80
#define BOAT_SLIP			10
#define BOAT_SIDE_SLIP		30
#define BOAT_FRONT			750
#define BOAT_SIDE			300
#define BOAT_RADIUS			500
#define BOAT_SNOW			500
#define BOAT_MAX_HEIGHT		CLICK(1)
#define DISMOUNT_DISTANCE		SECTOR(1)
#define BOAT_WAKE			700
#define BOAT_SOUND_CEILING	SECTOR(5)
#define BOAT_TIP			(BOAT_FRONT + 250)

#define	SBOAT_IN_ACCELERATE	IN_FORWARD
#define	SBOAT_IN_REVERSE	IN_BACK
#define	SBOAT_IN_LEFT		(IN_LEFT | IN_LSTEP)
#define	SBOAT_IN_RIGHT		(IN_RIGHT | IN_RSTEP)
#define	SBOAT_IN_SPEED		(IN_ACTION | IN_SPRINT)
#define SBOAT_IN_SLOW		IN_WALK
#define	SBOAT_IN_DISMOUNT	(IN_JUMP | IN_ROLL)

enum SpeedBoatState
{
	SBOAT_STATE_MOUNT = 0,
	SBOAT_STATE_IDLE = 1,
	SBOAT_STATE_MOVING = 2,
	SBOAT_STATE_DISMOUNT_RIGHT = 3,
	SBOAT_STATE_DISMOUNT_LEFT = 4,
	SBOAT_STATE_HIT = 5,
	SBOAT_STATE_FALL = 6,
	SBOAT_STATE_TURN_RIGHT = 7,
	SBOAT_STATE_DEATH = 8,
	SBOAT_STATE_TURN_LEFT = 9
};

enum SpeedBoatAnim
{
	SBOAT_ANIM_MOUNT_LEFT = 0,
	SBOAT_ANIM_IDLE = 1,	// ?
	SBOAT_ANIM_FORWARD = 2,	// ?

	SBOAT_ANIM_DISMOUNT_LEFT = 5,
	SBOAT_ANIM_MOUNT_JUMP = 6,
	SBOAT_ANIM_DISMOUNT_RIGHT = 7,
	SBOAT_ANIM_MOUNT_RIGHT = 8,

	SBOAT_ANIM_HIT_LEFT = 11,
	SBOAT_ANIM_HIT_RIGHT = 12,
	SBOAT_ANIM_HIT_FRONT = 13,
	SBOAT_ANIM_HIT_BACK = 14,
	SBOAT_ANIM_LEAP_START = 15,
	SBOAT_ANIM_LEAP = 16,
	SBOAT_ANIM_LEAP_END = 17,
	SBOAT_ANIM_DEATH = 18
};

void InitialiseSpeedBoat(short itemNumber)
{
	auto* sBoatItem = &g_Level.Items[itemNumber];
	sBoatItem->Data = SpeedBoatInfo();
	auto* sBoat = (SpeedBoatInfo*)sBoatItem->Data;

	sBoat->TurnRate = 0;
	sBoat->LeanAngle = 0;
	sBoat->ExtraRotation = 0;
	sBoat->LeftVerticalVelocity = 0;
	sBoat->RightVerticalVelocity = 0;
	sBoat->Water = 0;
	sBoat->Pitch = 0;
}

void DoBoatWakeEffect(ITEM_INFO* sBoatItem)
{
	SetupRipple(sBoatItem->Position.xPos, sBoatItem->Position.yPos, sBoatItem->Position.zPos, 512, RIPPLE_FLAG_RAND_POS, Objects[1368].meshIndex, TO_RAD(sBoatItem->Position.yRot));
	TEN::Effects::TriggerSpeedboatFoam(sBoatItem);

	// OLD WAKE EFFECT
	/*int c = phd_cos(boat->pos.yRot);
	int s = phd_sin(boat->pos.yRot);
	int c = phd_cos(boat->pos.yRot);
	
	for (int i = 0; i < 3; i++)
	{
		int h = BOAT_WAKE;
		int w = (1 - i) * BOAT_SIDE;
		int x = boat->pos.xPos + (-(c * w) - (h * s) >> W2V_SHIFT);
		int y = boat->pos.yPos;
		int z = boat->pos.zPos + ((s * w) - (h * c) >> W2V_SHIFT);

		SPARKS* spark = &Sparks[GetFreeSpark()];
		spark->on = 1;
		spark->sR = 64;
		spark->sG = 64;
		spark->sB = 64;
		spark->dR = 64;
		spark->dG = 64;
		spark->dB = 64;
		spark->colFadeSpeed = 1;
		spark->transType = TransTypeEnum::COLADD;
		spark->life = spark->sLife = (GetRandomControl() & 3) + 6;
		spark->fadeToBlack = spark->life - 4;
		spark->x = (BOAT_SIDE * phd_sin(boat->pos.yRot) >> W2V_SHIFT) + (GetRandomControl() & 128) + x - 8;
		spark->y = (GetRandomControl() & 0xF) + y - 8;
		spark->z = (BOAT_SIDE * phd_cos(boat->pos.yRot) >> W2V_SHIFT) + (GetRandomControl() & 128) + z - 8;
		spark->xVel = 0;
		spark->zVel = 0;
		spark->friction = 0;
		spark->flags = 538;
		spark->yVel = (GetRandomControl() & 0x7F) - 256;
		spark->rotAng = GetRandomControl() & 0xFFF;
		spark->scalar = 3;
		spark->maxYvel = 0;
		spark->rotAdd = (GetRandomControl() & 0x1F) - 16;
		spark->gravity = -spark->yVel >> 2;
		spark->sSize = spark->size = ((GetRandomControl() & 3) + 16) * 16;
		spark->dSize = 2 * spark->size;

		spark = &Sparks[GetFreeSpark()];
		spark->on = 1;
		spark->sR = 64;
		spark->sG = 64;
		spark->sB = 64;
		spark->dR = 64;
		spark->dG = 64;
		spark->dB = 64;
		spark->colFadeSpeed = 1;
		spark->transType = TransTypeEnum::COLADD;
		spark->life = spark->sLife = (GetRandomControl() & 3) + 6;
		spark->fadeToBlack = spark->life - 4;		
		spark->x = (BOAT_SIDE * phd_sin(boat->pos.yRot) >> W2V_SHIFT) + (GetRandomControl() & 128) + x - 8;
		spark->y = (GetRandomControl() & 0xF) + y - 8;
		spark->z = (BOAT_SIDE * phd_cos(boat->pos.yRot) >> W2V_SHIFT) + (GetRandomControl() & 128) + z - 8;
		spark->xVel = 0;
		spark->zVel = 0;
		spark->friction = 0;
		spark->flags = 538;
		spark->yVel = (GetRandomControl() & 0x7F) - 256;
		spark->rotAng = GetRandomControl() & 0xFFF;
		spark->scalar = 3;
		spark->maxYvel = 0;
		spark->rotAdd = (GetRandomControl() & 0x1F) - 16;
		spark->gravity = -spark->yVel >> 2;
		spark->sSize = spark->size = ((GetRandomControl() & 3) + 16) * 4;
		spark->dSize = 2 * spark->size;
		spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 17;
	}*/
}

BoatMountType GetSpeedBoatMountType(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(laraItem);

	BoatMountType mountType = BoatMountType::None;

	if (lara->Control.HandStatus != HandStatus::Free)
		return mountType;

	if (!TestBoundsCollide(sBoatItem, laraItem, coll->Setup.Radius))
		return mountType;

	if (!TestCollision(sBoatItem, laraItem))
		return mountType;

	int distance = (laraItem->Position.zPos - sBoatItem->Position.zPos) * phd_cos(-sBoatItem->Position.yRot) - (laraItem->Position.xPos - sBoatItem->Position.xPos) * phd_sin(-sBoatItem->Position.yRot);
	if (distance > 200)
		return mountType;

	short deltaAngle = sBoatItem->Position.yRot - laraItem->Position.yRot;
	if (lara->Control.WaterStatus == WaterStatus::TreadWater || lara->Control.WaterStatus == WaterStatus::Wade)
	{
		if (!(TrInput & IN_ACTION) || laraItem->Animation.Airborne || sBoatItem->Animation.Velocity)
			return mountType;

		if (deltaAngle > ANGLE(45.0f) && deltaAngle < ANGLE(135.0f))
			mountType = BoatMountType::WaterRight;
		else if (deltaAngle > -ANGLE(135.0f) && deltaAngle < -ANGLE(45.0f))
			mountType = BoatMountType::WaterLeft;
	}
	else if (lara->Control.WaterStatus == WaterStatus::Dry)
	{
		if (laraItem->Animation.VerticalVelocity > 0)
		{
			if (deltaAngle > -ANGLE(135.0f) && deltaAngle < ANGLE(135.0f) &&
				laraItem->Position.yPos > sBoatItem->Position.yPos)
			{
				mountType = BoatMountType::Jump;
			}
		}
		else if (laraItem->Animation.VerticalVelocity == 0)
		{
			if (deltaAngle > -ANGLE(135.0f) && deltaAngle < ANGLE(135.0f))
			{
				if (laraItem->Position.xPos == sBoatItem->Position.xPos &&
					laraItem->Position.yPos == sBoatItem->Position.yPos &&
					laraItem->Position.zPos == sBoatItem->Position.zPos)
				{
					mountType = BoatMountType::StartPosition;
				}
				else
					mountType = BoatMountType::Jump;
			}
		}
	}

	return mountType;
}

bool TestSpeedBoatDismount(ITEM_INFO* sBoatItem, int direction)
{
	short angle;
	if (direction < 0)
		angle = sBoatItem->Position.yRot - ANGLE(90.0f);
	else
		angle = sBoatItem->Position.yRot + ANGLE(90.0f);

	int x = sBoatItem->Position.xPos + DISMOUNT_DISTANCE * phd_sin(angle);
	int y = sBoatItem->Position.yPos;
	int z = sBoatItem->Position.zPos + DISMOUNT_DISTANCE * phd_cos(angle);
	auto probe = GetCollisionResult(x, y, z, sBoatItem->RoomNumber);

	if ((probe.Position.Floor - sBoatItem->Position.yPos) < -CLICK(2))
		return false;

	if (probe.Position.FloorSlope ||
		probe.Position.Floor == NO_HEIGHT)
	{
		return false;
	}

	if ((probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT ||
		(probe.Position.Ceiling - sBoatItem->Position.yPos) > -LARA_HEIGHT)
	{
		return false;
	}

	return true;
}

void DoSpeedBoatDismount(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem)
{
	auto* lara = GetLaraInfo(laraItem);

	if ((laraItem->Animation.ActiveState == SBOAT_STATE_DISMOUNT_LEFT ||
		laraItem->Animation.ActiveState == SBOAT_STATE_DISMOUNT_RIGHT) &&
		TestLastFrame(laraItem, laraItem->Animation.AnimNumber))
	{
		if (laraItem->Animation.ActiveState == SBOAT_STATE_DISMOUNT_LEFT)
			laraItem->Position.yRot -= ANGLE(90.0f);
		else if(laraItem->Animation.ActiveState == SBOAT_STATE_DISMOUNT_RIGHT)
			laraItem->Position.yRot += ANGLE(90.0f);

		SetAnimation(laraItem, LA_JUMP_FORWARD);
		laraItem->Animation.Velocity = 40;
		laraItem->Animation.VerticalVelocity = -50;
		laraItem->Animation.Airborne = true;
		laraItem->Position.xRot = 0;
		laraItem->Position.zRot = 0;
		lara->Vehicle = NO_ITEM;

		int x = laraItem->Position.xPos + 360 * phd_sin(laraItem->Position.yRot);
		int y = laraItem->Position.yPos - 90;
		int z = laraItem->Position.zPos + 360 * phd_cos(laraItem->Position.yRot);
		auto probe = GetCollisionResult(x, y, z, laraItem->RoomNumber);

		if (probe.Position.Floor >= (y - CLICK(1)))
		{
			laraItem->Position.xPos = x;
			laraItem->Position.zPos = z;

			if (probe.RoomNumber != laraItem->RoomNumber)
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
		}
		laraItem->Position.yPos = y;

		sBoatItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT].animIndex;
		sBoatItem->Animation.FrameNumber = g_Level.Anims[sBoatItem->Animation.AnimNumber].frameBase;
	}
}

int SpeedBoatTestWaterHeight(ITEM_INFO* sBoatItem, int zOffset, int xOffset, PHD_VECTOR* pos)
{
	float s = phd_sin(sBoatItem->Position.yRot);
	float c = phd_cos(sBoatItem->Position.yRot);

	pos->x = sBoatItem->Position.xPos + zOffset * s + xOffset * c;
	pos->y = sBoatItem->Position.yPos - zOffset * phd_sin(sBoatItem->Position.xRot) + xOffset * phd_sin(sBoatItem->Position.zRot);
	pos->z = sBoatItem->Position.zPos + zOffset * c - xOffset * s;

	auto probe = GetCollisionResult(pos->x, pos->y, pos->z, sBoatItem->RoomNumber);
	auto height = GetWaterHeight(pos->x, pos->y, pos->z, probe.RoomNumber);

	if (height == NO_HEIGHT)
	{
		height = probe.Position.Floor;
		if (height == NO_HEIGHT)
			return height;
	}

	return (height - 5);
}

void SpeedBoatDoBoatShift(ITEM_INFO* sBoatItem, int itemNumber)
{
	short itemNumber2 = g_Level.Rooms[sBoatItem->RoomNumber].itemNumber;
	while (itemNumber2 != NO_ITEM)
	{
		auto* item = &g_Level.Items[itemNumber2];

		if (item->ObjectNumber == ID_SPEEDBOAT && itemNumber2 != itemNumber && Lara.Vehicle != itemNumber2)
		{
			int x = item->Position.xPos - sBoatItem->Position.xPos;
			int z = item->Position.zPos - sBoatItem->Position.zPos;

			int distance = pow(x, 2) + pow(z, 2);
			int radius = pow(BOAT_RADIUS * 2, 2);
			if (distance < radius)
			{
				sBoatItem->Position.xPos = item->Position.xPos - x * radius / distance;
				sBoatItem->Position.zPos = item->Position.zPos - z * radius / distance;
			}

			return;
		}

		// TODO: mine and gondola

		itemNumber2 = item->NextItem;
	}
}

short SpeedBoatDoShift(ITEM_INFO* sBoatItem, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int x = pos->x / SECTOR(1);
	int z = pos->z / SECTOR(1);

	int xOld = old->x / SECTOR(1);
	int zOld = old->z / SECTOR(1);

	int shiftX = pos->x & (SECTOR(1) - 1);
	int shiftZ = pos->z & (SECTOR(1) - 1);

	if (x == xOld)
	{
		if (z == zOld)
		{
			sBoatItem->Position.zPos += (old->z - pos->z);
			sBoatItem->Position.xPos += (old->x - pos->x);
		}
		else if (z > zOld)
		{
			sBoatItem->Position.zPos -= shiftZ + 1;
			return (pos->x - sBoatItem->Position.xPos);
		}
		else
		{
			sBoatItem->Position.zPos += SECTOR(1) - shiftZ;
			return (sBoatItem->Position.xPos - pos->x);
		}
	}
	else if (z == zOld)
	{
		if (x > xOld)
		{
			sBoatItem->Position.xPos -= shiftX + 1;
			return (sBoatItem->Position.zPos - pos->z);
		}
		else
		{ 
			sBoatItem->Position.xPos += SECTOR(1) - shiftX;
			return (pos->z - sBoatItem->Position.zPos);
		}
	}
	else
	{
		x = 0;
		z = 0;

		auto probe = GetCollisionResult(old->x, pos->y, pos->z, sBoatItem->RoomNumber);
		if (probe.Position.Floor < (old->y - CLICK(1)))
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = SECTOR(1) - shiftZ;
		}

		probe = GetCollisionResult(pos->x, pos->y, old->z, sBoatItem->RoomNumber);
		if (probe.Position.Floor < (old->y - CLICK(1)))
		{
			if (pos->x > old->x)
				x = -shiftX - 1;
			else
				x = SECTOR(1) - shiftX;
		}

		if (x && z)
		{
			sBoatItem->Position.zPos += z;
			sBoatItem->Position.xPos += x;
		}
		else if (z)
		{
			sBoatItem->Position.zPos += z;
			if (z > 0)
				return (sBoatItem->Position.xPos - pos->x);
			else
				return (pos->x - sBoatItem->Position.xPos);
		}
		else if (x)
		{
			sBoatItem->Position.xPos += x;
			if (x > 0)
				return (pos->z - sBoatItem->Position.zPos);
			else
				return (sBoatItem->Position.zPos - pos->z);
		}
		else
		{
			sBoatItem->Position.zPos += (old->z - pos->z);
			sBoatItem->Position.xPos += (old->x - pos->x);
		}
	}

	return 0;
}

int GetSpeedBoatHitAnim(ITEM_INFO* sBoatItem, PHD_VECTOR* moved)
{
	moved->x = sBoatItem->Position.xPos - moved->x;
	moved->z = sBoatItem->Position.zPos - moved->z;

	if (moved->x || moved->z)
	{
		float s = phd_sin(sBoatItem->Position.yRot);
		float c = phd_cos(sBoatItem->Position.yRot);
		
		int front = moved->z * c + moved->x * s;
		int side = -moved->z * s + moved->x * c;
		
		if (abs(front) > abs(side))
		{
			if (front > 0)
				return SBOAT_ANIM_HIT_BACK;
			else
				return SBOAT_ANIM_HIT_FRONT;
		}
		else
		{
			if (side > 0)
				return SBOAT_ANIM_HIT_LEFT;
			else
				return SBOAT_ANIM_HIT_RIGHT;
		}
	}

	return 0;
}

int DoSpeedBoatDynamics(int height, int verticalVelocity, int* y)
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
		verticalVelocity += ((height - *y - verticalVelocity) / 8);
		if (verticalVelocity < BOAT_MAX_BACK)
			verticalVelocity = BOAT_MAX_BACK;

		if (*y > height)
			*y = height;
	}

	return verticalVelocity;
}

int SpeedBoatDynamics(ITEM_INFO* laraItem, short itemNumber)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* sBoatItem = &g_Level.Items[itemNumber];
	auto* sBoat = (SpeedBoatInfo*)sBoatItem->Data;

	sBoatItem->Position.zRot -= sBoat->LeanAngle;

	PHD_VECTOR old, frontLeftOld, frontRightOld, backLeftOld, backRightOld, frontOld;
	int heightFrontLeftOld = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, -BOAT_SIDE, &frontLeftOld);
	int heightFrontRightOld = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, BOAT_SIDE, &frontRightOld);
	int heightBackLeftOld = SpeedBoatTestWaterHeight(sBoatItem, -BOAT_FRONT, -BOAT_SIDE, &backLeftOld);
	int heightBackRightOld = SpeedBoatTestWaterHeight(sBoatItem, -BOAT_FRONT, BOAT_SIDE, &backRightOld);
	int heightFrontOld  = SpeedBoatTestWaterHeight(sBoatItem, BOAT_TIP, 0, &frontOld);

	old.x = sBoatItem->Position.xPos;
	old.y = sBoatItem->Position.yPos;
	old.z = sBoatItem->Position.zPos;

	if (backLeftOld.y > heightBackLeftOld)
		backLeftOld.y = heightBackLeftOld;
	if (backRightOld.y > heightBackRightOld)
		backRightOld.y = heightBackRightOld;
	if (frontLeftOld.y > heightFrontLeftOld)
		frontLeftOld.y = heightFrontLeftOld;
	if (frontRightOld.y > heightFrontRightOld)
		frontRightOld.y = heightFrontRightOld;
	if (frontOld.y > heightFrontOld)
		frontOld.y = heightFrontOld;

	sBoatItem->Position.yRot += sBoat->TurnRate + sBoat->ExtraRotation;
	sBoat->LeanAngle = sBoat->TurnRate * 6;

	sBoatItem->Position.xPos += sBoatItem->Animation.Velocity * phd_sin(sBoatItem->Position.yRot);
	sBoatItem->Position.zPos += sBoatItem->Animation.Velocity * phd_cos(sBoatItem->Position.yRot);
	
	int slip = BOAT_SIDE_SLIP * phd_sin(sBoatItem->Position.zRot);
	if (!slip && sBoatItem->Position.zRot)
		slip = (sBoatItem->Position.zRot > 0) ? 1 : -1;
	sBoatItem->Position.xPos += slip * phd_sin(sBoatItem->Position.yRot);
	sBoatItem->Position.zPos -= slip * phd_cos(sBoatItem->Position.yRot);
	
	slip = BOAT_SLIP * phd_sin(sBoatItem->Position.xRot);
	if (!slip && sBoatItem->Position.xRot)
		slip = (sBoatItem->Position.xRot > 0) ? 1 : -1;
	sBoatItem->Position.xPos -= slip * phd_sin(sBoatItem->Position.yRot);
	sBoatItem->Position.zPos -= slip * phd_cos(sBoatItem->Position.yRot);
	
	auto moved = PHD_VECTOR(sBoatItem->Position.xPos, 0, sBoatItem->Position.zPos);

	SpeedBoatDoBoatShift(sBoatItem, itemNumber);

	PHD_VECTOR fl, fr, br, bl, f;
	short rotation = 0;
	auto heightBackLeft = SpeedBoatTestWaterHeight(sBoatItem, -BOAT_FRONT, -BOAT_SIDE, &bl);
	if (heightBackLeft < (backLeftOld.y - CLICK(0.5f)))
		rotation = SpeedBoatDoShift(sBoatItem, &bl, &backLeftOld);

	auto heightBackRight = SpeedBoatTestWaterHeight(sBoatItem, -BOAT_FRONT, BOAT_SIDE, &br);
	if (heightBackRight < (backRightOld.y - CLICK(0.5f)))
		rotation += SpeedBoatDoShift(sBoatItem, &br, &backRightOld);

	auto heightFrontLeft = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, -BOAT_SIDE, &fl);
	if (heightFrontLeft < (frontLeftOld.y - CLICK(0.5f)))
		rotation += SpeedBoatDoShift(sBoatItem, &fl, &frontLeftOld);

	auto heightFrontRight = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, BOAT_SIDE, &fr);
	if (heightFrontRight < (frontRightOld.y - CLICK(0.5f)))
		rotation += SpeedBoatDoShift(sBoatItem, &fr, &frontRightOld);
	
	int heightFront = 0;
	if (!slip)
	{
		heightFront = SpeedBoatTestWaterHeight(sBoatItem, BOAT_TIP, 0, &f);
		if (heightFront < (frontOld.y - CLICK(0.5f)))
			SpeedBoatDoShift(sBoatItem, &f, &frontOld);
	}

	auto probe = GetCollisionResult(sBoatItem);
	auto height = GetWaterHeight(sBoatItem->Position.xPos, sBoatItem->Position.yPos - 5, sBoatItem->Position.zPos, probe.RoomNumber);

	if (height == NO_HEIGHT)
		height = GetFloorHeight(probe.Block, sBoatItem->Position.xPos, sBoatItem->Position.yPos - 5, sBoatItem->Position.zPos);

	if (height < (sBoatItem->Position.yPos - CLICK(0.5f)))
		SpeedBoatDoShift(sBoatItem, (PHD_VECTOR*)&sBoatItem->Position, &old);

	sBoat->ExtraRotation = rotation;

	auto collide = GetSpeedBoatHitAnim(sBoatItem, &moved);

	int newVelocity = 0;
	if (slip || collide)
	{
		newVelocity = (sBoatItem->Position.zPos - old.z) * phd_cos(sBoatItem->Position.yRot) + (sBoatItem->Position.xPos - old.x) * phd_sin(sBoatItem->Position.yRot);

		if (lara->Vehicle == itemNumber && sBoatItem->Animation.Velocity > BOAT_MAX_VELOCITY + BOAT_ACCELERATION && newVelocity < sBoatItem->Animation.Velocity - 10)
		{
			laraItem->HitPoints -= sBoatItem->Animation.Velocity;
			laraItem->HitStatus = 1;
			SoundEffect(SFX_TR4_LARA_INJURY, &laraItem->Position, 0);
			newVelocity /= 2;
			sBoatItem->Animation.Velocity /= 2;
		}

		if (slip)
		{ 
			if (sBoatItem->Animation.Velocity <= BOAT_MAX_VELOCITY + 10)
				sBoatItem->Animation.Velocity = newVelocity;
		}
		else
		{
			if (sBoatItem->Animation.Velocity > 0 && newVelocity < sBoatItem->Animation.Velocity)
				sBoatItem->Animation.Velocity = newVelocity;
			else if (sBoatItem->Animation.Velocity < 0 && newVelocity > sBoatItem->Animation.Velocity)
				sBoatItem->Animation.Velocity = newVelocity;
		}

		if (sBoatItem->Animation.Velocity < BOAT_MAX_BACK)
			sBoatItem->Animation.Velocity = BOAT_MAX_BACK;
	}

	return collide;
}

bool SpeedBoatUserControl(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem)
{
	auto* sBoat = (SpeedBoatInfo*)sBoatItem->Data;

	bool noTurn = true;
	int maxVelocity;
	
	if (sBoatItem->Position.yPos >= sBoat->Water - CLICK(0.5f) && sBoat->Water != NO_HEIGHT)
	{
		if (!(TrInput & SBOAT_IN_DISMOUNT) && !(TrInput & IN_LOOK) ||
			sBoatItem->Animation.Velocity)
		{
			if (TrInput & SBOAT_IN_LEFT && !(TrInput & SBOAT_IN_REVERSE) ||
				TrInput & SBOAT_IN_RIGHT && TrInput & SBOAT_IN_REVERSE)
			{
				if (sBoat->TurnRate > 0)
					sBoat->TurnRate -= BOAT_UNDO_TURN;
				else
				{
					sBoat->TurnRate -= BOAT_TURN;
					if (sBoat->TurnRate < -BOAT_MAX_TURN)
						sBoat->TurnRate = -BOAT_MAX_TURN;
				}

				noTurn = false;
			}
			else if (TrInput & SBOAT_IN_RIGHT && !(TrInput & SBOAT_IN_REVERSE) ||
				TrInput & SBOAT_IN_LEFT && TrInput & SBOAT_IN_REVERSE)
			{
				if (sBoat->TurnRate < 0)
					sBoat->TurnRate += BOAT_UNDO_TURN;
				else
				{
					sBoat->TurnRate += BOAT_TURN;
					if (sBoat->TurnRate > BOAT_MAX_TURN)
						sBoat->TurnRate = BOAT_MAX_TURN;
				}

				noTurn = false;
			}

			if (TrInput & SBOAT_IN_REVERSE)
			{
				if (sBoatItem->Animation.Velocity > 0)
					sBoatItem->Animation.Velocity -= BOAT_BRAKE;
				else if (sBoatItem->Animation.Velocity > BOAT_MAX_BACK)
					sBoatItem->Animation.Velocity += BOAT_REVERSE;
			}
			else if (TrInput & SBOAT_IN_ACCELERATE)
			{
				if (TrInput & SBOAT_IN_SPEED)
					maxVelocity = BOAT_FAST_SPEED;
				else
					maxVelocity = (TrInput & SBOAT_IN_SLOW) ? BOAT_SLOW_SPEED : BOAT_MAX_VELOCITY;

				if (sBoatItem->Animation.Velocity < maxVelocity)
					sBoatItem->Animation.Velocity += (BOAT_ACCELERATION / 2) + (BOAT_ACCELERATION * (sBoatItem->Animation.Velocity / (maxVelocity * 2)));
				else if (sBoatItem->Animation.Velocity > maxVelocity + BOAT_SLOWDOWN)
					sBoatItem->Animation.Velocity -= BOAT_SLOWDOWN;
			}
			else if (TrInput & (SBOAT_IN_LEFT | SBOAT_IN_RIGHT) &&
				sBoatItem->Animation.Velocity >= 0 &&
				sBoatItem->Animation.Velocity < BOAT_MIN_SPEED)
			{
				if (!(TrInput & SBOAT_IN_DISMOUNT) &&
					sBoatItem->Animation.Velocity == 0)
					sBoatItem->Animation.Velocity = BOAT_MIN_SPEED;
			}
			else if (sBoatItem->Animation.Velocity > BOAT_SLOWDOWN)
				sBoatItem->Animation.Velocity -= BOAT_SLOWDOWN;
			else
				sBoatItem->Animation.Velocity = 0;
		}
		else
		{
			if (TrInput & (SBOAT_IN_LEFT | SBOAT_IN_RIGHT) &&
				sBoatItem->Animation.Velocity >= 0 &&
				sBoatItem->Animation.Velocity < BOAT_MIN_SPEED)
			{
				if (sBoatItem->Animation.Velocity == 0 && !(TrInput & SBOAT_IN_DISMOUNT))
					sBoatItem->Animation.Velocity = BOAT_MIN_SPEED;
			}
			else if (sBoatItem->Animation.Velocity > BOAT_SLOWDOWN)
				sBoatItem->Animation.Velocity -= BOAT_SLOWDOWN;
			else
				sBoatItem->Animation.Velocity = 0;

			if (TrInput & IN_LOOK && sBoatItem->Animation.Velocity == 0)
				LookUpDown(laraItem);
		}
	}

	return noTurn;
}

void SpeedBoatAnimation(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem, int collide)
{
	auto* sBoat = (SpeedBoatInfo*)sBoatItem->Data;

	if (laraItem->HitPoints <= 0)
	{
		if (laraItem->Animation.ActiveState != SBOAT_STATE_DEATH)
		{
			laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_DEATH;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraItem->Animation.ActiveState = laraItem->Animation.TargetState = SBOAT_STATE_DEATH;
		}
	}
	else if (sBoatItem->Position.yPos < sBoat->Water - CLICK(0.5f) && sBoatItem->Animation.VerticalVelocity > 0)
	{
		if (laraItem->Animation.ActiveState != SBOAT_STATE_FALL)
		{
			laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_LEAP_START;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraItem->Animation.ActiveState = laraItem->Animation.TargetState = SBOAT_STATE_FALL;
		}
	}
	else if (collide)
	{
		if (laraItem->Animation.ActiveState != SBOAT_STATE_HIT)
		{
			laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + collide;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraItem->Animation.ActiveState = laraItem->Animation.TargetState = SBOAT_STATE_HIT;
		}
	}
	else
	{
		switch (laraItem->Animation.ActiveState)
		{
		case SBOAT_STATE_IDLE:
			if (TrInput & SBOAT_IN_DISMOUNT)
			{
				if (sBoatItem->Animation.Velocity == 0)
				{
					if (TrInput & SBOAT_IN_RIGHT && TestSpeedBoatDismount(sBoatItem, sBoatItem->Position.yRot + ANGLE(90.0f)))
						laraItem->Animation.TargetState = SBOAT_STATE_DISMOUNT_RIGHT;
					else if (TrInput & SBOAT_IN_LEFT && TestSpeedBoatDismount(sBoatItem, sBoatItem->Position.yRot - ANGLE(90.0f)))
						laraItem->Animation.TargetState = SBOAT_STATE_DISMOUNT_LEFT;
				}
			}

			if (sBoatItem->Animation.Velocity > 0)
				laraItem->Animation.TargetState = SBOAT_STATE_MOVING;

			break;

		case SBOAT_STATE_MOVING:
			if (TrInput & SBOAT_IN_DISMOUNT)
			{
				if (TrInput & SBOAT_IN_RIGHT)
					laraItem->Animation.TargetState = SBOAT_STATE_DISMOUNT_RIGHT;
				else if (TrInput & SBOAT_IN_RIGHT)
					laraItem->Animation.TargetState = SBOAT_STATE_DISMOUNT_LEFT;
			}
			else if (sBoatItem->Animation.Velocity <= 0)
				laraItem->Animation.TargetState = SBOAT_STATE_IDLE;

			break;

		case SBOAT_STATE_FALL:
			laraItem->Animation.TargetState = SBOAT_STATE_MOVING;
			break;

		//case BOAT_TURNR:
			if (sBoatItem->Animation.Velocity <= 0)
				laraItem->Animation.TargetState = SBOAT_STATE_IDLE;
			else if (!(TrInput & SBOAT_IN_RIGHT))
				laraItem->Animation.TargetState = SBOAT_STATE_MOVING;

			break;

		case SBOAT_STATE_TURN_LEFT:
			if (sBoatItem->Animation.Velocity <= 0)
				laraItem->Animation.TargetState = SBOAT_STATE_IDLE;
			else if (!(TrInput & SBOAT_IN_LEFT))
				laraItem->Animation.TargetState = SBOAT_STATE_MOVING;

			break;
		}
	}
}

void SpeedBoatSplash(ITEM_INFO* item, long verticalVelocity, long water)
{
	//OLD SPLASH
	/*
	splash_setup.x = item->pos.x_pos;
	splash_setup.y = water;
	splash_setup.z = item->pos.z_pos;
	splash_setup.InnerXZoff = 16 << 2;
	splash_setup.InnerXZsize = 12 << 2;
	splash_setup.InnerYsize = -96 << 2;
	splash_setup.InnerXZvel = 0xa0;
	splash_setup.InnerYvel = -fallspeed << 7;
	splash_setup.InnerGravity = 0x80;
	splash_setup.InnerFriction = 7;
	splash_setup.MiddleXZoff = 24 << 2;
	splash_setup.MiddleXZsize = 24 << 2;
	splash_setup.MiddleYsize = -64 << 2;
	splash_setup.MiddleXZvel = 0xe0;
	splash_setup.MiddleYvel = -fallspeed << 6;
	splash_setup.MiddleGravity = 0x48;
	splash_setup.MiddleFriction = 8;
	splash_setup.OuterXZoff = 32 << 2;
	splash_setup.OuterXZsize = 32 << 2;
	splash_setup.OuterXZvel = 0x110;
	splash_setup.OuterFriction = 9;
	SetupSplash(&splash_setup);
	SplashCount = 16;
	*/
}

void SpeedBoatCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(laraItem);

	if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
		return;

	auto* sBoatItem = &g_Level.Items[itemNumber];

	switch (GetSpeedBoatMountType(laraItem, sBoatItem, coll))
	{
	case BoatMountType::None:
		coll->Setup.EnableObjectPush = true;
		ObjectCollision(itemNumber, laraItem, coll);
		return;

	case BoatMountType::WaterLeft:
		laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_MOUNT_LEFT;
		break;

	case BoatMountType::WaterRight:
		laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_MOUNT_RIGHT;
		break;

	case BoatMountType::Jump:
		laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_MOUNT_JUMP;
		break;

	case BoatMountType::StartPosition:
		laraItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_IDLE;
		break;
	}

	laraItem->Position.xPos = sBoatItem->Position.xPos;
	laraItem->Position.yPos = sBoatItem->Position.yPos - 5;
	laraItem->Position.zPos = sBoatItem->Position.zPos;
	laraItem->Position.xRot = 0;
	laraItem->Position.yRot = sBoatItem->Position.yRot;
	laraItem->Position.zRot = 0;
	laraItem->Animation.Velocity = 0;
	laraItem->Animation.VerticalVelocity = 0;
	laraItem->Animation.Airborne = false;
	laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
	laraItem->Animation.ActiveState = SBOAT_STATE_MOUNT;
	laraItem->Animation.TargetState = SBOAT_STATE_MOUNT;
	lara->Control.WaterStatus = WaterStatus::Dry;

	if (laraItem->RoomNumber != sBoatItem->RoomNumber)
		ItemNewRoom(lara->ItemNumber, sBoatItem->RoomNumber);

	AnimateItem(laraItem);

	if (g_Level.Items[itemNumber].Status != ITEM_ACTIVE)
	{
		AddActiveItem(itemNumber);
		g_Level.Items[itemNumber].Status = ITEM_ACTIVE;
	}
 
	lara->Vehicle = itemNumber;
}

void SpeedBoatControl(short itemNumber)
{
	auto* laraItem = LaraItem;
	auto* lara = GetLaraInfo(laraItem);
	auto* sBoatItem = &g_Level.Items[itemNumber];
	auto* sBoat = (SpeedBoatInfo*)sBoatItem->Data;

	int collide = SpeedBoatDynamics(laraItem, itemNumber);

	PHD_VECTOR frontLeft, frontRight;
	int heightFrontLeft = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, -BOAT_SIDE, &frontLeft);
	int heightFrontRight = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, BOAT_SIDE, &frontRight);

	auto probe = GetCollisionResult(sBoatItem);

	if (lara->Vehicle == itemNumber)
	{
		TestTriggers(sBoatItem, true);
		TestTriggers(sBoatItem, false);
	}

	auto water = GetWaterHeight(sBoatItem->Position.xPos, sBoatItem->Position.yPos, sBoatItem->Position.zPos, probe.RoomNumber);
	sBoat->Water = water;

	bool noTurn = true;
	bool drive = false;

	if (lara->Vehicle == itemNumber && laraItem->HitPoints > 0)
	{
		switch (laraItem->Animation.ActiveState)
		{
		case SBOAT_STATE_MOUNT:
		case SBOAT_STATE_DISMOUNT_RIGHT:
		case SBOAT_STATE_DISMOUNT_LEFT:
			break;

		default:
			drive = true;
			noTurn = SpeedBoatUserControl(laraItem, sBoatItem);
			break;
		}
	}
	else
	{
		if (sBoatItem->Animation.Velocity > BOAT_SLOWDOWN)
			sBoatItem->Animation.Velocity -= BOAT_SLOWDOWN;
		else
			sBoatItem->Animation.Velocity = 0;
	}

	if (noTurn)
	{
		if (sBoat->TurnRate < -BOAT_UNDO_TURN)
			sBoat->TurnRate += BOAT_UNDO_TURN;
		else if (sBoat->TurnRate > BOAT_UNDO_TURN)
			sBoat->TurnRate -= BOAT_UNDO_TURN;
		else
			sBoat->TurnRate = 0;
	}

	sBoatItem->Floor = probe.Position.Floor - 5;
	if (sBoat->Water == NO_HEIGHT)
		sBoat->Water = probe.Position.Floor;
	else
		sBoat->Water -= 5;

	sBoat->LeftVerticalVelocity = DoSpeedBoatDynamics(heightFrontLeft, sBoat->LeftVerticalVelocity, (int*)&frontLeft.y);
	sBoat->RightVerticalVelocity = DoSpeedBoatDynamics(heightFrontRight, sBoat->RightVerticalVelocity, (int*)&frontRight.y);
	sBoatItem->Animation.VerticalVelocity = DoSpeedBoatDynamics(sBoat->Water, sBoatItem->Animation.VerticalVelocity, (int*)&sBoatItem->Position.yPos);

	auto ofs = sBoatItem->Animation.VerticalVelocity;
	if (ofs - sBoatItem->Animation.VerticalVelocity > 32 && sBoatItem->Animation.VerticalVelocity == 0 && water != NO_HEIGHT)
		SpeedBoatSplash(sBoatItem, ofs - sBoatItem->Animation.VerticalVelocity, water);

	probe.Position.Floor = (frontLeft.y + frontRight.y);
	if (probe.Position.Floor < 0)
		probe.Position.Floor = -(abs(probe.Position.Floor) / 2);
	else
		probe.Position.Floor /= 2;

	short xRot = phd_atan(BOAT_FRONT, sBoatItem->Position.yPos - probe.Position.Floor);
	short zRot = phd_atan(BOAT_SIDE, probe.Position.Floor - frontLeft.y);

	sBoatItem->Position.xRot += ((xRot - sBoatItem->Position.xRot) / 2);
	sBoatItem->Position.zRot += ((zRot - sBoatItem->Position.zRot) / 2);
 
	if (!xRot && abs(sBoatItem->Position.xRot) < 4)
		sBoatItem->Position.xRot = 0;
	if (!zRot && abs(sBoatItem->Position.zRot) < 4)
		sBoatItem->Position.zRot = 0;

	if (lara->Vehicle == itemNumber)
	{
		SpeedBoatAnimation(laraItem, sBoatItem, collide);

		if (probe.RoomNumber != sBoatItem->RoomNumber)
		{
			ItemNewRoom(lara->Vehicle, probe.RoomNumber);
			ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
		}

		laraItem->Position.xPos = sBoatItem->Position.xPos;
		laraItem->Position.yPos = sBoatItem->Position.yPos;
		laraItem->Position.zPos = sBoatItem->Position.zPos;
		laraItem->Position.xRot = sBoatItem->Position.xRot;
		laraItem->Position.yRot = sBoatItem->Position.yRot;
		laraItem->Position.zRot = sBoatItem->Position.zRot;
		sBoatItem->Position.zRot += sBoat->LeanAngle;

		AnimateItem(laraItem);

		if (laraItem->HitPoints > 0)
		{
			sBoatItem->Animation.AnimNumber = Objects[ID_SPEEDBOAT].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex);
			sBoatItem->Animation.FrameNumber = g_Level.Anims[sBoatItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);
		}

		Camera.targetElevation = -ANGLE(20.0f);
		Camera.targetDistance = SECTOR(2);
	}
	else
	{
		if (probe.RoomNumber != sBoatItem->RoomNumber)
			ItemNewRoom(itemNumber, probe.RoomNumber);

		sBoatItem->Position.zRot += sBoat->LeanAngle;
	}

	auto pitch = sBoatItem->Animation.Velocity;
	sBoat->Pitch += (pitch - sBoat->Pitch) / 4;

	int fx = (sBoatItem->Animation.Velocity > 8) ? SFX_TR2_BOAT_MOVING : (drive ? SFX_TR2_BOAT_IDLE : SFX_TR2_BOAT_ACCELERATE);
	SoundEffect(fx, &sBoatItem->Position, 0, sBoat->Pitch / (float)BOAT_MAX_VELOCITY);

	if (sBoatItem->Animation.Velocity && (water - 5) == sBoatItem->Position.yPos)
		DoBoatWakeEffect(sBoatItem);

	if (lara->Vehicle != itemNumber)
		return;

	DoSpeedBoatDismount(laraItem, sBoatItem);
}
