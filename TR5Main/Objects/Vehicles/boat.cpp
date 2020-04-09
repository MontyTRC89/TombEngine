#include "../newobjects.h"
#include "../../Game/lara.h"
#include "../../Game/items.h"
#include "../../Game/collide.h"
#include "../../Game/sphere.h"
#include "../../Specific/setup.h"

extern LaraExtraInfo g_LaraExtra;

enum BOAT_STATE {
	BOAT_GETON,
	BOAT_STILL,
	BOAT_MOVING,
	BOAT_JUMPR,
	BOAT_JUMPL,
	BOAT_HIT,
	BOAT_FALL,
	BOAT_TURNR,
	BOAT_DEATH,
	BOAT_TURNL
};

#define	IN_ACCELERATE		IN_ACTION
#define	IN_REVERSE			IN_JUMP
#define	IN_DISMOUNT			IN_ROLL
#define	IN_TURBO			(IN_SPRINT|IN_DUCK)
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

// TODO: (boat) render problem, water height problem, enter problem.

void GetBoatGetOff(ITEM_INFO* boat)
{
	/* Wait for last frame of getoff anims before returning to normal Lara control */
	if ((LaraItem->currentAnimState == BOAT_JUMPR || LaraItem->currentAnimState == BOAT_JUMPL) && LaraItem->frameNumber == Anims[LaraItem->animNumber].frameEnd)
	{
		short roomNumber;
		int x, y, z;
		FLOOR_INFO* floor;

		if (LaraItem->currentAnimState == BOAT_JUMPL)
			LaraItem->pos.yRot -= 0x4000;
		else
			LaraItem->pos.yRot += 0x4000;

		LaraItem->animNumber = 77;
		LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
		LaraItem->currentAnimState = LaraItem->goalAnimState = 3;
		LaraItem->gravityStatus = true;
		LaraItem->fallspeed = -40;
		LaraItem->speed = 20;
		LaraItem->pos.xRot = LaraItem->pos.zRot = 0;
		g_LaraExtra.Vehicle = NO_ITEM;

		roomNumber = LaraItem->roomNumber;
		x = LaraItem->pos.xPos + (360 * SIN(LaraItem->pos.yRot) >> W2V_SHIFT);
		y = LaraItem->pos.yPos - 90;
		z = LaraItem->pos.zPos + (360 * COS(LaraItem->pos.yRot) >> W2V_SHIFT);
		floor = GetFloor(x, y, z, &roomNumber);
		if (GetFloorHeight(floor, x, y, z) >= y - STEP_SIZE)
		{
			LaraItem->pos.xPos = x;
			LaraItem->pos.zPos = z;
			if (roomNumber != LaraItem->roomNumber)
				ItemNewRoom(Lara.itemNumber, roomNumber);
		}
		LaraItem->pos.yPos = y;

		/* Set boat to still anim */
		boat->animNumber = Objects[ID_SPEEDBOAT].animIndex;
		boat->frameNumber = Anims[boat->animNumber].frameBase;
	}
}

int CanGetOff(int direction)
{
	ITEM_INFO* v;
	FLOOR_INFO* floor;
	short roomNumber, angle;
	int x, y, z, height, ceiling;

	v = &Items[g_LaraExtra.Vehicle];

	if (direction < 0)
		angle = v->pos.yRot - 0x4000;
	else
		angle = v->pos.yRot + 0x4000;

	x = v->pos.xPos + (GETOFF_DIST * SIN(angle) >> W2V_SHIFT);
	y = v->pos.yPos;
	z = v->pos.zPos + (GETOFF_DIST * COS(angle) >> W2V_SHIFT);

	roomNumber = v->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	height = GetFloorHeight(floor, x, y, z);

	if ((height - v->pos.yPos) < -(WALL_SIZE / 2))
		return 0;

	if ((HeightType == BIG_SLOPE) || (HeightType == DIAGONAL))
		return 0;

	ceiling = GetCeiling(floor, x, y, z);
	if ((ceiling - v->pos.yPos > -LARA_HITE) || (height - ceiling < LARA_HITE))
		return 0;

	return 1;
}

