#include "framework.h"
#include "boat.h"
#include "lara.h"
#include "items.h"
#include "collide.h"
#include "sphere.h"
#include "camera.h"
#include "setup.h"
#include "level.h"
#include "input.h"
#include "animation.h"
#include "Sound/sound.h"
#include "effects/effects.h"
#include "particle/SimpleParticle.h"
#include "boat_info.h"

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
	sBoatItem->data = BOAT_INFO();
	BOAT_INFO* sBoatInfo = sBoatItem->data;

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
	SetupRipple(sBoatItem->pos.xPos, sBoatItem->pos.yPos, sBoatItem->pos.zPos, 512, RIPPLE_FLAG_RAND_POS, Objects[1368].meshIndex, TO_RAD(sBoatItem->pos.yRot));
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
	LaraInfo*& laraInfo = laraItem->data;

	BoatMountType mountType = BoatMountType::None;

	if (laraInfo->gunStatus != LG_NO_ARMS)
		return mountType;

	if (!TestBoundsCollide(sBoatItem, laraItem, coll->Setup.Radius))
		return mountType;

	if (!TestCollision(sBoatItem, laraItem))
		return mountType;

	int dist = (laraItem->pos.zPos - sBoatItem->pos.zPos) * phd_cos(-sBoatItem->pos.yRot) - (laraItem->pos.xPos - sBoatItem->pos.xPos) * phd_sin(-sBoatItem->pos.yRot);
	if (dist > 200)
		return mountType;

	short rot = sBoatItem->pos.yRot - laraItem->pos.yRot;
	if (laraInfo->waterStatus == LW_SURFACE || laraInfo->waterStatus == LW_WADE)
	{
		if (!(TrInput & IN_ACTION) || laraItem->gravityStatus || sBoatItem->speed)
			return mountType;

		if (rot > ANGLE(45.0f) && rot < ANGLE(135.0f))
			mountType = BoatMountType::WaterRight;
		else if (rot > -ANGLE(135.0f) && rot < -ANGLE(45.0f))
			mountType = BoatMountType::WaterLeft;
	}
	else if (laraInfo->waterStatus == LW_ABOVE_WATER)
	{
		if (laraItem->fallspeed > 0)
		{
			if (rot > -ANGLE(135.0f) && rot < ANGLE(135.0f) &&
				laraItem->pos.yPos > sBoatItem->pos.yPos)
			{
				mountType = BoatMountType::Jump;
			}
		}
		else if (laraItem->fallspeed == 0)
		{
			if (rot > -ANGLE(135.0f) && rot < ANGLE(135.0f))
			{
				if (laraItem->pos.xPos == sBoatItem->pos.xPos &&
					laraItem->pos.yPos == sBoatItem->pos.yPos &&
					laraItem->pos.zPos == sBoatItem->pos.zPos)
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
		angle = sBoatItem->pos.yRot - ANGLE(90.0f);
	else
		angle = sBoatItem->pos.yRot + ANGLE(90.0f);

	int x = sBoatItem->pos.xPos + DISMOUNT_DIST * phd_sin(angle);
	int y = sBoatItem->pos.yPos;
	int z = sBoatItem->pos.zPos + DISMOUNT_DIST * phd_cos(angle);
	auto probe = GetCollisionResult(x, y, z, sBoatItem->roomNumber);

	if ((probe.Position.Floor - sBoatItem->pos.yPos) < -CLICK(2))
		return false;

	if (probe.Position.Slope ||
		probe.Position.Floor == NO_HEIGHT)
	{
		return false;
	}

	if ((probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT ||
		(probe.Position.Ceiling - sBoatItem->pos.yPos) > -LARA_HEIGHT)
	{
		return false;
	}

	return true;
}

void DoSpeedBoatDismount(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem)
{
	LaraInfo*& laraInfo = laraItem->data;

	if ((laraItem->currentAnimState == SBOAT_STATE_DISMOUNT_LEFT ||
		laraItem->currentAnimState == SBOAT_STATE_DISMOUNT_RIGHT) &&
		TestLastFrame(laraItem, laraItem->animNumber))
	{
		if (laraItem->currentAnimState == SBOAT_STATE_DISMOUNT_LEFT)
			laraItem->pos.yRot -= ANGLE(90.0f);
		else if(laraItem->currentAnimState == SBOAT_STATE_DISMOUNT_RIGHT)
			laraItem->pos.yRot += ANGLE(90.0f);

		SetAnimation(laraItem, LA_JUMP_FORWARD);
		laraItem->gravityStatus = true;
		laraItem->fallspeed = -50;
		laraItem->speed = 40;
		laraItem->pos.xRot = 0;
		laraItem->pos.zRot = 0;
		laraInfo->Vehicle = NO_ITEM;

		int x = laraItem->pos.xPos + 360 * phd_sin(laraItem->pos.yRot);
		int y = laraItem->pos.yPos - 90;
		int z = laraItem->pos.zPos + 360 * phd_cos(laraItem->pos.yRot);
		auto probe = GetCollisionResult(x, y, z, laraItem->roomNumber);

		if (probe.Position.Floor >= y - STEP_SIZE)
		{
			laraItem->pos.xPos = x;
			laraItem->pos.zPos = z;

			if (probe.RoomNumber != laraItem->roomNumber)
				ItemNewRoom(laraInfo->itemNumber, probe.RoomNumber);
		}
		laraItem->pos.yPos = y;

		sBoatItem->animNumber = Objects[ID_SPEEDBOAT].animIndex;
		sBoatItem->frameNumber = g_Level.Anims[sBoatItem->animNumber].frameBase;
	}
}

int SpeedBoatTestWaterHeight(ITEM_INFO* sBoatItem, int zOff, int xOff, PHD_VECTOR* pos)
{
	float s = phd_sin(sBoatItem->pos.yRot);
	float c = phd_cos(sBoatItem->pos.yRot);

	pos->x = sBoatItem->pos.xPos + zOff * s + xOff * c;
	pos->y = sBoatItem->pos.yPos - zOff * phd_sin(sBoatItem->pos.xRot) + xOff * phd_sin(sBoatItem->pos.zRot);
	pos->z = sBoatItem->pos.zPos + zOff * c - xOff * s;

	auto probe = GetCollisionResult(pos->x, pos->y, pos->z, sBoatItem->roomNumber);
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

	auto item_number = g_Level.Rooms[sBoatItem->roomNumber].itemNumber;
	while (item_number != NO_ITEM)
	{
		item = &g_Level.Items[item_number];

		if (item->objectNumber == ID_SPEEDBOAT && item_number != itemNum && Lara.Vehicle != item_number)
		{
			x = item->pos.xPos - sBoatItem->pos.xPos;
			z = item->pos.zPos - sBoatItem->pos.zPos;
			distance = SQUARE(x) + SQUARE(z);
			radius = SQUARE(BOAT_RADIUS * 2);

			if (distance < radius)
			{
				sBoatItem->pos.xPos = item->pos.xPos - x * radius / distance;
				sBoatItem->pos.zPos = item->pos.zPos - z * radius / distance;
			}

			return;
		}

		// TODO: mine and gondola

		item_number = item->nextItem;
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
			skidoo->pos.zPos += (old->z - pos->z);
			skidoo->pos.xPos += (old->x - pos->x);
		}
		else if (z > zOld)
		{
			skidoo->pos.zPos -= shiftZ + 1;
			return (pos->x - skidoo->pos.xPos);
		}
		else
		{
			skidoo->pos.zPos += WALL_SIZE - shiftZ;
			return (skidoo->pos.xPos - pos->x);
		}
	}
	else if (z == zOld)
	{
		if (x > xOld)
		{
			skidoo->pos.xPos -= shiftX + 1;
			return (skidoo->pos.zPos - pos->z);
		}
		else
		{ 
			skidoo->pos.xPos += WALL_SIZE - shiftX;
			return (pos->z - skidoo->pos.zPos);
		}
	}
	else
	{
		x = 0;
		z = 0;

		short roomNumber = skidoo->roomNumber;
		FLOOR_INFO* floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
		int height = GetFloorHeight(floor, old->x, pos->y, pos->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = WALL_SIZE - shiftZ;
		}

		roomNumber = skidoo->roomNumber;
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
			skidoo->pos.zPos += z;
			skidoo->pos.xPos += x;
		}
		else if (z)
		{
			skidoo->pos.zPos += z;
			if (z > 0)
				return (skidoo->pos.xPos - pos->x);
			else
				return (pos->x - skidoo->pos.xPos);
		}
		else if (x)
		{
			skidoo->pos.xPos += x;
			if (x > 0)
				return (pos->z - skidoo->pos.zPos);
			else
				return (skidoo->pos.zPos - pos->z);
		}
		else
		{
			skidoo->pos.zPos += (old->z - pos->z);
			skidoo->pos.xPos += (old->x - pos->x);
		}
	}

	return 0;
}

int GetSpeedBoatHitAnim(ITEM_INFO* sBoatItem, PHD_VECTOR* moved)
{
	moved->x = sBoatItem->pos.xPos - moved->x;
	moved->z = sBoatItem->pos.zPos - moved->z;

	if (moved->x || moved->z)
	{
		float s = phd_sin(sBoatItem->pos.yRot);
		float c = phd_cos(sBoatItem->pos.yRot);
		
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
	LaraInfo*& laraInfo = laraItem->data;
	auto sBoatItem = &g_Level.Items[itemNum];
	BOAT_INFO* sBoatInfo = sBoatItem->data;

	sBoatItem->pos.zRot -= sBoatInfo->tiltAngle;

	PHD_VECTOR old, fl_old, fr_old, bl_old, br_old, f_old;
	auto hfl_old = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, -BOAT_SIDE, &fl_old);
	auto hfr_old = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, BOAT_SIDE, &fr_old);
	auto hbl_old = SpeedBoatTestWaterHeight(sBoatItem, -BOAT_FRONT, -BOAT_SIDE, &bl_old);
	auto hbr_old = SpeedBoatTestWaterHeight(sBoatItem, -BOAT_FRONT, BOAT_SIDE, &br_old);
	auto hf_old  = SpeedBoatTestWaterHeight(sBoatItem, BOAT_TIP, 0, &f_old);

	old.x = sBoatItem->pos.xPos;
	old.y = sBoatItem->pos.yPos;
	old.z = sBoatItem->pos.zPos;

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

	sBoatItem->pos.yRot += sBoatInfo->boatTurn + sBoatInfo->extraRotation;
	sBoatInfo->tiltAngle = sBoatInfo->boatTurn * 6;

	sBoatItem->pos.xPos += sBoatItem->speed * phd_sin(sBoatItem->pos.yRot);
	sBoatItem->pos.zPos += sBoatItem->speed * phd_cos(sBoatItem->pos.yRot);
	
	int slip = BOAT_SIDE_SLIP * phd_sin(sBoatItem->pos.zRot);
	if (!slip && sBoatItem->pos.zRot)
		slip = (sBoatItem->pos.zRot > 0) ? 1 : -1;
	sBoatItem->pos.xPos += slip * phd_sin(sBoatItem->pos.yRot);
	sBoatItem->pos.zPos -= slip * phd_cos(sBoatItem->pos.yRot);
	
	slip = BOAT_SLIP * phd_sin(sBoatItem->pos.xRot);
	if (!slip && sBoatItem->pos.xRot)
		slip = (sBoatItem->pos.xRot > 0) ? 1 : -1;
	sBoatItem->pos.xPos -= slip * phd_sin(sBoatItem->pos.yRot);
	sBoatItem->pos.zPos -= slip * phd_cos(sBoatItem->pos.yRot);
	
	auto moved = PHD_VECTOR(sBoatItem->pos.xPos, 0, sBoatItem->pos.zPos);

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
	auto height = GetWaterHeight(sBoatItem->pos.xPos, sBoatItem->pos.yPos - 5, sBoatItem->pos.zPos, probe.RoomNumber);

	if (height == NO_HEIGHT)
		height = GetFloorHeight(probe.Block, sBoatItem->pos.xPos, sBoatItem->pos.yPos - 5, sBoatItem->pos.zPos);

	if (height < sBoatItem->pos.yPos - CLICK(0.5f))
		SpeedBoatDoShift(sBoatItem, (PHD_VECTOR*)&sBoatItem->pos, &old);

	sBoatInfo->extraRotation = rot;

	auto collide = GetSpeedBoatHitAnim(sBoatItem, &moved);

	int newspeed = 0;
	if (slip || collide)
	{
		newspeed = (sBoatItem->pos.zPos - old.z) * phd_cos(sBoatItem->pos.yRot) + (sBoatItem->pos.xPos - old.x) * phd_sin(sBoatItem->pos.yRot);

		if (laraInfo->Vehicle == itemNum && sBoatItem->speed > BOAT_MAX_SPEED + BOAT_ACCELERATION && newspeed < sBoatItem->speed - 10)
		{
			laraItem->hitPoints -= sBoatItem->speed;
			laraItem->hitStatus = 1;
			SoundEffect(SFX_TR4_LARA_INJURY, &laraItem->pos, 0);
			newspeed /= 2;
			sBoatItem->speed /= 2;
		}

		if (slip)
		{ 
			if (sBoatItem->speed <= BOAT_MAX_SPEED + 10)
				sBoatItem->speed = newspeed;
		}
		else
		{
			if (sBoatItem->speed > 0 && newspeed < sBoatItem->speed)
				sBoatItem->speed = newspeed;
			else if (sBoatItem->speed < 0 && newspeed > sBoatItem->speed)
				sBoatItem->speed = newspeed;
		}

		if (sBoatItem->speed < BOAT_MAX_BACK)
			sBoatItem->speed = BOAT_MAX_BACK;
	}

	return collide;
}

