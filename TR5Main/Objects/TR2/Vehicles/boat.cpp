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
#include "sound.h"
#include "effect2.h"
#include "particle/SimpleParticle.h"
#include "boat_info.h"
enum BOAT_STATE
{
	STATE_BOAT_GETON,
	STATE_BOAT_STILL,
	BOAT_MOVING,
	STATE_BOAT_JUMPR,
	STATE_BOAT_JUMPL,
	STATE_BOAT_HIT,
	STATE_BOAT_FALL,
	STATE_BOAT_TURNR,
	STATE_BOAT_DEATH,
	STATE_BOAT_TURNL
};

#define	IN_ACCELERATE		IN_FORWARD
#define	IN_REVERSE			IN_BACK
#define	IN_DISMOUNT			IN_JUMP
#define	IN_TURBO			(IN_ACTION)
#define	IN_TURNL			(IN_LEFT|IN_LSTEP)
#define	IN_TURNR			(IN_RIGHT|IN_RSTEP)
#define BOAT_GETONLW_ANIM	0
#define BOAT_GETONRW_ANIM	8
#define BOAT_GETONJ_ANIM	6
#define BOAT_GETON_START	1
#define BOAT_FALL_ANIM		15
#define BOAT_DEATH_ANIM		18
#define BOAT_UNDO_TURN		(ONE_DEGREE/4)
#define BOAT_TURN			(ONE_DEGREE/8)
#define BOAT_MAX_TURN		ANGLE(4)
#define BOAT_MAX_SPEED		110
#define BOAT_SLOW_SPEED		(BOAT_MAX_SPEED/3)
#define BOAT_FAST_SPEED		(BOAT_MAX_SPEED+75)
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
#define BOAT_MAX_HEIGHT		(STEP_SIZE)
#define GETOFF_DIST			(1024)
#define BOAT_WAKE			700
#define BOAT_SOUND_CEILING	(WALL_SIZE*5)
#define BOAT_TIP			(BOAT_FRONT+250)
#define SKIDOO_HIT_LEFT		11
#define SKIDOO_HIT_RIGHT	12
#define SKIDOO_HIT_FRONT	13
#define SKIDOO_HIT_BACK		14