int BoatCheckGetOn(short itemNum, COLL_INFO* coll)
{
	/* Returns 0 if no get on, 1 if right get on and 2 if left get on and 3 if jump geton */
	int geton = 0, dist;
	short rot;
	ITEM_INFO* boat;

	if (Lara.gunStatus != LG_NO_ARMS)
		return 0;

	boat = &Items[itemNum];

	dist = ((LaraItem->pos.zPos - boat->pos.zPos) * COS(-boat->pos.yRot) - (LaraItem->pos.xPos - boat->pos.xPos) * SIN(-boat->pos.yRot)) >> W2V_SHIFT;
	if (dist > 200)
		return 0;

	/* Check if Lara is close enough and in right position to get onto boat */
	rot = boat->pos.yRot - LaraItem->pos.yRot;
	if (Lara.waterStatus == LW_SURFACE || Lara.waterStatus == LW_WADE)
	{
		if (!(TrInput & IN_ACTION) || LaraItem->gravityStatus || boat->speed)
			return 0;

		if (rot > 0x2000 && rot < 0x6000)
			geton = 1; //right
		else if (rot > -0x6000 && rot < -0x2000)
			geton = 2; //left
	}
	else if (Lara.waterStatus == LW_ABOVE_WATER)
	{
		if (LaraItem->fallspeed > 0)
		{
			if (rot > -0x6000 && rot < 0x6000 && (LaraItem->pos.yPos + 512) > boat->pos.yPos)
				geton = 3; //jump
		}
		else if (LaraItem->fallspeed == 0)
		{
			if (rot > -0x6000 && rot < 0x6000)
			{
				if (LaraItem->pos.xPos == boat->pos.xPos && LaraItem->pos.yPos == boat->pos.yPos && LaraItem->pos.zPos == boat->pos.zPos)
					geton = 4; // must have started on same spot as boat
				else
					geton = 3; //jump
			}
		}
	}

	if (!geton)
		return 0;

	/* Is Lara actually close enough to get on the thing? */
	if (!TestBoundsCollide(boat, LaraItem, coll->radius))
		return 0;

	if (!TestCollision(boat, LaraItem))
		return 0;

	return geton;
}

int TestWaterHeight(ITEM_INFO* item, int z_off, int x_off, PHD_VECTOR* pos)
{
	/* Get water height at a position offset from the origin.
		Moves the vector in 'pos' to the required test position too */
	FLOOR_INFO* floor;
	int s, c, height;
	short roomNumber;

	/* Get y pos correctly, but don't bother changing z_off and x_off using x_rot and z_rot */
	pos->y = item->pos.yPos - (z_off * SIN(item->pos.xRot) >> W2V_SHIFT) +
							  (x_off * SIN(item->pos.zRot) >> W2V_SHIFT);

	c = COS(item->pos.yRot);
	s = SIN(item->pos.yRot);

	pos->z = item->pos.zPos + ((z_off * c - x_off * s) >> W2V_SHIFT);
	pos->x = item->pos.xPos + ((z_off * s + x_off * c) >> W2V_SHIFT);

	/* Try to get water height; if none get ground height instead */
	roomNumber = item->roomNumber;
	GetFloor(pos->x, pos->y, pos->z, &roomNumber); // get correct room (as GetWaterHeight doesn't)
	height = GetWaterHeight(pos->x, pos->y, pos->z, roomNumber);
	if (height == NO_HEIGHT)
	{
		floor = GetFloor(pos->x, pos->y, pos->z, &roomNumber);
		height = GetFloorHeight(floor, pos->x, pos->y, pos->z);
		if (height == NO_HEIGHT)
			return height;
	}

	return height - 5; // make sure boat is above water line else all sorts of weirdness results
}

