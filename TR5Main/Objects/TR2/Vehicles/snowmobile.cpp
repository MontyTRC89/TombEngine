#include "framework.h"
#include "snowmobile.h"
#include "lara.h"
#include "items.h"
#include "collide.h"
#include "effect.h"
#include "larafire.h"
#include "lara1gun.h"
#include "effect2.h"
#include "laraflar.h"
#include "lot.h"
#include "tomb4fx.h"
#include "sphere.h"
#include "setup.h"
#include "level.h"
#include "input.h"
#include "sound.h"
using std::vector;
// TODO: recreate the DrawSkidoo for the snowmobile.

enum SKIDOO_STATE { SKID_SIT, SKID_GETON, SKID_LEFT, SKID_RIGHT, SKID_FALL, SKID_HIT, SKID_GETONL, SKID_GETOFFL, SKID_STILL, SKID_GETOFF, SKID_LETGO, SKID_DEATH, SKID_FALLOFF };

#define SWIM_DEPTH 730
#define WADE_DEPTH STEP_SIZE
#define DAMAGE_START 140   // DAMAGE_START 140
#define DAMAGE_LENGTH 14
#define SKIDOO_GETON_ANIM 1
#define SKIDOO_GETONL_ANIM 18
#define SKIDOO_FALL_ANIM 8
#define SKIDOO_DEAD_ANIM 15
#define SKIDOO_HIT_LEFT 11
#define SKIDOO_HIT_RIGHT 12
#define SKIDOO_HIT_FRONT 13
#define SKIDOO_HIT_BACK 14
#define SKIDOO_GETOFF_DIST 330
#define SKIDOO_UNDO_TURN ANGLE(2)
#define SKIDOO_TURN ((ANGLE(1)/2) + SKIDOO_UNDO_TURN)
#define SKIDOO_MAX_TURN ANGLE(6)
#define SKIDOO_MOMENTUM_TURN ANGLE(3)
#define SKIDOO_MAX_MOM_TURN ANGLE(150)
#define SKIDOO_FAST_SPEED 150
#define SKIDOO_MAX_SPEED 100
#define SKIDOO_SLOW_SPEED 50
#define SKIDOO_MIN_SPEED 15
#define SKIDOO_ACCELERATION 10
#define SKIDOO_BRAKE 5
#define SKIDOO_SLOWDOWN 2
#define SKIDOO_REVERSE -5
#define SKIDOO_MAX_BACK -30
#define SKIDOO_MAX_KICK -80
#define SKIDOO_SLIP 100
#define SKIDOO_SLIP_SIDE 50
#define SKIDOO_FRONT 550
#define SKIDOO_SIDE 260
#define SKIDOO_RADIUS 500
#define SKIDOO_SNOW 500
#define SKIDOO_MAX_HEIGHT STEP_SIZE
#define SKIDOO_MIN_BOUNCE ((SKIDOO_MAX_SPEED/2)>>8)

void InitialiseSkidoo(short itemNum)
{
	ITEM_INFO* skidoo;
	SKIDOO_INFO* skinfo;

	skidoo = &g_Level.Items[itemNum];
	skinfo = game_malloc<SKIDOO_INFO>();
	skidoo->data = (void*)skinfo;
	skinfo->already_cd_played = false;

	// change to true for armed skidoo
	//if (skidoo->objectNumber == ID_SNOWMOBILE_GUN)
	//	skinfo->armed = true;
	//else
		skinfo->armed = false;

	skinfo->extra_rotation = 0;
	skinfo->flash_timer = 0;
	skinfo->left_fallspeed = 0;
	skinfo->momentum_angle = skidoo->pos.yRot;
	skinfo->pitch = 0;
	skinfo->right_fallspeed = 0;
	skinfo->skidoo_turn = 0;

	if (skidoo->status != ITEM_ACTIVE)
	{
		AddActiveItem(itemNum);
		skidoo->status = ITEM_ACTIVE;
	}
}

static void SkidooBaddieCollision(short itemNum, ITEM_INFO* skidoo)
{
	vector<short> roomsList;
	roomsList.push_back(skidoo->roomNumber);

	ROOM_INFO* room = &g_Level.Rooms[skidoo->roomNumber];
	for (int i = 0; i < room->doors.size(); i++)
	{
		roomsList.push_back(room->doors[i].room);
	}

	for (int i = 0; i < roomsList.size(); i++)
	{
		short itemNum = g_Level.Rooms[roomsList[i]].itemNumber;

		while (itemNum != NO_ITEM)
		{
			ITEM_INFO* target = &g_Level.Items[itemNum];
			if (target->collidable && target->status != ITEM_INVISIBLE && target != LaraItem && target != skidoo)
			{
				OBJECT_INFO* object = &Objects[target->objectNumber];
				if (object->collision && (object->intelligent || target->objectNumber == ID_ROLLINGBALL))
				{
					int x = skidoo->pos.xPos - target->pos.xPos;
					int y = skidoo->pos.yPos - target->pos.yPos;
					int z = skidoo->pos.zPos - target->pos.zPos;
					if (x > -2048 && x < 2048 && z > -2048 && z < 2048 && y > -2048 && y < 2048)
					{
						if (target->objectNumber == ID_ROLLINGBALL)
						{
							if (TestBoundsCollide(target, LaraItem, 100))
							{
								if (LaraItem->hitPoints > 0)
								{
									DoLotsOfBlood(LaraItem->pos.xPos, LaraItem->pos.yPos - (STEP_SIZE*2), LaraItem->pos.zPos, GetRandomControl() & 3, LaraItem->pos.yRot, LaraItem->roomNumber, 5);
									target->hitPoints -= 8;
								}
							}
						}
						else
						{
							if (TestBoundsCollide(target, skidoo, SKIDOO_FRONT))
							{
								DoLotsOfBlood(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, GetRandomControl() & 3, LaraItem->pos.yRot, LaraItem->roomNumber, 3);
								target->hitPoints = 0;
							}
						}
					}
				}
			}
		}
	}
}