void DoBoatWakeEffect(ITEM_INFO* boat)
{
	SetupRipple(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, 512, RIPPLE_FLAG_RAND_POS, Objects[1368].meshIndex, TO_RAD(boat->pos.yRot));
	ten::Effects::TriggerSpeedboatFoam(boat);

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
		spark->transType = COLADD;
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
		spark->transType = COLADD;
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

void SpeedBoatGetOff(ITEM_INFO* boat)
{
	if ((LaraItem->currentAnimState == STATE_BOAT_JUMPR 
		|| LaraItem->currentAnimState == STATE_BOAT_JUMPL) 
		&& LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd)
	{
		if (LaraItem->currentAnimState == STATE_BOAT_JUMPL)
			LaraItem->pos.yRot -= ANGLE(90);
		else
			LaraItem->pos.yRot += ANGLE(90);

		LaraItem->animNumber = LA_JUMP_FORWARD;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		LaraItem->currentAnimState = LaraItem->goalAnimState = LS_JUMP_FORWARD;
		LaraItem->gravityStatus = true;
		LaraItem->fallspeed = -40;
		LaraItem->speed = 20;
		LaraItem->pos.xRot = LaraItem->pos.zRot = 0;
		Lara.Vehicle = NO_ITEM;

		int x = LaraItem->pos.xPos + 360 * phd_sin(LaraItem->pos.yRot);
		int y = LaraItem->pos.yPos - 90;
		int z = LaraItem->pos.zPos + 360 * phd_cos(LaraItem->pos.yRot);
		
		short roomNumber = LaraItem->roomNumber;
		FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
		if (GetFloorHeight(floor, x, y, z) >= y - STEP_SIZE)
		{
			LaraItem->pos.xPos = x;
			LaraItem->pos.zPos = z;
			if (roomNumber != LaraItem->roomNumber)
				ItemNewRoom(Lara.itemNumber, roomNumber);
		}
		LaraItem->pos.yPos = y;

		boat->animNumber = Objects[ID_SPEEDBOAT].animIndex;
		boat->frameNumber = g_Level.Anims[boat->animNumber].frameBase;
	}
}

bool SpeedBoatCanGetOff(int direction)
{
	ITEM_INFO* v = &g_Level.Items[Lara.Vehicle];

	short angle;
	if (direction < 0)
		angle = v->pos.yRot - ANGLE(90);
	else
		angle = v->pos.yRot + ANGLE(90);

	int x = v->pos.xPos + GETOFF_DIST * phd_sin(angle);
	int y = v->pos.yPos;
	int z = v->pos.zPos + GETOFF_DIST * phd_cos(angle);

	short roomNumber = v->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height = GetFloorHeight(floor, x, y, z);

	if ((height - v->pos.yPos) < -(WALL_SIZE / 2))
		return false;

	if ((HeightType == BIG_SLOPE) || (HeightType == DIAGONAL))
		return false;

	int ceiling = GetCeiling(floor, x, y, z);
	if ((ceiling - v->pos.yPos > -LARA_HITE) || (height - ceiling < LARA_HITE))
		return false;

	return true;
}

BOAT_GETON SpeedBoatCheckGeton(short itemNum, COLL_INFO* coll)
{
	BOAT_GETON geton = BOAT_GETON::NONE;

	if (Lara.gunStatus != LG_NO_ARMS)
		return BOAT_GETON::NONE;

	ITEM_INFO* boat = &g_Level.Items[itemNum];

	if (!TestBoundsCollide(boat, LaraItem, coll->radius))
		return BOAT_GETON::NONE;

	if (!TestCollision(boat, LaraItem))
		return BOAT_GETON::NONE;

	int dist = (LaraItem->pos.zPos - boat->pos.zPos) * phd_cos(-boat->pos.yRot) - (LaraItem->pos.xPos - boat->pos.xPos) * phd_sin(-boat->pos.yRot);
	if (dist > 200)
		return BOAT_GETON::NONE;

	short rot = boat->pos.yRot - LaraItem->pos.yRot;
	if (Lara.waterStatus == LW_SURFACE || Lara.waterStatus == LW_WADE)
	{
		if (!(TrInput & IN_ACTION) || LaraItem->gravityStatus || boat->speed)
			return BOAT_GETON::NONE;

		if (rot > ANGLE(45) && rot < ANGLE(135))
			geton = BOAT_GETON::WATER_RIGHT;
		else if (rot > -ANGLE(135) && rot < -ANGLE(45))
			geton = BOAT_GETON::WATER_LEFT;
	}
	else if (Lara.waterStatus == LW_ABOVE_WATER)
	{
		if (LaraItem->fallspeed > 0)
		{
			if (rot > -ANGLE(135) && rot < ANGLE(135) && (LaraItem->pos.yPos + 512) > boat->pos.yPos)
				geton = BOAT_GETON::JUMP;
		}
		else if (LaraItem->fallspeed == 0)
		{
			if (rot > -ANGLE(135) && rot < ANGLE(135))
			{
				if (LaraItem->pos.xPos == boat->pos.xPos
					&& LaraItem->pos.yPos == boat->pos.yPos 
					&& LaraItem->pos.zPos == boat->pos.zPos)
					geton = BOAT_GETON::STARTPOS;
				else
					geton = BOAT_GETON::JUMP;
			}
		}
	}
	return geton;
}

int SpeedBoatTestWaterHeight(ITEM_INFO* item, int zOff, int xOff, PHD_VECTOR* pos)
{
	pos->y = item->pos.yPos - zOff * phd_sin(item->pos.xRot) + xOff * phd_sin(item->pos.zRot);

	float s = phd_sin(item->pos.yRot);
	float c = phd_cos(item->pos.yRot);
	
	pos->x = item->pos.xPos + zOff * s + xOff * c;
	pos->z = item->pos.zPos + zOff * c - xOff * s;
	
	short roomNumber = item->roomNumber;
	GetFloor(pos->x, pos->y, pos->z, &roomNumber);
	int height = GetWaterHeight(pos->x, pos->y, pos->z, roomNumber);
	if (height == NO_HEIGHT)
	{
		FLOOR_INFO* floor = GetFloor(pos->x, pos->y, pos->z, &roomNumber);
		height = GetFloorHeight(floor, pos->x, pos->y, pos->z);
		if (height == NO_HEIGHT)
			return height;
	}

	return height - 5;
}

void SpeedBoatDoBoatShift(int itemNum)
{
	ITEM_INFO* item, * boat;
	int item_number, distance, x, z, radius;

	boat = &g_Level.Items[itemNum];

	item_number = g_Level.Rooms[boat->roomNumber].itemNumber;
	while (item_number != NO_ITEM)
	{
		item = &g_Level.Items[item_number];

		if (item->objectNumber == ID_SPEEDBOAT && item_number != itemNum && Lara.Vehicle != item_number)
		{
			x = item->pos.xPos - boat->pos.xPos;
			z = item->pos.zPos - boat->pos.zPos;
			radius = SQUARE(BOAT_RADIUS * 2);
			distance = SQUARE(x) + SQUARE(z);
			if (distance < radius)
			{
				boat->pos.xPos = item->pos.xPos - x * radius / distance;
				boat->pos.zPos = item->pos.zPos - z * radius / distance;
			}
			return;
		}

		// TODO: mine and gondola

		item_number = item->nextItem;
	}
}

short SpeedBoatDoShif(ITEM_INFO* skidoo, PHD_VECTOR* pos, PHD_VECTOR* old)
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
		x = z = 0;

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

int SpeedBoatGetCollisionAnim(ITEM_INFO* skidoo, PHD_VECTOR* moved)
{
	moved->x = skidoo->pos.xPos - moved->x;
	moved->z = skidoo->pos.zPos - moved->z;

	if (moved->x || moved->z)
	{
		float s = phd_sin(skidoo->pos.yRot);
		float c = phd_cos(skidoo->pos.yRot);
		
		int front = moved->z * c + moved->x * s;
		int side = -moved->z * s + moved->x * c;
		
		if (abs(front) > abs(side))
		{
			if (front > 0)
				return SKIDOO_HIT_BACK;
			else
				return SKIDOO_HIT_FRONT;
		}
		else
		{
			if (side > 0)
				return SKIDOO_HIT_LEFT;
			else
				return SKIDOO_HIT_RIGHT;
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

int SpeedBoatDynamics(short itemNum)
{
	ITEM_INFO* boat;
	BOAT_INFO* binfo;
	PHD_VECTOR moved, fl, fr, br, bl, f;
	PHD_VECTOR old, fl_old, fr_old, bl_old, br_old, f_old;
	int hfl, hfr, hbr, hbl, hf;
	int hfr_old, hfl_old, hbr_old, hbl_old, hf_old;
	FLOOR_INFO* floor;
	int height, slip, collide;
	short roomNumber, rot;
	int newspeed;

	boat = &g_Level.Items[itemNum];
	binfo = (BOAT_INFO*)boat->data;

	boat->pos.zRot -= binfo->tiltAngle;

	hfl_old = SpeedBoatTestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl_old);
	hfr_old = SpeedBoatTestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr_old);
	hbl_old = SpeedBoatTestWaterHeight(boat, -BOAT_FRONT, -BOAT_SIDE, &bl_old);
	hbr_old = SpeedBoatTestWaterHeight(boat, -BOAT_FRONT, BOAT_SIDE, &br_old);
	hf_old = SpeedBoatTestWaterHeight(boat, BOAT_TIP, 0, &f_old);
	old.x = boat->pos.xPos;
	old.y = boat->pos.yPos;
	old.z = boat->pos.zPos;

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

	boat->pos.yRot += binfo->boatTurn + binfo->extraRotation;
	binfo->tiltAngle = binfo->boatTurn * 6;

	boat->pos.xPos += boat->speed * phd_sin(boat->pos.yRot);
	boat->pos.zPos += boat->speed * phd_cos(boat->pos.yRot);
	
	slip = BOAT_SIDE_SLIP * phd_sin(boat->pos.zRot);
	if (!slip && boat->pos.zRot)
		slip = (boat->pos.zRot > 0) ? 1 : -1;
	boat->pos.xPos += slip * phd_sin(boat->pos.yRot);
	boat->pos.zPos -= slip * phd_cos(boat->pos.yRot);
	
	slip = BOAT_SLIP * phd_sin(boat->pos.xRot);
	if (!slip && boat->pos.xRot)
		slip = (boat->pos.xRot > 0) ? 1 : -1;
	boat->pos.xPos -= slip * phd_sin(boat->pos.yRot);
	boat->pos.zPos -= slip * phd_cos(boat->pos.yRot);
	
	moved.x = boat->pos.xPos;
	moved.z = boat->pos.zPos;

	SpeedBoatDoBoatShift(itemNum);

	rot = 0;
	hbl = SpeedBoatTestWaterHeight(boat, -BOAT_FRONT, -BOAT_SIDE, &bl);
	if (hbl < bl_old.y - STEP_SIZE / 2)
		rot = SpeedBoatDoShif(boat, &bl, &bl_old);

	hbr = SpeedBoatTestWaterHeight(boat, -BOAT_FRONT, BOAT_SIDE, &br);
	if (hbr < br_old.y - STEP_SIZE / 2)
		rot += SpeedBoatDoShif(boat, &br, &br_old);

	hfl = SpeedBoatTestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl);
	if (hfl < fl_old.y - STEP_SIZE / 2)
		rot += SpeedBoatDoShif(boat, &fl, &fl_old);

	hfr = SpeedBoatTestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr);
	if (hfr < fr_old.y - STEP_SIZE / 2)
		rot += SpeedBoatDoShif(boat, &fr, &fr_old);

	if (!slip)
	{
		hf = SpeedBoatTestWaterHeight(boat, BOAT_TIP, 0, &f);
		if (hf < f_old.y - STEP_SIZE / 2)
			SpeedBoatDoShif(boat, &f, &f_old);
	}

	roomNumber = boat->roomNumber;
	floor = GetFloor(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, &roomNumber);
	height = GetWaterHeight(boat->pos.xPos, boat->pos.yPos - 5, boat->pos.zPos, roomNumber);

	if (height == NO_HEIGHT)
		height = GetFloorHeight(floor, boat->pos.xPos, boat->pos.yPos - 5, boat->pos.zPos);

	if (height < boat->pos.yPos - (STEP_SIZE / 2))
		SpeedBoatDoShif(boat, (PHD_VECTOR*)&boat->pos, &old);

	binfo->extraRotation = rot;

	collide = SpeedBoatGetCollisionAnim(boat, &moved);

	if (slip || collide)
	{
		newspeed = (boat->pos.zPos - old.z) * phd_cos(boat->pos.yRot) + (boat->pos.xPos - old.x) * phd_sin(boat->pos.yRot);

		if (Lara.Vehicle == itemNum && boat->speed > BOAT_MAX_SPEED + BOAT_ACCELERATION && newspeed < boat->speed - 10)
		{
			LaraItem->hitPoints -= boat->speed;
			LaraItem->hitStatus = 1;
			SoundEffect(SFX_TR4_LARA_INJURY, &LaraItem->pos, 0);
			newspeed /= 2;
			boat->speed /= 2;
		}

		if (slip)
		{ 
			if (boat->speed <= BOAT_MAX_SPEED + 10)
				boat->speed = newspeed;
		}
		else
		{
			if (boat->speed > 0 && newspeed < boat->speed)
				boat->speed = newspeed;
			else if (boat->speed < 0 && newspeed > boat->speed)
				boat->speed = newspeed;
		}

		if (boat->speed < BOAT_MAX_BACK)
			boat->speed = BOAT_MAX_BACK;
	}

	return collide;
}