void DoBoatCollision(int itemNum)
{
	ITEM_INFO* item, * boat;
	int item_number, distance, x, z, radius;

	boat = &Items[itemNum];

	/* Check if hit something in the water */
	item_number = Rooms[boat->roomNumber].itemNumber;
	while (item_number != NO_ITEM)
	{
		item = &Items[item_number];

		if (item->objectNumber == ID_SPEEDBOAT && item_number != itemNum && g_LaraExtra.Vehicle != item_number)
		{
			// other boat
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

		// TODO: adding mine and gondola ! (boat)

		item_number = item->nextItem;
	}
}

short DoBoatShift(ITEM_INFO* skidoo, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int x, z;
	int x_old, z_old;
	int shift_x, shift_z;

	x = pos->x >> WALL_SHIFT;
	z = pos->z >> WALL_SHIFT;

	x_old = old->x >> WALL_SHIFT;
	z_old = old->z >> WALL_SHIFT;

	shift_x = pos->x & (WALL_SIZE - 1);
	shift_z = pos->z & (WALL_SIZE - 1);

	if (x == x_old)
	{
		if (z == z_old)
		{
			/* Neither shift; may have hit a very steep slope, so need to push back to old position */
			skidoo->pos.zPos += (old->z - pos->z);
			skidoo->pos.xPos += (old->x - pos->x);
		}
		else if (z > z_old)
		{
			/* Z shift left */
			skidoo->pos.zPos -= shift_z + 1;
			return (pos->x - skidoo->pos.xPos);
		}
		else
		{
			/* Z shift right */
			skidoo->pos.zPos += WALL_SIZE - shift_z;
			return (skidoo->pos.xPos - pos->x);
		}
	}
	else if (z == z_old)
	{
		if (x > x_old)
		{
			/* X shift up */
			skidoo->pos.xPos -= shift_x + 1;
			return (skidoo->pos.zPos - pos->z);
		}
		else
		{
			/* X shift down */
			skidoo->pos.xPos += WALL_SIZE - shift_x;
			return (pos->z - skidoo->pos.zPos);
		}
	}
	else
	{
		/* A diagonal hit; means a barrage of tests needed to determine best shift */
		short roomNumber;
		FLOOR_INFO* floor;
		int height;

		x = z = 0;

		roomNumber = skidoo->roomNumber;
		floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
		height = GetFloorHeight(floor, old->x, pos->y, pos->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -shift_z - 1;
			else
				z = WALL_SIZE - shift_z;
		}

		roomNumber = skidoo->roomNumber;
		floor = GetFloor(pos->x, pos->y, old->z, &roomNumber);
		height = GetFloorHeight(floor, pos->x, pos->y, old->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->x > old->x)
				x = -shift_x - 1;
			else
				x = WALL_SIZE - shift_x;
		}

		if (x && z)
		{
			/* Corner or side collision */
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
			/* Pure diagonal collision */
			skidoo->pos.zPos += (old->z - pos->z);
			skidoo->pos.xPos += (old->x - pos->x);
		}
	}

	return 0;
}

