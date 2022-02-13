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

enum SpeedBoatState
{
	SBOAT_STATE_MOUNT,
	SBOAT_STATE_IDLE,
	SBOAT_STATE_MOVING,
	SBOAT_STATE_DISMOUNT_RIGHT,
	SBOAT_STATE_DISMOUNT_LEFT,
	SBOAT_STATE_HIT,
	SBOAT_STATE_FALL,
	SBOAT_STATE_TURN_RIGHT,
	SBOAT_STATE_DEATH,
	SBOAT_STATE_TURN_LEFT
};

enum SpeedBoatAnim
{
	SBOAT_ANIM_MOUNT_LEFT = 0,
	SBOAT_ANIM_IDLE = 1, // ?
	SBOAT_ANIM_FORWARD = 2, // ?

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

#define BOAT_UNDO_TURN		(ONE_DEGREE / 4)
#define BOAT_TURN			(ONE_DEGREE / 8)
#define BOAT_MAX_TURN		ANGLE(4.0f)
#define BOAT_MAX_SPEED		110
#define BOAT_SLOW_SPEED		(BOAT_MAX_SPEED / 3)
#define BOAT_FAST_SPEED		(BOAT_MAX_SPEED + 75)
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
#define DISMOUNT_DIST		WALL_SIZE
#define BOAT_WAKE			700
#define BOAT_SOUND_CEILING	(WALL_SIZE * 5)
#define BOAT_TIP			(BOAT_FRONT + 250)

#define	SBOAT_IN_ACCELERATE		IN_FORWARD
#define	SBOAT_IN_REVERSE		IN_BACK
#define	SBOAT_IN_LEFT			(IN_LEFT | IN_LSTEP)
#define	SBOAT_IN_RIGHT			(IN_RIGHT | IN_RSTEP)
#define	SBOAT_IN_SPEED			(IN_ACTION | IN_SPRINT)
#define SBOAT_IN_SLOW			IN_WALK
#define	SBOAT_IN_DISMOUNT		(IN_JUMP | IN_ROLL)

void InitialiseSpeedBoat(short itemNum)
{
	ITEM_INFO* sBoatItem = &g_Level.Items[itemNum];
	sBoatItem->Data = BOAT_INFO();
	BOAT_INFO* sBoatInfo = sBoatItem->Data;

	sBoatInfo->boatTurn = 0;
	sBoatInfo->leftFallspeed = 0;
	sBoatInfo->rightFallspeed = 0;
	sBoatInfo->tiltAngle = 0;
	sBoatInfo->extraRotation = 0;
	sBoatInfo->water = 0;
	sBoatInfo->pitch = 0;
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
	auto laraInfo = GetLaraInfo(laraItem);

	BoatMountType mountType = BoatMountType::None;

	if (laraInfo->Control.HandStatus != HandStatus::Free)
		return mountType;

	if (!TestBoundsCollide(sBoatItem, laraItem, coll->Setup.Radius))
		return mountType;

	if (!TestCollision(sBoatItem, laraItem))
		return mountType;

	int dist = (laraItem->Position.zPos - sBoatItem->Position.zPos) * phd_cos(-sBoatItem->Position.yRot) - (laraItem->Position.xPos - sBoatItem->Position.xPos) * phd_sin(-sBoatItem->Position.yRot);
	if (dist > 200)
		return mountType;

	short rot = sBoatItem->Position.yRot - laraItem->Position.yRot;
	if (laraInfo->Control.WaterStatus == WaterStatus::TreadWater || laraInfo->Control.WaterStatus == WaterStatus::Wade)
	{
		if (!(TrInput & IN_ACTION) || laraItem->Airborne || sBoatItem->Velocity)
			return mountType;

		if (rot > ANGLE(45.0f) && rot < ANGLE(135.0f))
			mountType = BoatMountType::WaterRight;
		else if (rot > -ANGLE(135.0f) && rot < -ANGLE(45.0f))
			mountType = BoatMountType::WaterLeft;
	}
	else if (laraInfo->Control.WaterStatus == WaterStatus::Dry)
	{
		if (laraItem->VerticalVelocity > 0)
		{
			if (rot > -ANGLE(135.0f) && rot < ANGLE(135.0f) &&
				laraItem->Position.yPos > sBoatItem->Position.yPos)
			{
				mountType = BoatMountType::Jump;
			}
		}
		else if (laraItem->VerticalVelocity == 0)
		{
			if (rot > -ANGLE(135.0f) && rot < ANGLE(135.0f))
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

	int x = sBoatItem->Position.xPos + DISMOUNT_DIST * phd_sin(angle);
	int y = sBoatItem->Position.yPos;
	int z = sBoatItem->Position.zPos + DISMOUNT_DIST * phd_cos(angle);
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
	auto laraInfo = GetLaraInfo(laraItem);

	if ((laraItem->ActiveState == SBOAT_STATE_DISMOUNT_LEFT ||
		laraItem->ActiveState == SBOAT_STATE_DISMOUNT_RIGHT) &&
		TestLastFrame(laraItem, laraItem->AnimNumber))
	{
		if (laraItem->ActiveState == SBOAT_STATE_DISMOUNT_LEFT)
			laraItem->Position.yRot -= ANGLE(90.0f);
		else if(laraItem->ActiveState == SBOAT_STATE_DISMOUNT_RIGHT)
			laraItem->Position.yRot += ANGLE(90.0f);

		SetAnimation(laraItem, LA_JUMP_FORWARD);
		laraItem->Airborne = true;
		laraItem->VerticalVelocity = -50;
		laraItem->Velocity = 40;
		laraItem->Position.xRot = 0;
		laraItem->Position.zRot = 0;
		laraInfo->Vehicle = NO_ITEM;

		int x = laraItem->Position.xPos + 360 * phd_sin(laraItem->Position.yRot);
		int y = laraItem->Position.yPos - 90;
		int z = laraItem->Position.zPos + 360 * phd_cos(laraItem->Position.yRot);
		auto probe = GetCollisionResult(x, y, z, laraItem->RoomNumber);

		if (probe.Position.Floor >= y - STEP_SIZE)
		{
			laraItem->Position.xPos = x;
			laraItem->Position.zPos = z;

			if (probe.RoomNumber != laraItem->RoomNumber)
				ItemNewRoom(laraInfo->ItemNumber, probe.RoomNumber);
		}
		laraItem->Position.yPos = y;

		sBoatItem->AnimNumber = Objects[ID_SPEEDBOAT].animIndex;
		sBoatItem->FrameNumber = g_Level.Anims[sBoatItem->AnimNumber].frameBase;
	}
}

int SpeedBoatTestWaterHeight(ITEM_INFO* sBoatItem, int zOff, int xOff, PHD_VECTOR* pos)
{
	float s = phd_sin(sBoatItem->Position.yRot);
	float c = phd_cos(sBoatItem->Position.yRot);

	pos->x = sBoatItem->Position.xPos + zOff * s + xOff * c;
	pos->y = sBoatItem->Position.yPos - zOff * phd_sin(sBoatItem->Position.xRot) + xOff * phd_sin(sBoatItem->Position.zRot);
	pos->z = sBoatItem->Position.zPos + zOff * c - xOff * s;

	auto probe = GetCollisionResult(pos->x, pos->y, pos->z, sBoatItem->RoomNumber);
	auto height = GetWaterHeight(pos->x, pos->y, pos->z, probe.RoomNumber);

	if (height == NO_HEIGHT)
	{
		height = probe.Position.Floor;
		if (height == NO_HEIGHT)
			return height;
	}

	return height - 5;
}

void SpeedBoatDoBoatShift(ITEM_INFO* sBoatItem, int itemNum)
{
	ITEM_INFO* item;
	int x, z, distance, radius;

	auto item_number = g_Level.Rooms[sBoatItem->RoomNumber].itemNumber;
	while (item_number != NO_ITEM)
	{
		item = &g_Level.Items[item_number];

		if (item->ObjectNumber == ID_SPEEDBOAT && item_number != itemNum && Lara.Vehicle != item_number)
		{
			x = item->Position.xPos - sBoatItem->Position.xPos;
			z = item->Position.zPos - sBoatItem->Position.zPos;
			distance = SQUARE(x) + SQUARE(z);
			radius = SQUARE(BOAT_RADIUS * 2);

			if (distance < radius)
			{
				sBoatItem->Position.xPos = item->Position.xPos - x * radius / distance;
				sBoatItem->Position.zPos = item->Position.zPos - z * radius / distance;
			}

			return;
		}

		// TODO: mine and gondola

		item_number = item->NextItem;
	}
}

short SpeedBoatDoShift(ITEM_INFO* skidoo, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int x = pos->x / SECTOR(1);
	int z = pos->z / SECTOR(1);

	int xOld = old->x / SECTOR(1);
	int zOld = old->z / SECTOR(1);

	int shiftX = pos->x & (WALL_SIZE - 1);
	int shiftZ = pos->z & (WALL_SIZE - 1);

	if (x == xOld)
	{
		if (z == zOld)
		{
			skidoo->Position.zPos += (old->z - pos->z);
			skidoo->Position.xPos += (old->x - pos->x);
		}
		else if (z > zOld)
		{
			skidoo->Position.zPos -= shiftZ + 1;
			return (pos->x - skidoo->Position.xPos);
		}
		else
		{
			skidoo->Position.zPos += WALL_SIZE - shiftZ;
			return (skidoo->Position.xPos - pos->x);
		}
	}
	else if (z == zOld)
	{
		if (x > xOld)
		{
			skidoo->Position.xPos -= shiftX + 1;
			return (skidoo->Position.zPos - pos->z);
		}
		else
		{ 
			skidoo->Position.xPos += WALL_SIZE - shiftX;
			return (pos->z - skidoo->Position.zPos);
		}
	}
	else
	{
		x = 0;
		z = 0;

		short roomNumber = skidoo->RoomNumber;
		FLOOR_INFO* floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
		int height = GetFloorHeight(floor, old->x, pos->y, pos->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = WALL_SIZE - shiftZ;
		}

		roomNumber = skidoo->RoomNumber;
		floor = GetFloor(pos->x, pos->y, old->z, &roomNumber);
		height = GetFloorHeight(floor, pos->x, pos->y, old->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->x > old->x)
				x = -shiftX - 1;
			else
				x = WALL_SIZE - shiftX;
		}

		if (x && z)
		{
			skidoo->Position.zPos += z;
			skidoo->Position.xPos += x;
		}
		else if (z)
		{
			skidoo->Position.zPos += z;
			if (z > 0)
				return (skidoo->Position.xPos - pos->x);
			else
				return (pos->x - skidoo->Position.xPos);
		}
		else if (x)
		{
			skidoo->Position.xPos += x;
			if (x > 0)
				return (pos->z - skidoo->Position.zPos);
			else
				return (skidoo->Position.zPos - pos->z);
		}
		else
		{
			skidoo->Position.zPos += (old->z - pos->z);
			skidoo->Position.xPos += (old->x - pos->x);
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

int SpeedBoatDoBoatDynamics(int height, int fallspeed, int* y)
{
	if (height > * y)
	{
		*y += fallspeed;
		if (*y > height)
		{
			*y = height;
			fallspeed = 0;
		}
		else
			fallspeed += GRAVITY;
	}
	else
	{
		fallspeed += ((height - *y - fallspeed) / 8);
		if (fallspeed < BOAT_MAX_BACK)
			fallspeed = BOAT_MAX_BACK;

		if (*y > height)
			*y = height;
	}

	return fallspeed;
}

int SpeedBoatDynamics(ITEM_INFO* laraItem, short itemNum)
{
	auto laraInfo = GetLaraInfo(laraItem);
	auto sBoatItem = &g_Level.Items[itemNum];
	BOAT_INFO* sBoatInfo = sBoatItem->Data;

	sBoatItem->Position.zRot -= sBoatInfo->tiltAngle;

	PHD_VECTOR old, fl_old, fr_old, bl_old, br_old, f_old;
	auto hfl_old = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, -BOAT_SIDE, &fl_old);
	auto hfr_old = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, BOAT_SIDE, &fr_old);
	auto hbl_old = SpeedBoatTestWaterHeight(sBoatItem, -BOAT_FRONT, -BOAT_SIDE, &bl_old);
	auto hbr_old = SpeedBoatTestWaterHeight(sBoatItem, -BOAT_FRONT, BOAT_SIDE, &br_old);
	auto hf_old  = SpeedBoatTestWaterHeight(sBoatItem, BOAT_TIP, 0, &f_old);

	old.x = sBoatItem->Position.xPos;
	old.y = sBoatItem->Position.yPos;
	old.z = sBoatItem->Position.zPos;

	if (bl_old.y > hbl_old)
		bl_old.y = hbl_old;
	if (br_old.y > hbr_old)
		br_old.y = hbr_old;
	if (fl_old.y > hfl_old)
		fl_old.y = hfl_old;
	if (fr_old.y > hfr_old)
		fr_old.y = hfr_old;
	if (f_old.y > hf_old)
		f_old.y = hf_old;

	sBoatItem->Position.yRot += sBoatInfo->boatTurn + sBoatInfo->extraRotation;
	sBoatInfo->tiltAngle = sBoatInfo->boatTurn * 6;

	sBoatItem->Position.xPos += sBoatItem->Velocity * phd_sin(sBoatItem->Position.yRot);
	sBoatItem->Position.zPos += sBoatItem->Velocity * phd_cos(sBoatItem->Position.yRot);
	
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

	SpeedBoatDoBoatShift(sBoatItem, itemNum);

	PHD_VECTOR fl, fr, br, bl, f;
	short rot = 0;
	auto heightBackLeft = SpeedBoatTestWaterHeight(sBoatItem, -BOAT_FRONT, -BOAT_SIDE, &bl);
	if (heightBackLeft < bl_old.y - CLICK(0.5f))
		rot = SpeedBoatDoShift(sBoatItem, &bl, &bl_old);

	auto heightBackRight = SpeedBoatTestWaterHeight(sBoatItem, -BOAT_FRONT, BOAT_SIDE, &br);
	if (heightBackRight < br_old.y - CLICK(0.5f))
		rot += SpeedBoatDoShift(sBoatItem, &br, &br_old);

	auto heightFrontLeft = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, -BOAT_SIDE, &fl);
	if (heightFrontLeft < fl_old.y - CLICK(0.5f))
		rot += SpeedBoatDoShift(sBoatItem, &fl, &fl_old);

	auto heightFrontRight = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, BOAT_SIDE, &fr);
	if (heightFrontRight < fr_old.y - CLICK(0.5f))
		rot += SpeedBoatDoShift(sBoatItem, &fr, &fr_old);
	
	int heightFront = 0;
	if (!slip)
	{
		heightFront = SpeedBoatTestWaterHeight(sBoatItem, BOAT_TIP, 0, &f);
		if (heightFront < f_old.y - CLICK(0.5f))
			SpeedBoatDoShift(sBoatItem, &f, &f_old);
	}

	auto probe = GetCollisionResult(sBoatItem);
	auto height = GetWaterHeight(sBoatItem->Position.xPos, sBoatItem->Position.yPos - 5, sBoatItem->Position.zPos, probe.RoomNumber);

	if (height == NO_HEIGHT)
		height = GetFloorHeight(probe.Block, sBoatItem->Position.xPos, sBoatItem->Position.yPos - 5, sBoatItem->Position.zPos);

	if (height < sBoatItem->Position.yPos - CLICK(0.5f))
		SpeedBoatDoShift(sBoatItem, (PHD_VECTOR*)&sBoatItem->Position, &old);

	sBoatInfo->extraRotation = rot;

	auto collide = GetSpeedBoatHitAnim(sBoatItem, &moved);

	int newspeed = 0;
	if (slip || collide)
	{
		newspeed = (sBoatItem->Position.zPos - old.z) * phd_cos(sBoatItem->Position.yRot) + (sBoatItem->Position.xPos - old.x) * phd_sin(sBoatItem->Position.yRot);

		if (laraInfo->Vehicle == itemNum && sBoatItem->Velocity > BOAT_MAX_SPEED + BOAT_ACCELERATION && newspeed < sBoatItem->Velocity - 10)
		{
			laraItem->HitPoints -= sBoatItem->Velocity;
			laraItem->HitStatus = 1;
			SoundEffect(SFX_TR4_LARA_INJURY, &laraItem->Position, 0);
			newspeed /= 2;
			sBoatItem->Velocity /= 2;
		}

		if (slip)
		{ 
			if (sBoatItem->Velocity <= BOAT_MAX_SPEED + 10)
				sBoatItem->Velocity = newspeed;
		}
		else
		{
			if (sBoatItem->Velocity > 0 && newspeed < sBoatItem->Velocity)
				sBoatItem->Velocity = newspeed;
			else if (sBoatItem->Velocity < 0 && newspeed > sBoatItem->Velocity)
				sBoatItem->Velocity = newspeed;
		}

		if (sBoatItem->Velocity < BOAT_MAX_BACK)
			sBoatItem->Velocity = BOAT_MAX_BACK;
	}

	return collide;
}

bool SpeedBoatUserControl(ITEM_INFO* sBoatItem)
{
	BOAT_INFO* sBoatInfo = (BOAT_INFO*)sBoatItem->Data;

	bool noTurn = true;
	int maxSpeed;
	
	if (sBoatItem->Position.yPos >= sBoatInfo->water - STEP_SIZE / 2 && sBoatInfo->water != NO_HEIGHT)
	{
		if (!(TrInput & SBOAT_IN_DISMOUNT) && !(TrInput & IN_LOOK) ||
			sBoatItem->Velocity)
		{
			if (TrInput & SBOAT_IN_LEFT && !(TrInput & SBOAT_IN_REVERSE) ||
				TrInput & SBOAT_IN_RIGHT && TrInput & SBOAT_IN_REVERSE)
			{
				if (sBoatInfo->boatTurn > 0)
					sBoatInfo->boatTurn -= BOAT_UNDO_TURN;
				else
				{
					sBoatInfo->boatTurn -= BOAT_TURN;
					if (sBoatInfo->boatTurn < -BOAT_MAX_TURN)
						sBoatInfo->boatTurn = -BOAT_MAX_TURN;
				}

				noTurn = false;
			}
			else if (TrInput & SBOAT_IN_RIGHT && !(TrInput & SBOAT_IN_REVERSE) ||
				TrInput & SBOAT_IN_LEFT && TrInput & SBOAT_IN_REVERSE)
			{
				if (sBoatInfo->boatTurn < 0)
					sBoatInfo->boatTurn += BOAT_UNDO_TURN;
				else
				{
					sBoatInfo->boatTurn += BOAT_TURN;
					if (sBoatInfo->boatTurn > BOAT_MAX_TURN)
						sBoatInfo->boatTurn = BOAT_MAX_TURN;
				}

				noTurn = false;
			}

			if (TrInput & SBOAT_IN_REVERSE)
			{
				if (sBoatItem->Velocity > 0)
					sBoatItem->Velocity -= BOAT_BRAKE;
				else if (sBoatItem->Velocity > BOAT_MAX_BACK)
					sBoatItem->Velocity += BOAT_REVERSE;
			}
			else if (TrInput & SBOAT_IN_ACCELERATE)
			{
				if (TrInput & SBOAT_IN_SPEED)
					maxSpeed = BOAT_FAST_SPEED;
				else
					maxSpeed = (TrInput & SBOAT_IN_SLOW) ? BOAT_SLOW_SPEED : BOAT_MAX_SPEED;

				if (sBoatItem->Velocity < maxSpeed)
					sBoatItem->Velocity += BOAT_ACCELERATION / 2 + BOAT_ACCELERATION * sBoatItem->Velocity / (2 * maxSpeed);
				else if (sBoatItem->Velocity > maxSpeed + BOAT_SLOWDOWN)
					sBoatItem->Velocity -= BOAT_SLOWDOWN;
			}
			else if (TrInput & (SBOAT_IN_LEFT | SBOAT_IN_RIGHT) &&
				sBoatItem->Velocity >= 0 && sBoatItem->Velocity < BOAT_MIN_SPEED)
			{
				if (!(TrInput & SBOAT_IN_DISMOUNT) && sBoatItem->Velocity == 0)
					sBoatItem->Velocity = BOAT_MIN_SPEED;
			}
			else if (sBoatItem->Velocity > BOAT_SLOWDOWN)
				sBoatItem->Velocity -= BOAT_SLOWDOWN;
			else
				sBoatItem->Velocity = 0;
		}
		else
		{
			if (TrInput & (SBOAT_IN_LEFT | SBOAT_IN_RIGHT) &&
				sBoatItem->Velocity >= 0 && sBoatItem->Velocity < BOAT_MIN_SPEED)
			{
				if (sBoatItem->Velocity == 0 && !(TrInput & SBOAT_IN_DISMOUNT))
					sBoatItem->Velocity = BOAT_MIN_SPEED;
			}
			else if (sBoatItem->Velocity > BOAT_SLOWDOWN)
				sBoatItem->Velocity -= BOAT_SLOWDOWN;
			else
				sBoatItem->Velocity = 0;

			if (TrInput & IN_LOOK && sBoatItem->Velocity == 0)
				LookUpDown();
		}
	}

	return noTurn;
}

void SpeedBoatAnimation(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem, int collide)
{
	BOAT_INFO* sBoatInfo = sBoatItem->Data;

	if (laraItem->HitPoints <= 0)
	{
		if (laraItem->ActiveState != SBOAT_STATE_DEATH)
		{
			laraItem->AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_DEATH;
			laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
			laraItem->ActiveState = laraItem->TargetState = SBOAT_STATE_DEATH;
		}
	}
	else if (sBoatItem->Position.yPos < sBoatInfo->water - CLICK(0.5f) && sBoatItem->VerticalVelocity > 0)
	{
		if (laraItem->ActiveState != SBOAT_STATE_FALL)
		{
			laraItem->AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_LEAP_START;
			laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
			laraItem->ActiveState = laraItem->TargetState = SBOAT_STATE_FALL;
		}
	}
	else if (collide)
	{
		if (laraItem->ActiveState != SBOAT_STATE_HIT)
		{
			laraItem->AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + collide;
			laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
			laraItem->ActiveState = laraItem->TargetState = SBOAT_STATE_HIT;
		}
	}
	else
	{
		switch (laraItem->ActiveState)
		{
		case SBOAT_STATE_IDLE:
			if (TrInput & SBOAT_IN_DISMOUNT)
			{
				if (sBoatItem->Velocity == 0)
				{
					if (TrInput & SBOAT_IN_RIGHT && TestSpeedBoatDismount(sBoatItem, sBoatItem->Position.yRot + ANGLE(90.0f)))
						laraItem->TargetState = SBOAT_STATE_DISMOUNT_RIGHT;
					else if (TrInput & SBOAT_IN_LEFT && TestSpeedBoatDismount(sBoatItem, sBoatItem->Position.yRot - ANGLE(90.0f)))
						laraItem->TargetState = SBOAT_STATE_DISMOUNT_LEFT;
				}
			}

			if (sBoatItem->Velocity > 0)
				laraItem->TargetState = SBOAT_STATE_MOVING;

			break;

		case SBOAT_STATE_MOVING:
			if (TrInput & SBOAT_IN_DISMOUNT)
			{
				if (TrInput & SBOAT_IN_RIGHT)
					laraItem->TargetState = SBOAT_STATE_DISMOUNT_RIGHT;
				else if (TrInput & SBOAT_IN_RIGHT)
					laraItem->TargetState = SBOAT_STATE_DISMOUNT_LEFT;
			}
			else if (sBoatItem->Velocity <= 0)
				laraItem->TargetState = SBOAT_STATE_IDLE;

			break;

		case SBOAT_STATE_FALL:
			laraItem->TargetState = SBOAT_STATE_MOVING;
			break;

		//case BOAT_TURNR:
			if (sBoatItem->Velocity <= 0)
				laraItem->TargetState = SBOAT_STATE_IDLE;
			else if (!(TrInput & SBOAT_IN_RIGHT))
				laraItem->TargetState = SBOAT_STATE_MOVING;

			break;

		case SBOAT_STATE_TURN_LEFT:
			if (sBoatItem->Velocity <= 0)
				laraItem->TargetState = SBOAT_STATE_IDLE;
			else if (!(TrInput & SBOAT_IN_LEFT))
				laraItem->TargetState = SBOAT_STATE_MOVING;

			break;
		}
	}
}

void SpeedBoatSplash(ITEM_INFO* item, long fallspeed, long water)
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

void SpeedBoatCollision(short itemNum, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto laraInfo = GetLaraInfo(laraItem);

	if (laraItem->HitPoints < 0 || laraInfo->Vehicle != NO_ITEM)
		return;

	auto sBoatItem = &g_Level.Items[itemNum];

	switch (GetSpeedBoatMountType(laraItem, sBoatItem, coll))
	{
	case BoatMountType::None:
		coll->Setup.EnableObjectPush = true;
		ObjectCollision(itemNum, laraItem, coll);
		return;

	case BoatMountType::WaterLeft:
		laraItem->AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_MOUNT_LEFT;
		break;

	case BoatMountType::WaterRight:
		laraItem->AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_MOUNT_RIGHT;
		break;

	case BoatMountType::Jump:
		laraItem->AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_MOUNT_JUMP;
		break;

	case BoatMountType::StartPosition:
		laraItem->AnimNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_IDLE;
		break;
	}

	laraInfo->Control.WaterStatus = WaterStatus::Dry;
	laraItem->Position.xPos = sBoatItem->Position.xPos;
	laraItem->Position.yPos = sBoatItem->Position.yPos - 5;
	laraItem->Position.zPos = sBoatItem->Position.zPos;
	laraItem->Position.yRot = sBoatItem->Position.yRot;
	laraItem->Position.xRot = 0;
	laraItem->Position.zRot = 0;
	laraItem->Airborne = false;
	laraItem->Velocity = 0;
	laraItem->VerticalVelocity = 0;
	laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
	laraItem->ActiveState = SBOAT_STATE_MOUNT;
	laraItem->TargetState = SBOAT_STATE_MOUNT;

	if (laraItem->RoomNumber != sBoatItem->RoomNumber)
		ItemNewRoom(laraInfo->ItemNumber, sBoatItem->RoomNumber);

	AnimateItem(laraItem);

	if (g_Level.Items[itemNum].Status != ITEM_ACTIVE)
	{
		AddActiveItem(itemNum);
		g_Level.Items[itemNum].Status = ITEM_ACTIVE;
	}
 
	laraInfo->Vehicle = itemNum;
}

void SpeedBoatControl(short itemNum)
{
	auto* laraItem = LaraItem;
	auto laraInfo = GetLaraInfo(laraItem);
	auto sBoatItem = &g_Level.Items[itemNum];
	BOAT_INFO* sBoatInfo = sBoatItem->Data;

	PHD_VECTOR frontLeft, frontRight;
	bool noTurn = true;
	bool drive = false;

	int collide = SpeedBoatDynamics(laraItem, itemNum);

	int heightFrontLeft = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, -BOAT_SIDE, &frontLeft);
	int heightFrontRight = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, BOAT_SIDE, &frontRight);

	auto probe = GetCollisionResult(sBoatItem);

	if (laraInfo->Vehicle == itemNum)
	{
		TestTriggers(sBoatItem, true);
		TestTriggers(sBoatItem, false);
	}

	auto water = GetWaterHeight(sBoatItem->Position.xPos, sBoatItem->Position.yPos, sBoatItem->Position.zPos, probe.RoomNumber);
	sBoatInfo->water = water;

	if (laraInfo->Vehicle == itemNum && laraItem->HitPoints > 0)
	{
		switch (laraItem->ActiveState)
		{
		case SBOAT_STATE_MOUNT:
		case SBOAT_STATE_DISMOUNT_RIGHT:
		case SBOAT_STATE_DISMOUNT_LEFT:
			break;

		default:
			drive = true;
			noTurn = SpeedBoatUserControl(sBoatItem);
			break;
		}
	}
	else
	{
		if (sBoatItem->Velocity > BOAT_SLOWDOWN)
			sBoatItem->Velocity -= BOAT_SLOWDOWN;
		else
			sBoatItem->Velocity = 0;
	}

	if (noTurn)
	{
		if (sBoatInfo->boatTurn < -BOAT_UNDO_TURN)
			sBoatInfo->boatTurn += BOAT_UNDO_TURN;
		else if (sBoatInfo->boatTurn > BOAT_UNDO_TURN)
			sBoatInfo->boatTurn -= BOAT_UNDO_TURN;
		else
			sBoatInfo->boatTurn = 0;
	}

	sBoatItem->Floor = probe.Position.Floor - 5;
	if (sBoatInfo->water == NO_HEIGHT)
		sBoatInfo->water = probe.Position.Floor;
	else
		sBoatInfo->water -= 5;

	sBoatInfo->leftFallspeed = SpeedBoatDoBoatDynamics(heightFrontLeft, sBoatInfo->leftFallspeed, (int*)&frontLeft.y);
	sBoatInfo->rightFallspeed = SpeedBoatDoBoatDynamics(heightFrontRight, sBoatInfo->rightFallspeed, (int*)&frontRight.y);
	auto ofs = sBoatItem->VerticalVelocity;
	sBoatItem->VerticalVelocity = SpeedBoatDoBoatDynamics(sBoatInfo->water, sBoatItem->VerticalVelocity, (int*)&sBoatItem->Position.yPos);
	if (ofs - sBoatItem->VerticalVelocity > 32 && sBoatItem->VerticalVelocity == 0 && water != NO_HEIGHT)
		SpeedBoatSplash(sBoatItem, ofs - sBoatItem->VerticalVelocity, water);

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

	if (laraInfo->Vehicle == itemNum)
	{
		SpeedBoatAnimation(laraItem, sBoatItem, collide);

		if (probe.RoomNumber != sBoatItem->RoomNumber)
		{
			ItemNewRoom(laraInfo->Vehicle, probe.RoomNumber);
			ItemNewRoom(laraInfo->ItemNumber, probe.RoomNumber);
		}

		sBoatItem->Position.zRot += sBoatInfo->tiltAngle;
		laraItem->Position.xPos = sBoatItem->Position.xPos;
		laraItem->Position.yPos = sBoatItem->Position.yPos;
		laraItem->Position.zPos = sBoatItem->Position.zPos;
		laraItem->Position.xRot = sBoatItem->Position.xRot;
		laraItem->Position.yRot = sBoatItem->Position.yRot;
		laraItem->Position.zRot = sBoatItem->Position.zRot;

		AnimateItem(laraItem);

		if (laraItem->HitPoints > 0)
		{
			sBoatItem->AnimNumber = Objects[ID_SPEEDBOAT].animIndex + (laraItem->AnimNumber - Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex);
			sBoatItem->FrameNumber = g_Level.Anims[sBoatItem->AnimNumber].frameBase + (laraItem->FrameNumber - g_Level.Anims[laraItem->AnimNumber].frameBase);
		}

		Camera.targetElevation = -ANGLE(20.0f);
		Camera.targetDistance = WALL_SIZE * 2;
	}
	else
	{
		if (probe.RoomNumber != sBoatItem->RoomNumber)
			ItemNewRoom(itemNum, probe.RoomNumber);

		sBoatItem->Position.zRot += sBoatInfo->tiltAngle;
	}

	auto pitch = sBoatItem->Velocity;
	sBoatInfo->pitch += ((pitch - sBoatInfo->pitch) / 4);

	int fx = (sBoatItem->Velocity > 8) ? SFX_TR2_BOAT_MOVING : (drive ? SFX_TR2_BOAT_IDLE : SFX_TR2_BOAT_ACCELERATE);
	SoundEffect(fx, &sBoatItem->Position, 0, sBoatInfo->pitch / (float)BOAT_MAX_SPEED);

	if (sBoatItem->Velocity && water - 5 == sBoatItem->Position.yPos)
		DoBoatWakeEffect(sBoatItem);

	if (laraInfo->Vehicle != itemNum)
		return;

	DoSpeedBoatDismount(laraItem, sBoatItem);
}