bool SpeedBoatUserControl(ITEM_INFO* boat)
{
	int no_turn = 1, max_speed;
	
	BOAT_INFO* binfo = (BOAT_INFO*)boat->data;

	if (boat->pos.yPos >= binfo->water - STEP_SIZE / 2 && binfo->water != NO_HEIGHT)
	{
		if ((!(TrInput & IN_DISMOUNT) && !(TrInput & IN_LOOK)) || boat->speed)
		{
			if (((TrInput & IN_TURNL) && !(TrInput & IN_REVERSE)) || ((TrInput & IN_TURNR) && (TrInput & IN_REVERSE)))
			{
				if (binfo->boatTurn > 0)
					binfo->boatTurn -= BOAT_UNDO_TURN;
				else
				{
					binfo->boatTurn -= BOAT_TURN;
					if (binfo->boatTurn < -BOAT_MAX_TURN)
						binfo->boatTurn = -BOAT_MAX_TURN;
				}
				no_turn = 0;
			}
			else if (((TrInput & IN_TURNR) && !(TrInput & IN_REVERSE)) || ((TrInput & IN_TURNL) && (TrInput & IN_REVERSE)))
			{
				if (binfo->boatTurn < 0)
					binfo->boatTurn += BOAT_UNDO_TURN;
				else
				{
					binfo->boatTurn += BOAT_TURN;
					if (binfo->boatTurn > BOAT_MAX_TURN)
						binfo->boatTurn = BOAT_MAX_TURN;
				}
				no_turn = 0;
			}

			if (TrInput & IN_REVERSE)
			{
				if (boat->speed > 0)
					boat->speed -= BOAT_BRAKE;
				else if (boat->speed > BOAT_MAX_BACK)
					boat->speed += BOAT_REVERSE;
			}
			else if (TrInput & IN_ACCELERATE)
			{
				if (TrInput & IN_TURBO)
					max_speed = BOAT_FAST_SPEED;
				else
					max_speed = (TrInput & IN_STEPSHIFT) ? BOAT_SLOW_SPEED : BOAT_MAX_SPEED;

				if (boat->speed < max_speed)
					boat->speed += BOAT_ACCELERATION / 2 + BOAT_ACCELERATION * boat->speed / (2 * max_speed);
				else if (boat->speed > max_speed + BOAT_SLOWDOWN)
					boat->speed -= BOAT_SLOWDOWN;
			}
			else if (boat->speed >= 0 && boat->speed < BOAT_MIN_SPEED && (TrInput & (IN_TURNL | IN_TURNR)))
			{
				if (boat->speed == 0 && !(TrInput & IN_DISMOUNT))
					boat->speed = BOAT_MIN_SPEED;
			}
			else if (boat->speed > BOAT_SLOWDOWN)
				boat->speed -= BOAT_SLOWDOWN;
			else
				boat->speed = 0;
		}
		else
		{
			if (boat->speed >= 0 && boat->speed < BOAT_MIN_SPEED && (TrInput & (IN_TURNL | IN_TURNR)))
			{
				if (boat->speed == 0 && !(TrInput & IN_DISMOUNT))
					boat->speed = BOAT_MIN_SPEED;
			}
			else if (boat->speed > BOAT_SLOWDOWN)
				boat->speed -= BOAT_SLOWDOWN;
			else
				boat->speed = 0;

			if ((TrInput & IN_LOOK) && boat->speed == 0)
				LookUpDown();
		}
	}

	return no_turn;
}