int GetBoatCollisionAnim(ITEM_INFO* skidoo, PHD_VECTOR* moved)
{
	int c, s, front, side;

	moved->x = skidoo->pos.xPos - moved->x;
	moved->z = skidoo->pos.zPos - moved->z;

	if (moved->x || moved->z)
	{
		/* Get direction of movement relative to facing */
		c = COS(skidoo->pos.yRot);
		s = SIN(skidoo->pos.yRot);
		front = (moved->z * c + moved->x * s) >> W2V_SHIFT;
		side = (-moved->z * s + moved->x * c) >> W2V_SHIFT;
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

int DoBoatDynamics(int height, int fallspeed, int* y)
{
	if (height > * y)
	{
		/* In air */
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
		/* On ground: get up push from height change (if not a closed door and so NO_HEIGHT) */
		fallspeed += ((height - *y - fallspeed) >> 3);
		if (fallspeed < BOAT_MAX_BACK)
			fallspeed = BOAT_MAX_BACK;

		if (*y > height)
			*y = height;
	}

	return fallspeed;
}

int BoatDynamics(short itemNum)
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

	boat = &Items[itemNum];
	binfo = (BOAT_INFO*)boat->data;

	/* Remove tilt angle (and add again at the end) as effects control */
	boat->pos.zRot -= binfo->tilt_angle;

	/* First get positions and heights of boat's corners + centre */
	hfl_old = TestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl_old);
	hfr_old = TestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr_old);
	hbl_old = TestWaterHeight(boat, -BOAT_FRONT, -BOAT_SIDE, &bl_old);
	hbr_old = TestWaterHeight(boat, -BOAT_FRONT, BOAT_SIDE, &br_old);
	hf_old = TestWaterHeight(boat, BOAT_TIP, 0, &f_old);
	old.x = boat->pos.xPos;
	old.y = boat->pos.yPos;
	old.z = boat->pos.zPos;

	/* Back left/right may be slightly below ground, so correct for this */
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

	boat->pos.yRot += binfo->boat_turn + binfo->extra_rotation;
	binfo->tilt_angle = binfo->boat_turn * 6;

	/* Move boat according to speed */
	boat->pos.zPos += boat->speed * COS(boat->pos.yRot) >> W2V_SHIFT;
	boat->pos.xPos += boat->speed * SIN(boat->pos.yRot) >> W2V_SHIFT;
	if (boat->speed >= 0)
		binfo->prop_rot += (boat->speed * ANGLE(3));
	else
		binfo->prop_rot += ANGLE(33);

	/* Slide boat according to tilts (to avoid getting stuck on slopes) */
	slip = BOAT_SIDE_SLIP * SIN(boat->pos.zRot) >> W2V_SHIFT;
	if (!slip && boat->pos.zRot)
		slip = (boat->pos.zRot > 0) ? 1 : -1;
	boat->pos.zPos -= slip * SIN(boat->pos.yRot) >> W2V_SHIFT;
	boat->pos.xPos += slip * COS(boat->pos.yRot) >> W2V_SHIFT;

	slip = BOAT_SLIP * SIN(boat->pos.xRot) >> W2V_SHIFT;
	if (!slip && boat->pos.xRot)
		slip = (boat->pos.xRot > 0) ? 1 : -1;
	boat->pos.zPos -= slip * COS(boat->pos.yRot) >> W2V_SHIFT;
	boat->pos.xPos -= slip * SIN(boat->pos.yRot) >> W2V_SHIFT;

	/* Remember desired position in case of collisions moving us about */
	moved.x = boat->pos.xPos;
	moved.z = boat->pos.zPos;

	/* Collision with other boat? */
	DoBoatCollision(itemNum);

	/* Test new positions of points (one at a time) and shift boat accordingly */
	rot = 0;
	hbl = TestWaterHeight(boat, -BOAT_FRONT, -BOAT_SIDE, &bl);
	if (hbl < bl_old.y - STEP_SIZE / 2)
		rot = DoBoatShift(boat, &bl, &bl_old);

	hbr = TestWaterHeight(boat, -BOAT_FRONT, BOAT_SIDE, &br);
	if (hbr < br_old.y - STEP_SIZE / 2)
		rot += DoBoatShift(boat, &br, &br_old);

	hfl = TestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl);
	if (hfl < fl_old.y - STEP_SIZE / 2)
		rot += DoBoatShift(boat, &fl, &fl_old);

	hfr = TestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr);
	if (hfr < fr_old.y - STEP_SIZE / 2)
		rot += DoBoatShift(boat, &fr, &fr_old);

	if (!slip)
	{
		hf = TestWaterHeight(boat, BOAT_TIP, 0, &f);
		if (hf < f_old.y - STEP_SIZE / 2)
			DoBoatShift(boat, &f, &f_old);
	}

	roomNumber = boat->roomNumber;
	floor = GetFloor(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, &roomNumber);
	height = GetWaterHeight(boat->pos.xPos, boat->pos.yPos - 5, boat->pos.zPos, roomNumber);

	if (height == NO_HEIGHT)
		height = GetFloorHeight(floor, boat->pos.xPos, boat->pos.yPos - 5, boat->pos.zPos);

	if (height < boat->pos.yPos - (STEP_SIZE / 2))
		DoBoatShift(boat, (PHD_VECTOR*)&boat->pos, &old);

	binfo->extra_rotation = rot;

	/* Get collision anim if boat has been moved from desired position by collisions */
	collide = GetBoatCollisionAnim(boat, &moved);

	/* Check final movement if slipped or collided and adjust speed */
	if (slip || collide)
	{
		newspeed = ((boat->pos.zPos - old.z) * COS(boat->pos.yRot) + (boat->pos.xPos - old.x) * SIN(boat->pos.yRot)) >> W2V_SHIFT;

		if (boat->speed > BOAT_MAX_SPEED + BOAT_ACCELERATION && newspeed < boat->speed - 10)
		{
			LaraItem->hitPoints -= boat->speed;
			LaraItem->hitStatus = 1;
			SoundEffect(31, &LaraItem->pos, 0);
			newspeed >>= 1;
			boat->speed >>= 1;
		}

		/* Adjust speed if serious change */
		if (slip)
		{
			/* Only if slip is above certain amount, and boat is not in FAST speed range */
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

int BoatUserControl(ITEM_INFO* boat)
{
	/* Return whether to straighten up or not */
	int no_turn = 1, max_speed;
	BOAT_INFO* binfo;

	binfo = (BOAT_INFO*)boat->data;

	if (boat->pos.yPos >= binfo->water - STEP_SIZE / 2 && binfo->water != NO_HEIGHT)
	{
		/* If on the water surface, user has control; allow for reversing! */
		if ((!(TrInput & IN_DISMOUNT) && !(TrInput & IN_LOOK)) || boat->speed)
		{
			if (((TrInput & IN_TURNL) && !(TrInput & IN_REVERSE)) || ((TrInput & IN_TURNR) && (TrInput & IN_REVERSE)))
			{
				if (binfo->boat_turn > 0)
					binfo->boat_turn -= BOAT_UNDO_TURN;
				else
				{
					binfo->boat_turn -= BOAT_TURN;
					if (binfo->boat_turn < -BOAT_MAX_TURN)
						binfo->boat_turn = -BOAT_MAX_TURN;
				}
				no_turn = 0;
			}
			else if (((TrInput & IN_TURNR) && !(TrInput & IN_REVERSE)) || ((TrInput & IN_TURNL) && (TrInput & IN_REVERSE)))
			{
				if (binfo->boat_turn < 0)
					binfo->boat_turn += BOAT_UNDO_TURN;
				else
				{
					binfo->boat_turn += BOAT_TURN;
					if (binfo->boat_turn > BOAT_MAX_TURN)
						binfo->boat_turn = BOAT_MAX_TURN;
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
					boat->speed = BOAT_MIN_SPEED; // If user wants to turn, boat will move forward
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
					boat->speed = BOAT_MIN_SPEED; // If user wants to turn, boat will move forward
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

void BoatAnimation(ITEM_INFO* boat, int collide)
{
	BOAT_INFO* binfo;

	binfo = (BOAT_INFO*)boat->data;

	/* Do animation stuff */
	if (LaraItem->hitPoints <= 0)
	{
		if (LaraItem->currentAnimState != BOAT_DEATH)
		{
			LaraItem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + BOAT_DEATH_ANIM;
			LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = LaraItem->goalAnimState = BOAT_DEATH;
		}
	}
	else if (boat->pos.yPos < binfo->water - (STEP_SIZE / 2) && boat->fallspeed > 0)
	{
		if (LaraItem->currentAnimState != BOAT_FALL)
		{
			LaraItem->animNumber = Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + BOAT_FALL_ANIM;
			LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = LaraItem->goalAnimState = BOAT_FALL;
		}
	}
	else if (collide)
	{
		if (LaraItem->currentAnimState != BOAT_HIT)
		{
			LaraItem->animNumber = (short)(Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex + collide);
			LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = LaraItem->goalAnimState = BOAT_HIT;
		}
	}
	else
	{
		switch (LaraItem->currentAnimState)
		{
		case BOAT_STILL:
			if (TrInput & IN_DISMOUNT)
			{
				if (boat->speed == 0)
				{
					if ((TrInput & IN_TURNR) && CanGetOff(boat->pos.yRot + 0x4000))
						LaraItem->goalAnimState = BOAT_JUMPR;
					else if (TrInput & IN_TURNL && CanGetOff(boat->pos.yRot - 0x4000))
						LaraItem->goalAnimState = BOAT_JUMPL;
				}
			}
			if (boat->speed > 0)
				LaraItem->goalAnimState = BOAT_MOVING;
			break;
		case BOAT_MOVING:
			if (boat->speed <= 0)
				LaraItem->goalAnimState = BOAT_STILL;
			else if (TrInput & IN_TURNR)
				LaraItem->goalAnimState = BOAT_TURNR;
			else if (TrInput & IN_TURNL)
				LaraItem->goalAnimState = BOAT_TURNL;
			break;
		case BOAT_FALL:
			LaraItem->goalAnimState = BOAT_MOVING;
			break;
		case BOAT_TURNR:
			if (boat->speed <= 0)
				LaraItem->goalAnimState = BOAT_STILL;
			else if (!(TrInput & IN_TURNR))
				LaraItem->goalAnimState = BOAT_MOVING;
			break;
		case BOAT_TURNL:
			if (boat->speed <= 0)
				LaraItem->goalAnimState = BOAT_STILL;
			else if (!(TrInput & IN_TURNL))
				LaraItem->goalAnimState = BOAT_MOVING;
			break;
		}
	}
}

void BoatSplash(ITEM_INFO* item, long fallspeed, long water)
{
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

void InitialiseBoat(short itemNum)
{
	ITEM_INFO* boat;
	BOAT_INFO* binfo;

	boat = &Items[itemNum];
	binfo = (BOAT_INFO*)GameMalloc(sizeof(BOAT_INFO));
	boat->data = (void*)binfo;
	binfo->boat_turn = 0;
	binfo->left_fallspeed = 0;
	binfo->right_fallspeed = 0;
	binfo->tilt_angle = 0;
	binfo->extra_rotation = 0;
	binfo->water = 0;
	binfo->pitch = 0;
}

void BoatCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll)
{
	/* This routine is only for when Lara is not on the boat and she would like to be */
	int geton;
	ITEM_INFO* boat;

	/* If Lara dead or already on the boat, then no collision */
	if (litem->hitPoints < 0 || g_LaraExtra.Vehicle != NO_ITEM)
		return;

	boat = &Items[itemNum];

	/* If player isn't pressing control or Lara is busy, then do normal object collision */
	geton = BoatCheckGetOn(itemNum, coll);
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
	litem->frameNumber = Anims[litem->animNumber].frameBase;
	litem->currentAnimState = litem->goalAnimState = BOAT_GETON;

	if (litem->roomNumber != boat->roomNumber)
		ItemNewRoom(Lara.itemNumber, boat->roomNumber);

	AnimateItem(litem);

	/* Add to active item list from this point onward */
	if (Items[itemNum].status != ITEM_ACTIVE)
	{
		AddActiveItem(itemNum);
		Items[itemNum].status = ITEM_ACTIVE;
	}

	// TODO: play a cd when starting ! (boat)
	//S_CDPlay(12, 0);

	/* Yeeha! Get in that boat girly */
	g_LaraExtra.Vehicle = itemNum;
}

void BoatControl(short itemNumber)
{
	ITEM_INFO* boat;
	BOAT_INFO* binfo;
	PHD_VECTOR fl, fr, prop;
	int hfl, hfr, no_turn = 1, drive = 0;
	FLOOR_INFO* floor;
	int height, collide, water, ceiling, pitch, h, ofs, nowake;
	short roomNumber, x_rot, z_rot;

	boat = &Items[itemNumber];
	binfo = (BOAT_INFO*)boat->data;
	collide = BoatDynamics(itemNumber);

	/* Now got final position, so get heights under middle and corners (will only have changed
		from above if collision occurred, but recalc anyway as hardly big maths) */
	hfl = TestWaterHeight(boat, BOAT_FRONT, -BOAT_SIDE, &fl);
	hfr = TestWaterHeight(boat, BOAT_FRONT, BOAT_SIDE, &fr);

	roomNumber = boat->roomNumber;
	floor = GetFloor(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, &roomNumber);
	height = GetFloorHeight(floor, boat->pos.xPos, boat->pos.yPos, boat->pos.zPos);
	ceiling = GetCeiling(floor, boat->pos.xPos, boat->pos.yPos, boat->pos.zPos);
	
	TestTriggers(TriggerIndex, 0, 0);
	TestTriggers(TriggerIndex, 1, 0); // HEAVY too

	binfo->water = water = GetWaterHeight(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, roomNumber);

	/* Deal with user input; will effect next frame, but relies on info calced this frame */
	if (g_LaraExtra.Vehicle == itemNumber && LaraItem->hitPoints > 0)
	{
		switch (LaraItem->currentAnimState)
		{
			case BOAT_GETON:
			case BOAT_JUMPR:
			case BOAT_JUMPL:
				break;

			default:
				drive = 1;
				no_turn = BoatUserControl(boat);
				break;
		}
	}
	else
	{
		/* Boat has no driver */
		if (boat->speed > BOAT_SLOWDOWN)
			boat->speed -= BOAT_SLOWDOWN;
		else
			boat->speed = 0;
	}

	if (no_turn)
	{
		/* Straighten up if not under control */
		if (binfo->boat_turn < -BOAT_UNDO_TURN)
			binfo->boat_turn += BOAT_UNDO_TURN;
		else if (binfo->boat_turn > BOAT_UNDO_TURN)
			binfo->boat_turn -= BOAT_UNDO_TURN;
		else
			binfo->boat_turn = 0;
	}

	/* Get floor height (keep slightly above water surface) */
	boat->floor = height - 5;
	if (binfo->water == NO_HEIGHT)
		binfo->water = height;
	else
		binfo->water -= 5;

	/* Do fallspeed effects on boat */
	binfo->left_fallspeed = DoBoatDynamics(hfl, binfo->left_fallspeed, (int*)&fl.y);
	binfo->right_fallspeed = DoBoatDynamics(hfr, binfo->right_fallspeed, (int*)&fr.y);
	ofs = boat->fallspeed;
	boat->fallspeed = DoBoatDynamics(binfo->water, boat->fallspeed, (int*)&boat->pos.yPos);
	if (ofs - boat->fallspeed > 32 && boat->fallspeed == 0 && water != NO_HEIGHT)
		BoatSplash(boat, ofs - boat->fallspeed, water);

	/* Rotate boat to match these heights */
	height = (fl.y + fr.y);
	if (height < 0)
		height = -(abs(height) >> 1);
	else
		height = height >> 1;

	x_rot = ATAN(BOAT_FRONT, boat->pos.yPos - height);
	z_rot = ATAN(BOAT_SIDE, height - fl.y);

	boat->pos.xRot += (x_rot - boat->pos.xRot) >> 1;
	boat->pos.zRot += (z_rot - boat->pos.zRot) >> 1;

	/* Auto level the boat on flat water (to stop evil shifts) */
	if (!x_rot && abs(boat->pos.xRot) < 4)
		boat->pos.xRot = 0;
	if (!z_rot && abs(boat->pos.zRot) < 4)
		boat->pos.zRot = 0;

	/* If Lara is on the boat, do all her animation and move her to same position as boat, etc */
	if (g_LaraExtra.Vehicle == itemNumber)
	{
		BoatAnimation(boat, collide);

		if (roomNumber != boat->roomNumber)
		{
			ItemNewRoom(g_LaraExtra.Vehicle, roomNumber);
			ItemNewRoom(Lara.itemNumber, roomNumber);
		}

		/* Move Lara to the boat position */
		boat->pos.zRot += binfo->tilt_angle;
		LaraItem->pos.xPos = boat->pos.xPos;
		LaraItem->pos.yPos = boat->pos.yPos;
		LaraItem->pos.zPos = boat->pos.zPos;
		LaraItem->pos.xRot = boat->pos.xRot;
		LaraItem->pos.yRot = boat->pos.yRot;
		LaraItem->pos.zRot = boat->pos.zRot;

		AnimateItem(LaraItem);

		/* Set boat on the exact same anim frame */
		if (LaraItem->hitPoints > 0)
		{
			boat->animNumber = Objects[ID_SPEEDBOAT].animIndex + (LaraItem->animNumber - Objects[ID_SPEEDBOAT_LARA_ANIMS].animIndex);
			boat->frameNumber = Anims[boat->animNumber].frameBase + (LaraItem->frameNumber - Anims[LaraItem->animNumber].frameBase);
		}

		/* Set camera */
		Camera.targetElevation = -ANGLE(20);
		Camera.targetDistance = WALL_SIZE * 2;
	}
	else
	{
		/* If Lara isn't in the boat, then just move the boat */
		if (roomNumber != boat->roomNumber)
			ItemNewRoom(itemNumber, roomNumber);
		boat->pos.zRot += binfo->tilt_angle;
	}

	/* Do sound effect */
	pitch = boat->speed;
	binfo->pitch += (pitch - binfo->pitch) >> 2;

	if (boat->speed > 8)
		SoundEffect(197, &boat->pos, 4 + ((0x10000 - (BOAT_MAX_SPEED - binfo->pitch) * 100) << 8));
	else if (drive)
		SoundEffect(195, &boat->pos, 4 + ((0x10000 - (BOAT_MAX_SPEED - binfo->pitch) * 100) << 8));

	//	/* If boat is moving, then do wake */
	//	if (boat->speed && water-5 == boat->pos.y_pos)
	//		DoWakeEffect(boat);

	if (g_LaraExtra.Vehicle != itemNumber)
		return;

	GetBoatGetOff(boat);
}