static void SkidooGuns(void)
{
	ITEM_INFO* skidoo;
	SKIDOO_INFO* skinfo;
	WEAPON_INFO* winfo;
	short angles[2];

	winfo = &Weapons[WEAPON_SNOWMOBILE];
	skidoo = &g_Level.Items[Lara.Vehicle];
	skinfo = (SKIDOO_INFO*)skidoo->data;

	/* Get Target Information; skidoo retargets all the time */
	LaraGetNewTarget(winfo);
	AimWeapon(winfo, &Lara.rightArm);

	/* Skidoo guns don't animate; just fire */
	if (!skidoo->itemFlags[0] && (TrInput & IN_ACTION))
	{
		angles[0] = Lara.rightArm.yRot + LaraItem->pos.yRot;
		angles[1] = Lara.rightArm.xRot;
		if (FireWeapon(WEAPON_PISTOLS, Lara.target, LaraItem, angles) +
			FireWeapon(WEAPON_PISTOLS, Lara.target, LaraItem, angles))
		{
			skinfo->flash_timer = 2; // for custom render
			SoundEffect(winfo->sampleNum, &LaraItem->pos, 0);
			skidoo->itemFlags[0] = 4;
		}
	}

	if (skidoo->itemFlags[0])
		skidoo->itemFlags[0]--;
}

static  void SkidooExplode(ITEM_INFO* skidoo)
{
	if (g_Level.Rooms[skidoo->roomNumber].flags & ENV_FLAG_WATER)
	{
		TriggerUnderwaterExplosion(skidoo, 1);
	}
	else
	{
		TriggerExplosionSparks(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, 3, -2, 0, skidoo->roomNumber);
		for (int i = 0; i < 3; i++)
			TriggerExplosionSparks(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, 3, -1, 0, skidoo->roomNumber);
	}

	ExplodingDeath(Lara.Vehicle, -1, 256);
	KillItem(Lara.Vehicle);
	skidoo->status = ITEM_DEACTIVATED;
	SoundEffect(SFX_EXPLOSION1, 0, 0);
	SoundEffect(SFX_EXPLOSION2, 0, 0);
	Lara.Vehicle = NO_ITEM;
}

static int SkidooCheckGetOffOK(int direction)
{
	/* Check if getting off skidoo here is possible in the direction required by player */
	int x, y, z, height, ceiling;
	short roomNumber, angle;
	ITEM_INFO* skidoo;
	FLOOR_INFO* floor;

	skidoo = &g_Level.Items[Lara.Vehicle];

	if (direction == SKID_GETOFFL)
		angle = skidoo->pos.yRot + 0x4000;
	else
		angle = skidoo->pos.yRot - 0x4000;

	x = skidoo->pos.xPos - (SKIDOO_GETOFF_DIST * phd_sin(angle) >> W2V_SHIFT);
	y = skidoo->pos.yPos;
	z = skidoo->pos.zPos - (SKIDOO_GETOFF_DIST * phd_cos(angle) >> W2V_SHIFT);

	roomNumber = skidoo->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);

	height = GetFloorHeight(floor, x, y, z);

	// global var: height_type == BIG_SLOPE/SMALL_SLOPE or WALL
	if (HeightType == BIG_SLOPE || height == NO_HEIGHT || HeightType == DIAGONAL)
		return 0;

	if (abs(height - skidoo->pos.yPos) > WALL_SIZE / 2)
		return 0;

	ceiling = GetCeiling(floor, x, y, z);
	if (ceiling - skidoo->pos.yPos > -LARA_HITE || height - ceiling < LARA_HITE)
		return 0;

	return 1;
}

/* Check if Lara is still under skidoo control. Return 0 if she is in that limbo state of the skidoo still needing
		control (it is falling) and her needing normal control (so is she) */