void SpeedBoatAnimation(ITEM_INFO* boat, int collide)
{
	BOAT_INFO* binfo;

	binfo = (BOAT_INFO*)boat->data;

	if (LaraItem->hitPoints <= 0)
	{
		if (LaraItem->currentAnimState != STATE_BOAT_DEATH)
		{
			LaraItem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + BOAT_DEATH_ANIM;
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = LaraItem->goalAnimState = STATE_BOAT_DEATH;
		}
	}
	else if (boat->pos.yPos < binfo->water - (STEP_SIZE / 2) && boat->fallspeed > 0)
	{
		if (LaraItem->currentAnimState != STATE_BOAT_FALL)
		{
			LaraItem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + BOAT_FALL_ANIM;
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = LaraItem->goalAnimState = STATE_BOAT_FALL;
		}
	}
	else if (collide)
	{
		if (LaraItem->currentAnimState != STATE_BOAT_HIT)
		{
			LaraItem->animNumber = (short)(Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + collide);
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = LaraItem->goalAnimState = STATE_BOAT_HIT;
		}
	}
	else
	{
		switch (LaraItem->currentAnimState)
		{
		case STATE_BOAT_STILL:
			if (TrInput & IN_JUMP)
			{
				if (boat->speed == 0)
				{
					if ((TrInput & IN_TURNR) && SpeedBoatCanGetOff(boat->pos.yRot + ANGLE(90)))
						LaraItem->goalAnimState = STATE_BOAT_JUMPR;
					else if (TrInput & IN_TURNL && SpeedBoatCanGetOff(boat->pos.yRot - ANGLE(90)))
						LaraItem->goalAnimState = STATE_BOAT_JUMPL;
				}
			}
			if (boat->speed > 0)
				LaraItem->goalAnimState = BOAT_MOVING;
			break;
		case BOAT_MOVING:
			if (TrInput & IN_JUMP)
			{
				if (TrInput & IN_RIGHT)
					LaraItem->goalAnimState = STATE_BOAT_JUMPR;
				else if (TrInput & IN_LEFT)
					LaraItem->goalAnimState = STATE_BOAT_JUMPL;
			}
			else if (boat->speed <= 0)
				LaraItem->goalAnimState = STATE_BOAT_STILL;
			break;

		case STATE_BOAT_FALL:
			LaraItem->goalAnimState = BOAT_MOVING;
			break;
		//case BOAT_TURNR:
			if (boat->speed <= 0)
				LaraItem->goalAnimState = STATE_BOAT_STILL;
			else if (!(TrInput & IN_TURNR))
				LaraItem->goalAnimState = BOAT_MOVING;
			break;
		case STATE_BOAT_TURNL:
			if (boat->speed <= 0)
				LaraItem->goalAnimState = STATE_BOAT_STILL;
			else if (!(TrInput & IN_TURNL))
				LaraItem->goalAnimState = BOAT_MOVING;
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

void InitialiseSpeedBoat(short itemNum)
{
	ITEM_INFO* boat;
	BOAT_INFO* binfo;

	boat = &g_Level.Items[itemNum];
	binfo = game_malloc<BOAT_INFO>();
	boat->data = (void*)binfo;
	binfo->boatTurn = 0;
	binfo->leftFallspeed = 0;
	binfo->rightFallspeed = 0;
	binfo->tiltAngle = 0;
	binfo->extraRotation = 0;
	binfo->water = 0;
	binfo->pitch = 0;
}

void SpeedBoatCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll)
{
	int geton;
	ITEM_INFO* boat;

	if (litem->hitPoints < 0 || Lara.Vehicle != NO_ITEM)
		return;

	boat = &g_Level.Items[itemNum];

	geton = SpeedBoatCheckGeton(itemNum, coll);
	if (!geton)
	{
		coll->enableBaddiePush = true;
		ObjectCollision(itemNum, litem, coll);
		return;
	}

	if (geton == 2)
		litem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + BOAT_GETONLW_ANIM;
	else if (geton == 1)
		litem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + BOAT_GETONRW_ANIM;
	else if (geton == 3)
		litem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + BOAT_GETONJ_ANIM;
	else
		litem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + BOAT_GETON_START;

	Lara.waterStatus = LW_ABOVE_WATER;
	litem->pos.xPos = boat->pos.xPos;
	litem->pos.yPos = boat->pos.yPos - 5;
	litem->pos.zPos = boat->pos.zPos;
	litem->pos.yRot = boat->pos.yRot;
	litem->pos.xRot = litem->pos.zRot = 0;
	litem->gravityStatus = false;
	litem->speed = 0;
	litem->fallspeed = 0;
	litem->frameNumber = g_Level.Anims[litem->animNumber].frameBase;
	litem->currentAnimState = litem->goalAnimState = STATE_BOAT_GETON;

	if (litem->roomNumber != boat->roomNumber)
		ItemNewRoom(Lara.itemNumber, boat->roomNumber);

	AnimateItem(litem);

	if (g_Level.Items[itemNum].status != ITEM_ACTIVE)
	{
		AddActiveItem(itemNum);
		g_Level.Items[itemNum].status = ITEM_ACTIVE;
	}

	// TODO: play a cd when starting ! (boat)
	//S_CDPlay(12, 0);
 
	Lara.Vehicle = itemNum;
}

void SpeedBoatControl(short itemNumber)
{
	ITEM_INFO* boat;
	BOAT_INFO* binfo;
	PHD_VECTOR fl, fr, prop;
	int hfl, hfr, no_turn = 1, drive = 0;
	FLOOR_INFO* floor;
	int height, collide, water, ceiling, pitch, h, ofs, nowake;
	short roomNumber, x_rot, z_rot;

	boat = &g_Level.Items[itemNumber];
	binfo = (BOAT_INFO*)boat->data;
	collide = SpeedBoatDynamics(itemNumber);

	hfl = SpeedBoatTestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl);
	hfr = SpeedBoatTestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr);

	roomNumber = boat->roomNumber;
	floor = GetFloor(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, &roomNumber);
	height = GetFloorHeight(floor, boat->pos.xPos, boat->pos.yPos, boat->pos.zPos);
	ceiling = GetCeiling(floor, boat->pos.xPos, boat->pos.yPos, boat->pos.zPos);

	if (Lara.Vehicle == itemNumber)
	{
		TestTriggers(TriggerIndex, 0, 0);
		TestTriggers(TriggerIndex, 1, 0);
	}

	binfo->water = water = GetWaterHeight(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, roomNumber);

	if (Lara.Vehicle == itemNumber && LaraItem->hitPoints > 0)
	{
		switch (LaraItem->currentAnimState)
		{
		case STATE_BOAT_GETON:
		case STATE_BOAT_JUMPR:
		case STATE_BOAT_JUMPL:
			break;

		default:
			drive = 1;
			no_turn = SpeedBoatUserControl(boat);
			break;
		}
	}
	else
	{

		if (boat->speed > BOAT_SLOWDOWN)
			boat->speed -= BOAT_SLOWDOWN;
		else
			boat->speed = 0;
	}

	if (no_turn)
	{
		if (binfo->boatTurn < -BOAT_UNDO_TURN)
			binfo->boatTurn += BOAT_UNDO_TURN;
		else if (binfo->boatTurn > BOAT_UNDO_TURN)
			binfo->boatTurn -= BOAT_UNDO_TURN;
		else
			binfo->boatTurn = 0;
	}

	boat->floor = height - 5;
	if (binfo->water == NO_HEIGHT)
		binfo->water = height;
	else
		binfo->water -= 5;

	binfo->leftFallspeed = SpeedBoatDoBoatDynamics(hfl, binfo->leftFallspeed, (int*)&fl.y);
	binfo->rightFallspeed = SpeedBoatDoBoatDynamics(hfr, binfo->rightFallspeed, (int*)&fr.y);
	ofs = boat->fallspeed;
	boat->fallspeed = SpeedBoatDoBoatDynamics(binfo->water, boat->fallspeed, (int*)&boat->pos.yPos);
	if (ofs - boat->fallspeed > 32 && boat->fallspeed == 0 && water != NO_HEIGHT)
		SpeedBoatSplash(boat, ofs - boat->fallspeed, water);

	height = (fl.y + fr.y);
	if (height < 0)
		height = -(abs(height) / 2);
	else
		height /= 2;

	x_rot = phd_atan(BOAT_FRONT, boat->pos.yPos - height);
	z_rot = phd_atan(BOAT_SIDE, height - fl.y);

	boat->pos.xRot += ((x_rot - boat->pos.xRot) / 2);
	boat->pos.zRot += ((z_rot - boat->pos.zRot) / 2);
 
	if (!x_rot && abs(boat->pos.xRot) < 4)
		boat->pos.xRot = 0;
	if (!z_rot && abs(boat->pos.zRot) < 4)
		boat->pos.zRot = 0;

	if (Lara.Vehicle == itemNumber)
	{
		SpeedBoatAnimation(boat, collide);

		if (roomNumber != boat->roomNumber)
		{
			ItemNewRoom(Lara.Vehicle, roomNumber);
			ItemNewRoom(Lara.itemNumber, roomNumber);
		}

		boat->pos.zRot += binfo->tiltAngle;
		LaraItem->pos.xPos = boat->pos.xPos;
		LaraItem->pos.yPos = boat->pos.yPos;
		LaraItem->pos.zPos = boat->pos.zPos;
		LaraItem->pos.xRot = boat->pos.xRot;
		LaraItem->pos.yRot = boat->pos.yRot;
		LaraItem->pos.zRot = boat->pos.zRot;

		AnimateItem(LaraItem);

		if (LaraItem->hitPoints > 0)
		{
			boat->animNumber = Objects[ID_SPEEDBOAT].animIndex + (LaraItem->animNumber - Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex);
			boat->frameNumber = g_Level.Anims[boat->animNumber].frameBase + (LaraItem->frameNumber - g_Level.Anims[LaraItem->animNumber].frameBase);
		}

		Camera.targetElevation = -ANGLE(20);
		Camera.targetDistance = WALL_SIZE * 2;
	}
	else
	{
		if (roomNumber != boat->roomNumber)
			ItemNewRoom(itemNumber, roomNumber);
		boat->pos.zRot += binfo->tiltAngle;
	}

	pitch = boat->speed;
	binfo->pitch += ((pitch - binfo->pitch) / 4);

	if (boat->speed > 8)
		SoundEffect(SFX_TR2_BOAT_ACCELERATE, &boat->pos, 4 + ((0x10000 - (BOAT_MAX_SPEED - binfo->pitch) * 100) * 256));
	else if (drive)
		SoundEffect(SFX_TR2_BOAT_IDLE, &boat->pos, 4 + ((0x10000 - (BOAT_MAX_SPEED - binfo->pitch) * 100) * 256));

	if (boat->speed && water - 5 == boat->pos.yPos)
		DoBoatWakeEffect(boat);

	if (Lara.Vehicle != itemNumber)
		return;

	SpeedBoatGetOff(boat);
}