bool SpeedBoatUserControl(ITEM_INFO* sBoatItem)
{
	BOAT_INFO* sBoatInfo = (BOAT_INFO*)sBoatItem->data;

	bool noTurn = true;
	int maxSpeed;
	
	if (sBoatItem->pos.yPos >= sBoatInfo->water - STEP_SIZE / 2 && sBoatInfo->water != NO_HEIGHT)
	{
		if (!(TrInput & SBOAT_IN_DISMOUNT) && !(TrInput & IN_LOOK) ||
			sBoatItem->speed)
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
				if (sBoatItem->speed > 0)
					sBoatItem->speed -= BOAT_BRAKE;
				else if (sBoatItem->speed > BOAT_MAX_BACK)
					sBoatItem->speed += BOAT_REVERSE;
			}
			else if (TrInput & SBOAT_IN_ACCELERATE)
			{
				if (TrInput & SBOAT_IN_SPEED)
					maxSpeed = BOAT_FAST_SPEED;
				else
					maxSpeed = (TrInput & SBOAT_IN_SLOW) ? BOAT_SLOW_SPEED : BOAT_MAX_SPEED;

				if (sBoatItem->speed < maxSpeed)
					sBoatItem->speed += BOAT_ACCELERATION / 2 + BOAT_ACCELERATION * sBoatItem->speed / (2 * maxSpeed);
				else if (sBoatItem->speed > maxSpeed + BOAT_SLOWDOWN)
					sBoatItem->speed -= BOAT_SLOWDOWN;
			}
			else if (TrInput & (SBOAT_IN_LEFT | SBOAT_IN_RIGHT) &&
				sBoatItem->speed >= 0 && sBoatItem->speed < BOAT_MIN_SPEED)
			{
				if (!(TrInput & SBOAT_IN_DISMOUNT) && sBoatItem->speed == 0)
					sBoatItem->speed = BOAT_MIN_SPEED;
			}
			else if (sBoatItem->speed > BOAT_SLOWDOWN)
				sBoatItem->speed -= BOAT_SLOWDOWN;
			else
				sBoatItem->speed = 0;
		}
		else
		{
			if (TrInput & (SBOAT_IN_LEFT | SBOAT_IN_RIGHT) &&
				sBoatItem->speed >= 0 && sBoatItem->speed < BOAT_MIN_SPEED)
			{
				if (sBoatItem->speed == 0 && !(TrInput & SBOAT_IN_DISMOUNT))
					sBoatItem->speed = BOAT_MIN_SPEED;
			}
			else if (sBoatItem->speed > BOAT_SLOWDOWN)
				sBoatItem->speed -= BOAT_SLOWDOWN;
			else
				sBoatItem->speed = 0;

			if (TrInput & IN_LOOK && sBoatItem->speed == 0)
				LookUpDown();
		}
	}

	return noTurn;
}