static int SkidooCheckGetOff()
{
	ITEM_INFO* skidoo;
	skidoo = &g_Level.Items[Lara.Vehicle];

	if ((LaraItem->currentAnimState == SKID_GETOFF || LaraItem->currentAnimState == SKID_GETOFFL) && LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd)
	{
		/* Wait for last frame of GETOFF anim before returning to normal Lara control */
		if (LaraItem->currentAnimState == SKID_GETOFFL)
			LaraItem->pos.yRot += 0x4000;
		else
			LaraItem->pos.yRot -= 0x4000;

		LaraItem->animNumber = 11;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		LaraItem->currentAnimState = LaraItem->goalAnimState = 2;
		LaraItem->pos.xPos -= SKIDOO_GETOFF_DIST * phd_sin(LaraItem->pos.yRot) >> W2V_SHIFT;
		LaraItem->pos.zPos -= SKIDOO_GETOFF_DIST * phd_cos(LaraItem->pos.yRot) >> W2V_SHIFT;
		LaraItem->pos.xRot = LaraItem->pos.zRot = 0;
		Lara.Vehicle = NO_ITEM;
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (LaraItem->currentAnimState == SKID_LETGO && (skidoo->pos.yPos == skidoo->floor || LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd))
	{
		/* Has it hit the ground? If so, explode and kill Lara */
		LaraItem->animNumber = 23;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		LaraItem->currentAnimState = 9;

		if (skidoo->pos.yPos == skidoo->floor)
		{
			LaraItem->goalAnimState = 8;
			LaraItem->fallspeed = DAMAGE_START + DAMAGE_LENGTH;
			LaraItem->speed = 0;
			SkidooExplode(skidoo);
		}
		else
		{
			/* Or come off because skidoo has fallen too far */
			LaraItem->goalAnimState = 9;
			LaraItem->pos.yPos -= 200;
			LaraItem->fallspeed = skidoo->fallspeed;
			LaraItem->speed = skidoo->speed;
			SoundEffect(SFX_LARA_FALL, &LaraItem->pos, 0);
		}

		LaraItem->pos.xRot = LaraItem->pos.zRot = 0;
		LaraItem->gravityStatus = 1;
		Lara.gunStatus = LG_NO_ARMS;
		Lara.moveAngle = skidoo->pos.yRot;
		skidoo->flags |= ONESHOT; // flag that skidoo is dead
		skidoo->collidable = false;

		return 0;
	}

	return 1;
}

void DoSnowEffect(ITEM_INFO* skidoo)
{
	/*
	int c, s, x, random;
	short fx_number;
	FX_INFO* fx;

	fx_number = CreateNewEffect(skidoo->roomNumber);
	if (fx_number != NO_ITEM)
	{
		s = phd_sin(skidoo->pos.yRot);
		c = phd_cos(skidoo->pos.yRot);

		x = (GetRandomControl() - 0x4000) * SKIDOO_SIDE >> 14;

		fx = &Effects[fx_number];
		fx->pos.xPos = skidoo->pos.xPos - (SKIDOO_SNOW * s + x * c >> W2V_SHIFT);
		fx->pos.yPos = skidoo->pos.yPos + (SKIDOO_SNOW * phd_sin(skidoo->pos.xRot) >> W2V_SHIFT);
		fx->pos.zPos = skidoo->pos.zPos - (SKIDOO_SNOW * c - x * s >> W2V_SHIFT);
		fx->roomNumber = skidoo->roomNumber;
		fx->frameNumber = 0;
		fx->objectNumber = ID_DEFAULT_SPRITES;
		fx->speed = 0;
		if (skidoo->speed < 64)
		{
			random = abs(skidoo->speed) - 64;
			fx->fallspeed = GetRandomControl() * (abs(skidoo->speed) - 64) >> 15;
		}
		else
			fx->fallspeed = 0;
		//*(phd_mxptr + M23) = 0; // no depthQ
		//S_CalculateLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx->roomNumber);
		//fx->shade = ls_adder - 0x200;
		//if (fx->shade < 0)
		//	fx->shade = 0;
	}
	*/
}

static void SkidooAnimation(ITEM_INFO* skidoo, int collide, int dead)
{
	short cd;
	SKIDOO_INFO* skinfo;
	skinfo = (SKIDOO_INFO*)skidoo->data;

	/* Do animation stuff */
	if (skidoo->pos.yPos != skidoo->floor && skidoo->fallspeed > 0 && LaraItem->currentAnimState != SKID_FALL && !dead)
	{
		LaraItem->animNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_FALL_ANIM;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		LaraItem->currentAnimState = LaraItem->goalAnimState = SKID_FALL;
	}
	else if (collide && !dead && LaraItem->currentAnimState != SKID_FALL)
	{
		if (LaraItem->currentAnimState != SKID_HIT)
		{
			if (collide == SKIDOO_HIT_FRONT)
				SoundEffect(SFX_TR2_CLATTER_1, &skidoo->pos, 0);
			else
				SoundEffect(SFX_TR2_CLATTER_2, &skidoo->pos, 0);
			LaraItem->animNumber = (short)(Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + collide);
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = LaraItem->goalAnimState = SKID_HIT;
		}
	}
	else
	{
		switch (LaraItem->currentAnimState)
		{
		case SKID_STILL:
			/* Play skidoo track if first go */
			cd = (skinfo->already_cd_played == false) ? 53 : 52;
			if (!skinfo->already_cd_played)
			{
				S_CDPlay(cd, FALSE);
				skinfo->already_cd_played = true;
			}

			if (dead)
			{
				LaraItem->goalAnimState = SKID_DEATH;
				break;
			}

			LaraItem->goalAnimState = SKID_STILL; // needed because SKID_SIT == 0

			if (TrInput & IN_JUMP)
			{
				if ((TrInput & IN_RIGHT) && SkidooCheckGetOffOK(SKID_GETOFF))
				{
					LaraItem->goalAnimState = SKID_GETOFF;
					skidoo->speed = 0;
				}
				else if ((TrInput & IN_LEFT) && SkidooCheckGetOffOK(SKID_GETOFFL))
				{
					LaraItem->goalAnimState = SKID_GETOFFL;
					skidoo->speed = 0;
				}
			}
			else if (TrInput & IN_LEFT)
				LaraItem->goalAnimState = SKID_LEFT;
			else if (TrInput & IN_RIGHT)
				LaraItem->goalAnimState = SKID_RIGHT;
			else if (TrInput & (IN_FORWARD | IN_BACK))
				LaraItem->goalAnimState = SKID_SIT;
			break;

		case SKID_SIT:
			if (skidoo->speed == 0)
				LaraItem->goalAnimState = SKID_STILL;

			if (dead)
				LaraItem->goalAnimState = SKID_FALLOFF;
			else if (TrInput & IN_LEFT)
				LaraItem->goalAnimState = SKID_LEFT;
			else if (TrInput & IN_RIGHT)
				LaraItem->goalAnimState = SKID_RIGHT;
			break;

		case SKID_LEFT:
			if (!(TrInput & IN_LEFT))
				LaraItem->goalAnimState = SKID_SIT;
			break;

		case SKID_RIGHT:
			if (!(TrInput & IN_RIGHT))
				LaraItem->goalAnimState = SKID_SIT;
			break;

		case SKID_FALL:
			if (skidoo->fallspeed <= 0 || skinfo->left_fallspeed <= 0 || skinfo->right_fallspeed <= 0)
			{
				SoundEffect(SFX_TR2_CLATTER_3, &skidoo->pos, 0);
				LaraItem->goalAnimState = SKID_SIT;
			}
			else if (skidoo->fallspeed > DAMAGE_START + DAMAGE_LENGTH) // when Lara let's go, it's terminal
				LaraItem->goalAnimState = SKID_LETGO;
			break;
		}
	}
}

static int GetSkidooCollisionAnim(ITEM_INFO* skidoo, PHD_VECTOR* moved)
{
	int c, s, front, side;

	moved->x = skidoo->pos.xPos - moved->x;
	moved->z = skidoo->pos.zPos - moved->z;
	if (moved->x || moved->z)
	{
		/* Get direction of movement relative to facing */
		c = phd_cos(skidoo->pos.yRot);
		s = phd_sin(skidoo->pos.yRot);
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

static int SkidooUserControl(ITEM_INFO* skidoo, int height, int* pitch)
{
	int drive = 0, max_speed;
	SKIDOO_INFO* skinfo;
	skinfo = (SKIDOO_INFO*)skidoo->data;

	/* Deal with user input; will effect next frame, but relies on info calced this frame */
	if (skidoo->pos.yPos >= height - STEP_SIZE)
	{
		/* Engine pitch depends on speed + height off ground*/
		*pitch = skidoo->speed + (height - skidoo->pos.yPos);

		if (skidoo->speed == 0 && (TrInput & IN_LOOK))
			LookUpDown();

		/* If tracks on the ground, user has control; allow for reversing! */
		if (((TrInput & IN_LEFT) && !(TrInput & IN_BACK)) || ((TrInput & IN_RIGHT) && (TrInput & IN_BACK)))
		{
			skinfo->skidoo_turn -= SKIDOO_TURN;
			if (skinfo->skidoo_turn < -SKIDOO_MAX_TURN)
				skinfo->skidoo_turn = -SKIDOO_MAX_TURN;
		}

		if (((TrInput & IN_RIGHT) && !(TrInput & IN_BACK)) || ((TrInput & IN_LEFT) && (TrInput & IN_BACK)))
		{
			skinfo->skidoo_turn += SKIDOO_TURN;
			if (skinfo->skidoo_turn > SKIDOO_MAX_TURN)
				skinfo->skidoo_turn = SKIDOO_MAX_TURN;
		}

		if (TrInput & IN_BACK)
		{
			if (skidoo->speed > 0)
				skidoo->speed -= SKIDOO_BRAKE;
			else
			{
				if (skidoo->speed > SKIDOO_MAX_BACK)
					skidoo->speed += SKIDOO_REVERSE;
				drive = 1;
			}
		}
		else if (TrInput & IN_FORWARD)
		{
			if ((TrInput & IN_ACTION) && !skinfo->armed) // red skidoo can go faster than bandit ones
				max_speed = SKIDOO_FAST_SPEED;
			else if (TrInput & IN_STEPSHIFT)
				max_speed = SKIDOO_SLOW_SPEED;
			else
				max_speed = SKIDOO_MAX_SPEED;

			if (skidoo->speed < max_speed)
				skidoo->speed += SKIDOO_ACCELERATION / 2 + SKIDOO_ACCELERATION * skidoo->speed / (2 * max_speed);
			else if (skidoo->speed > max_speed + SKIDOO_SLOWDOWN)
				skidoo->speed -= SKIDOO_SLOWDOWN;
			drive = 1;
		}
		else if (skidoo->speed >= 0 && skidoo->speed < SKIDOO_MIN_SPEED && (TrInput & (IN_LEFT | IN_RIGHT)))
		{
			skidoo->speed = SKIDOO_MIN_SPEED; // If user wants to turn, skidoo will move forward
			drive = 1;
		}
		else if (skidoo->speed > SKIDOO_SLOWDOWN)
		{
			skidoo->speed -= SKIDOO_SLOWDOWN;
			if ((GetRandomControl() & 0x7f) < skidoo->speed)
				drive = 1;
		}
		else
			skidoo->speed = 0;
	}
	else if (TrInput & (IN_FORWARD | IN_BACK))
	{
		drive = 1;
		*pitch = skinfo->pitch + 50;
	}

	return drive;
}

static int DoSkidooDynamics(int height, int fallspeed, int* y)
{
	int kick;

	if (height > * y)
	{
		/* In air */
		*y += fallspeed;
		if (*y > height - SKIDOO_MIN_BOUNCE)
		{
			*y = height;
			fallspeed = 0;
		}
		else
			fallspeed += GRAVITY;
	}
	else
	{
		/* On ground: get up push from height change */
		kick = height - *y << 2;
		if (kick < SKIDOO_MAX_KICK)
			kick = SKIDOO_MAX_KICK;
		fallspeed += (kick - fallspeed >> 3);
		if (*y > height)
			*y = height;
	}

	return fallspeed;
}

/* Returns 0 if no get on, 1 if right get on and 2 if left get on */
static int SkidooCheckGetOn(short itemNum, COLL_INFO* coll)
{
	int geton;
	short rot, roomNumber;
	ITEM_INFO* skidoo;
	FLOOR_INFO* floor;

	/* Check if Lara is close enough and in right position to get onto skidoo */
	if (!(TrInput & IN_ACTION) || Lara.gunStatus != LG_NO_ARMS || LaraItem->gravityStatus)
		return 0;

	skidoo = &g_Level.Items[itemNum];

	rot = (skidoo->pos.yRot - LaraItem->pos.yRot);
	if (rot > 0x2000 && rot < 0x6000)
		geton = 1; //right
	else if (rot > -0x6000 && rot < -0x2000)
		geton = 2; //left
	else
		return 0;

	if (!TestBoundsCollide(skidoo, LaraItem, coll->radius))
		return 0;

	if (!TestCollision(skidoo, LaraItem))
		return 0;

	roomNumber = skidoo->roomNumber;
	floor = GetFloor(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, &roomNumber);
	if (GetFloorHeight(floor, skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos) < -32000)
		return 0;

	return geton;
}

void SkidooCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll)
{
	/* This routine is only for when Lara is not on the skidoo and she would like to be */
	int geton;
	ITEM_INFO* skidoo;

	/* If Lara dead or already on the skidoo, then no collision */
	if (litem->hitPoints < 0 || Lara.Vehicle != NO_ITEM)
		return;

	/* If player isn't pressing control or Lara is busy, then do normal object collision */
	geton = SkidooCheckGetOn(itemNum, coll);
	if (!geton)
	{
		ObjectCollision(itemNum, litem, coll);
		return;
	}

	/* Yeeha! Get on that skidoo girly */
	Lara.Vehicle = itemNum;

	/* Drop flare if in hand */
	if (Lara.gunType == WEAPON_FLARE)
	{
		CreateFlare(ID_FLARE_ITEM, FALSE);
		undraw_flare_meshes();
		Lara.flareControlLeft = 0;
		Lara.requestGunType = WEAPON_NONE;
		Lara.gunStatus = LG_NO_ARMS;
	}

	if (geton == 1)
		litem->animNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_GETON_ANIM;
	else
		litem->animNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_GETONL_ANIM;

	litem->frameNumber = g_Level.Anims[litem->animNumber].frameBase;
	litem->currentAnimState = SKID_GETON;
	Lara.gunStatus = LG_HANDS_BUSY;

	skidoo = &g_Level.Items[itemNum];
	litem->pos.yRot = skidoo->pos.yRot;
	litem->pos.xPos = skidoo->pos.xPos;
	litem->pos.yPos = skidoo->pos.yPos;
	litem->pos.zPos = skidoo->pos.zPos;

	skidoo->collidable = true;
}

/* Get height at a position offset from the origin. Moves the vector in 'pos' to the required test position too */
static int TestSkidooHeight(ITEM_INFO* item, int z_off, int x_off, PHD_VECTOR* pos)
{
	pos->y = item->pos.yPos - (z_off * phd_sin(item->pos.xRot) >> W2V_SHIFT) +
		                      (x_off * phd_sin(item->pos.zRot) >> W2V_SHIFT);

	int c = phd_cos(item->pos.yRot);
	int s = phd_sin(item->pos.yRot);

	pos->z = item->pos.zPos + ((z_off * c - x_off * s) >> W2V_SHIFT);
	pos->x = item->pos.xPos + ((z_off * s + x_off * c) >> W2V_SHIFT);

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(pos->x, pos->y, pos->z, &roomNumber);
	int ceiling = GetCeiling(floor, pos->x, pos->y, pos->z);
	if (pos->y < ceiling || ceiling == NO_HEIGHT)
		return NO_HEIGHT;

	return GetFloorHeight(floor, pos->x, pos->y, pos->z);
}

static short DoSkidooShift(ITEM_INFO* skidoo, PHD_VECTOR* pos, PHD_VECTOR* old)
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
			skidoo->speed -= 50;
		}
		else if (z)
		{
			skidoo->pos.zPos += z;
			skidoo->speed -= 50;
			if (z > 0)
				return (skidoo->pos.xPos - pos->x);
			else
				return (pos->x - skidoo->pos.xPos);
		}
		else if (x)
		{
			skidoo->pos.xPos += x;
			skidoo->speed -= 50;
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
			skidoo->speed -= 50;
		}
	}

	return 0;
}

static int SkidooDynamics(ITEM_INFO* skidoo)
{
	/* Does all skidoo movement and collision and returns if collide value */
	SKIDOO_INFO* skinfo;
	PHD_VECTOR moved, fl, fr, br, bl;
	PHD_VECTOR old, fl_old, fr_old, bl_old, br_old;
	int hfl, hfr, hbr, hbl;
	int hfr_old, hfl_old, hbr_old, hbl_old;
	FLOOR_INFO* floor;
	int height, slip, collide;
	short roomNumber, rot;
	int newspeed;

	skinfo = (SKIDOO_INFO*)skidoo->data;

	/* First get positions and heights of skidoo's corners + centre */
	hfl_old = TestSkidooHeight(skidoo, SKIDOO_FRONT, -SKIDOO_SIDE, &fl_old);
	hfr_old = TestSkidooHeight(skidoo, SKIDOO_FRONT, SKIDOO_SIDE, &fr_old);
	hbl_old = TestSkidooHeight(skidoo, -SKIDOO_FRONT, -SKIDOO_SIDE, &bl_old);
	hbr_old = TestSkidooHeight(skidoo, -SKIDOO_FRONT, SKIDOO_SIDE, &br_old);
	old.x = skidoo->pos.xPos;
	old.y = skidoo->pos.yPos;
	old.z = skidoo->pos.zPos;

	/* Back left/right may be slightly below ground, so correct for this */
	if (bl_old.y > hbl_old)
		bl_old.y = hbl_old;
	if (br_old.y > hbr_old)
		br_old.y = hbr_old;
	if (fl_old.y > hfl_old)
		fl_old.y = hfl_old;
	if (fr_old.y > hfr_old)
		fr_old.y = hfr_old;

	/* First undo any turn the skidoo may have applied */
	if (skidoo->pos.yPos > skidoo->floor - STEP_SIZE)
	{
		if (skinfo->skidoo_turn < -SKIDOO_UNDO_TURN)
			skinfo->skidoo_turn += SKIDOO_UNDO_TURN;
		else if (skinfo->skidoo_turn > SKIDOO_UNDO_TURN)
			skinfo->skidoo_turn -= SKIDOO_UNDO_TURN;
		else
			skinfo->skidoo_turn = 0;
		skidoo->pos.yRot += skinfo->skidoo_turn + skinfo->extra_rotation;

		/* Deal with momentum; do it with an angle that tracks direction of travel, but slower than turn */
		rot = skidoo->pos.yRot - skinfo->momentum_angle;
		if (rot < -SKIDOO_MOMENTUM_TURN)
		{
			if (rot < -SKIDOO_MAX_MOM_TURN)
			{
				rot = -SKIDOO_MAX_MOM_TURN;
				skinfo->momentum_angle = skidoo->pos.yRot - rot;
			}
			else
				skinfo->momentum_angle -= SKIDOO_MOMENTUM_TURN;
		}
		else if (rot > SKIDOO_MOMENTUM_TURN)
		{
			if (rot > SKIDOO_MAX_MOM_TURN)
			{
				rot = SKIDOO_MAX_MOM_TURN;
				skinfo->momentum_angle = skidoo->pos.yRot - rot;
			}
			else
				skinfo->momentum_angle += SKIDOO_MOMENTUM_TURN;
		}
		else
			skinfo->momentum_angle = skidoo->pos.yRot;
	}
	else
		skidoo->pos.yRot += skinfo->skidoo_turn + skinfo->extra_rotation;

	/* Move skidoo according to speed */
	skidoo->pos.zPos += skidoo->speed * phd_cos(skinfo->momentum_angle) >> W2V_SHIFT;
	skidoo->pos.xPos += skidoo->speed * phd_sin(skinfo->momentum_angle) >> W2V_SHIFT;

	/* Slide skidoo according to tilts (to avoid getting stuck on slopes) */
	slip = SKIDOO_SLIP * phd_sin(skidoo->pos.xRot) >> W2V_SHIFT;
	if (abs(slip) > SKIDOO_SLIP / 2)
	{
		skidoo->pos.zPos -= slip * phd_cos(skidoo->pos.yRot) >> W2V_SHIFT;
		skidoo->pos.xPos -= slip * phd_sin(skidoo->pos.yRot) >> W2V_SHIFT;
	}

	slip = SKIDOO_SLIP_SIDE * phd_sin(skidoo->pos.zRot) >> W2V_SHIFT;
	if (abs(slip) > SKIDOO_SLIP_SIDE / 2)
	{
		skidoo->pos.zPos -= slip * phd_sin(skidoo->pos.yRot) >> W2V_SHIFT;
		skidoo->pos.xPos += slip * phd_cos(skidoo->pos.yRot) >> W2V_SHIFT;
	}

	/* Remember desired position in case of collisions moving us about */
	moved.x = skidoo->pos.xPos;
	moved.z = skidoo->pos.zPos;

	/* Test against bad guys too */
	if (!(skidoo->flags & ONESHOT)) // ONESHOT flag set if skidoo no longer travelling with Lara
		SkidooBaddieCollision(Lara.Vehicle, skidoo);

	/* Test new positions of points (one at a time) and shift skidoo accordingly */
	rot = 0;
	hbl = TestSkidooHeight(skidoo, -SKIDOO_FRONT, -SKIDOO_SIDE, &bl);
	if (hbl < bl_old.y - STEP_SIZE)
		rot = DoSkidooShift(skidoo, &bl, &bl_old);

	hbr = TestSkidooHeight(skidoo, -SKIDOO_FRONT, SKIDOO_SIDE, &br);
	if (hbr < br_old.y - STEP_SIZE)
		rot += DoSkidooShift(skidoo, &br, &br_old);

	hfl = TestSkidooHeight(skidoo, SKIDOO_FRONT, -SKIDOO_SIDE, &fl);
	if (hfl < fl_old.y - STEP_SIZE)
		rot += DoSkidooShift(skidoo, &fl, &fl_old);

	hfr = TestSkidooHeight(skidoo, SKIDOO_FRONT, SKIDOO_SIDE, &fr);
	if (hfr < fr_old.y - STEP_SIZE)
		rot += DoSkidooShift(skidoo, &fr, &fr_old);

	roomNumber = skidoo->roomNumber;
	floor = GetFloor(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, &roomNumber);
	height = GetFloorHeight(floor, skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos);
	if (height < skidoo->pos.yPos - STEP_SIZE)
		DoSkidooShift(skidoo, (PHD_VECTOR*)&skidoo->pos, &old);

	skinfo->extra_rotation = rot;

	/* Get collision anim if skidoo has been moved from desired position by collisions */
	collide = GetSkidooCollisionAnim(skidoo, &moved);

	/* Check final actual movement; if speed is more than halved then reduce to zero */
	if (collide)
	{
		newspeed = ((skidoo->pos.zPos - old.z) * phd_cos(skinfo->momentum_angle) + (skidoo->pos.xPos - old.x) * phd_sin(skinfo->momentum_angle)) >> W2V_SHIFT;
		if (skidoo->speed > SKIDOO_MAX_SPEED + SKIDOO_ACCELERATION && newspeed < skidoo->speed - 10)
		{
			LaraItem->hitPoints -= (skidoo->speed - newspeed) >> 1;
			LaraItem->hitStatus = true;
		}

		if (skidoo->speed > 0 && newspeed < skidoo->speed)
			skidoo->speed = (newspeed < 0) ? 0 : newspeed;
		else if (skidoo->speed < 0 && newspeed > skidoo->speed)
			skidoo->speed = (newspeed > 0) ? 0 : newspeed;

		if (skidoo->speed < SKIDOO_MAX_BACK)
			skidoo->speed = SKIDOO_MAX_BACK;
	}

	return collide;
}