void SpeedBoatAnimation(ITEM_INFO* laraItem, ITEM_INFO* sBoatItem, int collide)
{
	BOAT_INFO* sBoatInfo = sBoatItem->data;

	if (laraItem->hitPoints <= 0)
	{
		if (laraItem->currentAnimState != SBOAT_STATE_DEATH)
		{
			laraItem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_DEATH;
			laraItem->frameNumber = g_Level.Anims[laraItem->animNumber].frameBase;
			laraItem->currentAnimState = laraItem->goalAnimState = SBOAT_STATE_DEATH;
		}
	}
	else if (sBoatItem->pos.yPos < sBoatInfo->water - CLICK(0.5f) && sBoatItem->fallspeed > 0)
	{
		if (laraItem->currentAnimState != SBOAT_STATE_FALL)
		{
			laraItem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_LEAP_START;
			laraItem->frameNumber = g_Level.Anims[laraItem->animNumber].frameBase;
			laraItem->currentAnimState = laraItem->goalAnimState = SBOAT_STATE_FALL;
		}
	}
	else if (collide)
	{
		if (laraItem->currentAnimState != SBOAT_STATE_HIT)
		{
			laraItem->animNumber = (short)(Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + collide);
			laraItem->frameNumber = g_Level.Anims[laraItem->animNumber].frameBase;
			laraItem->currentAnimState = laraItem->goalAnimState = SBOAT_STATE_HIT;
		}
	}
	else
	{
		switch (laraItem->currentAnimState)
		{
		case SBOAT_STATE_IDLE:
			if (TrInput & SBOAT_IN_DISMOUNT)
			{
				if (sBoatItem->speed == 0)
				{
					if (TrInput & SBOAT_IN_RIGHT && TestSpeedBoatDismount(sBoatItem, sBoatItem->pos.yRot + ANGLE(90.0f)))
						laraItem->goalAnimState = SBOAT_STATE_DISMOUNT_RIGHT;
					else if (TrInput & SBOAT_IN_LEFT && TestSpeedBoatDismount(sBoatItem, sBoatItem->pos.yRot - ANGLE(90.0f)))
						laraItem->goalAnimState = SBOAT_STATE_DISMOUNT_LEFT;
				}
			}

			if (sBoatItem->speed > 0)
				laraItem->goalAnimState = SBOAT_STATE_MOVING;

			break;

		case SBOAT_STATE_MOVING:
			if (TrInput & SBOAT_IN_DISMOUNT)
			{
				if (TrInput & SBOAT_IN_RIGHT)
					laraItem->goalAnimState = SBOAT_STATE_DISMOUNT_RIGHT;
				else if (TrInput & SBOAT_IN_RIGHT)
					laraItem->goalAnimState = SBOAT_STATE_DISMOUNT_LEFT;
			}
			else if (sBoatItem->speed <= 0)
				laraItem->goalAnimState = SBOAT_STATE_IDLE;

			break;

		case SBOAT_STATE_FALL:
			laraItem->goalAnimState = SBOAT_STATE_MOVING;
			break;

		//case BOAT_TURNR:
			if (sBoatItem->speed <= 0)
				laraItem->goalAnimState = SBOAT_STATE_IDLE;
			else if (!(TrInput & SBOAT_IN_RIGHT))
				laraItem->goalAnimState = SBOAT_STATE_MOVING;

			break;

		case SBOAT_STATE_TURN_LEFT:
			if (sBoatItem->speed <= 0)
				laraItem->goalAnimState = SBOAT_STATE_IDLE;
			else if (!(TrInput & SBOAT_IN_LEFT))
				laraItem->goalAnimState = SBOAT_STATE_MOVING;

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
	LaraInfo*& laraInfo = laraItem->data;

	if (laraItem->hitPoints < 0 || laraInfo->Vehicle != NO_ITEM)
		return;

	auto sBoatItem = &g_Level.Items[itemNum];

	switch (GetSpeedBoatMountType(laraItem, sBoatItem, coll))
	{
	case BoatMountType::None:
		coll->Setup.EnableObjectPush = true;
		ObjectCollision(itemNum, laraItem, coll);
		return;

	case BoatMountType::WaterLeft:
		laraItem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_MOUNT_LEFT;
		break;

	case BoatMountType::WaterRight:
		laraItem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_MOUNT_RIGHT;
		break;

	case BoatMountType::Jump:
		laraItem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_MOUNT_JUMP;
		break;

	case BoatMountType::StartPosition:
		laraItem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + SBOAT_ANIM_IDLE;
		break;
	}

	laraInfo->waterStatus = LW_ABOVE_WATER;
	laraItem->pos.xPos = sBoatItem->pos.xPos;
	laraItem->pos.yPos = sBoatItem->pos.yPos - 5;
	laraItem->pos.zPos = sBoatItem->pos.zPos;
	laraItem->pos.yRot = sBoatItem->pos.yRot;
	laraItem->pos.xRot = 0;
	laraItem->pos.zRot = 0;
	laraItem->gravityStatus = false;
	laraItem->speed = 0;
	laraItem->fallspeed = 0;
	laraItem->frameNumber = g_Level.Anims[laraItem->animNumber].frameBase;
	laraItem->currentAnimState = SBOAT_STATE_MOUNT;
	laraItem->goalAnimState = SBOAT_STATE_MOUNT;

	if (laraItem->roomNumber != sBoatItem->roomNumber)
		ItemNewRoom(laraInfo->itemNumber, sBoatItem->roomNumber);

	AnimateItem(laraItem);

	if (g_Level.Items[itemNum].status != ITEM_ACTIVE)
	{
		AddActiveItem(itemNum);
		g_Level.Items[itemNum].status = ITEM_ACTIVE;
	}
 
	laraInfo->Vehicle = itemNum;
}

void SpeedBoatControl(short itemNum)
{
	auto* laraItem = LaraItem;
	LaraInfo*& laraInfo = laraItem->data;
	auto sBoatItem = &g_Level.Items[itemNum];
	BOAT_INFO* sBoatInfo = sBoatItem->data;

	PHD_VECTOR frontLeft, frontRight;
	bool noTurn = true;
	bool drive = false;

	int collide = SpeedBoatDynamics(laraItem, itemNum);

	int heightFrontLeft = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, -BOAT_SIDE, &frontLeft);
	int heightFrontRight = SpeedBoatTestWaterHeight(sBoatItem, BOAT_FRONT, BOAT_SIDE, &frontRight);

	auto roomNum = sBoatItem->roomNumber;
	auto floor = GetFloor(sBoatItem->pos.xPos, sBoatItem->pos.yPos, sBoatItem->pos.zPos, &roomNum);
	auto height = GetFloorHeight(floor, sBoatItem->pos.xPos, sBoatItem->pos.yPos, sBoatItem->pos.zPos);
	auto ceiling = GetCeiling(floor, sBoatItem->pos.xPos, sBoatItem->pos.yPos, sBoatItem->pos.zPos);

	if (laraInfo->Vehicle == itemNum)
	{
		TestTriggers(sBoatItem, true);
		TestTriggers(sBoatItem, false);
	}

	auto water = GetWaterHeight(sBoatItem->pos.xPos, sBoatItem->pos.yPos, sBoatItem->pos.zPos, roomNum);
	sBoatInfo->water = water;

	if (laraInfo->Vehicle == itemNum && laraItem->hitPoints > 0)
	{
		switch (laraItem->currentAnimState)
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
		if (sBoatItem->speed > BOAT_SLOWDOWN)
			sBoatItem->speed -= BOAT_SLOWDOWN;
		else
			sBoatItem->speed = 0;
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

	sBoatItem->floor = height - 5;
	if (sBoatInfo->water == NO_HEIGHT)
		sBoatInfo->water = height;
	else
		sBoatInfo->water -= 5;

	sBoatInfo->leftFallspeed = SpeedBoatDoBoatDynamics(heightFrontLeft, sBoatInfo->leftFallspeed, (int*)&frontLeft.y);
	sBoatInfo->rightFallspeed = SpeedBoatDoBoatDynamics(heightFrontRight, sBoatInfo->rightFallspeed, (int*)&frontRight.y);
	auto ofs = sBoatItem->fallspeed;
	sBoatItem->fallspeed = SpeedBoatDoBoatDynamics(sBoatInfo->water, sBoatItem->fallspeed, (int*)&sBoatItem->pos.yPos);
	if (ofs - sBoatItem->fallspeed > 32 && sBoatItem->fallspeed == 0 && water != NO_HEIGHT)
		SpeedBoatSplash(sBoatItem, ofs - sBoatItem->fallspeed, water);

	height = (frontLeft.y + frontRight.y);
	if (height < 0)
		height = -(abs(height) / 2);
	else
		height /= 2;

	short xRot = phd_atan(BOAT_FRONT, sBoatItem->pos.yPos - height);
	short zRot = phd_atan(BOAT_SIDE, height - frontLeft.y);

	sBoatItem->pos.xRot += ((xRot - sBoatItem->pos.xRot) / 2);
	sBoatItem->pos.zRot += ((zRot - sBoatItem->pos.zRot) / 2);
 
	if (!xRot && abs(sBoatItem->pos.xRot) < 4)
		sBoatItem->pos.xRot = 0;
	if (!zRot && abs(sBoatItem->pos.zRot) < 4)
		sBoatItem->pos.zRot = 0;

	if (laraInfo->Vehicle == itemNum)
	{
		SpeedBoatAnimation(laraItem, sBoatItem, collide);

		if (roomNum != sBoatItem->roomNumber)
		{
			ItemNewRoom(laraInfo->Vehicle, roomNum);
			ItemNewRoom(laraInfo->itemNumber, roomNum);
		}

		sBoatItem->pos.zRot += sBoatInfo->tiltAngle;
		laraItem->pos.xPos = sBoatItem->pos.xPos;
		laraItem->pos.yPos = sBoatItem->pos.yPos;
		laraItem->pos.zPos = sBoatItem->pos.zPos;
		laraItem->pos.xRot = sBoatItem->pos.xRot;
		laraItem->pos.yRot = sBoatItem->pos.yRot;
		laraItem->pos.zRot = sBoatItem->pos.zRot;

		AnimateItem(laraItem);

		if (laraItem->hitPoints > 0)
		{
			sBoatItem->animNumber = Objects[ID_SPEEDBOAT].animIndex + (laraItem->animNumber - Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex);
			sBoatItem->frameNumber = g_Level.Anims[sBoatItem->animNumber].frameBase + (laraItem->frameNumber - g_Level.Anims[laraItem->animNumber].frameBase);
		}

		Camera.targetElevation = -ANGLE(20.0f);
		Camera.targetDistance = WALL_SIZE * 2;
	}
	else
	{
		if (roomNum != sBoatItem->roomNumber)
			ItemNewRoom(itemNum, roomNum);

		sBoatItem->pos.zRot += sBoatInfo->tiltAngle;
	}

	auto pitch = sBoatItem->speed;
	sBoatInfo->pitch += ((pitch - sBoatInfo->pitch) / 4);

	int fx = (sBoatItem->speed > 8) ? SFX_TR2_BOAT_MOVING : (drive ? SFX_TR2_BOAT_IDLE : SFX_TR2_BOAT_ACCELERATE);
	SoundEffect(fx, &sBoatItem->pos, 0, sBoatInfo->pitch / (float)BOAT_MAX_SPEED);

	if (sBoatItem->speed && water - 5 == sBoatItem->pos.yPos)
		DoBoatWakeEffect(sBoatItem);

	if (laraInfo->Vehicle != itemNum)
		return;

	DoSpeedBoatDismount(laraItem, sBoatItem);
}