/* Returns 1 if this controls Lara too, 0 if skidoo is no longer moving with Lara (so need normal Lara control) */
int SkidooControl(void)
{
	ITEM_INFO* skidoo;
	SKIDOO_INFO* skinfo;
	PHD_VECTOR fl, fr;
	int hfl, hfr;
	FLOOR_INFO* floor;
	int height, collide, drive;
	short roomNumber, x_rot, z_rot, bandit_skidoo;
	int pitch, dead = 0;

	skidoo = &g_Level.Items[Lara.Vehicle];
	skinfo = (SKIDOO_INFO*)skidoo->data;
	collide = SkidooDynamics(skidoo);

	/* Now got final position, so get heights under middle and corners (will only have changed
		from above if collision occurred, but recalc anyway as hardly big maths) */
	hfl = TestSkidooHeight(skidoo, SKIDOO_FRONT, -SKIDOO_SIDE, &fl);
	hfr = TestSkidooHeight(skidoo, SKIDOO_FRONT, SKIDOO_SIDE, &fr);

	roomNumber = skidoo->roomNumber;
	floor = GetFloor(skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos, &roomNumber);
	height = GetFloorHeight(floor, skidoo->pos.xPos, skidoo->pos.yPos, skidoo->pos.zPos);

	TestTriggers(TriggerIndex, 0, 0);
	TestTriggers(TriggerIndex, 1, 0);

	/* Need to know what status Lara has w.r.t. the skidoo; has she died or fallen off? */
	if (LaraItem->hitPoints <= 0)
	{
		/* Disable user input if Lara is dead */
		TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		dead = 1;
	}
	else if (LaraItem->currentAnimState == SKID_LETGO)
	{
		dead = 1;
		collide = 0;
	}

	/* Deal with user input (if allowed) */
	if (skidoo->flags & ONESHOT)
	{
		drive = 0;
		collide = 0;
	}
	else
	{
		switch (LaraItem->currentAnimState)
		{
		case SKID_GETON:
		case SKID_GETOFF:
		case SKID_GETOFFL:
		case SKID_LETGO:
			/* No control */
			drive = -1;
			collide = 0;
			break;

		default:
			/* Reduce user input if Lara is dead */
			drive = SkidooUserControl(skidoo, height, &pitch);
			break;
		}
	}

	/* Do track meshes */
	bandit_skidoo = skinfo->armed;
	if (drive > 0)
	{
		skinfo->track_mesh = ((skinfo->track_mesh & 3) == 1) ? 2 : 1;

		/* Do engine noise */
		skinfo->pitch += (pitch - skinfo->pitch) >> 2;
		SoundEffect(SFX_TR2_SNOWMOBILE_HIGH_ENGINE_RPM, &skidoo->pos, 4 + ((0x10000 - (SKIDOO_MAX_SPEED - skinfo->pitch) * 100) << 8));
	}
	else
	{
		skinfo->track_mesh = 0;
		if (!drive)
			SoundEffect(SFX_TR2_SNOWMOBILE_IDLE, &skidoo->pos, 0);
		skinfo->pitch = 0;
	}
	skidoo->floor = height;

	/* Do fallspeed effects on skidoo */
	skinfo->left_fallspeed = DoSkidooDynamics(hfl, skinfo->left_fallspeed, (int*)&fl.y);
	skinfo->right_fallspeed = DoSkidooDynamics(hfr, skinfo->right_fallspeed, (int*)&fr.y);
	skidoo->fallspeed = DoSkidooDynamics(height, skidoo->fallspeed, (int*)&skidoo->pos.yPos);

	/* Rotate skidoo to match these heights */
	height = (fl.y + fr.y) >> 1;
	x_rot = phd_atan(SKIDOO_FRONT, skidoo->pos.yPos - height);
	z_rot = phd_atan(SKIDOO_SIDE, height - fl.y);

	skidoo->pos.xRot += (x_rot - skidoo->pos.xRot) >> 1;
	skidoo->pos.zRot += (z_rot - skidoo->pos.zRot) >> 1;

	//Utils.checkWaterHeight_Vehicles(skidoo, SkidooExplode);

	if (skidoo->flags & ONESHOT)
	{
		/* This is a falling skidoo - Lara is elsewhere */
		if (roomNumber != skidoo->roomNumber)
		{
			ItemNewRoom(Lara.Vehicle, roomNumber);
			ItemNewRoom(Lara.itemNumber, roomNumber);
		}

		AnimateItem(LaraItem);

		/* Has it hit the ground? If so, explode */
		if (skidoo->pos.yPos == skidoo->floor)
			SkidooExplode(skidoo);
		return 0;
	}

	SkidooAnimation(skidoo, collide, dead);

	if (roomNumber != skidoo->roomNumber)
	{
		ItemNewRoom(Lara.Vehicle, roomNumber);
		ItemNewRoom(Lara.itemNumber, roomNumber);
	}

	/* Move Lara to the skidoo position */
	if (LaraItem->currentAnimState != SKID_FALLOFF)
	{
		LaraItem->pos.xPos = skidoo->pos.xPos;
		LaraItem->pos.yPos = skidoo->pos.yPos;
		LaraItem->pos.zPos = skidoo->pos.zPos;
		LaraItem->pos.yRot = skidoo->pos.yRot;
		if (drive >= 0)
		{
			LaraItem->pos.xRot = skidoo->pos.xRot;
			LaraItem->pos.zRot = skidoo->pos.zRot;
		}
		else
			/* Don't tilt Lara during geton/off */
			LaraItem->pos.xRot = LaraItem->pos.zRot = 0;
	}
	else
		LaraItem->pos.xRot = LaraItem->pos.zRot = 0;

	AnimateItem(LaraItem);

	if (!dead && drive >= 0 && bandit_skidoo)
		SkidooGuns();

	/* Set skidoo on the exact same anim frame */
	if (!dead)
	{
		skidoo->animNumber = Objects[ID_SNOWMOBILE].animIndex + (LaraItem->animNumber - Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex);
		skidoo->frameNumber = g_Level.Anims[skidoo->animNumber].frameBase + (LaraItem->frameNumber - g_Level.Anims[LaraItem->animNumber].frameBase);
	}
	else
	{
		skidoo->animNumber = Objects[ID_SNOWMOBILE].animIndex + SKIDOO_DEAD_ANIM;
		skidoo->frameNumber = g_Level.Anims[skidoo->animNumber].frameBase;
	}

	/* If skidoo is moving, then set off a snow spray sprite */
	if (skidoo->speed && skidoo->floor == skidoo->pos.yPos/* && Utils.getFloorSound(skidoo, FS_SNOW) */)
	{
		DoSnowEffect(skidoo);
		if (skidoo->speed < 50)
			DoSnowEffect(skidoo);
	}

	return SkidooCheckGetOff();
}

void DrawSkidoo(ITEM_INFO* item)
{

}